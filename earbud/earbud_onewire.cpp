#include "earbud_onewire.h"
#include "earbud_internal.h"
#include "mew_type_define.h"
#include "utils/utils.h"

#define ONEWIRE_PAYLOAD_LENGTH_MAC 6
#define ONEWIRE_PAYLOAD_LENGTH_VERSION 7
#define ONEWIRE_PAYLOAD_LENGTH_CHANNEL 1
#define ONEWIRE_PAYLOAD_LENGTH_NTC 4
#define ONEWIRE_PAYLOAD_LENGTH_SET_LIC_KEY 1
#define ONEWIRE_PAYLOAD_LENGTH_GET_LIC_RESULT 1
#define ONEWIRE_PAYLOAD_LENGTH_GET_LIC_KEY 1

#define ONEWIRE_PAYLOAD_LENGTH_CAPTOUCH_START_INT            1
#define ONEWIRE_PAYLOAD_LENGTH_CAPTOUCH_GET_INT_RESULT       1
#define ONEWIRE_PAYLOAD_LENGTH_CAPTOUCH_READ_VERSION         1
#define ONEWIRE_PAYLOAD_LENGTH_CAPTOUCH_START_CALIB          1
#define ONEWIRE_PAYLOAD_LENGTH_CAPTOUCH_GET_CALIB_RESULT     1
#define ONEWIRE_PAYLOAD_LENGTH_CAPTOUCH_READ_VALUE           11

#define ONEWIRE_PAYLOAD_LENGTH_OPTIC_COMMU       1
#define ONEWIRE_PAYLOAD_LENGTH_OPTIC_INT_START           1
#define ONEWIRE_PAYLOAD_LENGTH_OPTIC_INT_END             1
#define ONEWIRE_PAYLOAD_LENGTH_OPTIC_LASER_START         1
#define ONEWIRE_PAYLOAD_LENGTH_OPTIC_LASER_END           2
#define ONEWIRE_PAYLOAD_LENGTH_OPTIC_FULL_SCALE_START        1
#define ONEWIRE_PAYLOAD_LENGTH_OPTIC_FULL_SCALE_END          2
#define ONEWIRE_PAYLOAD_LENGTH_OPTIC_BG_NOISE_START      1
#define ONEWIRE_PAYLOAD_LENGTH_OPTIC_BG_NOISE_END        2

#define ONEWIRE_PAYLOAD_LENGTH_FORCE_START_DETECT    1
#define ONEWIRE_PAYLOAD_LENGTH_FORCE_GET_FW_VER      1
#define ONEWIRE_PAYLOAD_LENGTH_FORCE_GET_ASSEMBLE    1
#define ONEWIRE_PAYLOAD_LENGTH_FORCE_GET_NOISE_PEAK      2
#define ONEWIRE_PAYLOAD_LENGTH_FORCE_GET_BURST_PRESSURE  2
#define ONEWIRE_PAYLOAD_LENGTH_FORCE_GET_SEMPH           2

#define ONEWIRE_PAYLOAD_LENGTH_ENTER_AGE_MODE    1
#define ONEWIRE_PAYLOAD_LENGTH_SET_BAUD_RATE    0

#define ONEWIRE_PAYLOAD_LENGTH_READ_BAT_POWER 1
#define ONEWIRE_CMD_READ_MAC             0x2E
#define ONEWIRE_CMD_READ_VERSION         0x23
#define ONEWIRE_CMD_READ_CHANNEL         0x69
#define ONEWIRE_CMD_READ_NTC     0x6E
#define ONEWIRE_CMD_SET_LIC_KEY      0x64
#define ONEWIRE_CMD_GET_LIC_RESULT   0x65
#define ONEWIRE_CMD_GET_LIC_KEY      0x66

#define ONEWIRE_CMD_CAPTOUCH_START_INT           0x85
#define ONEWIRE_CMD_CAPTOUCH_GET_INT_RESULT      0x86
#define ONEWIRE_CMD_CAPTOUCH_READ_VERSION        0x83
#define ONEWIRE_CMD_CAPTOUCH_START_CALIB         0x81
#define ONEWIRE_CMD_CAPTOUCH_GET_CALIB_RESULT    0x82
#define ONEWIRE_CMD_CAPTOUCH_READ_VALUE          0x84

