#ifndef BUTTON
#define BUTTON

#include <QPushButton>

class Button : public QPushButton
{
    Q_OBJECT

public:
    Button(QWidget *parent);
    void setNormalPicture(QString pic);
    void setPressPicture(QString pic);
    void setHoverPicture(QString pic);
    void setTriState();//设置setstylesheet

private slots:
//    void mouseDoubleClickEvent(QMouseEvent *event);
//    void mousePressEvent (QMouseEvent *event);
//    void mouseMoveEvent(QMouseEvent *event);
//    void mouseReleaseEvent (QMouseEvent *event);

private:
    QString normalPicture;
    QString pressPicture;
    QString hoverPicture;
};

#endif
