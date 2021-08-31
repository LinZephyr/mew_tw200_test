#ifndef EARBUD_COMMU_MODE_H
#define EARBUD_COMMU_MODE_H

#include "mew_type_define.h"

#include <QString>
#include <QByteArray>
#include <QJsonArray>

parse_func_list_t* get_commu_mode_parse_func_list();
bool is_chgbox_commu_mode_reply(const QByteArray &hexdata);
QString get_chgbox_commu_mode_reply_key(const QByteArray &hexdata);

int make_chgbox_cmd_enter_race_commu_mode(QByteArray &cmd, uint8_t earside);
int make_chgbox_cmd_enter_1wire_commu_mode(QByteArray &cmd, uint8_t earside);

int parse_chgbox_reply_set_commu_mode(const QByteArray hexdata, QJsonArray &jsarr);

#endif // EARBUD_COMMU_MODE_H
