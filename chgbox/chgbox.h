#ifndef MEW_CHGBOX_H
#define MEW_CHGBOX_H

#include "mew_type_define.h"
#include <QByteArray>
#include <QJsonObject>

#define CHGBOX_FT_SN_LEN  9

bool is_rsp_from_chgbox(const QByteArray &hexdata);
QString chgbox_get_rsp_key(const QByteArray &hexrsp);

int init_chgbox_parse_func_map(parse_func_map_t &map);

QByteArray construct_chgbox_basic_ft_cmd(bool led1, bool led2, bool led3, bool chg_mode);
int parse_chgbox_basic_ft_rsp(const QByteArray hexdata, QJsonArray &jsarr);

int construct_chgbox_ft_w_sn_cmd(const QString snStr, QByteArray &hexcmd);
int parse_chgbox_ft_w_sn_rsp(const QByteArray hexdata, QJsonArray &jsarr);

int construct_chgbox_ft_r_sn_cmd(QByteArray &hexcmd);
int parse_chgbox_ft_r_sn_rsp(const QByteArray hexdata, QJsonArray &jsarr);



#endif // end of MEW_CHGBOX_H
