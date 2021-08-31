#include "worker_thread.h"
#include "utils/utils.h"
#include "chgbox/chgbox.h"
#include "earbud/earbud.h"

#include <QDebug>
#include <QThread>
#include <QString>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QDir>
#include <QDateTime>

parse_func_map_t parse_func_map;

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

    init_chgbox_parse_func_map(parse_func_map);
    init_earbud_parse_func_map(parse_func_map);
}

SerialPortWorker::~SerialPortWorker() {
    json_file.close();
    qDebug() << "close json file";
}

void SerialPortWorker::doWork(const QByteArray hexdata) {
#ifdef DUMP_THREAD_ID
    qDebug() << "SerialPortWorkerThread:" << __FUNCTION__ << ", thread_id:" << QThread::currentThreadId();
#endif
    QJsonArray jsarr;
    QString key;
    key.clear();
    if( is_rsp_from_chgbox(hexdata) ) {
        key = chgbox_get_rsp_key(hexdata);
    }
    else if(is_1wire_reply(hexdata)){
        key = get_1wire_reply_key(hexdata);
    }
    else if(is_chgbox_commu_mode_reply(hexdata)) {
        key = get_chgbox_commu_mode_reply_key(hexdata);
    }
    else if(is_race_reply(hexdata)) {
        key = get_race_reply_key(hexdata);
    }

    if(key.isEmpty()) {
        return;
    }
    parse_func_map_t::iterator it = parse_func_map.find(key);
    if(it != parse_func_map.end()) {
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