#define ONEWIRE_CMD_OPTIC_COMMU          0X8D
#define ONEWIRE_CMD_OPTIC_INT_START              0X8E
#define ONEWIRE_CMD_OPTIC_INT_END                0X8F
#define ONEWIRE_CMD_OPTIC_LASER_START            0X90
#define ONEWIRE_CMD_OPTIC_LASER_END              0X91
#define ONEWIRE_CMD_OPTIC_FULL_SCALE_START           0X92
#define ONEWIRE_CMD_OPTIC_FULL_SCALE_END             0X93
#define ONEWIRE_CMD_OPTIC_BG_NOISE_START         0X94
#define ONEWIRE_CMD_OPTIC_BG_NOISE_END           0X95

#define ONEWIRE_CMD_FORCE_START_DETECT           0X87
#define ONEWIRE_CMD_FORCE_GET_FW_VER             0X88
#define ONEWIRE_CMD_FORCE_GET_ASSEMBLE           0X89
#define ONEWIRE_CMD_FORCE_GET_NOISE_PEAK         0X8A
#define ONEWIRE_CMD_FORCE_GET_BURST_PRESSURE     0X8B
#define ONEWIRE_CMD_FORCE_GET_SEMPH              0X8C

#define ONEWIRE_CMD_ENTER_AGE_MODE       0x24
#define ONEWIRE_CMD_SET_BAUD_RATE   0x0F
#define ONEWIRE_CMD_READ_BAT_POWER   0X06

QString make_1wire_key(uint8_t usr_id1, uint8_t usr_id2, uint8_t cmd)
{
    QString str;
    str.sprintf("%02X%02X%02X", usr_id1, usr_id2, cmd);
    return str;
}

parse_func_list_t* get_1wire_parse_func_list()
{
    static parse_func_list_t earbud_1wire_parse_func_list = {
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_READ_MAC), parse_1wire_reply_read_mac},
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_READ_VERSION), parse_1wire_reply_read_version},
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_READ_CHANNEL), parse_1wire_reply_read_channel},
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_READ_NTC), parse_1wire_reply_read_ntc},
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_SET_LIC_KEY), parse_1wire_reply_set_lic_key},
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_GET_LIC_RESULT), parse_1wire_reply_get_lic_result},
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_GET_LIC_KEY), parse_1wire_reply_get_lic_key},

        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_CAPTOUCH_START_INT), parse_1wire_reply_captouch_start_int},
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_CAPTOUCH_GET_INT_RESULT), parse_1wire_reply_captouch_get_int_result},
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_CAPTOUCH_READ_VERSION), earbud_parse_notify_captouch_read_version},
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_CAPTOUCH_START_CALIB), parse_1wire_reply_captouch_start_calib},
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_CAPTOUCH_GET_CALIB_RESULT), parse_1wire_reply_captouch_get_calib_result},
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_CAPTOUCH_READ_VALUE), parse_1wire_reply_captouch_read_value},

        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_OPTIC_COMMU), parse_1wire_reply_optic_communicate},
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_OPTIC_INT_START), parse_1wire_reply_optic_int_start},
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_OPTIC_INT_END), parse_1wire_reply_optic_int_end},
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_OPTIC_LASER_START), parse_1wire_reply_optic_laser_start},
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_OPTIC_LASER_END), parse_1wire_reply_optic_laser_end},
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_OPTIC_FULL_SCALE_START), parse_1wire_reply_optic_full_scale_start},
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_OPTIC_FULL_SCALE_END), parse_1wire_reply_optic_full_scale_end},
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_OPTIC_BG_NOISE_START), parse_1wire_reply_optic_bg_noise_start},
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_OPTIC_BG_NOISE_END), parse_1wire_reply_optic_bg_noise_end},

        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_FORCE_START_DETECT), parse_1wire_reply_force_start_detect },
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_FORCE_GET_FW_VER), parse_1wire_reply_force_get_fw_ver },
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_FORCE_GET_ASSEMBLE), parse_1wire_reply_force_get_assemble },
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_FORCE_GET_NOISE_PEAK), parse_1wire_reply_force_get_noise_peak },
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_FORCE_GET_BURST_PRESSURE), parse_1wire_reply_force_get_burst_pressure },
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_FORCE_GET_SEMPH), parse_1wire_reply_force_get_semph },

        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_ENTER_AGE_MODE), parse_1wire_reply_enter_age_mode },
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_SET_BAUD_RATE), parse_1wire_reply_set_baud_rate },
        {make_1wire_key(EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_READ_BAT_POWER), parse_1wire_reply_read_bat_power },
    };

    return &earbud_1wire_parse_func_list;
}

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
    uint8_t cmd;
    uint8_t earside;
} onewire_cmd_header_t;
typedef onewire_cmd_header_t onewire_reply_header_t;

