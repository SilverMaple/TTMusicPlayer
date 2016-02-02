#include <QtGui>

#include "lyrics.h"
#include "mainwindow.h"
#include "canceldottedline.h"

//自定义标题栏的矩形区域
const static int posMinX = 0;
const static int posMaxX = 1000;
const static int posMinY = 0;
const static int posMaxY = 200;

MainWindow::MainWindow()
{
    audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);//音频输出流
    mediaObject = new Phonon::MediaObject(this);//媒体文件

    metaInformationResolver = new Phonon::MediaObject(this);//页面信息处理器
    metaInformationAudioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);

    mediaObject->setTickInterval(100);//设置间隔

    //关联播放进度和时间
    connect(mediaObject, SIGNAL(tick(qint64)),
             this, SLOT(tick(qint64)));
    //关联媒体文件播放状态和按钮状态
    connect(mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this, SLOT(stateChanged(Phonon::State,Phonon::State)));
    //关联处理器状态变化
    connect(metaInformationResolver, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this, SLOT(metaStateChanged(Phonon::State,Phonon::State)));
    //关联处理器与媒体文件队列
    connect(mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)),
            this, SLOT(sourceChanged(Phonon::MediaSource)));
    //关联切歌前准备
    connect(mediaObject, SIGNAL(aboutToFinish()),
            this, SLOT(aboutToFinish()));

    //建立连接路径
    Phonon::createPath(mediaObject, audioOutput);
    //下面这句如果加上的话会出现未知错误
    Phonon::createPath(metaInformationResolver, metaInformationAudioOutput);

    QPixmap round(":/pictures/picture/round.png");
    resize(round.size());
    setMask(round.mask());

    setupActions();
    //setupMenus();
    setupUi();
    this->setWindowFlags(Qt::FramelessWindowHint);
    //currentTimeLcd->display("00:00");//初始化播放时间为00:00
    //totalTimeLcd->display("00:00");//初始化总时间为00:00
    currentTimeLabel->setText("00:00");
    totalTimeLabel->setText("00:00");
//    mediaObject->setCurrentSource(Phonon::MediaSource("C:/Users/SilverMaple/Music/甜甜的.mp3"));
//    mediaObject->play();

    //加载上次播放列表的内容
    loadLastMusicTable();
}

