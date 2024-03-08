//
// Created by dingjing on 24-3-8.
//
#include <QDebug>

#include "../qt5/qlog.h"


int main (int argc, char* argv[])
{
    C_QLOG_INIT_IF_NOT_INIT

    qDebug() << "aaaaa";

    qWarning() << "aaabbb";

    qCritical() << "ccc";

    return 0;
}