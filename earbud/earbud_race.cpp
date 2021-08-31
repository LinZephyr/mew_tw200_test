#include "earbud_race.h"
#include "earbud_internal.h"
#include "mew_type_define.h"
#include "utils/utils.h"

#define RACE_PAYLOAD_LENGTH_POWER_OFF  0
#define RACE_PAYLOAD_LENGTH_READ_GSENSOR  6


#define RACE_CMD_ENTER_STANDBY    0x18
#define RACE_CMD_RESTART          0x19

typedef struct {
    uint8_t chgbox_lead1;
    uint8_t chgbox_lead2;
    uint8_t chgbox_len1;
    uint8_t chgbox_len2;
    uint8_t indicator;
    uint8_t type;
    uint8_t len1;
    uint8_t len2;
    uint8_t id1;
    uint8_t id2;
    uint8_t value;
} race_cmd_header_t;
typedef race_cmd_header_t race_reply_header_t;

const uint8_t RACE_REPLY_HEADER_LEN = sizeof(race_reply_header_t);

#define RACE_REPLY_READ_GSENSOR_ID1 0x01
#define RACE_REPLY_READ_GSENSOR_ID2 0x00

#define RACE_REPLY_SET_BT_VISIBLE_ID1 0X01
#define RACE_REPLY_SET_BT_VISIBLE_ID2 0X11

#define RACE_REPLY_SELECT_TALK_MIC_ID1 0x0C
#define RACE_REPLY_SELECT_TALK_MIC_ID2 0x0E

#define RACE_REPLY_ENTER_EXIT_DUT_ID1 0X01
#define RACE_REPLY_ENTER_EXIT_DUT_ID2 0X0A

#define RACE_REPLY_POWER_OFF_ID1    0x27
#define RACE_REPLY_POWER_OFF_ID2    0x00

#define RACE_REPLY_STB_OR_RESTART_ID1   0x01
#define RACE_REPLY_STB_OR_RESTART_ID2   0x11


QString make_race_key(uint8_t usr_id1, uint8_t usr_id2)
{
    QString str;
    str.sprintf("%02X%02X", usr_id1, usr_id2);
    return str;
}

parse_func_list_t* get_race_parse_func_list()
{
    static parse_func_list_t earbud_race_parse_func_list = {
        {make_race_key(RACE_REPLY_READ_GSENSOR_ID1, RACE_REPLY_READ_GSENSOR_ID2), parse_race_reply_read_gsensor },
        {make_race_key(RACE_REPLY_SET_BT_VISIBLE_ID1, RACE_REPLY_SET_BT_VISIBLE_ID2), parse_race_reply_set_bt_visible },
        {make_race_key(RACE_REPLY_SELECT_TALK_MIC_ID1, RACE_REPLY_SELECT_TALK_MIC_ID2), parse_race_reply_select_talk_mic},
        {make_race_key(RACE_REPLY_ENTER_EXIT_DUT_ID1, RACE_REPLY_ENTER_EXIT_DUT_ID2), parse_race_reply_enter_exit_dut },
        {make_race_key(RACE_REPLY_POWER_OFF_ID1, RACE_REPLY_POWER_OFF_ID2), parse_race_reply_power_off },
        //{make_race_key(RACE_REPLY_STB_OR_RESTART_ID1, RACE_REPLY_STB_OR_RESTART_ID2), parse_race_reply_standby_or_restart },  // id duplicat with "BT VISIBLE" reply.
    };

    return &earbud_race_parse_func_list;
}

bool is_race_reply(const QByteArray &hexdata)
{
    if((unsigned int)hexdata.count() >= RACE_REPLY_HEADER_LEN - 1) {
        race_reply_header_t *header = (race_reply_header_t *)hexdata.data();
        if( header->type == EARBUD_MSG_TYPE_RSP ) {
            return true;
        }
    }

    return false;
}

QString get_race_reply_key(const QByteArray &hexdata)
{
    QString key;
    if(RACE_REPLY_HEADER_LEN <= hexdata.count() ) {
        key.sprintf("%02X%02X", (uint8_t)hexdata[8], (uint8_t)hexdata[9]);
    }

    return key;
}

