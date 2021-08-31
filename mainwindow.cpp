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

#define CHGBOX_CMD_ENTER_1WIRE_COMMU_MODE    "开始1-Wire通信"
#define ONEWIRE_CMD_READ_MAC    "读MAC地址"
#define ONEWIRE_CMD_READ_VER    "读耳机版本"
#define ONEWIRE_CMD_READ_CHANNEL     "读声道"
#define ONEWIRE_CMD_READ_NTC    "读NTC温度"
#define ONEWIRE_CMD_READ_BAT_POWER    "读电量"
#define ONEWIRE_CMD_ACTIV_LIC       "授权码激活"
#define ONEWIRE_CMD_CAPTOUCH_SENSOR     "容触测试"
#define ONEWIRE_CMD_OPTIC_SENSOR    "光感测试"
#define ONEWIRE_CMD_FORCE_SENSOR    "压感测试"

#define CHGBOX_CMD_ENTER_RACE_COMMU_MODE  "开始RACE通信"
#define ONEWIRE_CMD_SET_RACE_BAUTE_RATE "设置RACE波特率"
#define RACE_CMD_READ_GSENSOR  "读GSensor"
#define RACE_CMD_SET_BT_VISIBLE  "蓝牙可见"
#define RACE_CMD_SELECT_PCB_MIC "选择主板MIC"
#define RACE_CMD_SELECT_FF_MIC "选择FF MIC"
#define RACE_CMD_SELECT_FB_MIC "选择FB MIC"
#define RACE_CMD_ENTER_DUT  "进入DUT"
#define RACE_CMD_EXIT_DUT   "退出DUT"
#define RACE_CMD_POWEROFF   "关机"
#define RACE_CMD_STANDBY    "待机"
#define RACE_CMD_RESTART    "重启"
#define RACE_CMD_READ_CHG_CUR  "读充电电流"
#define RACE_CMD_READ_WORK_CUR "读工作电流"
#define RACE_CMD_READ_BG_CUR   "读底电流"
#define RACE_CMD_READ_POFF_CUR "读关机电流"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    myCom = NULL;
    init_1wire_tbl();
    init_race_tbl();
    initCom();
    startThread();
    setRelatedWidgetsStatus(false);
    ui->hexRecvRadioBtn->setChecked(true);
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

int MainWindow::initialize_cmd_func_map(cmd_func_map_t &map, const cmd_func_list_t list)
{
    for(cmd_func_list_t::const_iterator it = list.begin(); it != list.end(); ++it) {
        if(map.find(it->first) != map.end()) {
            QString str;
            str.sprintf("[%s] duplicate map key: %s", __FUNCTION__, it->first.toStdString().data());
            qWarning() << str;
            //return -1;
        }
        map.insert(it->first, it->second);
    }

    return 0;
}

void MainWindow::init_table_widget(QTableWidget *tbl, const cmd_func_list_t func_list, int col_cnt)
{
    //tbl->setWindowTitle(tr("1-Wire指令"));
    //tbl->setWindowFlags(Qt::WindowTitleHint);
    tbl->setFrameShape(QFrame::NoFrame);

    tbl->horizontalHeader()->setVisible(false);
    tbl->verticalHeader()->setVisible(false);
    tbl->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    tbl->setSelectionMode(QAbstractItemView::SingleSelection);
    tbl->setSelectionBehavior(QAbstractItemView::SelectItems);
    tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);

    int row_cnt =  func_list.size() / col_cnt;
    if(func_list.size() % col_cnt != 0) {
        row_cnt += 1;
    }
    tbl->setRowCount(row_cnt);
    tbl->setColumnCount(col_cnt);
    //tbl->setBackgroundRole(QPalette::Base);

    int i = 0;
    for(cmd_func_list_t::const_iterator it = func_list.begin(); it != func_list.end(); ++it) {
        QTableWidgetItem *newItem = new QTableWidgetItem(it->first);
        newItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        newItem->setBackgroundColor(Qt::black);
        QFont font = newItem->font();
        font.setBold(true);
        newItem->setFont(font);
        newItem->setTextColor(Qt::white);
        tbl->setItem(i / tbl->columnCount(), i % tbl->columnCount(), newItem);
        ++i;
    }

    tbl->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tbl->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    initialize_cmd_func_map(cmd_func_map, func_list);
}

