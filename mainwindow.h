#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>

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
    void handleResults(QJsonArray jsarr);

    void on_comCheckBtn_clicked();
    void on_clearUpBtn_clicked();
    void on_clearUpBtn2_clicked();
    void on_chgbox_basic_FT_btn_clicked();
    void on_chgbox_wSN_btn_clicked();

    void on_chbox_r_sn_btn_clicked();

    void on_r_mac_btn_clicked();

    void on_r_fw_ver_btn_clicked();

    void on_r_channel_btn_clicked();

    void on_r_ntc_btn_clicked();

    void on_r_active_license_btn_clicked();

    void on_captouch_test_btn_clicked();

    void on_optic_test_btn_clicked();

    void on_force_sensor_test_btn_clicked();

    void on_charge_cur_btn_clicked();

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

    void read_mac_addr();
    void read_fw_ver_addr();
    void read_channel();
    void read_temperature();

    void active_license_key();

    void captouch_start_interrupt();
    void captouch_get_interrupt_result();
    void captouch_read_version();
    void captouch_start_calib();
    void captouch_get_calib_result();
    void captouch_read_value();
    void captouch_test();

    void optic_communicate();
    void optic_int_start();
    void optic_int_end();
    void optic_laser_start();
    void optic_laser_end();
    void optic_full_scale();
    void optic_bg_noise_start();
    void optic_bg_noise_end();
    void optic_test();

    void force_start_detect();
    void force_get_fw_ver();
    void force_get_assemble();
    void force_get_noise_peak();
    void force_get_burst_pressure();
    void force_get_semph();
    void force_test();


private:
    Ui::MainWindow *ui;
    QThread comWorkerThread;
    QextSerialPort *myCom;
    bool s;

};
#endif // MAINWINDOW_H
