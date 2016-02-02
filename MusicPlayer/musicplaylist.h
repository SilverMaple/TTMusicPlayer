#ifndef MUSICPLAYLIST
#define MUSICPLAYLIST

#include <QTableWidget>
#include <QMenu>
#include <QContextMenuEvent>
#include "canceldottedline.h"
#include "mainwindow.h"

class MainWindow;

class MusicPlaylist : public QTableWidget
{
    //加了之后有很多bug
    Q_OBJECT

public:
    //MusicPlaylist(int i, int j/*, QList<Phonon::MediaSource> _sources, Phonon::MediaObject *_mediaObject*/);
    MusicPlaylist(int i, int j, MainWindow *mainWindow);

    void createActions();
    void setupPlaylist(MusicPlaylist *musicTable);
    void contextMenuEvent(QContextMenuEvent *event);

    //获取private成员的函数
    QAction* getPlayAction(){
        return rcPlayAction;
    }
    QAction* getPauseAction(){
        return rcPauseAction;
    }

public slots:
    //void deleteItem();
    void tableClicked(int row, int column);
    void rcPlay();
    void rcPause();

private:
    MainWindow *mainWindow;
    /*QList<Phonon::MediaSource> &sources;
    Phonon::MediaObject *mediaObject;*/
    QMenu *popMenu;
    QMenu *sortMenu;
    //这些action的connect在mainwindow的setupaction中完成
    QAction *rcPlayAction;//为了区别工具栏中的action，加上rc(right clicked)
    QAction *rcPauseAction;
    QAction *deleteAction;
    QAction *downloadAction;
    QAction *numberAction;
    QAction *songAction;
    QAction *singerAction;
    QAction *lengthAction;
};

#endif // MUSICPLAYLIST

