#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils/utils.h"
#include "chgbox/chgbox.h"
#include "earbud/earbud.h"
#include "data_process/worker_thread.h"

#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include <QListView>
#include <QSerialPortInfo>
#include <QtConcurrent/QtConcurrent>
#include <QTextCursor>

#define  TIMER_INTERVAL_SEND_COMMAND  1000

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    myCom = NULL;
    initCom();
    startThread();
    setRelatedWidgetsStatus(false);
    qRegisterMetaType<QTextCursor>("QTextCursor");
#ifdef DUMP_THREAD_ID
    qDebug() << "MainWindow::" << __FUNCTION__ << ", thread_id:" << QThread::currentThreadId();
#endif
}

MainWindow::~MainWindow()
{
    if (myCom != NULL)
    {
        if (myCom->isOpen())
        {
            myCom->close();
        }
        delete myCom;
    }

    comWorkerThread.quit();
    comWorkerThread.deleteLater();
    comWorkerThread.wait();  //必须等待线程结束

    delete ui;
}

void MainWindow::startThread()
{
    SerialPortWorker *worker = new SerialPortWorker;
    worker->moveToThread(&comWorkerThread);

    // 退出、删除
    //connect(worker, SIGNAL(finished()), &comWorkerThread, SLOT(quit()));
    connect(&comWorkerThread, SIGNAL(finished()), worker, SLOT(deleteLater()) );

    connect(this, SIGNAL(dataReceived(QByteArray)), worker, SLOT(doWork(QByteArray)) );
    connect(worker, SIGNAL(resultReady(QJsonArray)), this, SLOT(handleResults(QJsonArray)) );

    comWorkerThread.start();
}

void MainWindow::handleResults(QJsonArray jsarr)
{
    //ui->parsedDataBrowser->append(str);
    /*
        Data format must be:
        [
            QString,
            QJsonObject,
            ...
        ]
    */
    ui->parsedDataBrowser->append("");
    for(QJsonArray::const_iterator it = jsarr.constBegin(); it != jsarr.constEnd(); ++it) {
        if(it->type() == QJsonValue::String) {
            QString str = it->toString();
            ui->parsedDataBrowser->append(str);
        }
        else if(it->type() == QJsonValue::Object) {
            QJsonObject jsobj = it->toObject();
            for(QJsonObject::const_iterator obj_it = jsobj.constBegin(); obj_it != jsobj.constEnd(); ++obj_it) {
                QString k = obj_it.key();
                QJsonValue v = obj_it.value();
                QString v_str = jsonValue2String(v);
                if(k.contains(KEY_STR_EXCEPTION) || v_str.contains(VALUE_STR_FAIL)) {
                    QColor cl = ui->parsedDataBrowser->textColor();
                    ui->parsedDataBrowser->setTextColor(Qt::red);
                    ui->parsedDataBrowser->append(k + " : " +  v_str);
                    ui->parsedDataBrowser->setTextColor(cl);
                }
                else {
                    ui->parsedDataBrowser->append(k + " : " + v_str );
                }
            }
        }
    }
}

void MainWindow::checkComPort()
{
    ui->portNameComboBox->clear();

    //查找可用的串口
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->portNameComboBox->addItem(info.portName());
    }
}

void MainWindow::initCom()
{
    ui->portNameComboBox->setView(new QListView);
    ui->baudRateComboBox->setView(new QListView);
    ui->dataBitsComboBox->setView(new QListView);
    ui->parityComboBox->setView(new QListView);
    ui->stopBitsComboBox->setView(new QListView);

    checkComPort();
}

int MainWindow::closeCom()
{
    myCom->close();
    delete myCom;
    myCom = NULL;
    setRelatedWidgetsStatus(false);

    return 0;
}

