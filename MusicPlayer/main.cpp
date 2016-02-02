#include <QtGui>
#include <QTextCodec>

#include "mainwindow.h"
#include "canceldottedline.h"

//![1]
int main(int argv, char **args)
{
    QApplication app(argv, args);
    QTextCodec *codec = QTextCodec::codecForName("UTF8");
    if(codec)
    {
        QTextCodec::setCodecForTr(codec);
        QTextCodec::setCodecForLocale(codec);
        QTextCodec::setCodecForCStrings(codec);
    }
    app.setApplicationName("TT Music Player");
    app.setWindowIcon(QIcon(":/pictures/Ticon"));
    app.setQuitOnLastWindowClosed(true);

    MainWindow window;

    window.show();

    return app.exec();
}
//![1]