bool onewire_cmd_length_field_is_valid(const QByteArray &hexdata, uint32_t payload_len)
{
    onewire_cmd_header_t *header = (onewire_cmd_header_t *)hexdata.data();
    unsigned int hex_bytes = (unsigned int)hexdata.count();

    if(hex_bytes < sizeof(onewire_cmd_header_t) + payload_len) {
        return false;
    }

    uint32_t vlen = calc_length_little_end(header->chgbox_len1, header->chgbox_len2);
    uint32_t rlen = calc_length_little_end(header->len1, header->len2);
    return hex_bytes >= vlen + 4 && vlen == rlen + 4;
}

bool is_1wire_reply(const QByteArray &hexdata)
{
    onewire_reply_header_t *header = (onewire_reply_header_t *)hexdata.data();
    unsigned int hex_bytes = (unsigned int)hexdata.count();
    if(hex_bytes >= sizeof(onewire_reply_header_t)
            && header->type == EARBUD_MSG_TYPE_NOTIFY
        ) {
        return true;
    }

    return false;
}

bool onewire_reply_length_field_is_valid(const QByteArray &hexdata, uint32_t payload_len)
{
    onewire_reply_header_t *header = (onewire_reply_header_t *)hexdata.data();
    unsigned int hex_bytes = (unsigned int)hexdata.count();

    if(hex_bytes < sizeof(onewire_reply_header_t) + payload_len) {
        return false;
    }

    uint32_t vlen = calc_length_little_end(header->chgbox_len1, header->chgbox_len2);
    uint32_t rlen = calc_length_big_end(header->len1, header->len2);
    return hex_bytes >= vlen + 4 && vlen == rlen + 4;
}

int check_onewire_reply_format(const QByteArray &hexdata, uint32_t payload_len, QString &topic, QJsonArray &jsarr) {
    //earbud_vbus_notify_header_t *header = (earbud_vbus_notify_header_t *)hexdata.data();
    if(!onewire_reply_length_field_is_valid(hexdata, payload_len)) {
        jsarr.append(topic);
        addInfo2Array(jsarr, KEY_STR_EXCEPTION, "回复数据长度错误！", true);
        return RET_FAIL;
    }

    //topic += (EARSIDE_LEFT == header->race_earside) ? " (左)" : " (右)";
    return RET_OK;
}

QString get_1wire_reply_key(const QByteArray &hexdata)
{
    QString key;
    onewire_reply_header_t *notify = (onewire_reply_header_t *)hexdata.data();
    key.sprintf("%02X%02X%02X", (uint8_t)notify->id1, (uint8_t)notify->id2, (uint8_t)notify->cmd);
    return key;
}

void make_onewire_cmd_header(QByteArray &cmd_arr, uint8_t vlen1, uint8_t vlen2, uint8_t rlen1, uint8_t rlen2, uint8_t race_id1, uint8_t race_id2, uint8_t cmd, uint8_t earside)
{
    onewire_cmd_header_t header;

    header.chgbox_lead1 = EARBUD_MSG_LEAD1;
    header.chgbox_lead2 = EARBUD_MSG_LEAD2;
    header.chgbox_len1 = vlen1;
    header.chgbox_len2 = vlen2;
    header.indicator = EARBUD_MSG_INDICATOR;
    header.type = EARBUD_MSG_TYPE_CMD;
    header.len1 = rlen1;
    header.len2 = rlen2;
    header.id1 = race_id1;
    header.id2 = race_id2;
    header.cmd = cmd;
    header.earside = earside;

    cmd_arr.append((const char*)&header, sizeof(header));
    //assert(earbud_vbus_cmd_length_field_is_valid(cmd_arr, 0));
}

