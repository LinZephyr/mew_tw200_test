#ifndef MEW_CHGBOX_H
#define MEW_CHGBOX_H

#include "mew_type_define.h"
#include <QByteArray>
#include <QJsonObject>

#define CHGBOX_FT_CMD_LEAD          0XAA
#define CHGBOX_FT_RSP_LEAD         0XAA

#define CHGBOX_BASIC_FT_CMD_FC           0XF1
#define CHGBOX_BASIC_FT_CMD_LEN          3
#define CHGBOX_BASIC_FT_RSP_FC           0X1F
#define CHGBOX_BASIC_FT_RSP_LEN          11

#define CHGBOX_FT_W_SN_CMD_FC          0XF2
#define CHGBOX_FT_W_SN_CMD_LEN         13
#define CHGBOX_FT_W_SN_RSP_FC          0X2F
#define CHGBOX_FT_W_SN_RSP_LEN         13

#define CHGBOX_FT_SN_LEN  10

#define CHGBOX_FT_R_SN_CMD_FC  0xF3
#define CHGBOX_FT_R_SN_CMD_LEN 2
#define CHGBOX_FT_R_SN_RSP_FC  0x3F
#define CHGBOX_FT_R_SN_RSP_LEN 13

int chgbox_initialize_parse_func_list(parse_func_map_t &map);

QByteArray construct_chgbox_basic_ft_cmd(bool led1, bool led2, bool led3, bool chg_mode);
int parse_chgbox_basic_ft_rsp(const QByteArray hexdata, QJsonArray &jsarr);

#define CHGBOX_SN_FLAG_W  0
#define CHGBOX_SN_FLAG_R  1
int chgbox_ft_get_sn(const QByteArray &hexdata, QJsonArray &jsarr, bool rw_flag);

int construct_chgbox_ft_w_sn_cmd(const QByteArray SN, QByteArray &hexcmd);
int parse_chgbox_ft_w_sn_rsp(const QByteArray hexdata, QJsonArray &jsarr);

int construct_chgbox_ft_r_sn_cmd(QByteArray &hexcmd);
int parse_chgbox_ft_r_sn_rsp(const QByteArray hexdata, QJsonArray &jsarr);



#endif // end of MEW_CHGBOX_H
