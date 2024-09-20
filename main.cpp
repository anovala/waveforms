#include "WaveformsWidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WaveformsWidget w;
    w.show();
    return a.exec();
}