//添加文件函数
void MainWindow::addFiles()
{
    //字符串的列表类，默认音乐存储路径
    QStringList files = QFileDialog::getOpenFileNames(this, tr("添加音乐文件"),
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

    //如果文件为空，结束这个函数
    if(files.isEmpty()) return;

    //qDebug()<<QDir::currentPath();
    //QFile playlistFile("save/playlist.txt");
    QFile playlistFile("save/playlist.txt");
    if(!playlistFile.open(QIODevice::Append | QIODevice::Text))
    {
        qDebug() << "Open playlist file failed.";
        return;
    }
    QTextStream playlistOutput(&playlistFile);

    int index = sources.size();
    //为数组的每一个元素重复同一个for循环
    foreach(QString string, files)
    {
        //以每一个字符串创建一个媒体源
        Phonon::MediaSource source(string);
        sources.append(source);
        qDebug() << "add" << string;
        //还要保存列表信息
        playlistOutput << string << '\n';
    }
    playlistFile.close();

    if(!sources.isEmpty())
        metaInformationResolver->setCurrentSource(sources.at(index));
    //mediaObject->setCurrentSource(sources.at(index));
}

void MainWindow::locateSong()
{
    int currentRow = 0;
    int index = sources.indexOf(mediaObject->currentSource())+1;
    for(int i=0; i<sources.size(); i++)
    {
        if(musicTable->item(i, 0)->text().toInt() == index)
        {
            currentRow = i;
            qDebug()<<currentRow;
            break;
        }
    }
    musicTable->setCurrentCell(currentRow, 0);
    musicTable->setFocus();
    musicTable->scrollToItem(musicTable->currentItem());
}

void MainWindow::about()
{
    QMessageBox::information(this, tr("Something you should know about TT Music Player"),
        tr("The TT Music Player is my homework as well as my first program"
           " which is coded with Qt.\n天台音乐播放器是我的c++作业也是我第一个用QT写的程序。"));
}

//对控制按钮状态的调整
void MainWindow::stateChanged(Phonon::State newState, Phonon::State /*oldState*/)
{
    //这里是为下面计算歌曲总时间准备
    qint64 total = mediaObject->totalTime();
    QTime totalTime(0, (total/60000)%60, (total/1000)%60);
    switch(newState)
    {
    //发生错误的状态
    case Phonon::ErrorState:
        if(mediaObject->errorType() == Phonon::FatalError)
        {
            QMessageBox::warning(this, tr("Fatal Error"),
                                 mediaObject->errorString());
        } else
        {
            QMessageBox::warning(this, tr("Error"),
                                 mediaObject->errorString());
        }
        break;//不要忘了加这个
    //播放状态
    case Phonon::PlayingState:
        playAction->setEnabled(false);
        pauseAction->setEnabled(true);
        stopAction->setEnabled(true);
        playPauseButton->setStyleSheet("QPushButton{border-image: url(:/icons/pauseNormal);}"
                                       "QPushButton:hover{border-image: url(:/icons/pauseHover);}"
                                       "QPushButton:pressed{border-image: url(:/icons/pausePress);}");
        musicTable->getPlayAction()->setEnabled(false);
        musicTable->getPauseAction()->setEnabled(true);
        //songName->setText(QFileInfo(mediaObject->currentSource().fileName()).baseName());
        //totalTimeLcd->display(totalTime.toString("mm:ss"));
        totalTimeLabel->setText(totalTime.toString("mm:ss"));
        lrcLabel->setText(QFileInfo(mediaObject->currentSource().fileName()).baseName());
        resolveLrc(mediaObject->currentSource().fileName());
        break;
    //停止状态
    case Phonon::StoppedState:
        stopAction->setEnabled(false);
        playAction->setEnabled(true);
        pauseAction->setEnabled(false);
        playPauseButton->setStyleSheet("QPushButton{border-image: url(:/icons/playNormal);}"
                                       "QPushButton:hover{border-image: url(:/icons/playHover);}"
                                       "QPushButton:pressed{border-image: url(:/icons/playPress);}");
        musicTable->getPlayAction()->setEnabled(true);
        musicTable->getPauseAction()->setEnabled(false);
        //currentTimeLcd->display("00:00");
        //totalTimeLcd->display("00:00");
        currentTimeLabel->setText("00:00");
        totalTimeLabel->setText("00:00");
        break;
    //暂停状态
    case Phonon::PausedState:
        pauseAction->setEnabled(false);
        stopAction->setEnabled(true);
        playAction->setEnabled(true);
        playPauseButton->setStyleSheet("QPushButton{border-image: url(:/icons/playNormal);}"
                                       "QPushButton:hover{border-image: url(:/icons/playHover);}"
                                       "QPushButton:pressed{border-image: url(:/icons/playPress);}");
        musicTable->getPlayAction()->setEnabled(true);
        musicTable->getPauseAction()->setEnabled(true);
        //下面是为了解决这两种状态下按钮显示不正确和进度不变
        if(modeState == once || modeState == order)
        {
            mediaObject->stop();
        }
        break;
    //缓冲状态
    case Phonon::BufferingState:
        break;
    default:
        ;
    }
}

//算当前时间
void MainWindow::tick(qint64 time)
{
    //time是以毫秒为单位的，displaytime中三个参数分别是
    //时分秒，
    QTime currentTime(0, (time/60000)%60, (time/1000)%60);
    //currentTimeLcd->display(currentTime.toString("mm:ss"));
    currentTimeLabel->setText(currentTime.toString("mm:ss"));

    qint64 total = mediaObject->totalTime();
    QTime totalTime(0, (total/60000)%60, (total/1000)%60);
    //totalTimeLcd->display(totalTime.toString("mm:ss"));
    totalTimeLabel->setText(totalTime.toString("mm:ss"));

    int currentLine = 0;//用来记录现在是第几句歌词,初始化为0
    //获取这一秒的歌词
    if(!lrcMap.isEmpty())
    {
        qint64 previous = 0;//开始时间
        qint64 later = 0;
        //不断遍历所有时间，直到找到这个时间点对应歌词的时间
        foreach(qint64 value, lrcMap.keys())
        {
            if(time >= value)
            {
                if(previous <= testRepeat)
                    currentLine++;
                previous = value;
            }
            else
            {
                later = value;

                break;//找到后终止循环
            }
        }

        //later=0证明是最后一行
        if(later == 0)
            later = total;

        //找到对应歌词
        QString currentLrc = lrcMap.value(previous);

        //刚好换下一行歌词
        //一种是歌词不同需要换，另一种是连续重复歌词也要重新绘制
        if(currentLrc != lrc->text() || testRepeat != previous)
        {
            lineNo = 22 + (currentLine*2);
            QTextCursor textCursor = textEdit->textCursor();
            gotoLine(textEdit, 0);
            gotoLine(textEdit, lineNo);
            //int prePos = textEdit->document()->findBlockByNumber(lineNo - 15).position();
            int beginPos = textEdit->document()->findBlockByNumber (lineNo-13).position();
            int endPos = textEdit->document()->findBlockByNumber (lineNo-11).position();
            //这样的话调整歌曲进度后有可能没有还原颜色
            //changeLineStyle(textCursor, defaultFormat, prePos, beginPos);
            changeLineStyle(textCursor, defaultFormat, 0, textEdit->toPlainText().count()-1);
            changeLineStyle(textCursor, format, beginPos, endPos);
            //lineNo+=2;
            //qDebug() << lineNo;
            lineNo = textEdit->textCursor().blockNumber()+3;
            lrc->setText(currentLrc);
            lrcLabel->setText(currentLrc);
            qint64 intervalTime = later - previous;
            lrc->showCover(intervalTime);            
            testRepeat = previous;
        }

        //滚动歌词
//        int rows = textEdit->document()->blockCount();
//        double percent = double(time)/total;
//        int line = percent * rows;
//        gotoLine(textEdit, 26+line);
//        qDebug() << rows;
//        qDebug() << percent;
//        qDebug() << "gotoline " << (26+line);
    }
    else
    {
        //如果没歌词就显示歌曲名（播放器内歌词）
        lrcLabel->setText(QFileInfo(mediaObject->currentSource().fileName()).baseName());
    }
}

//切歌
void MainWindow::sourceChanged(const Phonon::MediaSource &source)
{
    musicTable->selectRow(sources.indexOf(source));
    songName->setText(QFileInfo(mediaObject->currentSource().fileName()).baseName());
    lrcLabel->setText(QFileInfo(mediaObject->currentSource().fileName()).baseName());
    resolveLrc(mediaObject->currentSource().fileName());
    //currentTimeLcd->display("00:00");
    currentTimeLabel->setText("00:00");
}

void MainWindow::metaStateChanged(Phonon::State newState, Phonon::State /*oldState*/)
{
//    qDebug() << "meta file name" << metaInformationResolver->currentSource().fileName();
//    Phonon::MediaSource(metaInformationResolver->currentSource().fileName());
//    qDebug() << "时间长度为： " << metaInformationResolver->totalTime();
//    if(Phonon::StoppedState == mediaObject->state())
//        qDebug() << "1";
//    else
//        qDebug() << "0";
    //qint64 timeLength  = metaInformationResolver->totalTime();
    //QTime time(0, (timeLength/60000)%60, (timeLength/10000)%60);
    //下个状态为errorState
    if(newState == Phonon::ErrorState)
    {
        QMessageBox::warning(this, tr("Error opening files"),
                             metaInformationResolver->errorString());
        while(!sources.isEmpty() &&
              !(sources.takeLast() == metaInformationResolver->currentSource()))
        {
            //空循环
        }
        return;
    }

    //下个状态为播放状态
    if(newState != Phonon::StoppedState &&
            newState != Phonon::PausedState)
        return;

    if(metaInformationResolver->currentSource().type()
            == Phonon::MediaSource::Invalid)
        return;

    //键值表泛型(KEY - VALUE)
    QMap<QString , QString> metaData = metaInformationResolver->metaData();

    //找到值为“TITLE”的键KEY
    QString title = metaData.value("TITLE");
    //这部分真心不懂为什么会为“”= =
    if(title == "")
        //这种歌名可能会出现路径
        //title = metaInformationResolver->currentSource().fileName();
        //这种歌名可能会有重复（实际歌曲没重复）
        //title = QFileInfo(mediaObject->currentSource().fileName()).baseName();
        //采用这种
        title = QFileInfo(metaInformationResolver->currentSource().fileName()).baseName();

    //设置每一个列表头
    //id
    int num = musicTable->rowCount() + 1;
    QTableWidgetItem *IDItem;
    if(num <10)
        IDItem = new QTableWidgetItem("0" + QString("%1").arg(num));
    else
        IDItem = new QTableWidgetItem(QString("%1").arg(num));
    //歌名
    QTableWidgetItem *titleItem = new QTableWidgetItem(title);
    //设置表格项目的特性，包括能否被选择，编辑和check
    titleItem->setFlags(titleItem->flags() ^ Qt::ItemIsEditable);
    //歌手/艺术家
    QTableWidgetItem *artistItem = new QTableWidgetItem(metaData.value("ARTIST"));
    artistItem->setFlags(artistItem->flags() ^ Qt::ItemIsEditable);
    //歌曲总时长
    //敲多了一个0，结果时间计算错了
//    qint64 timeLength  = metaInformationResolver->totalTime();
//    QTime time(0, (timeLength/60000)%60, (timeLength/10000)%60);
    qint64 totalTime = metaInformationResolver->totalTime();
    QTime time(0, (totalTime / 60000) % 60, (totalTime / 1000) % 60);
    QTableWidgetItem *timeItem = new QTableWidgetItem(time.toString("mm:ss"));
    timeItem->setFlags(timeItem->flags() ^ Qt::ItemIsEditable);
    //专辑
    //QTableWidgetItem *albumItem = new QTableWidgetItem(metaData.value("ALBUM"));
    //albumItem->setFlags(albumItem->flags() ^ Qt::ItemIsEditable);
    //年份
    //QTableWidgetItem *yearItem = new QTableWidgetItem(metaData.value("DATE"));
    //yearItem->setFlags(yearItem->flags() ^ Qt::ItemIsEditable);

    //初始化为总行数
    int currentRow = musicTable->rowCount();
    musicTable->insertRow((currentRow));
    //为每一个列表头分配坐标位置
    musicTable->setItem(currentRow, 0, IDItem);
    musicTable->setItem(currentRow, 1, titleItem);
    musicTable->setItem(currentRow, 2, artistItem);
    musicTable->setItem(currentRow, 3, timeItem);

    //musicTable->setItem(currentRow, 2, albumItem);
    //musicTable->setItem(currentRow, 3, yearItem);

    //如果选择的单元格是空的，默认为第一行
    if(musicTable->selectedItems().isEmpty())
    {
        musicTable->selectRow(0);

        mediaObject->setCurrentSource((metaInformationResolver->currentSource()));
    }

    //Phonon::MediaSource source = metaInformationResolver->currentSource();
    int index = sources.indexOf(metaInformationResolver->currentSource()) + 1;
    if(sources.size() > index)
    {
        metaInformationResolver->setCurrentSource(sources.at(index));
    }
    else
    {
        //每一列宽度设置
        //musicTable->resizeColumnsToContents();
        //如果第零列宽度大于600，设置为600
        //if(musicTable->columnWidth(1) > 600)
        //    musicTable->setColumnWidth(0, 600);
//        musicTable->setColumnWidth(0, 10);
//        musicTable->setColumnWidth(1, 100);
//        musicTable->setColumnWidth(2, 100);
    }
}

//当一首歌结束时，判断资源队列还有没有下一首，有的话加入播放队列
void MainWindow::aboutToFinish()
{
    int index = sources.indexOf(mediaObject->currentSource()) + 1;

    switch (modeState) {
    case once:
        mediaObject->clearQueue();
        break;
    case order:
        if(index != sources.size())
        {
            mediaObject->enqueue(sources.at(index));
        }
        else
            mediaObject->clearQueue();
        break;
    case shuffle:
    {
        int size = sources.size();
        if(size <= 0)
            return;
        else
        {
            QTime t;
            t= QTime::currentTime();
            qsrand(t.msec() + t.second()*1000);
            int n = qrand() % size;
            mediaObject->enqueue(sources.at(n));
        }
        break;
    }
    case allRepeat:
        if(sources.size() > index)
        {
            mediaObject->enqueue(sources.at(index));
        }//如果最后一首快播完时，就把第一首歌加入队列
        else if(sources.size() == index && sources.size() > 0)
        {
            mediaObject->enqueue(sources.at(0));
        }
        break;
    case repeatOnce:
        mediaObject->enqueue(sources.at(index-1));
        break;
    default:
        break;
    }

}

//实现下一首的功能
void MainWindow::forwardPlay()
{
    switch (modeState)
    {
    case once:
    case order:
    case allRepeat:
    case repeatOnce:
    {
        //这些都默认为循环播放的下一首
        //循环播放
        int index = sources.indexOf(mediaObject->currentSource()) + 1;
        mediaObject->clearQueue();//清空播放队列
        if(sources.size() > index)
        {
            mediaObject->setCurrentSource(sources.at(index));
        }//如果最后一首快播完时，就播放第一首歌
        else if(sources.size() == index && sources.size() > 0)
        {
            mediaObject->setCurrentSource(sources.at(0));
        }
        else
            return;

        mediaObject->play();
        break;
    }
    case shuffle:
    {
        int size = sources.size();
        if(size <= 0)
            return;
        else
        {
            QTime t;
            t= QTime::currentTime();
            qsrand(t.msec() + t.second()*1000);
            int n = qrand() % size;
            mediaObject->setCurrentSource(sources.at(n));
            mediaObject->play();
        }
        break;
    }
    default:
        break;
    }
    locateSong();
}

void MainWindow::backwardPlay()
{
    /*
    int index = sources.indexOf(preSource);
    mediaObject->clearQueue();
    if(index == -1 && sources.size() > 0)
        mediaObject->setCurrentSource(sources.at(0));
    else
        mediaObject->setCurrentSource(preSource);

    mediaObject->play();
    */
    switch (modeState)
    {
    case once:
    case order:
    case allRepeat:
    case repeatOnce:
    {
        int index = sources.indexOf(mediaObject->currentSource()) - 1;
        mediaObject->clearQueue();
        if(!sources.size() > 0)
            return;
        if(index == -1)
            mediaObject->setCurrentSource(sources.at(sources.size()-1));
        else
            mediaObject->setCurrentSource(sources.at(index));

        mediaObject->play();

        break;
    }
    case shuffle:
    {
        int size = sources.size();
        if(size <= 0)
            return;
        else
        {
            QTime t;
            t= QTime::currentTime();
            qsrand(t.msec() + t.second()*1000);
            int n = qrand() % size;
            mediaObject->setCurrentSource(sources.at(n));
            mediaObject->play();
        }
        break;
    }
    default:
        break;
    }
    locateSong();
}

void MainWindow::playPause()
{
    switch (mediaObject->state())
    {
    case Phonon::PlayingState:

        mediaObject->pause();
        //暂停后显示播放按钮
        playPauseButton->setStyleSheet("QPushButton{border-image: url(:/icons/playNormal);}"
                                       "QPushButton:hover{border-image: url(:/icons/playHover);}"
                                       "QPushButton:pressed{border-image: url(:/icons/playPress);}");
        playPauseButton->setChecked(false);
        break;
    case Phonon::PausedState:
        mediaObject->play();
        //播放后显示暂停按钮
        playPauseButton->setStyleSheet("QPushButton{border-image: url(:/icons/pauseNormal);}"
                                       "QPushButton:hover{border-image: url(:/icons/pauseHover);}"
                                       "QPushButton:pressed{border-image: url(:/icons/pausePress);}");
        break;
    case Phonon::StoppedState:
        mediaObject->setCurrentSource(sources.at(musicTable->currentRow()));
        mediaObject->play();
        //播放后显示暂停按钮
        playPauseButton->setStyleSheet("QPushButton{border-image: url(:/icons/pauseNormal);}"
                                       "QPushButton:hover{border-image: url(:/icons/pauseHover);}"
                                       "QPushButton:pressed{border-image: url(:/icons/pausePress);}");
        break;
    case Phonon::LoadingState:
        playPauseButton->setChecked(false);
        break;
    case Phonon::BufferingState:
    case Phonon::ErrorState:
        break;
    }
}

QList<Phonon::MediaSource> MainWindow::getSources()
{
    return sources;
}

Phonon::MediaObject* MainWindow::getMediaObject()
{
    return mediaObject;
}

QPushButton* MainWindow::getLRCButton()
{
    return LRCButton;
}

//播放列表歌曲删除
void MainWindow::deleteItem()
{
    int row = musicTable->currentRow();
    Phonon::State state = mediaObject->state();
    Phonon::MediaSource source = mediaObject->currentSource();
    //被删除的歌处于mediaObject中,自动播放下一首
    if(source == sources.at(row))
    {
        forwardPlay();
        if(state == Phonon::StoppedState)
            mediaObject->stop();
        else if(state == Phonon::PausedState)
            mediaObject->pause();
    }

    sources.removeAt(row);
    qDebug() << "hangshu" << row;
    qDebug() << "daxiao wei " << sources.size();
    foreach(Phonon::MediaSource source, sources)
    {
        qDebug() << source.fileName();
    }

    //表格内删除
    musicTable->removeRow(row);
    for(int i=0; i<musicTable->rowCount(); i++)
    {
        musicTable->item(i, 0)->setText(QString("%1").arg(i+1));
    }

//    QString str = QDir::currentPath();
//    QMessageBox::information(this, tr("Something you should know about TT Music Player"),
//        str);
    //对playlist内容进行保存到文件中
    QFile playlistFile("save/playlist.txt");
    if(!playlistFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Open playlist file failed.";
        return;
    }
    QTextStream playlistOutput(&playlistFile);
    playlistFile.resize(0);
    QString fileName;
    for(int i=0; i<sources.size(); i++)
    {
        fileName = sources.at(i).fileName();
        playlistOutput << fileName << '\n';
    }
    playlistFile.close();
}

void MainWindow::sortBySong()
{
    musicTable->sortByColumn(1, Qt::AscendingOrder);
//    Phonon::MediaSource temp;
//    for(int i=0; i<sources.size()-1; i++)
//    {
//        for(int j=i+1; j<sources.size(); j++)
//        {
//            if(QFileInfo(sources.at(i).fileName()).baseName() > QFileInfo(sources.at(j).fileName()).baseName())
//            {
//                temp = sources.at(i);
//                sources.at(i) = sources.at(j);
//                sources.at(j) = temp;
//            }
//        }
//    }
}

void MainWindow::sortByLength()
{
    musicTable->sortByColumn(3, Qt::AscendingOrder);
}

void MainWindow::sortBySinger()
{
    musicTable->sortByColumn(2, Qt::AscendingOrder);
}

void MainWindow::sortByNumber()
{
    musicTable->sortByColumn(0, Qt::AscendingOrder);
}

void MainWindow::searchOnline()
{
    QDesktopServices::openUrl(QUrl("http://music.baidu.com/"));
    //QProcess *process = new QProcess;
    //process->start("C:/Users/SilverMaple/Desktop/MusicPlayer/build-MusicPlayer-unknown_e4e883-Debug/debug/MusicPlayer.exe");
}

void MainWindow::resolveLrc(const QString &sourceFileName)
{
    locateSong();
    lrcMap.clear();
    lrc->clear();
    textEdit->clear();//清空文本框内容
    textEdit->setAlignment(Qt::AlignHCenter);
    textEdit->setFontPointSize(15);
    textEdit->setTextColor(QColor(255, 255, 255));
    defaultFormat = textEdit->textCursor().charFormat();
    testRepeat = 0;
    for(int i=0; i<5; i++) textEdit->append("\n");
    if(sourceFileName.isEmpty())
        return;
    int index = sourceFileName.lastIndexOf("/");
    //qDebug()<< index;
    QString fileName = sourceFileName;
    fileName.insert(index+1, "Lyric/");
    //将音频文件后缀改为lrc
    QString lrcFileName = fileName.remove(fileName.right(3)) + "lrc";
    qDebug() << "正在解析" << lrcFileName;

    //打开歌词文件
    QFile file(lrcFileName);
    //判断以只读模式能否打开文件
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        lrc->setText(QFileInfo(mediaObject->currentSource().fileName()).baseName()
                     + tr(" --- 未找到歌词文件！"));
        return;
    }

    //如果能打开，开始设置字符串编码,通常歌词编码格式为GB2312
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF8"));
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("ASNI"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GB2312"));
    QString allText = QString(file.readAll());//获取所有文本
    //QTextStream in(&file);
    //QString allText = in.readAll();
    file.close();
    //按行分解
    QStringList lines = allText.split("\n");

    //使用正则表达式匹配歌词【00:00.00】
    //如果正则表达式写错，就会匹配不到内容，后果很严重！！！
    QRegExp rx("\\[\\d{2}:\\d{2}.\\d{2}\\]");
    foreach(QString oneline, lines)
    {
        QString temp = oneline;
        //qDebug() << temp;
        //为每一句都创建一个临时变量，把时间换成空字符串
        temp.replace(rx, "");
        //qDebug() << temp;
        //textEdit->append("\n" + temp);//这里会把歌曲信息也弄进去
        int pos = rx.indexIn(oneline, 0);//从0开始oneline的位置
        while(pos != -1)//匹配成功时
        {
            textEdit->append("\n" + temp);
            QString cap = rx.cap(0);//返回第一个匹配的内容
            //cap()函数的用法总结：
            //cap(0)表示与正则表达式匹配的内容
            //cap(1)表示与第一个（）内匹配的内容
            //cap(n)表示与第n个（）内匹配的内容
            //把时间转化为数值
            QRegExp regexp;
            regexp.setPattern("\\d{2}(?=:)");
            //下面是indexIn函数的解释，不过还不明白作用
            //Attempts to find a match in str from position offset (0 by default).
            //If offset is -1, the search starts at the last character;
            //if -2, at the next to last character; etc.
            //Returns the position of the first match, or -1 if there was no match.
            regexp.indexIn(cap);
            int minute = regexp.cap(0).toInt();//转为int类型
            regexp.setPattern("\\d{2}(?=\\.)");
            regexp.indexIn(cap);
            int second = regexp.cap(0).toInt();
            regexp.setPattern("\\d{2}(?=\\])");
            regexp.indexIn(cap);
            int millisecond = regexp.cap(0).toInt();

            //计算出时间，以毫秒为单位
            qint64 totalTime = minute*60000 + second*1000 + millisecond*10;
            lrcMap.insert(totalTime, temp);//把时间与对应的歌词文本放进map
            pos += rx.matchedLength();
            pos = rx.indexIn(oneline, pos);//匹配上一句歌词后下一个时间的位置
        }
    }
    for(int i=0; i<6; i++) textEdit->append("\n");

    QTextCursor textCursor = textEdit->textCursor();
    textCursor.movePosition(QTextCursor::Start);
    textEdit->setTextCursor(textCursor);
    if(mediaObject->currentTime() < 1000)
    {
        lineNo = 24;
        int beginPos = textEdit->document()->findBlockByNumber (lineNo-15).position();
        int endPos = textEdit->document()->findBlockByNumber (lineNo-13).position();
        changeLineStyle(textEdit->textCursor(), format, beginPos, endPos);
    }
    else
    {
        lineNo-=2;
        gotoLine(textEdit, lineNo);
        int prePos = textEdit->document()->findBlockByLineNumber(lineNo - 15).position();
        int beginPos = textEdit->document()->findBlockByNumber (lineNo-13).position();
        int endPos = textEdit->document()->findBlockByNumber (lineNo-11).position();
        changeLineStyle(textCursor, defaultFormat, prePos, beginPos);
        changeLineStyle(textCursor, format, beginPos, endPos);
    }

    //如果map为空
    if(lrcMap.isEmpty())
    {
        lrc->setText(QFileInfo(mediaObject->currentSource().fileName()).baseName()
                     + tr(" --- 歌词文件内容错误！"));
        return;
    }
}

void MainWindow::setLrcShown()
{
    if(lrc->isHidden())
    {
        lrc->show();
        LRCButton->setStyleSheet("QPushButton{border-image: url(:/icons/LRCup);}"
                                       "QPushButton:hover{border-image: url(:/icons/LRCHover);}"
                                       "QPushButton:pressed{border-image: url(:/icons/LRCPress);}");
    }
    else
    {
        lrc->hide();
        LRCButton->setStyleSheet("QPushButton{border-image: url(:/icons/LRCdown);}"
                                       "QPushButton:hover{border-image: url(:/icons/LRCHover);}"
                                       "QPushButton:pressed{border-image: url(:/icons/LRCPress);}");
    }
}

void MainWindow::initTrayIconMenu()
{
    QMenu *menu = new QMenu;
    QList<QAction *> actions;
    //actions << playAction << pauseAction << stopAction << previousAction << nextAction;
    actions << playAction << pauseAction << previousAction << nextAction;
    menu->addActions(actions);
    menu->addSeparator();
    menu->addAction(LRCAction);
    menu->addSeparator();
    menu->addAction(tr("退出"), qApp, SLOT(quit()));
    trayIcon->setContextMenu(menu);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(TrayIconActivated(QSystemTrayIcon::ActivationReason)));
    trayIcon->show();
}

