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

    void on_work_cur_btn_clicked();

    void on_sleep_cur_btn_clicked();

    void on_power_off_cur_btn_clicked();

    void on_chgbox_enter_com_mode_clicked();

    void on_chgbox_exit_com_mode_2_clicked();

    void on_eb_enter_dut_btn_clicked();

    void on_eb_exit_dut_btn_clicked();

    void on_eb_restart_btn_clicked();

    void on_bat_data_btn_clicked();

    void on_gsensor_btn_clicked();

    void on_eb_standby_btn_clicked();

    void on_eb_poweroff_btn_clicked();

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

    void cmd_read_mac_addr();
    void cmd_read_fw_ver_addr();
    void cmd_read_channel();
    void cmd_read_temperature();
    void cmd_active_license_key();

    void cmd_captouch_start_interrupt();
    void cmd_captouch_get_interrupt_result();
    void cmd_captouch_read_version();
    void cmd_captouch_start_calib();
    void cmd_captouch_get_calib_result();
    void cmd_captouch_read_value();
    void cmd_list_captouch();

    void cmd_optic_communicate();
    void cmd_optic_int_start();
    void cmd_optic_int_end();
    void cmd_optic_laser_start();
    void cmd_optic_laser_end();
    void cmd_optic_full_scale_start();
    void cmd_optic_full_scale_end();
    void cmd_optic_bg_noise_start();
    void cmd_optic_bg_noise_end();
    void cmd_list_optic();

    void cmd_force_start_detect();
    void cmd_force_get_fw_ver();
    void cmd_force_get_assemble();
    void cmd_force_get_noise_peak();
    void cmd_force_get_burst_pressure();
    void cmd_force_get_semph();
    void cmd_list_force();

    void cmd_enter_age_mode();
    void cmd_chgbox_enter_com_mode();
    void cmd_chgbox_exit_com_mode();
    void cmd_set_vbus_baud_rate();
    void cmd_list_chbox_enter_com_mode();
    void cmd_list_get_work_cur();

    void cmd_enter_standby();
    void cmd_list_enter_standby();

    void cmd_earbud_power_off();
    void cmd_list_power_off();

    void cmd_earbud_restart();
    void cmd_list_earbud_restart();

    void cmd_earbud_enter_dut();
    void cmd_list_earbud_enter_dut();

    void cmd_earbud_exit_dut();
    void cmd_list_earbud_exit_dut();

    void cmd_read_gsensor();
    void cmd_list_read_gsensor();

    void cmd_read_bat_power();

private:
    Ui::MainWindow *ui;
    QThread comWorkerThread;
    QextSerialPort *myCom;
    bool s;

};
#endif // MAINWINDOW_H
