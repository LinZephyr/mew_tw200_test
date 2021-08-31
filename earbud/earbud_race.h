#ifndef EARBUD_RACE_H
#define EARBUD_RACE_H

#include "mew_type_define.h"

#include <QString>
#include <QByteArray>
#include <QJsonArray>

parse_func_list_t* get_race_parse_func_list();
bool is_race_reply(const QByteArray &hexdata);
QString get_race_reply_key(const QByteArray &hexdata);

void make_race_cmd_header(QByteArray &cmd_arr, uint8_t vlen1, uint8_t vlen2, uint8_t rlen1, uint8_t rlen2, uint8_t id1, uint8_t id2, bool has_val, uint8_t val);

int make_race_cmd_read_gsensor(QByteArray &cmd, uint8_t earside);
int parse_race_reply_read_gsensor(const QByteArray hexdata, QJsonArray &jsarr);

int make_race_cmd_set_bt_visible(QByteArray &cmd, uint8_t earside);
int parse_race_reply_set_bt_visible(const QByteArray hexdata, QJsonArray &jsarr);

int make_race_cmd_select_pcb_mic(QByteArray &cmd, uint8_t earside);
int make_race_cmd_select_ff_mic(QByteArray &cmd, uint8_t earside);
int make_race_cmd_select_fb_mic(QByteArray &cmd, uint8_t earside);
int parse_race_reply_select_talk_mic(const QByteArray hexdata, QJsonArray &jsarr);

int make_race_cmd_enter_dut(QByteArray &cmd, uint8_t earside);
int make_race_cmd_exit_dut(QByteArray &cmd, uint8_t earside);
int parse_race_reply_enter_exit_dut(const QByteArray hexdata, QJsonArray &jsarr);

int make_race_cmd_power_off(QByteArray &cmd, uint8_t earside);
int parse_race_reply_power_off(const QByteArray hexdata, QJsonArray &jsarr);

int make_race_cmd_enter_standby(QByteArray &cmd, uint8_t earside);
int make_race_cmd_restart(QByteArray &cmd, uint8_t earside);
int parse_race_reply_standby_or_restart(const QByteArray hexdata, QJsonArray &jsarr);

#endif // EARBUD_RACE_H