int make_1wire_cmd_read_mac(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_READ_MAC, earside);
    return 0;
}

int parse_1wire_reply_read_mac(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "读耳机MAC地址";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_MAC, topic, jsarr)) {
        return RET_FAIL;
    }

    //jsarr.append(topic);
    QJsonObject jsobj;
    QByteArray mac_array;
    for(int i = 0; i < ONEWIRE_PAYLOAD_LENGTH_MAC; ++i) {
        mac_array.append(hexdata[sizeof(onewire_reply_header_t) + 5 - i]);
    }
    QString mac_str = hexArray2StringPlusSpace(mac_array);
    jsobj.insert(topic, mac_str);
    jsarr.append(jsobj);

    return 0;
}

int make_1wire_cmd_read_version(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_READ_VERSION, earside);
    return 0;
}

int parse_1wire_reply_read_version(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "读耳机版本";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_VERSION, topic, jsarr)) {
        return RET_FAIL;
    }

    //jsarr.append(topic);
    QJsonObject jsobj;
    QString verStr;
    for(int i = 0; i < ONEWIRE_PAYLOAD_LENGTH_VERSION; ++i) {
        verStr.append(hexdata[sizeof(onewire_reply_header_t) + i]);
    }
    jsobj.insert(topic, verStr);
    jsarr.append(jsobj);

    return 0;
}

int make_1wire_cmd_read_channel(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_READ_CHANNEL, earside);
    return 0;
}

int parse_1wire_reply_read_channel(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "读耳机声道";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_CHANNEL, topic, jsarr)) {
        return RET_FAIL;
    }

    //jsarr.append(topic);
    QJsonObject jsobj;
    jsobj.insert(topic, hexdata[sizeof(onewire_reply_header_t)] == EARSIDE_LEFT ? "左" : "右");
    jsarr.append(jsobj);

    return 0;
}

int make_1wire_cmd_read_ntc(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_READ_NTC, earside);
    return 0;
}

int parse_1wire_reply_read_ntc(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "读NTC温度";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_NTC, topic, jsarr)) {
        return RET_FAIL;
    }

    uint32_t idx = sizeof(onewire_reply_header_t);
    int temperature = (uint32_t)hexdata[idx] | (uint32_t)hexdata[idx + 1] << 8 | (uint32_t)hexdata[idx + 2] << 16 | (uint32_t)hexdata[idx + 3] << 24;
    QJsonObject jsobj;
    jsobj.insert(topic, temperature);
    jsarr.append(jsobj);

    return 0;
}

int make_1wire_cmd_set_lic_key(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_SET_LIC_KEY, earside);
    return 0;
}

int parse_1wire_reply_set_lic_key(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "写授权key";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_SET_LIC_KEY, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, hexdata[sizeof(onewire_reply_header_t)] == RET_OK ? VALUE_STR_SUCCESS : VALUE_STR_FAIL);
    jsarr.append(jsobj);

    return 0;
}

int make_1wire_cmd_get_lic_result(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_GET_LIC_RESULT, earside);
    return 0;
}

int parse_1wire_reply_get_lic_result(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "获取授权结果";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_GET_LIC_RESULT, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, hexdata[sizeof(onewire_reply_header_t)] == RET_OK ? VALUE_STR_SUCCESS : VALUE_STR_FAIL);
    jsarr.append(jsobj);

    return 0;
}

int make_1wire_cmd_get_lic_key(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_GET_LIC_KEY, earside);
    return 0;
}

int parse_1wire_reply_get_lic_key(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "获取当前授权key";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_GET_LIC_KEY, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, hexdata[sizeof(onewire_reply_header_t)] == RET_OK ? VALUE_STR_SUCCESS : VALUE_STR_FAIL);
    jsarr.append(jsobj);

    return 0;
}


#define RESULT_OK       0
#define RESULT_FAIL     1
#define RESULT_TESTING  3
QString earbud_get_result_string(uint8_t res)
{
    QString str;
    switch(res) {
        case RESULT_OK:
            str = VALUE_STR_SUCCESS;
            break;
        case RESULT_FAIL:
            str = VALUE_STR_FAIL;
            break;
        case RESULT_TESTING:
            str = VALUE_STR_TESTING;
            break;
        default:
            str = "未知";
            break;
    }

    return str;
}

