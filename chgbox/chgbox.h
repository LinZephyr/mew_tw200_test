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

QByteArray construct_chgbox_basic_ft_cmd(bool led1, bool led2, bool led3, bool chg_mode);
int parse_chgbox_basic_ft_rsp(const QByteArray hexdata, QJsonObject &jsobj, QString &str);

int construct_chgbox_ft_w_sn_cmd(QByteArray SN, QByteArray &hexcmd);
int parse_chgbox_ft_w_sn_rsp(const QByteArray hexdata, QJsonObject &jsobj, QString &str);

#endif // end of MEW_CHGBOX_H
