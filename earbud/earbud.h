#ifndef EARBUD_H
#define EARBUD_H

#include "mew_type_define.h"

#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>

#define EARSIDE_LEFT 0
#define EARSIDE_RIGHT 1

/*
 *    耳机1-wire通信区分左右. 1-WIRE通信的前提: 盒子退出通信模式.
 *    耳机RACE通信不区分左右. RACE通信的前提: 盒子进入通信模式, 再设置耳机RACE波特率.
 *    盒子开机后默认进入通信模式.
 */

int earbud_initialize_parse_func_list(parse_func_map_t &map);
bool is_notify_from_earbud(const QByteArray &hexdata);
QString earbud_get_notify_key(const QByteArray &hexdata);

bool is_rsp_from_earbud(const QByteArray &hexdata);
QString earbud_get_rsp_key(const QByteArray &hexdata);

bool is_dut_rsp_from_earbud(const QByteArray &hexdata);
QString earbud_get_dut_rsp_key(const QByteArray &hexdata);

bool is_notify_from_earbud_chgbox_com_mode(const QByteArray &hexdata);
QString earbud_chgbox_com_mode_get_notify_key(const QByteArray &hexdata);

int earbud_construct_cmd_read_mac(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_read_mac(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construct_cmd_read_version(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_read_version(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construct_cmd_read_channel(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_read_channel(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construct_cmd_read_temperature(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_read_temperature(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construct_cmd_set_license_key(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_set_license_key(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construct_cmd_get_license_result(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_get_license_result(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construct_cmd_get_license_key(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_get_license_key(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construct_cmd_captouch_start_int(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_captouch_start_int(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construct_cmd_captouch_get_int_result(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_captouch_get_int_result(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construct_cmd_captouch_read_version(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_captouch_read_version(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construct_cmd_captouch_start_calib(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_captouch_start_calib(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construct_cmd_captouch_get_calib_result(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_captouch_get_calib_result(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construct_cmd_captouch_read_value(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_captouch_read_value(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_optic_communicate(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_optic_communicate(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_optic_int_start(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_optic_int_start(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_optic_int_end(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_optic_int_end(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_optic_laser_start(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_optic_laser_start(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_optic_laser_end(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_optic_laser_end(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_optic_full_scale(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_optic_full_scale(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_optic_bg_noise_start(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_optic_bg_noise_start(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_optic_bg_noise_end(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_optic_bg_noise_end(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_force_start_detect(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_force_start_detect(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_force_get_fw_ver(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_force_get_fw_ver(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_force_get_assemble(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_force_get_assemble(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_force_get_noise_peak(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_force_get_noise_peak(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_force_get_burst_pressure(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_force_get_burst_pressure(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_force_get_semph(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_force_get_semph(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_enter_age_mode(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_enter_age_mode(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_chgbox_enter_com_mode(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_chgbox_enter_com_mode(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_chgbox_exit_com_mode(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_chgbox_exit_com_mode(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_set_vbus_baud_rate(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_set_vbus_baud_rate(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_enter_standby(QByteArray &cmd, uint8_t earside);
int earbud_construc_cmd_restart(QByteArray &cmd, uint8_t earside);
int earbud_parse_rsp_standby_or_restart(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_power_off(QByteArray &cmd, uint8_t earside);
int earbud_parse_rsp_power_off(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_enter_dut(QByteArray &cmd, uint8_t earside);
int earbud_construc_cmd_exit_dut(QByteArray &cmd, uint8_t earside);
int earbud_parse_rsp_enter_exit_dut(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_read_gsensor(QByteArray &cmd, uint8_t earside);
int earbud_parse_rsp_read_gsensor(const QByteArray hexdata, QJsonArray &jsarr);

int earbud_construc_cmd_read_bat_power(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_read_bat_power(const QByteArray hexdata, QJsonArray &jsarr);




#endif // EARBUD_H
