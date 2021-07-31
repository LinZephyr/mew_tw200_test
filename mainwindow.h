#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QJsonObject>

#include "qextserial/qextserialport.h"

//延时，TIME_OUT是串口读写的延时
#define TIME_OUT 20

//连续发送定时器计时间隔,200ms
#define OBO_TIMER_INTERVAL 200

//载入文件时，如果文件大小超过TIP_FILE_SIZE字节则提示文件过大是否继续打开
#define TIP_FILE_SIZE 5000
//载入文件最大长度限制在MAX_FILE_SIZE字节内
#define MAX_FILE_SIZE 10000


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void startThread();

private slots:
    void on_openCloseBtn_clicked();
    void on_sendmsgBtn_clicked();

    void recv_com_data();
    void handleResults(QString str);

    void on_comCheckBtn_clicked();
    void on_clearUpBtn_clicked();
    void on_chgbox_basic_FT_btn_clicked();
    void on_chgbox_wSN_btn_clicked();

    void on_chbox_r_sn_btn_clicked();

signals:
    void dataReceived(QByteArray hexdata);

protected:
    void checkComPort();
    void initCom();
    int openCom();
    int closeCom();
    int sendHexMsg(QByteArray hexdata);
    int sendAsciiMsg(QString msg);


    void setRelatedWidgetsStatus(bool opened);
    void setComWidgetsStatus(bool opened);

private:
    Ui::MainWindow *ui;
    QThread comWorkerThread;
    QextSerialPort *myCom;
    bool s;

};
#endif // MAINWINDOW_H