int MainWindow::openCom()
{
    QString portName = ui->portNameComboBox->currentText();   //获取串口名
    myCom = new QextSerialPort(portName);
    connect(myCom, SIGNAL(readyRead()), this, SLOT(recv_com_data()));

    //设置波特率
    myCom->setBaudRate((BaudRateType)ui->baudRateComboBox->currentText().toInt());

    //设置数据位
    myCom->setDataBits((DataBitsType)ui->dataBitsComboBox->currentText().toInt());

    //设置校验
    switch(ui->parityComboBox->currentIndex()){
    case 0:
         myCom->setParity(PAR_NONE);
         break;
    case 1:
        myCom->setParity(PAR_ODD);
        break;
    case 2:
        myCom->setParity(PAR_EVEN);
        break;
    default:
        myCom->setParity(PAR_NONE);
        qDebug("set to default : PAR_NONE");
        break;
    }

    //设置停止位
    switch(ui->stopBitsComboBox->currentIndex()){
    case 0:
        myCom->setStopBits(STOP_1);
        break;
    case 1:
 #ifdef Q_OS_WIN
        myCom->setStopBits(STOP_1_5);
 #endif
        break;
    case 2:
        myCom->setStopBits(STOP_2);
        break;
    default:
        myCom->setStopBits(STOP_1);
        qDebug("set to default : STOP_1");
        break;
    }

    //设置数据流控制
    myCom->setFlowControl(FLOW_OFF);
    //设置延时
    myCom->setTimeout(TIME_OUT);

    if (myCom->open(QIODevice::ReadWrite))
    {
        setRelatedWidgetsStatus(true);
    }
    else
    {
        QMessageBox::critical(this, tr("打开失败"), tr("未能打开串口 ") + portName + tr("\n该串口设备不存在或已被占用"), QMessageBox::Ok);
        return -1;
    }

    return 0;
}

void MainWindow::setComWidgetsStatus(bool opened)
{
    bool status = !opened;

    ui->portNameComboBox->setEnabled(status);
    ui->baudRateComboBox->setEnabled(status);
    ui->dataBitsComboBox->setEnabled(status);
    ui->parityComboBox->setEnabled(status);
    ui->stopBitsComboBox->setEnabled(status);
    ui->comCheckBtn->setEnabled(status);
}

void MainWindow::setRelatedWidgetsStatus(bool opened)
{
    setComWidgetsStatus(opened);

    bool enable_status = opened;
    ui->sendmsgBtn->setEnabled(enable_status);
    //ui->chgbox_basic_FT_btn->setEnabled(enable_status);
    //ui->chgbox_wSN_btn->setEnabled(enable_status);
    //ui->chbox_r_sn_btn->setEnabled(enable_status);
    ui->tabWidget->setEnabled(enable_status);
}

void MainWindow::on_openCloseBtn_clicked()
{
    if( NULL == myCom ) {
        if(0 == openCom()) {
            ui->openCloseBtn->setText("关闭");
        }
    }
    else {
        closeCom();
        ui->openCloseBtn->setText("打开");
    }
}

void MainWindow::on_sendmsgBtn_clicked()
{
    QString strdata = ui->sendMsgLineEdit->text();

    //如果发送数据为空，给出提示并返回
    if(strdata.isEmpty()){
        //QMessageBox::information(this, tr("提示消息"), tr("没有需要发送的数据"), QMessageBox::Ok);
        return;
    }

    if(ui->hexSendRadioBtn->isChecked()){
        QByteArray hexdata;
        if(RET_OK != string2HexArray(strdata, hexdata)) {
            QMessageBox::information(this, tr("提示消息"), tr("输入的数据格式有错误！"), QMessageBox::Ok);
            return;
        }
        sendHexMsg(hexdata);
    }
    else{
        sendAsciiMsg(strdata);
    }
}

int MainWindow::sendAsciiMsg(QString msg)
{
    Q_UNUSED(msg);
    // TODO:
    return 0;
}

int MainWindow::sendHexMsg(QByteArray hexdata)
{
    if(NULL == myCom) {
        QMessageBox::information(this, tr("提示消息"), tr("串口未打开"), QMessageBox::Ok);
        return -1;
    }

    if(-1 == myCom->write(hexdata) ) {
        QMessageBox::critical(this, tr("警告"), tr("发送数据失败！ "), QMessageBox::Ok);
        return -1;
    }
    else {
        QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        QString timeStrLine= "[" + ts + "]";
        QString tmpstr = "[ " + QString::number(hexdata.count(), 10) + " ]: ";
        QString content = "<span style=\" color:green;\">" + timeStrLine + tmpstr + hexArray2StringPlusSpace(hexdata) +"\n\r</span>";
        ui->rawDataBrowser->append(content);
        //ui->textBrowser->setTextColor(Qt::lightGray);
    }
    return 0;
}

