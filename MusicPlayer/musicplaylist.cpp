#include "musicplaylist.h"
#include "mainwindow.h"
#include <QHeaderView>
#include <QScrollBar>

//MusicPlaylist::MusicPlaylist(int i, int j/*,  QList<Phonon::MediaSource> _sources, Phonon::MediaObject *_mediaObject*/)
MusicPlaylist::MusicPlaylist(int i, int j, MainWindow *mainWindow)//选择直接把主窗口的指针传进去
    : QTableWidget(i, j)//,sources(_sources)
{
    this->mainWindow = mainWindow;
    //*&sources = _sources;
    //*&mediaObject = _mediaObject;
    createActions();
    setupPlaylist(this);
    //好感动= =终于等到你这行代码了
    this->setStyleSheet("background-color:rgba(255,255,255,50%)");
}

void MusicPlaylist::createActions()
{
    //创建右键菜单(除了播放，暂停)
    popMenu = new QMenu();
    rcPlayAction = new QAction(this);
    rcPauseAction = new QAction(this);
    downloadAction = new QAction(this);
    deleteAction = new QAction(this);
    songAction = new QAction(this);
    singerAction = new QAction(this);
    lengthAction = new QAction(this);
    numberAction = new QAction(this);

    rcPlayAction->setText(QString(tr("播放")));
    rcPauseAction->setText(QString(tr("暂停")));
    downloadAction->setText(QString(tr("下载")));
    deleteAction->setText(QString(tr("从列表中删除")));
    songAction->setText(QString(tr("按歌曲名")));
    singerAction->setText(QString(tr("按歌手名")));
    lengthAction->setText(QString(tr("按时间长度")));
    numberAction->setText(QString(tr("按编号")));

    //设置快捷键
    deleteAction->setShortcut(QKeySequence::Delete);

    connect(rcPlayAction, SIGNAL(triggered()),
            this, SLOT(rcPlay()));
    connect(rcPauseAction, SIGNAL(triggered()),
            this, SLOT(rcPause()));
    connect(deleteAction, SIGNAL(triggered()),
            mainWindow, SLOT(deleteItem()));
    connect(songAction, SIGNAL(triggered()),
            mainWindow, SLOT(sortBySong()));
    connect(singerAction, SIGNAL(triggered()),
            mainWindow, SLOT(sortBySinger()));
    connect(lengthAction, SIGNAL(triggered()),
            mainWindow, SLOT(sortByLength()));
    connect(numberAction, SIGNAL(triggered()),
            mainWindow, SLOT(sortByNumber()));
}

void MusicPlaylist::rcPlay()
{
    //设置为播放当前行的歌曲
    Phonon::MediaSource currentSource = mainWindow->getMediaObject()->currentSource();
    int i = this->item(this->currentRow(), 0)->text().toInt()-1;
    qDebug() << i;
    Phonon::MediaSource source = mainWindow->getSources().at(i);
    //Phonon::MediaSource source = mainWindow->getSources().at(this->currentRow());
    //如果是同一首歌，就不用从头播放
    if(source == currentSource)
        mainWindow->getMediaObject()->play();
    else
    {
        mainWindow->getMediaObject()->setCurrentSource(source);
        mainWindow->getMediaObject()->play();
    }
}

void MusicPlaylist::rcPause()
{
    //因为只有正在播放的歌曲暂停按钮才可按，所以直接暂停
    mainWindow->getMediaObject()->pause();
}

//播放列表点击
void MusicPlaylist::tableClicked(int row, int /*column*/)
{
    //bool wasPlaying = (mediaObject->state() == Phonon::PlayingState);
    mainWindow->getMediaObject()->stop();
    mainWindow->getMediaObject()->clearQueue();//清空播放队列

    if(row >= mainWindow->getSources().size()) return;
    int index = this->item(row, 0)->text().toInt()-1;
    mainWindow->getMediaObject()->setCurrentSource(mainWindow->getSources()[index]);

    //双击后播放
    mainWindow->getMediaObject()->play();
    /*
    if(wasPlaying)
        mediaObject->play();
    else
        mediaObject->stop();
    */
}