void MainWindow::TrayIconActivated(QSystemTrayIcon::ActivationReason activationReason)
{
    if(activationReason == QSystemTrayIcon::Trigger)
        show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(isVisible())
    {
        hide();
        trayIcon->showMessage(tr("天台音乐播放器"), tr("戳我回界面O(∩_∩)O~~"));
        event->ignore();
    }
}

//窗口拖动
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    mousePosition = event->pos();
    //只对标题栏范围内的鼠标事件进行处理
    if (mousePosition.x()<=posMinX)
        return;
    if ( mousePosition.x()>=posMaxX)
        return;
    if (mousePosition.y()<=posMinY )
        return;
    if (mousePosition.y()>=posMaxY)
        return;
    isMousePressed = true;
}


void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if ( isMousePressed==true )
    {
        QPoint movePot = event->globalPos() - mousePosition;
        move(movePot);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *)
{
    isMousePressed=false;
}

void MainWindow::minimize()
{
    showMinimized();
    if(lrc->isHidden())
    {
        //继续隐藏

    }
    else
    {
        lrc->show();
    }
}

void MainWindow::setBackground()
{
    QString pictrue;
    QPoint pos = QCursor::pos();
    QPoint GlobalPoint(skinButton->mapToGlobal(QPoint(0, 0)));//获取控件在窗体中的坐标
    int y = GlobalPoint.y() + 15;
    int index = (pos.y() - y) / 23;
    QFile backgroundFile("save/background.txt");
    if(!backgroundFile.open(QIODevice::WriteOnly))
    {
        qDebug() << "Open playlist file failed.";
        return;
    }
    QTextStream backgroundOutput(&backgroundFile);
    switch (index) {
    case 0:
        pictrue = ":/backgrounds/aquarius";
        backgroundOutput << ":/backgrounds/aquarius";
        break;
    case 1:
        pictrue = ":/backgrounds/black";
        backgroundOutput << ":/backgrounds/black";
        break;
    case 2:
        pictrue = ":/backgrounds/blue";
        backgroundOutput << ":/backgrounds/blue";
        break;
    case 3:
        pictrue = ":/backgrounds/cat";
        backgroundOutput << ":/backgrounds/cat";
        break;
    case 4:
        pictrue = ":/backgrounds/gray";
        backgroundOutput << ":/backgrounds/gray";
        break;
    case 5:
        pictrue = ":/backgrounds/green";
        backgroundOutput << ":/backgrounds/green";
        break;
    case 6:
        pictrue = ":/backgrounds/light";
        backgroundOutput << ":/backgrounds/light";
        break;
    case 7:
        pictrue = ":/backgrounds/pink";
        backgroundOutput << ":/backgrounds/pink";
        break;
    default:
        return;
        break;
    }
    backgroundFile.close();
    //QString pictrue = ":/backgrounds/" + str;
    QPalette p;
    //p.setColor(QPalette::Background, QColor(192,253,123));
    //p.setBrush(QPalette::Background, QBrush(QPixmap(":/backgrounds/black")));
    p.setBrush(QPalette::Window,
                         QBrush(QPixmap(pictrue).scaled(//缩放背景
                                    QSize(999, 666),
                                    //rightWidget->size(),
                                    Qt::IgnoreAspectRatio,
                                    Qt::SmoothTransformation)));//平滑缩放
    this->setPalette(p);
}