int make_1wire_cmd_captouch_start_int(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_CAPTOUCH_START_INT, earside);
    return 0;
}

int parse_1wire_reply_captouch_start_int(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "启动容触中断测试";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_CAPTOUCH_START_INT, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(onewire_reply_header_t)]));
    jsarr.append(jsobj);

    return 0;
}

int make_1wire_cmd_captouch_get_int_result(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_CAPTOUCH_GET_INT_RESULT, earside);
    return 0;
}

int parse_1wire_reply_captouch_get_int_result(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "容触中断测试结果";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_CAPTOUCH_GET_INT_RESULT, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(onewire_reply_header_t)]));
    jsarr.append(jsobj);

    return 0;
}

int make_1wire_cmd_captouch_read_version(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_CAPTOUCH_READ_VERSION, earside);
    return 0;
}
int earbud_parse_notify_captouch_read_version(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "读容触版本";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_CAPTOUCH_READ_VERSION, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, hexdata[sizeof(onewire_reply_header_t)] );
    jsarr.append(jsobj);

    return 0;
}

int make_1wire_cmd_captouch_start_calib(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_CAPTOUCH_START_CALIB, earside);
    return 0;
}
int parse_1wire_reply_captouch_start_calib(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "启动容触校准";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_CAPTOUCH_START_CALIB, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(onewire_reply_header_t)]));
    jsarr.append(jsobj);

    return 0;
}

int make_1wire_cmd_captouch_get_calib_result(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_CAPTOUCH_GET_CALIB_RESULT, earside);
    return 0;
}
int parse_1wire_reply_captouch_get_calib_result(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "容触校准结果";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_CAPTOUCH_GET_INT_RESULT, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(onewire_reply_header_t)]));
    jsarr.append(jsobj);

    return 0;
}

int make_1wire_cmd_captouch_read_value(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_CAPTOUCH_READ_VALUE, earside);
    return 0;
}
int parse_1wire_reply_captouch_read_value(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "读abov电容值";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_CAPTOUCH_READ_VALUE, topic, jsarr)) {
        return RET_FAIL;
    }


    uint8_t idx = sizeof(onewire_reply_header_t);
    if(0 != hexdata[idx]) {
        QJsonObject jsobj;
        jsobj.insert(topic, "无效值");
        jsarr.append(jsobj);
        return RET_FAIL;
    }

    jsarr.append(topic);

    {
        QJsonObject jsobj;
        ++idx;
        uint16_t diff1 = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
        jsobj.insert("按键diff值", diff1);
        jsarr.append(jsobj);
    }


    {
        QJsonObject jsobj;
        idx += 2;
        uint16_t diff2 = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
        jsobj.insert("入耳diff值", diff2);
        jsarr.append(jsobj);
    }

    {
        QJsonObject jsobj;
        idx += 2;
        uint16_t cap1 = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
        jsobj.insert("按键容值", cap1);
        jsarr.append(jsobj);
    }

    {
        QJsonObject jsobj;
        idx += 2;
        uint16_t cap2 = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
        jsobj.insert("入耳容值", cap2);
        jsarr.append(jsobj);
    }

    {
        QJsonObject jsobj;
        idx += 2;
        uint16_t capref = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
        jsobj.insert("参考容值", capref);
        jsarr.append(jsobj);
    }

    return 0;
}

int make_1wire_cmd_optic_communicate(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_OPTIC_COMMU, earside);
    return 0;
}

int parse_1wire_reply_optic_communicate(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "光感通信测试";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_OPTIC_COMMU, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(onewire_reply_header_t)]));
    jsarr.append(jsobj);

    return 0;
}

int make_1wire_cmd_optic_int_start(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_OPTIC_INT_START, earside);
    return 0;
}

int parse_1wire_reply_optic_int_start(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "光感INT测试开始";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_OPTIC_INT_START, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(onewire_reply_header_t)]));
    jsarr.append(jsobj);

    return 0;
}

int make_1wire_cmd_optic_int_end(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_OPTIC_INT_END, earside);
    return 0;
}