/*
//播放列表歌曲删除
void MusicPlaylist::deleteItem()
{
    int row = this->currentRow();
    Phonon::State state = mainWindow->getMediaObject()->state();
    Phonon::MediaSource source = mainWindow->getMediaObject()->currentSource();
    //被删除的歌处于mediaObject中,自动播放下一首
    if(source == mainWindow->getSources().at(row))
    {
        mainWindow->next();
        if(state == Phonon::StoppedState)
            mainWindow->getMediaObject()->stop();
        else if(state == Phonon::PausedState)
            mainWindow->getMediaObject()->pause();
    }

    Phonon::MediaSource s = mainWindow->getSources().at(row);
    int n = mainWindow->getSources().removeAll(s);
    mainWindow->getSources().
    qDebug() << "hangshu" << row;
    qDebug() << "daxiao wei " << mainWindow->getSources().size() << "geshu " << n;
    foreach(Phonon::MediaSource source, mainWindow->getSources())
    {
        qDebug() << source.fileName();
    }

    this->removeRow(row);
}
*/
void MusicPlaylist::contextMenuEvent(QContextMenuEvent *event)
  {
    popMenu->clear(); //清除原有菜单
    QPoint point = event->pos(); //得到窗口坐标
    QTableWidgetItem *item = this->itemAt(point);
    if(item != NULL)
    {
       popMenu->addAction(rcPlayAction);
       popMenu->addAction(rcPauseAction);
       popMenu->addSeparator();
       popMenu->addAction(deleteAction);
       popMenu->addAction(downloadAction);
       popMenu->addSeparator();
       sortMenu = popMenu->addMenu(tr("排序"));
       sortMenu->addAction(numberAction);
       sortMenu->addAction(songAction);
       sortMenu->addAction(singerAction);
       sortMenu->addAction(lengthAction);

       //qDebug("this->currentRow() = ");
       //qDebug("%d", this->currentRow());//观察输出值
       //qDebug("%d",sources.size());//这时输出的是垃圾值

       if(mainWindow->getSources().at(this->currentRow()) == mainWindow->getMediaObject()->currentSource()
               &&
               mainWindow->getMediaObject()->state() == Phonon::PlayingState)
       {
            rcPlayAction->setEnabled(false);
            rcPauseAction->setEnabled(true);
       }
       else
       {
           rcPlayAction->setEnabled(true);
           rcPauseAction->setEnabled(false);
       }

       //菜单出现的位置为当前鼠标的位置
       popMenu->exec(QCursor::pos());
       event->accept();
    }
 }

void MusicPlaylist::setupPlaylist(MusicPlaylist *musicTable)
{
    QStringList headers;//好像是叫做标题栏
    //headers << tr("Title") << tr("Artist") << tr("Album") << tr("Year");
    headers << tr("ID") << tr("Title") << tr("Artist") << tr("Time");

    //musicTable = new QTableWidget(0, 4);//新建一个0行4列的table
    //musicTable = new QTableWidget(0, 2);//新建一个0行2列的table
    //musicTable = new MusicPlaylist(0, 2);//新建一个0行2列的table
    musicTable->setHorizontalHeaderLabels(headers);//将headers里面的每一个放进table的每一列列首
    musicTable->setSelectionMode(QAbstractItemView::SingleSelection);//不能多行选择
    musicTable->setSelectionBehavior(QAbstractItemView::SelectRows);//单行选择
    //musicTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    musicTable->horizontalHeader()->hide();//隐藏列头
    musicTable->verticalHeader()->hide();//隐藏行头
    //右键菜单
    //musicTable->setContextMenuPolicy(Qt::CustomContextMenu);//需要自己写slot而且connect鼠标点击信号
    musicTable->setContextMenuPolicy(Qt::DefaultContextMenu);//只需要重写contextMenuEvent()函数就行
    musicTable->setItemDelegate(new CancelDottedLine(musicTable));
    musicTable->setShowGrid(false);//去掉格子线
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//去掉水平滚动条
    //设置宽度
    musicTable->setColumnWidth(0, 28);
    musicTable->setColumnWidth(1, 150);
    musicTable->setColumnWidth(2, 100);
    musicTable->setColumnWidth(3, 40);
    //musicTable->horizontalHeader()->setHighlightSections(true);
    musicTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    //读取文件内容，设置滑条样式
    QFile styleSheet(":/styleSheet/scrollBar");
    styleSheet.open(QFile::ReadOnly);
    musicTable->verticalScrollBar()->setStyleSheet(styleSheet.readAll());
    musicTable->setFixedHeight(490);
    styleSheet.close();
//    QPalette pll = musicTable->palette();
//    pll.setBrush(QPalette::Base,QBrush(QColor(255,255,255,0)));
//    musicTable->setPalette(pll);
//    pll.setBrush(QPalette::Window, QBrush(QPixmap(":/mask/musicPlaylistMask")));
//    musicTable->setPalette(pll);
//    musicTable->setWindowOpacity(0.5);


    //connect留在mainwindow.cpp中写
    //关联鼠标点击与table点击
    //connect(musicTable, SIGNAL(cellPressed(int,int)),
    //        this, SLOT(tableClicked(int,int)));
    //关联右键播放与暂停功能
}

