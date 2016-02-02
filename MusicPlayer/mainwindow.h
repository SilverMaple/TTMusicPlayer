#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <phonon/audiooutput.h>
#include <phonon/seekslider.h>
#include <phonon/mediaobject.h>
#include <phonon/volumeslider.h>
#include <phonon/backendcapabilities.h>
#include <QPushButton>
#include <QList>
#include <QDir>
#include <QLabel>
#include <QSystemTrayIcon>
#include <QTextEdit>
#include "musicplaylist.h"
#include "lyrics.h"

QT_BEGIN_NAMESPACE
class QAction;
class QTableWidget;
class QLCDNumber;
QT_END_NAMESPACE

class MusicPlaylist;
class Lyrics;

enum ModeState{order, shuffle, allRepeat, repeatOnce, once};

class MainWindow : public QMainWindow
{
    Q_OBJECT

    //friend class MusicPlaylist;
    //设置为友元函数来访问私有成员
    //friend void MusicPlaylist::contextMenuEvent(QContextMenuEvent *event);

public:
    MainWindow();

    QList<Phonon::MediaSource> getSources();
    Phonon::MediaObject* getMediaObject();
    QPushButton *getLRCButton();
    void next()
    {
        forwardPlay();
    }

private slots:
    //底部栏中的槽函数
    void addFiles();
    void locateSong();
    void about();
    //播放控制的槽函数
    void forwardPlay();
    void backwardPlay();
    void playPause();
    void deleteItem();
    void sortBySong();
    void sortByLength();
    void sortBySinger();
    void sortByNumber();
    void searchOnline();
    void stateChanged(Phonon::State newState, Phonon::State oldState);
    void tick(qint64 time);
    //void tableClicked(int row, int column);
    void sourceChanged(const Phonon::MediaSource &source);
    void metaStateChanged(Phonon::State newState, Phonon::State oldState);
    void aboutToFinish();
    void setLrcShown();
    void TrayIconActivated(QSystemTrayIcon::ActivationReason activationReason);
    void showSkinMenu();
    void showPlayModeMenu();
    void minimize();//这个函数是为了最小化时歌词仍留在桌面
    void valueChanged(int);
    void volumeSwitch();
    void setBackground();
    //修改播放模式的slot
    void oncePlay() {updateModeState(once);}
    void orderPlay() {updateModeState(order);}
    void shufflePlay() {updateModeState(shuffle);}
    void allRepeatPlay() {updateModeState(allRepeat);}
    void repeatOncePlay() {updateModeState(repeatOnce);}

//    void on_actionMinimize_triggered();//最小化窗口
//    void on_actionClose_triggered();//关闭窗口

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);

private:
    void updateModeState(ModeState modeState);
    void resolveLrc(const QString &sourceFileName);
    void initSkinMenu();
    void initPlayModeMenu();
    void initTrayIconMenu();
    void closeEvent(QCloseEvent *event);
    void setupActions();
    //void setupMenus();
    void setupUi();
    void loadLastMusicTable();
    void gotoLine(QTextEdit*, int);
    void changeLineStyle(QTextCursor textCursor, QTextCharFormat charFormat, int beginPos, int endPos);

    Phonon::SeekSlider *seekSlider;
    QSlider *volumeSlider;
    Phonon::MediaObject *mediaObject;
    Phonon::MediaObject *metaInformationResolver;
    Phonon::AudioOutput *metaInformationAudioOutput;
    Phonon::AudioOutput *audioOutput;
    //Phonon::VolumeSlider *volumeSlider;
    QList<Phonon::MediaSource> sources;
    Phonon::MediaSource preSource;

    int lineNo;
    qint64 testRepeat;//这个是用来检测是否有连续重复歌词
    bool isMousePressed;
    QDir *dir;
    QMenu *skinMenu;
    QMenu *playModeMenu;
    QPoint mousePosition;
    QLabel *songName;
    QLabel *lrcLabel;
    QLabel *currentTimeLabel;
    QLabel *totalTimeLabel;
    QAction *playAction;
    QAction *pauseAction;
    QAction *stopAction;
    QAction *nextAction;
    QAction *previousAction;
    QAction *addFilesAction;
    QAction *exitAction;
    QAction *aboutAction;
    QAction *aboutQtAction;
    QAction *LRCAction;
    QList<QAction> *actions;

    QPushButton *openButton;
    QPushButton *locateButton;
    QPushButton *sortButton;
    QPushButton *searchButton;
    QPushButton *helpButton;
    QPushButton *skinButton;
    QPushButton *minButton;
    QPushButton *closeButton;

    QPushButton *volumeButton;
    QPushButton *LRCButton;
    QPushButton *playModeButton;
    QPushButton *playPauseButton;
    QPushButton *previousButton;
    QPushButton *nextButton;

    ModeState modeState;
    QActionGroup *modeActions;
    QAction *allRepeatPlayAction;
    QAction *oncePlayAction;
    QAction *orderPlayAction;
    QAction *repeatOncePlayAction;
    QAction *shufflePlayAction;
    //QLCDNumbercurrentTimeLcdcd;
    //QLCDNumber *totalTimeLcd;
    QTextEdit *textEdit;
    QTextCharFormat format;
    QTextCharFormat defaultFormat;
    QTextCharFormat midFormat;
    //QTableWidget *musicTable;
    MusicPlaylist *musicTable;
    Lyrics *lrc;
    QMap<qint64, QString> lrcMap;
    QSystemTrayIcon *trayIcon;
    QString back;
};

#endif