void MainWindow::initSkinMenu()
{
    skinMenu = new QMenu;
    dir=new QDir(":/backgrounds");//进去背景文件夹中
    //不知道为什么一过滤就全都没有了
    //QStringList filter;
    //filter<<"*.png";//过滤出png图片
    //dir->setNameFilters(filter);
    //QList<QFileInfo> *fileInfo=new QList<QFileInfo>(dir->entryInfoList(filter));
    QList<QFileInfo> *fileInfo=new QList<QFileInfo>(dir->entryInfoList());
    qDebug() << "文件数目： " << fileInfo->count();
    for(int i=0; i<fileInfo->count(); i++)
    {
        QString filename = QFileInfo(fileInfo->at(i).fileName()).baseName();
        skinMenu->addAction(filename, this, SLOT(setBackground()));
    }
}

void MainWindow::showSkinMenu()
{
    //menu.exec(skinButton->cursor().pos());//这个是在鼠标位置
    //下面是自己写的出现在按钮左下角的菜单
    QPoint GlobalPoint(skinButton->mapToGlobal(QPoint(0, 0)));//获取控件在窗体中的坐标
    int x = GlobalPoint.x();
    int y = GlobalPoint.y() + 15;
    skinMenu->exec(QPoint(x, y));
}

void MainWindow::initPlayModeMenu()
{
    playModeMenu = new QMenu;
    modeActions = new QActionGroup(this);

    //这明显的强迫症= =
    oncePlayAction = new QAction(tr("单曲播放"), this);
    orderPlayAction = new QAction(tr("顺序播放"), this);
    shufflePlayAction = new QAction(tr("随机播放"), this);
    allRepeatPlayAction = new QAction(tr("循环播放"), this);
    repeatOncePlayAction = new QAction(tr("单曲循环"), this);

    connect(oncePlayAction, SIGNAL(triggered()),
            this, SLOT(oncePlay()));
    connect(orderPlayAction, SIGNAL(triggered()),
            this, SLOT(orderPlay()));
    connect(shufflePlayAction, SIGNAL(triggered()),
            this, SLOT(shufflePlay()));
    connect(allRepeatPlayAction, SIGNAL(triggered()),
            this, SLOT(allRepeatPlay()));
    connect(repeatOncePlayAction, SIGNAL(triggered()),
            this, SLOT(repeatOncePlay()));

    playModeMenu->addAction(oncePlayAction);
    playModeMenu->addAction(orderPlayAction);
    playModeMenu->addAction(shufflePlayAction);
    playModeMenu->addAction(allRepeatPlayAction);
    playModeMenu->addAction(repeatOncePlayAction);

    //默认循环播放
    allRepeatPlayAction->setIcon(QPixmap(":/pictures/tick"));
}

