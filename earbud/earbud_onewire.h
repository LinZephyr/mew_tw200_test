#ifndef EARBUD_ONEWIRE_H
#define EARBUD_ONEWIRE_H

#include "mew_type_define.h"

#include <QString>
#include <QByteArray>
#include <QJsonArray>


parse_func_list_t* get_1wire_parse_func_list();
bool is_1wire_reply(const QByteArray &hexdata);
QString get_1wire_reply_key(const QByteArray &hexdata);

int make_1wire_cmd_read_mac(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_read_mac(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_read_version(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_read_version(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_read_channel(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_read_channel(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_read_ntc(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_read_ntc(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_set_lic_key(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_set_lic_key(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_get_lic_result(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_get_lic_result(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_get_lic_key(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_get_lic_key(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_captouch_start_int(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_captouch_start_int(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_captouch_get_int_result(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_captouch_get_int_result(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_captouch_read_version(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_captouch_read_version(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_captouch_start_calib(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_captouch_start_calib(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_captouch_get_calib_result(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_captouch_get_calib_result(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_captouch_read_value(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_captouch_read_value(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_optic_communicate(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_optic_communicate(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_optic_int_start(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_optic_int_start(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_optic_int_end(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_optic_int_end(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_optic_laser_start(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_optic_laser_start(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_optic_laser_end(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_optic_laser_end(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_optic_full_scale_start(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_optic_full_scale_start(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_optic_full_scale_end(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_optic_full_scale_end(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_optic_bg_noise_start(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_optic_bg_noise_start(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_optic_bg_noise_end(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_optic_bg_noise_end(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_optic_start_calib_bg_noise(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_optic_start_calib_bg_noise(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_optic_end_calib_bg_noise(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_optic_end_calib_bg_noise(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_optic_12mm(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_optic_calib_12mm(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_optic_3mm(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_optic_calib_3mm(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_force_start_detect(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_force_start_detect(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_force_get_fw_ver(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_force_get_fw_ver(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_force_get_assemble(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_force_get_assemble(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_force_get_noise_peak(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_force_get_noise_peak(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_force_get_burst_pressure(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_force_get_burst_pressure(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_force_get_semph(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_force_get_semph(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_enter_age_mode(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_enter_age_mode(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_set_baud_rate(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_set_baud_rate(const QByteArray hexdata, QJsonArray &jsarr);

int make_1wire_cmd_read_bat_power(QByteArray &cmd, uint8_t earside);
int parse_1wire_reply_read_bat_power(const QByteArray hexdata, QJsonArray &jsarr);

#endif // EARBUD_ONEWIRE_H