void MainWindow::recv_com_data()
{
    QThread::msleep(50);
    QByteArray hexdata = myCom->readAll();
    QString asciidata;

    if(hexdata.isEmpty()) {
        return;
    }

    //ui->textBrowser->setTextColor(Qt::black);
    if(ui->hexRecvRadioBtn->isChecked()){
        asciidata = hexArray2StringPlusSpace(hexdata);
    }
    else {
        asciidata = hexdata;
    }

    QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString timeStrLine="[" + ts + "]";
    QString tmpstr = "[ " + QString::number(hexdata.count(), 10) + " ]: ";
    QString content = "<span style=\" color:blue;\">" + timeStrLine + tmpstr + asciidata + "\n\r" + "</span>";
    ui->rawDataBrowser->append(content);
    emit dataReceived(hexdata);
}

void MainWindow::on_comCheckBtn_clicked()
{
    checkComPort();
}


void MainWindow::on_clearUpBtn_clicked()
{
    ui->rawDataBrowser->clear();
}

void MainWindow::on_clearUpBtn2_clicked()
{
    ui->parsedDataBrowser->clear();
}

void MainWindow::on_chgbox_basic_FT_btn_clicked()
{
    QByteArray hexCmd;
    hexCmd = construct_chgbox_basic_ft_cmd(ui->led1_on_rbtn->isChecked(), ui->led2_on_rbtn->isChecked(), ui->led3_on_rbtn->isChecked(), ui->chg_mode_wireless_rbtn->isChecked());
    sendHexMsg(hexCmd);
}

void MainWindow::on_chgbox_wSN_btn_clicked()
{
    QString snStr = ui->chgbox_SN_edit->text();

    //如果发送数据为空，给出提示并返回
    if(snStr.isEmpty()){
        //QMessageBox::information(this, tr("提示消息"), tr("没有需要发送的数据"), QMessageBox::Ok);
        return;
    }

    QByteArray hexcmd;
    if (RET_OK == construct_chgbox_ft_w_sn_cmd(snStr, hexcmd) ) {
        sendHexMsg(hexcmd);
    }
    else {
        QMessageBox::information(this, tr("提示消息"), tr("输入的数据格式有错误！"), QMessageBox::Ok);
        return;
    }

}


void MainWindow::on_chbox_r_sn_btn_clicked()
{
    QByteArray hexcmd;
    if (RET_OK == construct_chgbox_ft_r_sn_cmd(hexcmd) ) {
        sendHexMsg(hexcmd);
    }
}


void MainWindow::on_r_mac_btn_clicked()
{
    read_mac_addr();
}