void MainWindow::init_1wire_tbl()
{
    cmd_func_list_t cmd_onewire_func_list = {
        { CHGBOX_CMD_ENTER_1WIRE_COMMU_MODE, std::bind(&MainWindow::cmd_enter_1wire_com_mode, this) },
        { ONEWIRE_CMD_READ_MAC, std::bind(&MainWindow::cmd_read_mac_addr, this) },
        { ONEWIRE_CMD_READ_VER, std::bind(&MainWindow::cmd_read_fw_ver_addr, this) },
        { ONEWIRE_CMD_READ_CHANNEL, std::bind(&MainWindow::cmd_read_channel, this) },
        { ONEWIRE_CMD_READ_NTC, std::bind(&MainWindow::cmd_read_ntc, this) },
        { ONEWIRE_CMD_READ_BAT_POWER, std::bind(&MainWindow::cmd_read_bat_power, this) },
        { ONEWIRE_CMD_ACTIV_LIC, std::bind(&MainWindow::cmd_list_active_license_key, this) },
        { ONEWIRE_CMD_CAPTOUCH_SENSOR, std::bind(&MainWindow::cmd_list_captouch, this) },
        { ONEWIRE_CMD_OPTIC_SENSOR, std::bind(&MainWindow::cmd_list_optic, this) },
        { ONEWIRE_CMD_FORCE_SENSOR, std::bind(&MainWindow::cmd_list_force, this) },
    };
    init_table_widget(ui->onewire_tbl, cmd_onewire_func_list, 4);
    ui->onewire_tbl->item(0, 0)->setBackgroundColor(Qt::darkGreen);
}

void MainWindow::init_race_tbl()
{
    cmd_func_list_t cmd_race_func_list = {
        { CHGBOX_CMD_ENTER_RACE_COMMU_MODE, std::bind(&MainWindow::cmd_list_enter_race_com_mode, this) },
        //{ CMD_SET_RACE_BAUTE_RATE, std::bind(&MainWindow::cmd_set_race_baud_rate, this) },
        { RACE_CMD_READ_GSENSOR, std::bind(&MainWindow::cmd_read_gsensor, this) },
        { RACE_CMD_SET_BT_VISIBLE, std::bind(&MainWindow::cmd_set_bt_visible, this) },
        { RACE_CMD_SELECT_PCB_MIC, std::bind(&MainWindow::cmd_select_pcb_mic, this) },
        { RACE_CMD_SELECT_FF_MIC, std::bind(&MainWindow::cmd_select_ff_mic, this) },
        { RACE_CMD_SELECT_FB_MIC, std::bind(&MainWindow::cmd_select_fb_mic, this) },
        { RACE_CMD_ENTER_DUT, std::bind(&MainWindow::cmd_earbud_enter_dut, this) },
        { RACE_CMD_EXIT_DUT, std::bind(&MainWindow::cmd_earbud_exit_dut, this) },
        { RACE_CMD_POWEROFF, std::bind(&MainWindow::cmd_earbud_power_off, this) },
        { RACE_CMD_STANDBY, std::bind(&MainWindow::cmd_enter_standby, this) },
        { RACE_CMD_RESTART, std::bind(&MainWindow::cmd_earbud_restart, this) },
        { RACE_CMD_READ_CHG_CUR, std::bind(&MainWindow::cmd_read_chg_cur, this) },
        { RACE_CMD_READ_WORK_CUR, std::bind(&MainWindow::cmd_read_work_cur, this) },
        { RACE_CMD_READ_BG_CUR, std::bind(&MainWindow::cmd_read_bg_cur, this) },
        { RACE_CMD_READ_POFF_CUR, std::bind(&MainWindow::cmd_read_poff_cur, this) },
    };
    init_table_widget(ui->race_tbl, cmd_race_func_list, 4);
    ui->race_tbl->item(0, 0)->setBackgroundColor(Qt::darkGreen);
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

int MainWindow::closeCom()
{
    myCom->close();
    delete myCom;
    myCom = NULL;
    setRelatedWidgetsStatus(false);

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
        ui->rawDataBrowser->append("");
        ui->rawDataBrowser->append(content);
        //ui->textBrowser->setTextColor(Qt::lightGray);
    }
    return 0;
}

int MainWindow::sendAsciiMsg(QString msg)
{
    Q_UNUSED(msg);
    // TODO:
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

    //static QMutex mutex;
    //QMutexLocker locker(&mutex);
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
    ui->rawDataBrowser->moveCursor(QTextCursor::End);
    emit dataReceived(hexdata);
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
    static QMutex mutex;
    QMutexLocker locker(&mutex);
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
                    //QColor cl = ui->parsedDataBrowser->textColor();
                    ui->parsedDataBrowser->setTextColor(Qt::red);
                    ui->parsedDataBrowser->append(k + " : " +  v_str);
                    //ui->parsedDataBrowser->setTextColor(cl);
                }
                else {
                    ui->parsedDataBrowser->append(k + " : " + v_str );
                }
            }
        }
    }
    ui->parsedDataBrowser->moveCursor(QTextCursor::End);
    ui->parsedDataBrowser->setTextColor(Qt::black);
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
    ui->chgbox_SN_edit->clear();
    ui->chgbox_SN_edit->setFocus();

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

