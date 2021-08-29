#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QMap>
#include <QTableWidget>

#include "qextserial/qextserialport.h"

//延时，TIME_OUT是串口读写的延时
#define TIME_OUT 20

//连续发送定时器计时间隔,200ms
#define OBO_TIMER_INTERVAL 200

//载入文件时，如果文件大小超过TIP_FILE_SIZE字节则提示文件过大是否继续打开
#define TIP_FILE_SIZE 5000
//载入文件最大长度限制在MAX_FILE_SIZE字节内
#define MAX_FILE_SIZE 10000

typedef QString cmd_func_uuid_t;
typedef std::function<void()> cmd_func_t;
typedef QMap<cmd_func_uuid_t, cmd_func_t> cmd_func_map_t;
typedef std::pair<cmd_func_uuid_t, cmd_func_t> cmd_func_pair_t;
typedef std::initializer_list<cmd_func_pair_t> cmd_func_list_t;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    int initialize_cmd_func_map(cmd_func_map_t &map, const cmd_func_list_t list);
    void init_table_widget(QTableWidget *tbl, const cmd_func_list_t func_list, int col_cnt);
    void init_1wire_tbl();
    void init_race_tbl();
    void checkComPort();
    void initCom();
    int openCom();
    int closeCom();
    void setComWidgetsStatus(bool opened);
    void setRelatedWidgetsStatus(bool opened);

protected:
    void startThread();
    int sendHexMsg(QByteArray hexdata);
    int sendAsciiMsg(QString msg);

signals:
    void dataReceived(QByteArray hexdata);

private slots:
    void recv_com_data();
    void handleResults(QJsonArray jsarr);

    void on_openCloseBtn_clicked();
    void on_sendmsgBtn_clicked();

    void on_comCheckBtn_clicked();
    void on_clearUpBtn_clicked();
    void on_clearUpBtn2_clicked();
    void on_chgbox_basic_FT_btn_clicked();
    void on_chgbox_wSN_btn_clicked();
    void on_chbox_r_sn_btn_clicked();

    void on_onewire_tbl_cellClicked(int row, int column);
    void on_race_tbl_cellClicked(int row, int column);

private:
    void exec_cmd_func(QString k);
private:
    void cmd_enter_1wire_com_mode();
    void cmd_enter_race_com_mode();
    void cmd_set_race_baud_rate();

private:
    void cmd_read_mac_addr();
    void cmd_read_fw_ver_addr();
    void cmd_read_channel();
    void cmd_read_ntc();
    void cmd_read_bat_power();
    void cmd_list_active_license_key();

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

private:
    void cmd_earbud_enter_dut();
    void cmd_earbud_exit_dut();
    void cmd_earbud_power_off();
    void cmd_enter_standby();
    void cmd_earbud_restart();
    void cmd_read_chg_cur();
    void cmd_read_work_cur();
    void cmd_read_bg_cur();
    void cmd_read_poff_cur();
    void cmd_read_gsensor();
    void cmd_set_bt_visible();

    void cmd_enter_age_mode();

private:
    Ui::MainWindow *ui;
    QextSerialPort *myCom;
    QThread comWorkerThread;
    cmd_func_map_t cmd_func_map;
};
#endif // MAINWINDOW_H
