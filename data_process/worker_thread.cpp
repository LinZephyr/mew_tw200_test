#include "worker_thread.h"
#include "utils/utils.h"
#include "chgbox/chgbox.h"

#include <QDebug>
#include <QThread>
#include <QString>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QDir>
#include <QDateTime>

static parse_func_map_t parse_map;

SerialPortWorker::SerialPortWorker(QObject *parent) : QObject(parent),
    json_file(QDir::currentPath() + "/MEW-" + QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss") + ".json")
{
    jsfile_opened = json_file.open(QIODevice::Append);
    if(jsfile_opened) {
        qDebug() << "open json file success: " << json_file.fileName();
    }
    else {
        qDebug() << "open json file FAIL!!!";
    }

    chgbox_initialize_parse_func_list(parse_map);
}

SerialPortWorker::~SerialPortWorker() {
    json_file.close();
    qDebug() << "close json file";
}

void SerialPortWorker::doWork(const QByteArray hexdata) {
    //qDebug() << "SerialPortWorkerThread:" << __FUNCTION__ << ", thread_id:" << QThread::currentThreadId();
    if(hexdata.count() < 2) {
        return;
    }

    QJsonArray jsarr;
    const QString key =MAKE_STRING_UINT8_UINT8((uchar)hexdata[0], (uchar)hexdata[1]);
    parse_func_map_t::iterator it = parse_map.find(key);
    if(it != parse_map.end()) {
        it.value()(hexdata, jsarr);
    }

    if(jsarr.isEmpty()) {
        return;
    }
    emit resultReady(jsarr);
    if(jsfile_opened) {
        QJsonDocument jdoc;
        jdoc.setArray(jsarr);
        json_file.write(jdoc.toJson());
        json_file.flush();
    }
}