void MainWindow::showPlayModeMenu()
{
    QPoint GlobalPoint(playModeButton->mapToGlobal(QPoint(0, 0)));//获取控件在窗体中的坐标
    int x = GlobalPoint.x();
    int y = GlobalPoint.y() + 20;
    playModeMenu->exec(QPoint(x, y));
}

void MainWindow::updateModeState(ModeState newState)
{
    //先清除以前的图标
    switch(modeState)
    {
    case once:
        oncePlayAction->setIcon(QPixmap(""));
        break;
    case order:
        orderPlayAction->setIcon(QPixmap(""));
        break;
    case shuffle:
        shufflePlayAction->setIcon(QPixmap(""));
        break;
    case allRepeat:
        allRepeatPlayAction->setIcon(QPixmap(""));
        break;
    case repeatOnce:
        repeatOncePlayAction->setIcon(QPixmap(""));
        break;
    default:
        break;
    }

    switch(newState)
    {
    case once:
        oncePlayAction->setIcon(QPixmap(":/pictures/tick"));
        playModeButton->setStyleSheet("QPushButton{border-image: url(:/icons/oncePlayNormal);}"
                                      "QPushButton:hover{border-image: url(:/icons/oncePlayHover);}"
                                      "QPushButton:pressed{border-image: url(:/icons/oncePlayPress);}");
        break;
    case order:
        orderPlayAction->setIcon(QPixmap(":/pictures/tick"));
        playModeButton->setStyleSheet("QPushButton{border-image: url(:/icons/orderPlayNormal);}"
                                      "QPushButton:hover{border-image: url(:/icons/orderPlayHover);}"
                                      "QPushButton:pressed{border-image: url(:/icons/orderPlayPress);}");
        break;
    case shuffle:
        shufflePlayAction->setIcon(QPixmap(":/pictures/tick"));
        playModeButton->setStyleSheet("QPushButton{border-image: url(:/icons/shufflePlayNormal);}"
                                      "QPushButton:hover{border-image: url(:/icons/shufflePlayHover);}"
                                      "QPushButton:pressed{border-image: url(:/icons/shufflePlayPress);}");
        break;
    case allRepeat:
        allRepeatPlayAction->setIcon(QPixmap(":/pictures/tick"));
        playModeButton->setStyleSheet("QPushButton{border-image: url(:/icons/allRepeatPlayNormal);}"
                                      "QPushButton:hover{border-image: url(:/icons/allRepeatPlayHover);}"
                                      "QPushButton:pressed{border-image: url(:/icons/allRepeatPlayPress);}");
        break;
    case repeatOnce:
        repeatOncePlayAction->setIcon(QPixmap(":/pictures/tick"));
        playModeButton->setStyleSheet("QPushButton{border-image: url(:/icons/repeatOncePlayNormal);}"
                                      "QPushButton:hover{border-image: url(:/icons/repeatOncePlayHover);}"
                                      "QPushButton:pressed{border-image: url(:/icons/repeatOncePlayPress);}");
        break;
    default:
        break;
    }

    modeState = newState;
}

void MainWindow::valueChanged(int i)
{
    audioOutput->setVolume(qreal( (float)i/100 ));
}

void MainWindow::volumeSwitch()
{
    if(audioOutput->volume() > 0 && audioOutput->volume() < 100)
    {
        volumeSlider->setValue(0);
        volumeButton->setStyleSheet("QPushButton{border-image: url(:/icons/volumeDownIconNormal);}"
                                    "QPushButton:hover{border-image: url(:/icons/volumeDownIconHover);}"
                                    "QPushButton:pressed{border-image: url(:/icons/volumeDownIconPress);}");
    }
    else
    {
        volumeSlider->setValue(100);
        volumeButton->setStyleSheet("QPushButton{border-image: url(:/icons/volumeIconNormal);}"
                                    "QPushButton:hover{border-image: url(:/icons/volumeIconHover);}"
                                    "QPushButton:pressed{border-image: url(:/icons/volumeIconPress);}");
    }
}

void MainWindow::loadLastMusicTable()
{
    //QFile playlistFile("C:/Users/SilverMaple/Desktop/MusicPlayer/MusicPlayer/playlist/playlist.txt");
    //QFile playlistFile(":/playlist/playlist.txt");//这样是表示资源文件路径
    //QFile playlistFile("playlist/playlist.txt");//这样会找不到文件的。。
    //qDebug()<<QDir::currentPath();//会发现当前路径其实是在构建目录
    //下面这样写是正确的，但发布时需要修改
/*
    QDir dir = QDir::currentPath();
    dir.cdUp();
    dir.cd("MusicPlayer");
    QDir::setCurrent(dir.absolutePath());
*/
//    qDebug()<<dir.absolutePath();
//    qDebug()<<QDir::currentPath();

//    QString str = QDir::currentPath();
//    QMessageBox::information(this, tr("Something you should know about TT Music Player"),
//        str);
    //QFile playlistFile("save/playlist.txt");
    QFile playlistFile("save/playlist.txt");
    if(!playlistFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Open playlist file failed.";
        return;
    }

    QString allText = QString(playlistFile.readAll());//获取所有文本
    playlistFile.close();
    //按行分解
    QStringList songs = allText.split("\n");

    foreach(QString song, songs)
    {
        if(song != "")
        {
            song.remove(song.size()-1, 1);
            Phonon::MediaSource source(song);
            sources.append(source);
            qDebug() << "add" << song;
        }
    }
    if(!sources.isEmpty())
        metaInformationResolver->setCurrentSource(sources.at(0));
}