int parse_1wire_reply_optic_int_end(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "光感INT测试结束";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_OPTIC_INT_END, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(onewire_reply_header_t)]));
    jsarr.append(jsobj);

    return 0;
}

int make_1wire_cmd_optic_laser_start(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_OPTIC_LASER_START, earside);
    return 0;
}

int parse_1wire_reply_optic_laser_start(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "光感激光测试开始";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_OPTIC_LASER_START, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(onewire_reply_header_t)]));
    jsarr.append(jsobj);

    return 0;
}

int make_1wire_cmd_optic_laser_end(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_OPTIC_LASER_END, earside);
    return 0;
}

int parse_1wire_reply_optic_laser_end(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "光感激光测试结束";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_OPTIC_LASER_END, topic, jsarr)) {
        return RET_FAIL;
    }

    jsarr.append(topic);

    QJsonObject jsobj;
    uint8_t idx = sizeof(onewire_reply_header_t);
    uint16_t v = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
    QString k;
    k.sprintf("PXS = %d", (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8 );
    jsobj.insert(k,  v > 5 ? VALUE_STR_SUCCESS : VALUE_STR_FAIL);
    jsarr.append(jsobj);

    return 0;
}

int make_1wire_cmd_optic_full_scale_start(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_OPTIC_FULL_SCALE_START, earside);
    return 0;
}

int parse_1wire_reply_optic_full_scale_start(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "光感满量程测试开始";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_OPTIC_FULL_SCALE_START, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, (uint8_t)hexdata[sizeof(onewire_reply_header_t)] == 0x00 ? VALUE_STR_SUCCESS : VALUE_STR_FAIL);
    jsarr.append(jsobj);

    return RET_OK;
}

int make_1wire_cmd_optic_full_scale_end(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_OPTIC_FULL_SCALE_END, earside);
    return 0;
}

int parse_1wire_reply_optic_full_scale_end(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "光感满量程测试结束";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_OPTIC_FULL_SCALE_END, topic, jsarr)) {
        return RET_FAIL;
    }

    jsarr.append(topic);

    QJsonObject jsobj;
    uint8_t idx = sizeof(onewire_reply_header_t);
    uint16_t v = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
    QString k;
    k.sprintf("PXS = %d", (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8 );
    jsobj.insert(k,  v >= 900 && v <= 1023 ? VALUE_STR_SUCCESS : VALUE_STR_FAIL);
    jsarr.append(jsobj);

    return 0;
}

int make_1wire_cmd_optic_bg_noise_start(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_OPTIC_BG_NOISE_START, earside);
    return 0;
}

int parse_1wire_reply_optic_bg_noise_start(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "光感底噪测试开始";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_OPTIC_BG_NOISE_START, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(onewire_reply_header_t)]));
    jsarr.append(jsobj);

    return 0;
}

int make_1wire_cmd_optic_bg_noise_end(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_OPTIC_BG_NOISE_END, earside);
    return 0;
}

int parse_1wire_reply_optic_bg_noise_end(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "光感底噪测试结束";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_OPTIC_BG_NOISE_END, topic, jsarr)) {
        return RET_FAIL;
    }

    jsarr.append(topic);

    QJsonObject jsobj;
    uint8_t idx = sizeof(onewire_reply_header_t);
    uint16_t v = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
    QString k;
    k.sprintf("底噪值 = %d", v);
    jsobj.insert(k, v <= 100 ? VALUE_STR_SUCCESS : VALUE_STR_FAIL );
    jsarr.append(jsobj);

    return 0;
}

int make_1wire_cmd_force_start_detect(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_FORCE_START_DETECT, earside);
    return 0;
}

int parse_1wire_reply_force_start_detect(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "钛深启动压感检测";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_FORCE_START_DETECT, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(onewire_reply_header_t)]));
    jsarr.append(jsobj);

    return RET_OK;
}

int make_1wire_cmd_force_get_fw_ver(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_FORCE_GET_FW_VER, earside);
    return 0;
}

int parse_1wire_reply_force_get_fw_ver(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "钛深获取压感固件版本号";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_FORCE_GET_FW_VER, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, hexdata[sizeof(onewire_reply_header_t)] );
    jsarr.append(jsobj);

    return RET_OK;
}

