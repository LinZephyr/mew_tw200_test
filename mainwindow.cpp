#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils/utils.h"
#include "chgbox/chgbox.h"
#include "data_process/worker_thread.h"

#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include <QListView>
#include <QSerialPortInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    myCom = NULL;
    initCom();
    startThread();
    setRelatedWidgetsStatus(false);
    qDebug() << "MainWindow::" << __FUNCTION__ << ", thread_id:" << QThread::currentThreadId();
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
    connect(worker, SIGNAL(resultReady(QString)), this, SLOT(handleResults(QString)) );

    comWorkerThread.start();
}

void MainWindow::handleResults(QString str)
{
    ui->parsedDataBrowser->append(str);
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

    if(opened) {
        ui->sendmsgBtn->setEnabled(true);
        //ui->sendMsgLineEdit->setEnabled(true);
        //ui->sendMsgLineEdit->setFocus();
        //ui->hexSendRadioBtn->setEnabled(true);

        //ui->hexRecvRadioBtn->setEnabled(true);
        //ui->clearUpBtn->setEnabled(true);
        ui->chgbox_basic_FT_btn->setEnabled(true);
        ui->chgbox_wSN_btn->setEnabled(true);
    }
    else {
        ui->sendmsgBtn->setEnabled(false);
        //ui->sendMsgLineEdit->setEnabled(false);
        //ui->sendMsgLineEdit->clearFocus();
        //ui->hexSendRadioBtn->setEnabled(false);
        //ui->hexRecvRadioBtn->setEnabled(false);
        //ui->clearUpBtn->setEnabled(false);
        ui->chgbox_basic_FT_btn->setEnabled(false);
        ui->chgbox_wSN_btn->setEnabled(false);
    }

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
    if(-1 == myCom->write(hexdata) ) {
        QMessageBox::critical(this, tr("警告"), tr("发送数据失败！ "), QMessageBox::Ok);
        return -1;
    }
    else {
        QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        QString timeStrLine= "[" + ts + "]";
        QString tmpstr = "[ " + QString::number(hexdata.count(), 10) + " ]: ";
        QString content = "<span style=\" color:green;\">" + timeStrLine + tmpstr + hexArray2String(hexdata) +"\n\r</span>";
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

    if(!hexdata.isEmpty()){
            //ui->textBrowser->setTextColor(Qt::black);
            if(ui->hexRecvRadioBtn->isChecked()){
                asciidata = hexArray2String(hexdata);
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
}

void MainWindow::on_comCheckBtn_clicked()
{
    checkComPort();
}


void MainWindow::on_clearUpBtn_clicked()
{
    ui->rawDataBrowser->clear();
}


void MainWindow::on_chgbox_basic_FT_btn_clicked()
{
    QByteArray hexCmd;
    hexCmd = construct_chgbox_basic_ft_cmd(ui->led1_on_rbtn->isChecked(), ui->led2_on_rbtn->isChecked(), ui->led3_on_rbtn->isChecked(), ui->chg_mode_wireless_rbtn->isChecked());
    sendHexMsg(hexCmd);
}

void MainWindow::on_chgbox_wSN_btn_clicked()
{
    QString strSN = ui->chgbox_SN_edit->text();

    //如果发送数据为空，给出提示并返回
    if(strSN.isEmpty()){
        //QMessageBox::information(this, tr("提示消息"), tr("没有需要发送的数据"), QMessageBox::Ok);
        return;
    }

    QByteArray hexSN;
    if(RET_OK != string2HexArray(strSN, hexSN)) {
        QMessageBox::information(this, tr("提示消息"), tr("输入的数据格式有错误！"), QMessageBox::Ok);
        return;
    }

    if( CHGBOX_FT_SN_LEN > hexSN.count()) {
        QMessageBox::information(this, tr("提示消息"), tr("SN数据长度不够！"), QMessageBox::Ok);
        return;
    }

    QByteArray hexcmd;
    if (RET_OK == construct_chgbox_ft_w_sn_cmd(hexSN, hexcmd) ) {
        sendHexMsg(hexcmd);
    }

}


void MainWindow::on_chbox_r_sn_btn_clicked()
{
    QByteArray hexcmd;
    if (RET_OK == construct_chgbox_ft_r_sn_cmd(hexcmd) ) {
        sendHexMsg(hexcmd);
    }
}

