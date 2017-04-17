#include <QCoreApplication>
#include <qdebug.h>
#include "ftdi_Function.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug() << "hello beautiful world!" << endl << endl;

    FTDI_Function ftdi_Test = new FTDI_Function();

    ftdi_Test.main();

    return a.exec();
}
