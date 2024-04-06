#include "AVPlayer.h"
#include <QtWidgets/QApplication>

#undef main
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AVPlayer w;
    w.show();
    return a.exec();
}
