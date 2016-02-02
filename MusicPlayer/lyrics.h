#ifndef LYRICS
#define LYRICS

#include <QLabel>
#include "mainwindow.h"

class MainWindow;

class Lyrics : public QLabel
{
    Q_OBJECT

public:
    explicit Lyrics(MainWindow* parent);
    void showCover(qint64 intervaltime);
    void stopCover();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

private slots:
    void timeOut();
    void closeLrc();

private:
    MainWindow *mainWindow;
    QLinearGradient linearGradient;
    QLinearGradient coverLinearGradient;
    QFont font;
    QTimer *timer;
    qreal lrcCoverWidth;
    qreal lrcCoverWidthInterval;
    QPoint offset;
};

#endif // LYRICS