void MainWindow::gotoLine(QTextEdit *textEdit, int line)
{
    QTextCursor tc = textEdit->textCursor();
    int position = textEdit->document()->findBlockByNumber ( line-1 ).position();
    tc.setPosition(position, QTextCursor::MoveAnchor);
    textEdit->setTextCursor(tc);
}

void MainWindow::changeLineStyle(QTextCursor textCursor, QTextCharFormat charFormat, int beginPos, int endPos)
{
    textCursor.setPosition(beginPos, QTextCursor::MoveAnchor);
    textCursor.setPosition(endPos, QTextCursor::KeepAnchor);
    textCursor.setCharFormat(charFormat);
}

//void MainWindow::on_actionMinimize_triggered()
//{
//    //系统自定义的最小化窗口函数
//    showMinimized();
//    //最大化 showMaximized()；
//}

//void MainWindow::on_actionClose_triggered()
//{
//    //系统自定义的窗口关闭函数
//    close();
//}

//初始化在头文件中声明的各个Action
void MainWindow::setupActions()
{
    //设置图标，属性名，以及这个Action所属的窗口
    playAction = new QAction(style()->standardIcon(QStyle::SP_MediaPlay), tr("Play"), this);
    playAction->setShortcut(tr("Ctrl+P"));//设置快捷键
    playAction->setDisabled(true);//一开始播放列表没有歌曲，播放按钮肯定就不能按了

    pauseAction = new QAction(style()->standardIcon(QStyle::SP_MediaPause), tr("Pause"), this);
    pauseAction->setShortcut(tr("Ctrl+A"));
    pauseAction->setDisabled(true);

    stopAction = new QAction(style()->standardIcon(QStyle::SP_MediaStop), tr("Stop"), this);
    stopAction->setShortcut(tr("Ctrl+S"));
    stopAction->setDisabled(true);

    nextAction = new QAction(style()->standardIcon(QStyle::SP_MediaSkipForward), tr("Next"), this);
    nextAction->setShortcut(tr("Ctrl+N"));

    previousAction = new QAction(style()->standardIcon(QStyle::SP_MediaSkipBackward), tr("Previous"), this);
    previousAction->setShortcut(tr("Ctrl+R"));

    addFilesAction = new QAction(tr("Add &Files"), this);
    addFilesAction->setIcon(QPixmap((":/pictures/addfile")));
    addFilesAction->setShortcut(tr("Ctrl+F"));

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setIcon(QPixmap((":/pictures/exit")));
    exitAction->setShortcuts(QKeySequence::Quit);

    aboutAction = new QAction(tr("A&bout"), this);
    aboutAction->setIcon(QPixmap((":/pictures/about")));
    aboutAction->setShortcut(tr("Ctrl+B"));

    aboutQtAction = new QAction(tr("About &Qt"), this);
    aboutQtAction->setIcon(QPixmap((":/pictures/qt")));
    aboutQtAction->setShortcut(tr("Ctrl+Q"));

    LRCAction = new QAction(tr("显示/关闭桌面歌词"), this);
    LRCAction->setShortcut(QKeySequence("F2"));

    //播放控制与菜单按钮关联
    connect(playAction, SIGNAL(triggered()),
            mediaObject, SLOT(play()));
    connect(pauseAction, SIGNAL(triggered()),
            mediaObject, SLOT(pause()));
    connect(stopAction, SIGNAL(triggered()),
            mediaObject, SLOT(stop()));
    connect(nextAction, SIGNAL(triggered()),
            this, SLOT(forwardPlay()));
    connect(previousAction, SIGNAL(triggered()),
            this, SLOT(backwardPlay()));
    connect(LRCAction, SIGNAL(triggered()),
             this, SLOT(setLrcShown()));

    //文件与播放器控制与菜单关联
    connect(addFilesAction, SIGNAL(triggered()),
            this, SLOT(addFiles()));
    connect(exitAction, SIGNAL(triggered()),
            this, SLOT(close()));
    connect(aboutAction, SIGNAL(triggered()),
            this, SLOT(about()));
    connect(aboutQtAction, SIGNAL(triggered()),
            qApp, SLOT(aboutQt()));

    //托盘图标关联
    //connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
    //        this, SLOT(TrayIconActivated(QSystemTrayIcon::ActivationReason)));
}

//去掉菜单
//设置菜单栏中的每一个菜单
//void MainWindow::setupMenus()

//{
//    //文件菜单
//    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
//    fileMenu->addAction(addFilesAction);
//    fileMenu->addSeparator();
//    fileMenu->addAction(exitAction);

//    //关于菜单
//    QMenu *aboutMenu = menuBar()->addMenu(tr("&Help"));
//    aboutMenu->addAction(aboutAction);
//    aboutMenu->addAction(aboutQtAction);
//}