void MainWindow::on_onewire_tbl_cellClicked(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    QTableWidgetItem *item = ui->onewire_tbl->currentItem();
    if(item == nullptr) {
        qDebug() << "NULL item";
        return;
    }
    qDebug("race table, content:%s", item->text().toStdString().c_str());

    exec_cmd_func(item->text());
}

void MainWindow::on_race_tbl_cellClicked(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    QTableWidgetItem *item = ui->race_tbl->currentItem();
    if(item == nullptr) {
        qDebug() << "NULL item";
        return;
    }
    //qDebug("race table, r=%d, c=%d, content:%s", row, column, item->text().toStdString().c_str());
    qDebug("race table, content:%s", item->text().toStdString().c_str());

    exec_cmd_func(item->text());
}

void MainWindow::exec_cmd_func(QString k)
{
    cmd_func_map_t::iterator it = cmd_func_map.find(k);
    if(it != cmd_func_map.end()) {
        if(k == ONEWIRE_CMD_ACTIV_LIC
                || k == ONEWIRE_CMD_CAPTOUCH_SENSOR
                || k == ONEWIRE_CMD_OPTIC_SENSOR
                || k == ONEWIRE_CMD_FORCE_SENSOR
                || k == CHGBOX_CMD_ENTER_RACE_COMMU_MODE
        ) {
            qDebug() << "create a new thread to exec command list";
            QtConcurrent::run(it.value());
        }
        else {
            it.value()();
        }
    }
}

