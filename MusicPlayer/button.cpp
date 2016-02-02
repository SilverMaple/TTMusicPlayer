#include "button.h"
#include <QUrl>

Button::Button(QWidget *parent) : QPushButton(parent)
{
    //保存图片成员初始化
    normalPicture = new QString;
    pressPicture = new QString;
    hoverPicture = new QString;

    //关闭按钮的默认显示
    this -> setFlat(true);
}

void Button::setNormalPicture(QString pic)
{
    *normalPicture = pic;

    this -> setIcon(QIcon(*normalPicture));
}

void Button::setPressPicture(QString pic)
{
    *pressPicture = pic;
}

void Button::setHoverPicture(QString pic)
{
    *hoverPicture	= pic;
}

void Button::setTriState()
{
    QUrl u = new QUrl(hoverPicture);
    this->setStyleSheet("QPushButton{border-image: url(:/new/icons/icons/play-pause.png);}"
                        "QPushButton:hover{border-image: url(:/new/icons/icons/play-pause-hover.png);}"
                        "QPushButton:pressed{border-image: url(:/new/icons/icons/play-pause-pressed.png);}");
}

//void Button::mouseDoubleClickEvent(QMouseEvent *event)
//{
//    //null
//}

//void Button::mousePressEvent (QMouseEvent *event)
//{
//    this -> setIcon (QIcon(*pressPicture));
//}

////这个其实就是鼠标悬停在按钮上的slot
//void Button::mouseMoveEvent(QMouseEvent *event)
//{
//    //null
//}

////鼠标离开后要把图标弄回之前的
//void Button::mouseReleaseEvent (QMouseEvent *event)
//{
//    this -> setIcon(QIcon(*releasePicture));
//    emit clicked();//发出点击信号
//}

