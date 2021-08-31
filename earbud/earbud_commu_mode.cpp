#include "earbud_commu_mode.h"
#include "mew_type_define.h"
#include "utils/utils.h"

#define COMMU_MODE_MSG_LEN    9

parse_func_list_t* get_commu_mode_parse_func_list()
{
    static parse_func_list_t earbud_commu_mode_parse_func_list = {
        {"FE0F", parse_chgbox_reply_set_commu_mode },
    };

    return &earbud_commu_mode_parse_func_list;
}

bool is_chgbox_commu_mode_reply(const QByteArray &hexdata)
{
    if( COMMU_MODE_MSG_LEN <= hexdata.count() && 0xFE == (uint8_t)hexdata[0] && 0x0F == (uint8_t)hexdata[1] ) {
        return true;
    }
    return false;
}

QString get_chgbox_commu_mode_reply_key(const QByteArray &hexdata)
{
    QString key;
    if(COMMU_MODE_MSG_LEN <= hexdata.count() ) {
        key.sprintf("%02X%02X", (uint8_t)hexdata[0], (uint8_t)hexdata[1]);
    }

    return key;
}

int make_chgbox_cmd_enter_race_commu_mode(QByteArray &cmd, uint8_t earside)
{
    Q_UNUSED(earside);
    QString str = "FE F0 01 00 00 00 00 00 00";
    if(RET_FAIL == string2HexArray(str, cmd) ) {
        return RET_FAIL;
    }

    return RET_OK;
}

int make_chgbox_cmd_enter_1wire_commu_mode(QByteArray &cmd, uint8_t earside)
{
    Q_UNUSED(earside);
    QString str = "FE F0 00 00 00 00 00 00 00";
    if(RET_FAIL == string2HexArray(str, cmd) ) {
        return RET_FAIL;
    }

    return RET_OK;
}

int parse_chgbox_reply_set_commu_mode(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "设置通信模式";
    if(COMMU_MODE_MSG_LEN > hexdata.count() ) {
        jsarr.append(topic);
        addInfo2Array(jsarr, KEY_STR_EXCEPTION, VALUE_STR_LENGTH_NOT_ENOUGH, true);
        return RET_FAIL;
    }

    QJsonObject jsobj;
    uint8_t mode = (uint8_t)hexdata[4];
    QString mstr;
    enum {
      CHGBOX_COMMU_MODE_1WIRE = 0,
      CHGBOX_COMMU_MODE_RACE = 1,
    };
    if(CHGBOX_COMMU_MODE_1WIRE == mode) {
        mstr = "1-Wire";
    }
    else if(CHGBOX_COMMU_MODE_RACE == mode) {
        mstr = "RACE";
    }
    else {
        mstr = "未定义";
    }
    jsobj.insert(topic, mstr);
    jsarr.append(jsobj);

    return RET_OK;
}