//设置播放器的界面
void MainWindow::setupUi()
{
    //自定义标题栏的设置
    skinButton = new QPushButton(this);
    minButton = new QPushButton(this);
    closeButton= new QPushButton(this);

    // 设置按钮图像的样式
    skinButton->setStyleSheet("QPushButton{border-image: url(:/icons/skinNormal);}"
                              "QPushButton:hover{border-image: url(:/icons/skinHover);}"
                              "QPushButton:pressed{border-image: url(:/icons/skinPress);}");
    skinButton->setFixedSize(QSize(15, 15));
    skinButton->setContextMenuPolicy(Qt::CustomContextMenu);
    initSkinMenu();
    connect(skinButton, SIGNAL(clicked()),
            this, SLOT(showSkinMenu()));
    //其实可以直接setMenu来实现弹出窗口，但是会有下三角
    //skinButton->setMenu(skinMenu);

    helpButton = new QPushButton(this);
    helpButton->setStyleSheet("QPushButton{border-image: url(:/icons/helpNormal);}"
                              "QPushButton:hover{border-image: url(:/icons/helpHover);}"
                              "QPushButton:pressed{border-image: url(:/icons/helpPress);}"
                              "QPushButton::menu-indicator{image:None;}");//这一句是去掉三角符号
    helpButton->setFixedSize(QSize(15, 15));
    QMenu *helpMenu = new QMenu(this);
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);
    helpButton->setMenu(helpMenu);

    //minButton->setIcon(QPixmap(style()->standardPixmap(QStyle::SP_TitleBarMinButton)));
    minButton->setStyleSheet("QPushButton{border-image: url(:/icons/minimizeNormal);}"
                                   "QPushButton:hover{border-image: url(:/icons/minimizeHover);}"
                                   "QPushButton:pressed{border-image: url(:/icons/minimizePress);}");
    minButton->setFixedSize(QSize(15, 15));
    connect(minButton, SIGNAL(clicked()),
            this, SLOT(minimize()));

    //closeButton->setIcon(QPixmap(style()->standardPixmap(QStyle::SP_TitleBarCloseButton)));
    closeButton->setStyleSheet("QPushButton{border-image: url(:/icons/closeNormal);}"
                                   "QPushButton:hover{border-image: url(:/icons/closeHover);}"
                                   "QPushButton:pressed{border-image: url(:/icons/closePress);}");
    closeButton->setFixedSize(QSize(15, 15));
    connect(closeButton, SIGNAL(clicked()),
            this, SLOT(close()));

    openButton = new QPushButton(this);
    openButton->setStyleSheet("QPushButton{border-image: url(:/icons/openNormal);}"
                              "QPushButton:hover{border-image: url(:/icons/openHover);}"
                              "QPushButton:pressed{border-image: url(:/icons/openPress);}");
    openButton->setFixedSize(QSize(20, 20));
    connect(openButton, SIGNAL(clicked()),
            this, SLOT(addFiles()));

    locateButton = new QPushButton(this);
    locateButton->setStyleSheet("QPushButton{border-image: url(:/icons/locateNormal);}"
                              "QPushButton:hover{border-image: url(:/icons/locateHover);}"
                              "QPushButton:pressed{border-image: url(:/icons/locatePress);}");
    locateButton->setFixedSize(QSize(20, 20));
    connect(locateButton, SIGNAL(clicked()),
            this, SLOT(locateSong()));

    sortButton = new QPushButton(this);
    sortButton->setStyleSheet("QPushButton{border-image: url(:/icons/sortNormal);}"
                              "QPushButton:hover{border-image: url(:/icons/sortHover);}"
                              "QPushButton:pressed{border-image: url(:/icons/sortPress);}"
                              "QPushButton::menu-indicator{image:None;}");
    sortButton->setFixedSize(QSize(20, 20));
    QMenu *sortMenu = new QMenu(this);
    sortMenu->addAction(QString("按编号"), this, SLOT(sortByNumber()));
    sortMenu->addAction(QString("按歌曲名"), this, SLOT(sortBySong()));
    sortMenu->addAction(QString("按歌手名"), this, SLOT(sortBySinger()));
    sortMenu->addAction(QString("按长度"), this, SLOT(sortByLength()));
    sortButton->setMenu(sortMenu);

    searchButton = new QPushButton(this);
    searchButton->setStyleSheet("QPushButton{border-image: url(:/icons/searchNormal);}"
                              "QPushButton:hover{border-image: url(:/icons/searchHover);}"
                              "QPushButton:pressed{border-image: url(:/icons/searchPress);}");
    searchButton->setFixedSize(QSize(20, 20));
    connect(searchButton, SIGNAL(clicked()),
            this, SLOT(searchOnline()));

    //工具栏的设置
    //QToolBar *bar = new QToolBar;
    //QLabel *blankLabel = new QLabel;
    //之前是直接addAction，但是为了能方便设置图标给位addWidget

    volumeButton = new QPushButton(this);
    volumeButton->setStyleSheet("QPushButton{border-image: url(:/icons/volumeIconNormal);}"
                                "QPushButton:hover{border-image: url(:/icons/volumeIconHover);}"
                                "QPushButton:pressed{border-image: url(:/icons/volumeIconPress);}");
    volumeButton->setFixedSize(QSize(20, 20));
    connect(volumeButton, SIGNAL(clicked()),
            this, SLOT(volumeSwitch()));

    LRCButton = new QPushButton(this);
    LRCButton->setStyleSheet("QPushButton{border-image: url(:/icons/LRCup);}"
                                   "QPushButton:hover{border-image: url(:/icons/LRCHover);}"
                                   "QPushButton:pressed{border-image: url(:/icons/LRCPress);}");
    LRCButton->setFixedSize(QSize(20, 20));
    connect(LRCButton, SIGNAL(clicked()),
            this, SLOT(setLrcShown()));
    
    playModeButton = new QPushButton(this);
    modeState = allRepeat;
    //默认为顺序播放
    playModeButton->setStyleSheet("QPushButton{border-image: url(:/icons/allRepeatPlayNormal);}"
                                  "QPushButton:hover{border-image: url(:/icons/allRepeatPlayHover);}"
                                  "QPushButton:pressed{border-image: url(:/icons/allRepeatPlayPress);}");
    playModeButton->setFixedSize(QSize(20, 20));
    initPlayModeMenu();
    connect(playModeButton, SIGNAL(clicked()),
            this, SLOT(showPlayModeMenu()));
    
    playPauseButton = new QPushButton(this);
    playPauseButton->setStyleSheet("QPushButton{border-image: url(:/icons/playNormal);}"
                                   "QPushButton:hover{border-image: url(:/icons/playHover);}"
                                   "QPushButton:pressed{border-image: url(:/icons/playPress);}");
    playPauseButton->setFixedSize(QSize(50,50));
    connect(playPauseButton, SIGNAL(clicked()),
            this, SLOT(playPause()));

    previousButton = new QPushButton(this);
    previousButton->setStyleSheet("QPushButton{border-image: url(:/icons/previousNormal);}"
                                   "QPushButton:hover{border-image: url(:/icons/previousHover);}"
                                   "QPushButton:pressed{border-image: url(:/icons/previousPress);}");
    previousButton->setFixedSize(QSize(40,40));
    connect(previousButton, SIGNAL(clicked()),
            this, SLOT(backwardPlay()));

    nextButton = new QPushButton(this);
    nextButton->setStyleSheet("QPushButton{border-image: url(:/icons/nextNormal);}"
                                   "QPushButton:hover{border-image: url(:/icons/nextHover);}"
                                   "QPushButton:pressed{border-image: url(:/icons/nextPress);}");
    nextButton->setFixedSize(QSize(40,40));
    connect(nextButton, SIGNAL(clicked()),
            this, SLOT(forwardPlay()));

    //工具栏的边框去不掉，所以放弃工具栏改用layout
//    bar->addWidget(LRCButton);
//    bar->addWidget(previousButton);
//    //bar->addAction(playAction);
//    bar->addWidget(playPauseButton);
//    bar->addWidget(nextButton);

    QPalette fontPalette;
    fontPalette.setColor(QPalette::WindowText,Qt::white);

    songName = new QLabel;
    songName->setText(tr("天台音乐"));
    songName->setPalette(fontPalette);

    lrcLabel = new QLabel(tr(" ~ TT Music Player ~ "));
    lrcLabel->setTextFormat(Qt::RichText);
    lrcLabel->setAlignment(Qt::AlignCenter);//居中显示
    lrcLabel->setFocus();

    seekSlider = new Phonon::SeekSlider(this);
    seekSlider->setMediaObject(mediaObject);//关联媒体文件和控制条

    //volumeSlider = new Phonon::VolumeSlider(this);
    //volumeSlider->setAudioOutput(audioOutput);//关联音量条与音频输出流
    //volumeSlider->setFixedWidth(80);
    //volumeSlider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    //使用自定义的slider
    volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setFixedWidth(65);
    volumeSlider->setValue(100);
    connect(volumeSlider, SIGNAL(valueChanged(int)),
            this, SLOT(valueChanged(int)));

    //读取文件内容，设置滑条样式
    QFile styleSheet(":/styleSheet/slider");
    styleSheet.open(QFile::ReadOnly);
    seekSlider->setStyleSheet(styleSheet.readAll());
    //不能同时设置同一个stylesheet，不然后面设置的不能生效
    //volumeSlider->setStyleSheet(styleSheet.readAll());
    styleSheet.close();

    //分开设置才能成功
    QFile styleSheet1(":/styleSheet/slider");
    styleSheet1.open(QFile::ReadOnly);
    volumeSlider->setStyleSheet(styleSheet1.readAll());
    styleSheet1.close();

    QPalette palette;//调色板
    palette.setBrush(QPalette::Light, Qt::blue);

//    currentTimeLcd = new QLCDNumber;//设置当前时间数字
//    currentTimeLcd->setPalette(palette);
//    currentTimeLcd->setFrameStyle(0);
//    currentTimeLcd->setFixedSize(QSize(50, 20));
//    totalTimeLcd = new QLCDNumber;//设置总时间数字
//    totalTimeLcd->setPalette(palette);
//    totalTimeLcd->setFrameStyle(0);
//    totalTimeLcd->setFixedSize(QSize(50, 20));

    currentTimeLabel = new QLabel;
    totalTimeLabel = new QLabel;
    currentTimeLabel->setPalette(fontPalette);
    totalTimeLabel->setPalette(fontPalette);

    //musicTable = new MusicPlaylist(0, 2/*, sources, mediaObject*/);//行列数，以及对mediaObject的引用
    musicTable = new MusicPlaylist(0, 4, this);
    //关联鼠标点击与table点击
    //单击就能播放
    //connect(musicTable, SIGNAL(cellPressed(int,int)),
    //        this, SLOT(tableClicked(int,int)));
    //双击播放
    connect(musicTable, SIGNAL(cellDoubleClicked(int,int)),
            musicTable, SLOT(tableClicked(int,int)));
    //播放控制与右键菜单关联
    //此部分也在musicPlaylist里面实现