int make_1wire_cmd_force_get_assemble(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_FORCE_GET_ASSEMBLE, earside);
    return 0;
}

int parse_1wire_reply_force_get_assemble(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "钛深获取压感装配情况";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_FORCE_GET_ASSEMBLE, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    QString v_str;
    switch (hexdata[sizeof(onewire_reply_header_t)]) {
        case 0:
            v_str = "预压过松";
            break;
        case 1:
            v_str = "预压正常";
            break;
        case 2:
            v_str = "预压过紧";
            break;
        default:
            v_str = "未知";
            break;
    }
    jsobj.insert(topic, v_str);
    jsarr.append(jsobj);

    return RET_OK;
}

int make_1wire_cmd_force_get_noise_peak(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_FORCE_GET_NOISE_PEAK, earside);
    return 0;
}

int parse_1wire_reply_force_get_noise_peak(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "钛深获取压感噪声峰值";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_FORCE_GET_NOISE_PEAK, topic, jsarr)) {
        return RET_FAIL;
    }

    uint8_t idx = sizeof(onewire_reply_header_t);
    uint16_t v = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
    QJsonObject jsobj;
    jsobj.insert(topic, v);
    jsarr.append(jsobj);

    return RET_OK;
}

int make_1wire_cmd_force_get_burst_pressure(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_FORCE_GET_BURST_PRESSURE, earside);
    return 0;
}

int parse_1wire_reply_force_get_burst_pressure(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "钛深获取压感按压突发值";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_FORCE_GET_BURST_PRESSURE, topic, jsarr)) {
        return RET_FAIL;
    }

    uint8_t idx = sizeof(onewire_reply_header_t);
    uint16_t v = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
    QJsonObject jsobj;
    jsobj.insert(topic, v);
    jsarr.append(jsobj);

    return RET_OK;
}

int make_1wire_cmd_force_get_semph(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 8, 0, 4, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_FORCE_GET_SEMPH, earside);
    return 0;
}

int parse_1wire_reply_force_get_semph(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "钛深获取压感按压信号量";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_FORCE_GET_SEMPH, topic, jsarr)) {
        return RET_FAIL;
    }

    uint8_t idx = sizeof(onewire_reply_header_t);
    uint16_t v = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
    QJsonObject jsobj;
    jsobj.insert(topic, v);
    jsarr.append(jsobj);

    return RET_OK;
}

int make_1wire_cmd_enter_age_mode(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 9, 0, 5, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_ENTER_AGE_MODE, earside);
    cmd.append(0x01);
    return RET_OK;
}

int parse_1wire_reply_enter_age_mode(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "进入老化模式";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_ENTER_AGE_MODE, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(onewire_reply_header_t)]));
    jsarr.append(jsobj);

    return RET_OK;
}




int make_1wire_cmd_set_baud_rate(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 9, 5, 0, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_SET_BAUD_RATE, earside);

    // "0x05" means 9600 baud rate of vbus.
    cmd.append(0x05);
    return RET_OK;
}

int parse_1wire_reply_set_baud_rate(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "设置通信波特率";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_SET_BAUD_RATE, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, VALUE_STR_SUCCESS);
    jsarr.append(jsobj);

    return RET_OK;
}

int make_1wire_cmd_read_bat_power(QByteArray &cmd, uint8_t earside)
{
    make_onewire_cmd_header(cmd, 0, 9, 0, 5, EARBUD_MSG_USR_ID1, EARBUD_MSG_USR_ID2, ONEWIRE_CMD_READ_BAT_POWER, earside);
    cmd.append(0x64);
    return RET_OK;
}
int parse_1wire_reply_read_bat_power(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "电量";
    if(RET_FAIL == check_onewire_reply_format(hexdata, ONEWIRE_PAYLOAD_LENGTH_READ_BAT_POWER, topic, jsarr)) {
        return RET_FAIL;
    }

    uint8_t idx = sizeof(onewire_reply_header_t);
    QString v;
    v.sprintf("%%%02d", (uint8_t)hexdata[idx]);
    QJsonObject jsobj;
    jsobj.insert(topic, v);
    jsarr.append(jsobj);

    return RET_OK;
}




