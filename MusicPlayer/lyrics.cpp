#include "lyrics.h"
#include <QTimer>
#include <QPainter>

Lyrics::Lyrics(MainWindow *parent)
    : QLabel(parent)
{
    mainWindow = parent;
    //设置label出现位置
    setGeometry(500, 650, 800, 60);
    //去掉边界
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    //设置为半透明符合习惯
    setAttribute(Qt::WA_TranslucentBackground);
    setText(tr("天台音乐播放器"));
    //固定区域大小
    setFixedSize(QSize(800, 60));

    //歌词的填充规律
    linearGradient.setStart(0, 10);//起点
    linearGradient.setFinalStop(0,40);//终点
    //参数为比例位置与对应颜色
    //linearGradient.setColorAt(0.1, QColor(0, 170, 255));
    //linearGradient.setColorAt(0.5, QColor(114, 232, 255));
    //linearGradient.setColorAt(0.5, QColor(0, 191, 255));
    //linearGradient.setColorAt(0.9, QColor(0, 170, 255));
    //linearGradient.setColorAt(0.9, QColor(14, 179, 255));
    //暗蓝
    linearGradient.setColorAt(0.1, QColor(0, 94, 170));
    linearGradient.setColorAt(0.5, QColor(0, 104, 195));
    linearGradient.setColorAt(0.9, QColor(0, 94, 170));

    //遮罩的填充规律
    coverLinearGradient.setStart(0, 10);
    coverLinearGradient.setFinalStop(0, 40);
    //red
//    coverLinearGradient.setColorAt(0.1, QColor(222, 54, 4));
//    coverLinearGradient.setColorAt(0.5, QColor(255, 72, 16));
//    coverLinearGradient.setColorAt(0.9, QColor(222, 54, 4));
    //green
//    coverLinearGradient.setColorAt(0.1, QColor(122, 224, 0));
//    coverLinearGradient.setColorAt(0.5, QColor(124, 252, 0));
//    coverLinearGradient.setColorAt(0.9, QColor(122, 224, 0));
    //blue
    coverLinearGradient.setColorAt(0.1, QColor(0, 170, 180));
    coverLinearGradient.setColorAt(0.5, QColor(0, 220, 255));
    coverLinearGradient.setColorAt(0.9, QColor(0, 170, 180));

    //设置字体的样式
    font.setFamily("Times New Roman");
    font.setBold(true);
    font.setPointSize(30);

    //设置计时器
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()),
            this, SLOT(timeOut()));
    lrcCoverWidth = 0;
    lrcCoverWidthInterval = 0;
}

//显示遮罩，并制定当前一句歌词的始末时间间隔
void Lyrics::showCover(qint64 intervaltime)
{
    //每隔30毫秒刷新一次
    qreal count = intervaltime / 30;
    //计算增加单位宽度
    lrcCoverWidthInterval = 800 / count;
    lrcCoverWidth = 0;
    timer->start(30);
}

//暂停遮罩的刷新
void Lyrics::stopCover()
{
    timer->stop();
    lrcCoverWidth = 0;
    update();
}

void Lyrics::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setFont(font);

    //绘制底层阴影文字
    painter.setPen(QColor(0, 0, 0, 200));//cmyk形式
    //Qt::AlignLef为左对齐, 在(1, 1)绘制
    painter.drawText(1, 1, 800, 60, Qt::AlignLeft, text());

    //再绘制渐变文字
    painter.setPen(QPen(linearGradient, 0));
    painter.drawText(0, 0, 800, 60, Qt::AlignLeft, text());

    //设置歌词遮罩
    painter.setPen(QPen(coverLinearGradient, 0));
    painter.drawText(0, 0, lrcCoverWidth, 60, Qt::AlignLeft, text());
}

//鼠标左键单击
void Lyrics::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        offset = event->globalPos() - frameGeometry().topLeft();
}

//鼠标移动
void Lyrics::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        //设置为手型光标
        setCursor(Qt::PointingHandCursor);
        //拖动歌词
        move(event->globalPos() - offset);
    }
}

//鼠标右击后出现右键菜单
void Lyrics::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;
    menu.addAction(tr("关闭桌面歌词"), this, SLOT(closeLrc()));
    menu.exec(event->globalPos());
}

void Lyrics::timeOut()
{
    //每次加一点长度
    lrcCoverWidth += lrcCoverWidthInterval;
    update();
}

void Lyrics::closeLrc()
{
    hide();
    mainWindow->getLRCButton()->setStyleSheet("QPushButton{border-image: url(:/icons/LRCdown);}"
                                   "QPushButton:hover{border-image: url(:/icons/LRCHover);}"
                                   "QPushButton:pressed{border-image: url(:/icons/LRCPress);}");
}
