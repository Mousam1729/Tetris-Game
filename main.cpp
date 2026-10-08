#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication a(argc, argv);
    a.setApplicationName("TetrisRasterGame");
    a.setApplicationDisplayName("Tetris - Computer Graphics Lab");

    MainWindow w;
    w.show();

    return a.exec();
}
