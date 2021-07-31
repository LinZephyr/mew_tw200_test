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

    QJsonObject jsobj;
    QString str;
    int ret = RET_FAIL;

    if( CHGBOX_FT_RSP_LEAD == (uchar)hexdata[0] && CHGBOX_BASIC_FT_RSP_FC == (uchar)hexdata[1] ) {
        ret = parse_chgbox_basic_ft_rsp(hexdata, jsobj, str);
    }
    else if( CHGBOX_FT_RSP_LEAD == (uchar)hexdata[0] && CHGBOX_FT_W_SN_RSP_FC == (uchar)hexdata[1] ) {
        ret = parse_chgbox_ft_w_sn_rsp(hexdata, jsobj, str);
    }
    else if( CHGBOX_FT_RSP_LEAD == (uchar)hexdata[0] && CHGBOX_FT_R_SN_RSP_FC == (uchar)hexdata[1] ) {
        ret = parse_chgbox_ft_r_sn_rsp(hexdata, jsobj, str);
    }

    if(RET_OK == ret) {
        emit resultReady(str);
        if(jsfile_opened) {
            QJsonDocument jdoc;
            jdoc.setObject(jsobj);
            json_file.write(jdoc.toJson());
            json_file.flush();
        }
    }




}