//    connect(musicTable->getPlayAction(), SIGNAL(triggered()),
//            mediaObject, SLOT(play()));
//    connect(musicTable->getPauseAction(), SIGNAL(triggered()),
//            mediaObject, SLOT(pause()));

    //此部分已经转移到musicplaylist类中实现
/*
    QStringList headers;//好像是叫做标题栏
    //headers << tr("Title") << tr("Artist") << tr("Album") << tr("Year");
    headers << tr("Title") << tr("Artist");

    //musicTable = new QTableWidget(0, 4);//新建一个0行4列的table
    //musicTable = new QTableWidget(0, 2);//新建一个0行2列的table
    musicTable = new MusicPlaylist(0, 2);//新建一个0行2列的table
    musicTable->setHorizontalHeaderLabels(headers);//将headers里面的每一个放进table的每一列列首
    musicTable->setSelectionMode(QAbstractItemView::SingleSelection);//不能多行选择
    musicTable->setSelectionBehavior(QAbstractItemView::SelectRows);//单行选择
    musicTable->horizontalHeader()->hide();//隐藏列头
    //右键菜单
    //musicTable->setContextMenuPolicy(Qt::CustomContextMenu);//需要自己写slot而且connect鼠标点击信号
    musicTable->setContextMenuPolicy(Qt::DefaultContextMenu);//只需要重写contextMenuEvent()函数就行
    musicTable->setItemDelegate(new CancelDottedLine(musicTable));
    musicTable->setShowGrid(false);//去掉格子线
    //musicTable->horizontalHeader()->setHighlightSections(true);
    musicTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    //关联鼠标点击与table点击
    connect(musicTable, SIGNAL(cellPressed(int,int)),
            this, SLOT(tableClicked(int,int)));
*/

    //创建一个歌词类
    lrc = new Lyrics(this);
    lrc->setAlignment(Qt::AlignBottom);
    //setLrcShown();//在这里显示会导致主窗口位置右下移

    //创建滑动歌词界面
    textEdit = new QTextEdit(this);
    textEdit->setFontPointSize(15);
    textEdit->setTextColor(QColor(255, 255, 255));
    textEdit->setFixedWidth(626);
    textEdit->setReadOnly(true);
    textEdit->setAlignment(Qt::AlignCenter);
    format.setFontPointSize(20);
    format.setForeground(QColor(255, 255, 133));
    midFormat.setFontPointSize(17);
    midFormat.setForeground(QColor(255, 255, 0));

    //读取文件内容，设置滑条样式
    QFile file(":/styleSheet/scrollBar");
    file.open(QFile::ReadOnly);
    textEdit->verticalScrollBar()->setStyleSheet(file.readAll());
    file.close();
    QPalette pll = textEdit->palette();
    pll.setBrush(QPalette::Base,QBrush(QColor(255,255,255,0)));
    textEdit->setPalette(pll);

    //设置托盘图标
    trayIcon = new QSystemTrayIcon(QIcon(":/pictures/Ticon"), this);
    trayIcon->setToolTip(tr("天台音乐播放器"));
    //托盘右击菜单
    initTrayIconMenu();

    //建立一个水平布局，将标题栏的放进去
    QHBoxLayout *titleBarLayout = new QHBoxLayout;
    titleBarLayout->addWidget(songName);
    titleBarLayout->addStretch();
    titleBarLayout->addWidget(helpButton);
    titleBarLayout->addWidget(skinButton);
    titleBarLayout->addWidget(minButton);
    titleBarLayout->addWidget(closeButton);

    //建立一个水平布局，将滑条和时间数字放进去
    QHBoxLayout *timeLayout = new QHBoxLayout;
    //timeLayout->addWidget(currentTimeLcd);
    timeLayout->addWidget(currentTimeLabel);
    timeLayout->addStretch();
    //timeLayout->addWidget(totalTimeLcd);
    timeLayout->addWidget(totalTimeLabel);

    //建立一个水平布局，将工具栏和时间滑条放进去
    QHBoxLayout *playbackLayout = new QHBoxLayout;
    playbackLayout->addWidget(LRCButton);
    playbackLayout->addSpacing(10);
    playbackLayout->addWidget(playModeButton);
    playbackLayout->addStretch();//填充的空格
    playbackLayout->addWidget(previousButton);
    playbackLayout->addWidget(playPauseButton);
    playbackLayout->addWidget(nextButton);
    //playbackLayout->addSpacing(20);
    playbackLayout->addWidget(volumeButton);
    playbackLayout->addWidget(volumeSlider);

    //左布局，包括播放列表和上面三个布局
    QVBoxLayout *leftLayout = new QVBoxLayout;
    //leftLayout->addWidget(lrcLabel);
    leftLayout->addWidget(seekSlider);
    leftLayout->addLayout(timeLayout);
    leftLayout->addLayout(playbackLayout);
    leftLayout->addWidget(musicTable);

//    QVBoxLayout *rightLayout = new QVBoxLayout;
//    rightLayout->addWidget(new QTextEdit("hello"));

    QWidget *widget = new QWidget;//建立一个以主布局为布局的组件
    QWidget *leftWidget = new QWidget;
    leftWidget->setLayout(leftLayout);
    leftWidget->setFixedWidth(340);
    QWidget *rightWidget = new QWidget;
    rightWidget->setAutoFillBackground(true);//显示背景
    rightWidget->setFixedSize(QSize(850, 580));
    QPalette widgetPalette = rightWidget->palette();
    widgetPalette.setBrush(QPalette::Window,
                     QBrush(QPixmap("C:/Users/SilverMaple/Desktop/MusicPlayer/MusicPlayer/TT.png").scaled(//缩放背景
                                QSize(645, 600),
                                //rightWidget->size(),
                                Qt::IgnoreAspectRatio,
                                Qt::SmoothTransformation)));//平滑缩放
//    widgetPalette.setBrush(rightWidget->backgroundRole(),
//                           QBrush(QPixmap("C:/Users/SilverMaple/Desktop/MusicPlayer/background.jpg")));
    rightWidget->setPalette(widgetPalette);
    //rightWidget->setLayout(rightLayout);
    //QLabel *gifLabel = new QLabel;

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(leftWidget);
    //mainLayout->addWidget(rightWidget);
    mainLayout->addWidget(textEdit);

    QLabel *bottomMask = new QLabel(this);
    bottomMask->setStyleSheet("background-color:rgba(40, 40 ,40 ,30%)");
    bottomMask->setGeometry(0, 631, 999, 35);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addSpacing(45);
    bottomLayout->addWidget(openButton);
    bottomLayout->addSpacing(50);
    bottomLayout->addWidget(locateButton);
    bottomLayout->addSpacing(50);
    bottomLayout->addWidget(sortButton);
    bottomLayout->addSpacing(50);
    bottomLayout->addWidget(searchButton);
    bottomLayout->addStretch();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(titleBarLayout);
    layout->addLayout(mainLayout);
    layout->addLayout(bottomLayout);

    widget->setLayout(layout);
    this->setFixedSize(QSize(999,666));

    setCentralWidget(widget);//把组件居中
    setWindowTitle(tr("TT Music Player音乐播放器"));
    setLrcShown();
    this->setAutoFillBackground(true);
    QFile backgroundFile("save/background.txt");
    if(!backgroundFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Open playlist file failed.";
        return;
    }

    back = QString(backgroundFile.readAll());
    QPalette p;
    //p.setColor(QPalette::Background, QColor(192,253,123));
    //p.setBrush(QPalette::Background, QBrush(QPixmap(":/backgrounds/black")));
    p.setBrush(QPalette::Window,
                         QBrush(QPixmap(back).scaled(//缩放背景
                                    QSize(999, 666),
                                    //rightWidget->size(),
                                    Qt::IgnoreAspectRatio,
                                    Qt::SmoothTransformation)));//平滑缩放
    this->setPalette(p);
}