void MainWindow::read_mac_addr()
{
    QByteArray cmd;
    if(RET_OK == earbud_construct_cmd_read_mac(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::read_fw_ver_addr()
{
    QByteArray cmd;
    if(RET_OK == earbud_construct_cmd_read_version(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::on_r_fw_ver_btn_clicked()
{
    read_fw_ver_addr();
}

void MainWindow::read_channel()
{
    QByteArray cmd;
    if(RET_OK == earbud_construct_cmd_read_channel(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::on_r_channel_btn_clicked()
{
    read_channel();
}

void MainWindow::read_temperature()
{
    QByteArray cmd;
    if(RET_OK == earbud_construct_cmd_read_temperature(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::on_r_ntc_btn_clicked()
{
    read_temperature();
}

void MainWindow::active_license_key()
{
#ifdef DUMP_THREAD_ID
    qDebug() << "MainWindow:" << __FUNCTION__ << ", thread_id:" << QThread::currentThreadId();
#endif
    QByteArray cmd;
    if(RET_OK == earbud_construct_cmd_get_license_key(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    cmd.clear();
    if(RET_OK == earbud_construct_cmd_set_license_key(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    cmd.clear();
    if(RET_OK == earbud_construct_cmd_get_license_result(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::on_r_active_license_btn_clicked()
{
#ifdef DUMP_THREAD_ID
    qDebug() << "MainWindow:" << __FUNCTION__ << ", thread_id:" << QThread::currentThreadId();
#endif
    QtConcurrent::run(this, &MainWindow::active_license_key);
}

void MainWindow::captouch_start_interrupt()
{
    QByteArray cmd;
    if(RET_OK == earbud_construct_cmd_captouch_start_int(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::captouch_get_interrupt_result()
{
    QByteArray cmd;
    if(RET_OK == earbud_construct_cmd_captouch_get_int_result(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::captouch_read_version()
{
    QByteArray cmd;
    if(RET_OK == earbud_construct_cmd_captouch_read_version(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::captouch_start_calib()
{
    QByteArray cmd;
    if(RET_OK == earbud_construct_cmd_captouch_start_calib(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::captouch_get_calib_result()
{
    QByteArray cmd;
    if(RET_OK == earbud_construct_cmd_captouch_get_calib_result(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::captouch_read_value()
{
    QByteArray cmd;
    if(RET_OK == earbud_construct_cmd_captouch_read_value(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::captouch_test()
{
    captouch_start_interrupt();

    QThread::msleep(2000);
    captouch_get_interrupt_result();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    captouch_read_version();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    captouch_start_calib();

    QThread::msleep(3000);
    captouch_get_calib_result();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    captouch_read_value();

}

void MainWindow::on_captouch_test_btn_clicked()
{
    QtConcurrent::run(this, &MainWindow::captouch_test);
}


void MainWindow::optic_communicate()
{
    QByteArray cmd;
    if(RET_OK == earbud_construc_cmd_optic_communicate(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::optic_int_start()
{
    QByteArray cmd;
    if(RET_OK == earbud_construc_cmd_optic_int_start(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::optic_int_end()
{
    QByteArray cmd;
    if(RET_OK == earbud_construc_cmd_optic_int_end(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::optic_laser_start()
{
    QByteArray cmd;
    if(RET_OK == earbud_construc_cmd_optic_laser_start(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::optic_laser_end()
{
    QByteArray cmd;
    if(RET_OK == earbud_construc_cmd_optic_laser_end(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::optic_full_scale()
{
    QByteArray cmd;
    if(RET_OK == earbud_construc_cmd_optic_full_scale(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::optic_bg_noise_start()
{
    QByteArray cmd;
    if(RET_OK == earbud_construc_cmd_optic_bg_noise_start(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::optic_bg_noise_end()
{
    QByteArray cmd;
    if(RET_OK == earbud_construc_cmd_optic_bg_noise_end(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::optic_test()
{
    optic_communicate();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    optic_int_start();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    optic_int_end();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    optic_laser_start();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    optic_laser_end();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    optic_full_scale();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    optic_bg_noise_start();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    optic_bg_noise_end();
}

void MainWindow::on_optic_test_btn_clicked()
{
    QtConcurrent::run(this, &MainWindow::optic_test);
}

void MainWindow::force_start_detect()
{
    QByteArray cmd;
    if(RET_OK == earbud_construc_cmd_force_start_detect(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::force_get_fw_ver()
{
    QByteArray cmd;
    if(RET_OK == earbud_construc_cmd_force_get_fw_ver(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::force_get_assemble()
{
    QByteArray cmd;
    if(RET_OK == earbud_construc_cmd_force_get_assemble(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::force_get_noise_peak()
{
    QByteArray cmd;
    if(RET_OK == earbud_construc_cmd_force_get_noise_peak(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::force_get_burst_pressure()
{
    QByteArray cmd;
    if(RET_OK == earbud_construc_cmd_force_get_burst_pressure(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::force_get_semph()
{
    QByteArray cmd;
    if(RET_OK == earbud_construc_cmd_force_get_semph(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::force_test()
{
    force_start_detect();

    QThread::msleep(7000);
    force_get_fw_ver();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    force_get_assemble();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    force_get_noise_peak();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    force_get_burst_pressure();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    force_get_semph();
}

void MainWindow::on_force_sensor_test_btn_clicked()
{
    QtConcurrent::run(this, &MainWindow::force_test);
}

void MainWindow::on_charge_cur_btn_clicked()
{

}

void MainWindow::enter_age_mode()
{
    QByteArray cmd;
    if(RET_OK == earbud_construc_cmd_enter_age_mode(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::chgbox_enter_com_mode()
{
    QByteArray cmd;
    if(RET_OK == earbud_construc_cmd_chgbox_enter_com_mode(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::get_work_cur()
{
    enter_age_mode();
    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    chgbox_enter_com_mode();
}

void MainWindow::on_work_cur_btn_clicked()
{
    QtConcurrent::run(this, &MainWindow::get_work_cur);
}



void MainWindow::on_sleep_cur_btn_clicked()
{

}

void MainWindow::on_power_off_cur_btn_clicked()
{

}