void MainWindow::cmd_enter_1wire_com_mode()
{
    QByteArray cmd;
    if(RET_OK == make_chgbox_cmd_enter_1wire_commu_mode(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_enter_race_com_mode()
{
    QByteArray cmd;
    if(RET_OK == make_chgbox_cmd_enter_race_commu_mode(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_set_race_baud_rate()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_set_baud_rate(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_list_enter_race_com_mode()
{
    cmd_enter_race_com_mode();

    QThread::msleep(2000);
    cmd_set_race_baud_rate();
}

void MainWindow::cmd_read_mac_addr()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_read_mac(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_read_fw_ver_addr()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_read_version(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_read_channel()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_read_channel(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_read_ntc()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_read_ntc(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_read_bat_power()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_read_bat_power(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_list_active_license_key()
{
#ifdef DUMP_THREAD_ID
    qDebug() << "MainWindow:" << __FUNCTION__ << ", thread_id:" << QThread::currentThreadId();
#endif
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_get_lic_key(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    cmd.clear();
    if(RET_OK == make_1wire_cmd_set_lic_key(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    cmd.clear();
    if(RET_OK == make_1wire_cmd_get_lic_result(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_captouch_start_interrupt()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_captouch_start_int(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_captouch_get_interrupt_result()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_captouch_get_int_result(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_captouch_read_version()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_captouch_read_version(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_captouch_start_calib()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_captouch_start_calib(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_captouch_get_calib_result()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_captouch_get_calib_result(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_captouch_read_value()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_captouch_read_value(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_list_captouch()
{
    // 不要放在金属上面，校准的时候不要触摸
    cmd_captouch_start_calib();
    QThread::msleep(3000);

    cmd_captouch_get_calib_result();
    QThread::msleep(3000);

    // 靠近到目标距离固定不动，延时3S后读diff
    /*
        判断第13字节为0，则后 5个unsigned short 有效
        分别是 diff1，diff2，cap1，cap2，capref
        diff1：按键diff值, 0x0001, pass范围需要取样100
        diff2：入耳diff值, 0x0002, pass范围需要取样100
        cap1：按键容值, 0x099C, pass范围需要取样100
        cap2：入耳容值, 0x0A14, pass范围需要取样100
        capref：参考容值, 0x11E4, pass范围需要取样100

        此时不要按压，读初始容值，读3次所有参数并取平均，每次间隔1秒，diff1/diff2要等于0，cap1/cap2/capref要大于0（具体数值待取30pcs确认）
    */
    for(int i = 0; i < 3; ++i) {
        cmd_captouch_read_value();
        QThread::msleep(1000);
    }

    cmd_captouch_read_version();
    QThread::msleep(1000);

    // 00成功，01失败，03测试中，1s内读到03, 2s后读到00
    cmd_captouch_start_interrupt();
    QThread::msleep(2000);

    cmd_captouch_get_interrupt_result();
    QThread::msleep(3000);

    // 延时3秒，用于给测试人员按压并保持按压稳定
    // 靠近到目标距离固定不动，延时3S后读diff
    /*
        判断第13字节为0，则后 5个unsigned short 有效
        分别是 diff1，diff2，cap1，cap2，capref
        diff1：按键diff值, 0x0001, pass范围需要取样100
        diff2：入耳diff值, 0x0002, pass范围需要取样100
        cap1：按键容值, 0x099C, pass范围需要取样100
        cap2：入耳容值, 0x0A14, pass范围需要取样100
        capref：参考容值, 0x11E4, pass范围需要取样100

        此时按键和入耳都处于按压状态，读初始容值，读3次所有参数并取平均，每次间隔1秒，diff1/diff2要大于0，cap1/cap2/capref也要大于0（具体数值待取30pcs确认）
    */
    for(int i = 0; i < 3; ++i) {
        cmd_captouch_read_value();
        QThread::msleep(1000);
    }

}

void MainWindow::cmd_optic_communicate()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_optic_communicate(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_optic_int_start()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_optic_int_start(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_optic_int_end()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_optic_int_end(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_optic_laser_start()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_optic_laser_start(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_optic_laser_end()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_optic_laser_end(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_optic_full_scale_start()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_optic_full_scale_start(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_optic_full_scale_end()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_optic_full_scale_end(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_optic_bg_noise_start()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_optic_bg_noise_start(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_optic_bg_noise_end()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_optic_bg_noise_end(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_list_optic()
{
    cmd_optic_communicate();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    cmd_optic_int_start();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    cmd_optic_int_end();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    cmd_optic_laser_start();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    cmd_optic_laser_end();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    cmd_optic_full_scale_start();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    cmd_optic_full_scale_end();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    cmd_optic_bg_noise_start();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    cmd_optic_bg_noise_end();
}

void MainWindow::cmd_force_start_detect()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_force_start_detect(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_force_get_fw_ver()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_force_get_fw_ver(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_force_get_assemble()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_force_get_assemble(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_force_get_noise_peak()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_force_get_noise_peak(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_force_get_burst_pressure()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_force_get_burst_pressure(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_force_get_semph()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_force_get_semph(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_list_force()
{
    cmd_force_start_detect();

    QThread::msleep(7000);
    cmd_force_get_fw_ver();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    cmd_force_get_assemble();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    cmd_force_get_noise_peak();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    cmd_force_get_burst_pressure();

    QThread::msleep(TIMER_INTERVAL_SEND_COMMAND);
    cmd_force_get_semph();
}

void MainWindow::cmd_read_gsensor()
{
    QByteArray cmd;
    if(RET_OK == make_race_cmd_read_gsensor(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_set_bt_visible()
{
    QByteArray cmd;
    if(RET_OK == make_race_cmd_set_bt_visible(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_select_pcb_mic()
{
    QByteArray cmd;
    if(RET_OK == make_race_cmd_select_pcb_mic(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_select_ff_mic()
{
    QByteArray cmd;
    if(RET_OK == make_race_cmd_select_ff_mic(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_select_fb_mic()
{
    QByteArray cmd;
    if(RET_OK == make_race_cmd_select_fb_mic(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_earbud_enter_dut()
{
    QByteArray cmd;
    if(RET_OK == make_race_cmd_enter_dut(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_earbud_exit_dut()
{
    QByteArray cmd;
    if(RET_OK == make_race_cmd_enter_dut(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_earbud_power_off()
{
    QByteArray cmd;
    if(RET_OK == make_race_cmd_power_off(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_enter_standby()
{
    QByteArray cmd;
    if(RET_OK == make_race_cmd_enter_standby(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_earbud_restart()
{
    QByteArray cmd;
    if(RET_OK == make_race_cmd_restart(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}

void MainWindow::cmd_read_chg_cur()
{

}

void MainWindow::cmd_read_work_cur()
{

}

void MainWindow::cmd_read_bg_cur()
{

}

void MainWindow::cmd_read_poff_cur()
{

}


void MainWindow::cmd_enter_age_mode()
{
    QByteArray cmd;
    if(RET_OK == make_1wire_cmd_enter_age_mode(cmd, ui->earside_left_rbtn->isChecked() ? EARSIDE_LEFT : EARSIDE_RIGHT)) {
        sendHexMsg(cmd);
    }
}




