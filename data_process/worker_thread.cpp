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
    //int ret = RET_FAIL;

    if( CHGBOX_FT_RSP_LEAD == (uchar)hexdata[0] && CHGBOX_BASIC_FT_RSP_FC == (uchar)hexdata[1] ) {
        parse_chgbox_basic_ft_rsp(hexdata, jsarr);
    }
    else if( CHGBOX_FT_RSP_LEAD == (uchar)hexdata[0] && CHGBOX_FT_W_SN_RSP_FC == (uchar)hexdata[1] ) {
        parse_chgbox_ft_w_sn_rsp(hexdata, jsarr);
    }
    else if( CHGBOX_FT_RSP_LEAD == (uchar)hexdata[0] && CHGBOX_FT_R_SN_RSP_FC == (uchar)hexdata[1] ) {
        parse_chgbox_ft_r_sn_rsp(hexdata, jsarr);
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
