#ifndef EARBUD_H
#define EARBUD_H

#include "mew_type_define.h"

#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>

#define EARSIDE_LEFT 0
#define EARSIDE_RIGHT 1

int earbud_initialize_parse_func_list(parse_func_map_t &map);
bool is_notify_from_earbud(const QByteArray &hexdata);
QString earbud_get_notify_key(const QByteArray &hexdata);

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

int earbud_construct_cmd_calibrate_captouch(QByteArray &cmd, uint8_t earside);
int earbud_parse_notify_calibrate_captouch(const QByteArray hexdata, QJsonArray &jsarr);


#endif // EARBUD_H