void make_race_cmd_header(QByteArray &cmd_arr, uint8_t vlen1, uint8_t vlen2, uint8_t rlen1, uint8_t rlen2, uint8_t id1, uint8_t id2, bool has_val, uint8_t val)
{
    race_cmd_header_t header;

    header.chgbox_lead1 = EARBUD_MSG_LEAD1;
    header.chgbox_lead2 = EARBUD_MSG_LEAD2;
    header.chgbox_len1 = vlen1;
    header.chgbox_len2 = vlen2;
    header.indicator = EARBUD_MSG_INDICATOR;
    header.type = EARBUD_MSG_TYPE_CMD;
    header.len1 = rlen1;
    header.len2 = rlen2;
    header.id1 = id1;
    header.id2 = id2;
    if(has_val) {
        header.value = val;
    }

    cmd_arr.append((const char*)&header, sizeof(header));
}

int make_race_cmd_read_gsensor(QByteArray &cmd, uint8_t earside)
{
    Q_UNUSED(earside);
    QString str = "FE FC 00 06 05 5A 02 00 01 00";
    if(RET_FAIL == string2HexArray(str, cmd) ) {
        return RET_FAIL;
    }

    return RET_OK;
}
#define EARBUD_RSP_LENGTH_READ_GSENSOR  18
int parse_race_reply_read_gsensor(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "读G-Sensor数据";
    if(hexdata.count() < EARBUD_RSP_LENGTH_READ_GSENSOR) {
        jsarr.append(topic);
        addInfo2Array(jsarr, KEY_STR_EXCEPTION, VALUE_STR_LENGTH_NOT_ENOUGH, true);
        return RET_FAIL;
    }

    QJsonObject jsobj;

    uint8_t idx = 10;
    jsobj.insert("I2C通信", 0 == (uint8_t)hexdata[idx] && 0 == (uint8_t)hexdata[idx + 1] ? VALUE_STR_SUCCESS : VALUE_STR_FAIL );

    idx += 2;
    //int16_t v = (int16_t) ((uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8);
    //int16_t v =  (hexdata[idx] | hexdata[idx + 1] << 8);

    jsobj.insert("X轴", (float)(((int16_t) ((uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8)) / 4 ) * 0.244 );

    idx += 2;
    jsobj.insert("Y轴", (float)(((int16_t) ((uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8)) / 4 ) * 0.244 );

    idx += 2;
    jsobj.insert("Z轴", (float)(((int16_t) ((uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8)) / 4 ) * 0.244 );

    jsarr.append(jsobj);
    return RET_OK;
}

int make_race_cmd_set_bt_visible(QByteArray &cmd, uint8_t earside)
{
    Q_UNUSED(earside);
    QString str = "FE FC 00 08 05 5A 04 00 01 11 02 00";
    if(RET_FAIL == string2HexArray(str, cmd) ) {
        return RET_FAIL;
    }

    return RET_OK;
}

int parse_race_reply_set_bt_visible(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "蓝牙可见";
    QJsonObject jsobj;
    QString correct_rsp = "FE FC 00 07 05 5B 03 00 01 11 00";
    QByteArray correct_arr;

    string2HexArray(correct_rsp, correct_arr);
    if(hexdata.startsWith(correct_arr)) {
        jsobj.insert(topic, VALUE_STR_SUCCESS);
    }
    else {
        jsobj.insert(topic, VALUE_STR_FAIL);
    }

    jsarr.append(jsobj);

    return RET_OK;
}

int make_race_cmd_select_pcb_mic(QByteArray &cmd, uint8_t earside)
{
    Q_UNUSED(earside);
    QString str = "FE FC 00 07 05 5A 03 00 0C 0E 00";
    if(RET_FAIL == string2HexArray(str, cmd) ) {
        return RET_FAIL;
    }

    return RET_OK;
}

int make_race_cmd_select_ff_mic(QByteArray &cmd, uint8_t earside)
{
    Q_UNUSED(earside);
    QString str = "FE FC 00 07 05 5A 03 00 0C 0E 01";
    if(RET_FAIL == string2HexArray(str, cmd) ) {
        return RET_FAIL;
    }

    return RET_OK;
}

int make_race_cmd_select_fb_mic(QByteArray &cmd, uint8_t earside)
{
    Q_UNUSED(earside);
    QString str = "FE FC 00 07 05 5A 03 00 0C 0E 02";
    if(RET_FAIL == string2HexArray(str, cmd) ) {
        return RET_FAIL;
    }

    return RET_OK;
}

int parse_race_reply_select_talk_mic(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "切换通话MIC";
    QJsonObject jsobj;

    QString str_select_mic = "FE FC 00 07 05 5B 03 00 0C 0E 00";
    QByteArray arr_select_mic;

    string2HexArray(str_select_mic, arr_select_mic);
    if(hexdata.startsWith(arr_select_mic)) {
        jsobj.insert(topic, VALUE_STR_SUCCESS);
    }
    else {
        jsobj.insert(topic, VALUE_STR_FAIL);
    }

    jsarr.append(jsobj);

    return RET_OK;
}

int make_race_cmd_enter_dut(QByteArray &cmd, uint8_t earside)
{
    Q_UNUSED(earside);
    QString str = "FE FC 00 09 05 5A 05 00 01 0A E3 F2 01";
    if(RET_FAIL == string2HexArray(str, cmd) ) {
        return RET_FAIL;
    }

    return RET_OK;
}

int make_race_cmd_exit_dut(QByteArray &cmd, uint8_t earside)
{
    Q_UNUSED(earside);
    QString str = "FE FC 00 09 05 5A 05 00 01 0A E3 F2 00";
    if(RET_FAIL == string2HexArray(str, cmd) ) {
        return RET_FAIL;
    }

    return RET_OK;
}

int parse_race_reply_enter_exit_dut(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "进入/退出DUT";
    if(hexdata.count() < RACE_REPLY_HEADER_LEN - 1) {
        jsarr.append(topic);
        addInfo2Array(jsarr, KEY_STR_EXCEPTION, VALUE_STR_LENGTH_NOT_ENOUGH, true);
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, VALUE_STR_SUCCESS );
    jsarr.append(jsobj);
    return RET_OK;
}


int make_race_cmd_enter_standby(QByteArray &cmd, uint8_t earside)
{
    Q_UNUSED(earside);
    QString str = "FE FC 00 08 05 5A 04 00 01 11 18 00";
    if(RET_FAIL == string2HexArray(str, cmd) ) {
        return RET_FAIL;
    }

    return RET_OK;
}

int make_race_cmd_restart(QByteArray &cmd, uint8_t earside)
{
    Q_UNUSED(earside);
    QString str = "FE FC 00 08 05 5A 04 00 01 11 19 00";
    if(RET_FAIL == string2HexArray(str, cmd) ) {
        return RET_FAIL;
    }

    return RET_OK;
}


#define EARBUD_CMD_LENGTH_STANDBY_OR_RESTART  11
int parse_race_reply_standby_or_restart(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "待机/重启";
    if(hexdata.count() < EARBUD_CMD_LENGTH_STANDBY_OR_RESTART) {
        jsarr.append(topic);
        addInfo2Array(jsarr, KEY_STR_EXCEPTION, "回复数据长度错误！", true);
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, VALUE_STR_SUCCESS);
    jsarr.append(jsobj);
    return RET_OK;
}

int make_race_cmd_power_off(QByteArray &cmd, uint8_t earside)
{
    Q_UNUSED(earside);
    QString str = "FE FC 00 07 05 5A 03 00 27 00 01";
    if(RET_FAIL == string2HexArray(str, cmd) ) {
        return RET_FAIL;
    }

    return RET_OK;
}

int parse_race_reply_power_off(const QByteArray hexdata, QJsonArray &jsarr)
{
    Q_UNUSED(hexdata);
    Q_UNUSED(jsarr);
    return RET_OK;
}






