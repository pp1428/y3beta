#include <QtGui/QApplication>
#include "clinfo.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    clinfo w;
    w.show();

    return a.exec();
}
