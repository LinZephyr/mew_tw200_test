#include "earbud.h"
#include "mew_type_define.h"
#include "utils/utils.h"

#include <QDebug>

#define EARBUD_LEAD1 0xFE
#define EARBUD_LEAD2 0xFC

#define RACE_CHANNEL 0x05

#define RACE_TYPE_CMD 0x5A
#define RACE_TYPE_RSP 0x5B
// indication don't need response from the other side.
#define RACE_TYPE_IND 0x5C
#define RACE_TYPE_NOTIFY 0x5D

#define RACE_USR_ID1 0
#define RACE_USR_ID2 0x20
#define RACE_ID_LEN 2

#define EARBUD_PAYLOAD_LENGTH_MAC 6
#define EARBUD_PAYLOAD_LENGTH_VERSION 7
#define EARBUD_PAYLOAD_LENGTH_CHANNEL 1
#define EARBUD_PAYLOAD_LENGTH_TEMPERATURE 4
#define EARBUD_PAYLOAD_LENGTH_SET_LICENSE_KEY 1
#define EARBUD_PAYLOAD_LENGTH_GET_LICENSE_RESULT 1
#define EARBUD_PAYLOAD_LENGTH_GET_LICENSE_KEY 1

#define EARBUD_PAYLOAD_LENGTH_CAPTOUCH_START_INT            1
#define EARBUD_PAYLOAD_LENGTH_CAPTOUCH_GET_INT_RESULT       1
#define EARBUD_PAYLOAD_LENGTH_CAPTOUCH_READ_VERSION         1
#define EARBUD_PAYLOAD_LENGTH_CAPTOUCH_START_CALIB          1
#define EARBUD_PAYLOAD_LENGTH_CAPTOUCH_GET_CALIB_RESULT     1
#define EARBUD_PAYLOAD_LENGTH_CAPTOUCH_READ_VALUE           11

#define EARBUD_PAYLOAD_LENGTH_OPTIC_COMMUNICATION       1
#define EARBUD_PAYLOAD_LENGTH_OPTIC_INT_START           1
#define EARBUD_PAYLOAD_LENGTH_OPTIC_INT_END             1
#define EARBUD_PAYLOAD_LENGTH_OPTIC_LASER_START         1
#define EARBUD_PAYLOAD_LENGTH_OPTIC_LASER_END           2
#define EARBUD_PAYLOAD_LENGTH_OPTIC_FULL_SCALE          2
#define EARBUD_PAYLOAD_LENGTH_OPTIC_BG_NOISE_START      1
#define EARBUD_PAYLOAD_LENGTH_OPTIC_BG_NOISE_END        2

#define EARBUD_PAYLOAD_LENGTH_FORCE_START_DETECT    1
#define EARBUD_PAYLOAD_LENGTH_FORCE_GET_FW_VER      1
#define EARBUD_PAYLOAD_LENGTH_FORCE_GET_ASSEMBLE    1
#define EARBUD_PAYLOAD_LENGTH_FORCE_GET_NOISE_PEAK      2
#define EARBUD_PAYLOAD_LENGTH_FORCE_GET_BURST_PRESSURE  2
#define EARBUD_PAYLOAD_LENGTH_FORCE_GET_SEMPH           2

#define EARBUD_PAYLOAD_LENGTH_ENTER_AGE_MODE    1
#define EARBUD_PAYLOAD_LENGTH_SET_VBUS_BAUD_RATE    0
//#define EARBUD_PAYLOAD_LENGTH_ENTER_STANDBY  0
#define EARBUD_PAYLOAD_LENGTH_POWER_OFF  0

//#define EARBUD_PAYLOAD_LENGTH_RESTART   -1
//#define EARBUD_PAYLOAD_LENGTH_ENTER_DUT -1
//#define EARBUD_PAYLOAD_LENGTH_EXIT_DUT  -1
#define EARBUD_PAYLOAD_LENGTH_READ_GSENSOR  6
#define EARBUD_PAYLOAD_LENGTH_READ_BAT_POWER 1


#define EARBUD_CMD_READ_MAC             0x2E
#define EARBUD_CMD_READ_VERSION         0x23
#define EARBUD_CMD_READ_CHANNEL         0x69
#define EARBUD_CMD_READ_TEMPERATURE     0x6E
#define EARBUD_CMD_SET_LICENSE_KEY      0x64
#define EARBUD_CMD_GET_LICENSE_RESULT   0x65
#define EARBUD_CMD_GET_LICENSE_KEY      0x66

#define EARBUD_CMD_CAPTOUCH_START_INT           0x85
#define EARBUD_CMD_CAPTOUCH_GET_INT_RESULT      0x86
#define EARBUD_CMD_CAPTOUCH_READ_VERSION        0x83
#define EARBUD_CMD_CAPTOUCH_START_CALIB         0x81
#define EARBUD_CMD_CAPTOUCH_GET_CALIB_RESULT    0x82
#define EARBUD_CMD_CAPTOUCH_READ_VALUE          0x84

#define EARBUD_CMD_OPTIC_COMMUNICATION          0X8D
#define EARBUD_CMD_OPTIC_INT_START              0X8E
#define EARBUD_CMD_OPTIC_INT_END                0X8F
#define EARBUD_CMD_OPTIC_LASER_START            0X90
#define EARBUD_CMD_OPTIC_LASER_END              0X91
#define EARBUD_CMD_OPTIC_FULL_SCALE             0X92
#define EARBUD_CMD_OPTIC_BG_NOISE_START         0X93
#define EARBUD_CMD_OPTIC_BG_NOISE_END           0X94

#define EARBUD_CMD_FORCE_START_DETECT           0X87
#define EARBUD_CMD_FORCE_GET_FW_VER             0X88
#define EARBUD_CMD_FORCE_GET_ASSEMBLE           0X89
#define EARBUD_CMD_FORCE_GET_NOISE_PEAK         0X8A
#define EARBUD_CMD_FORCE_GET_BURST_PRESSURE     0X8B
#define EARBUD_CMD_FORCE_GET_SEMPH              0X8C

#define EARBUD_CMD_ENTER_AGE_MODE       0x24
#define EARBUD_CMD_SET_VBUS_BAUD_RATE   0x0F
#define EARBUD_CMD_ENTER_STANDBY    0x18
#define EARBUD_CMD_RESTART          0x19
#define EARBUD_CMD_READ_BAT_POWER   0X06


QString earbud_make_key(uint8_t usr_id1, uint8_t usr_id2, uint8_t cmd);
static parse_func_list_t earbud_parse_func_list = {
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_MAC), earbud_parse_notify_read_mac},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_VERSION), earbud_parse_notify_read_version},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_CHANNEL), earbud_parse_notify_read_channel},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_TEMPERATURE), earbud_parse_notify_read_temperature},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_SET_LICENSE_KEY), earbud_parse_notify_set_license_key},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_GET_LICENSE_RESULT), earbud_parse_notify_get_license_result},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_GET_LICENSE_KEY), earbud_parse_notify_get_license_key},

    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_CAPTOUCH_START_INT), earbud_parse_notify_captouch_start_int},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_CAPTOUCH_GET_INT_RESULT), earbud_parse_notify_captouch_get_int_result},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_CAPTOUCH_READ_VERSION), earbud_parse_notify_captouch_read_version},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_CAPTOUCH_START_CALIB), earbud_parse_notify_captouch_start_calib},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_CAPTOUCH_GET_CALIB_RESULT), earbud_parse_notify_captouch_get_calib_result},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_CAPTOUCH_READ_VALUE), earbud_parse_notify_captouch_read_value},

    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_OPTIC_COMMUNICATION), earbud_parse_notify_optic_communicate},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_OPTIC_INT_START), earbud_parse_notify_optic_int_start},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_OPTIC_INT_END), earbud_parse_notify_optic_int_end},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_OPTIC_LASER_START), earbud_parse_notify_optic_laser_start},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_OPTIC_LASER_END), earbud_parse_notify_optic_laser_end},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_OPTIC_FULL_SCALE), earbud_parse_notify_optic_full_scale},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_OPTIC_BG_NOISE_START), earbud_parse_notify_optic_bg_noise_start},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_OPTIC_BG_NOISE_END), earbud_parse_notify_optic_bg_noise_end},

    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_FORCE_START_DETECT), earbud_parse_notify_force_start_detect },
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_FORCE_GET_FW_VER), earbud_parse_notify_force_get_fw_ver },
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_FORCE_GET_ASSEMBLE), earbud_parse_notify_force_get_assemble },
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_FORCE_GET_NOISE_PEAK), earbud_parse_notify_force_get_noise_peak },
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_FORCE_GET_BURST_PRESSURE), earbud_parse_notify_force_get_burst_pressure },
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_FORCE_GET_SEMPH), earbud_parse_notify_force_get_semph },

    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_ENTER_AGE_MODE), earbud_parse_notify_enter_age_mode },
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_SET_VBUS_BAUD_RATE), earbud_parse_notify_set_vbus_baud_rate },
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_BAT_POWER), earbud_parse_notify_read_bat_power },

    {"FE0F00", earbud_parse_notify_chgbox_exit_com_mode },
    {"FE0F01", earbud_parse_notify_chgbox_enter_com_mode },

    {"0111", earbud_parse_rsp_standby_or_restart },
    {"2700", earbud_parse_rsp_power_off },
    {"010A", earbud_parse_rsp_enter_exit_dut },
    {"0100", earbud_parse_rsp_read_gsensor },

};

int earbud_initialize_parse_func_list(parse_func_map_t &map)
{
    for(parse_func_list_t::const_iterator it = earbud_parse_func_list.begin(); it != earbud_parse_func_list.end(); ++it) {
        if(map.find(it->first) != map.end()) {
            QString str;
            str.sprintf("[%s] duplicate map key: %s", __FUNCTION__, it->first.toStdString().data());
            qWarning() << str;
            //return -1;
        }
        map.insert(it->first, it->second);
    }

    return 0;
}

QString earbud_make_key(uint8_t usr_id1, uint8_t usr_id2, uint8_t cmd)
{
    QString str;
    str.sprintf("%02X%02X%02X", usr_id1, usr_id2, cmd);
    return str;
}

typedef struct {
    uint8_t vbus_lead1;
    uint8_t vbus_lead2;
    uint8_t vbus_len1;
    uint8_t vbus_len2;
    uint8_t race_channel;
    uint8_t race_type;
    uint8_t race_len1;
    uint8_t race_len2;
    uint8_t race_id1;
    uint8_t race_id2;
    uint8_t race_cmd;
    uint8_t race_earside;
} earbud_vbus_cmd_header_t;
typedef earbud_vbus_cmd_header_t earbud_vbus_notify_header_t;
typedef earbud_vbus_notify_header_t earbud_vbus_rsp_header_t;

uint32_t calc_length_little_end(uint32_t len1, uint32_t len2)
{
    return len1 << 8 | len2;
}

uint32_t calc_length_big_end(uint32_t len1, uint32_t len2)
{
    return len2 << 8 | len1;
}


bool earbud_vbus_cmd_length_field_is_valid(const QByteArray &hexdata, uint32_t payload_len)
{
    earbud_vbus_cmd_header_t *header = (earbud_vbus_cmd_header_t *)hexdata.data();
    unsigned int hex_bytes = (unsigned int)hexdata.count();

    if(hex_bytes < sizeof(earbud_vbus_cmd_header_t) + payload_len) {
        return false;
    }

    uint32_t vlen = calc_length_little_end(header->vbus_len1, header->vbus_len2);
    uint32_t rlen = calc_length_little_end(header->race_len1, header->race_len2);
    return hex_bytes >= vlen + 4 && vlen == rlen + 4;
}

bool is_notify_from_earbud(const QByteArray &hexdata)
{
    earbud_vbus_notify_header_t *header = (earbud_vbus_notify_header_t *)hexdata.data();
    unsigned int hex_bytes = (unsigned int)hexdata.count();
    if(hex_bytes >= sizeof(earbud_vbus_notify_header_t)
            && header->race_type == RACE_TYPE_NOTIFY
        ) {
        return true;
    }

    return false;
}

bool earbud_vbus_notify_length_field_is_valid(const QByteArray &hexdata, uint32_t payload_len)
{
    earbud_vbus_notify_header_t *header = (earbud_vbus_notify_header_t *)hexdata.data();
    unsigned int hex_bytes = (unsigned int)hexdata.count();

    if(hex_bytes < sizeof(earbud_vbus_notify_header_t) + payload_len) {
        return false;
    }

    uint32_t vlen = calc_length_little_end(header->vbus_len1, header->vbus_len2);
    uint32_t rlen = calc_length_big_end(header->race_len1, header->race_len2);
    return hex_bytes >= vlen + 4 && vlen == rlen + 4;
}

int earbud_vbus_notify_check_format(const QByteArray &hexdata, uint32_t payload_len, QString &topic, QJsonArray &jsarr) {
    //earbud_vbus_notify_header_t *header = (earbud_vbus_notify_header_t *)hexdata.data();
    if(!earbud_vbus_notify_length_field_is_valid(hexdata, payload_len)) {
        jsarr.append(topic);
        addInfo2Array(jsarr, KEY_STR_EXCEPTION, "回复数据长度错误！", true);
        return RET_FAIL;
    }

    //topic += (EARSIDE_LEFT == header->race_earside) ? " (左)" : " (右)";
    return RET_OK;
}

QString earbud_get_notify_key(const QByteArray &hexdata)
{
    QString key;
    earbud_vbus_notify_header_t *notify = (earbud_vbus_notify_header_t *)hexdata.data();
    key.sprintf("%02X%02X%02X", (uint8_t)notify->race_id1, (uint8_t)notify->race_id2, (uint8_t)notify->race_cmd);
    return key;
}

bool is_rsp_from_earbud(const QByteArray &hexdata)
{
    earbud_vbus_rsp_header_t *header = (earbud_vbus_rsp_header_t *)hexdata.data();
    unsigned int hex_bytes = (unsigned int)hexdata.count();
    if(hex_bytes >= sizeof(earbud_vbus_rsp_header_t) - 1
            && header->race_type == RACE_TYPE_RSP
        ) {
        return true;
    }

    return false;
}

QString earbud_get_rsp_key(const QByteArray &hexdata)
{
    QString key;
    if(10 <= hexdata.count() ) {
        key.sprintf("%02X%02X", (uint8_t)hexdata[8], (uint8_t)hexdata[9]);
    }

    return key;
}

#define EARBUD_CMD_LENGTH_DUT  11

bool is_dut_rsp_from_earbud(const QByteArray &hexdata)
{
    QString dut_str = "FE FC 00 07 05 5B 03 00 01 0A";
    if(hexdata.count() >= EARBUD_CMD_LENGTH_DUT) {
        QString data_str = hexArray2StringPlusSpace(hexdata);
        if(data_str.startsWith(dut_str)) {
            return true;
        }
    }

    return false;
}

QString earbud_get_dut_rsp_key(const QByteArray &hexdata)
{
    Q_UNUSED(hexdata);
    return "010A";
}

void earbud_construct_cmd_header(QByteArray &cmd_arr, uint8_t vlen1, uint8_t vlen2, uint8_t rlen1, uint8_t rlen2, uint8_t race_id1, uint8_t race_id2, uint8_t cmd, uint8_t earside)
{
    earbud_vbus_cmd_header_t header;

    header.vbus_lead1 = EARBUD_LEAD1;
    header.vbus_lead2 = EARBUD_LEAD2;
    header.vbus_len1 = vlen1;
    header.vbus_len2 = vlen2;
    header.race_channel = RACE_CHANNEL;
    header.race_type = RACE_TYPE_CMD;
    header.race_len1 = rlen1;
    header.race_len2 = rlen2;
    header.race_id1 = race_id1;
    header.race_id2 = race_id2;
    header.race_cmd = cmd;
    header.race_earside = earside;

    cmd_arr.append((const char*)&header, sizeof(header));
    //assert(earbud_vbus_cmd_length_field_is_valid(cmd_arr, 0));
}

int earbud_construct_cmd_read_mac(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_MAC, earside);
    return 0;
}

int earbud_parse_notify_read_mac(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "读耳机MAC地址";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_MAC, topic, jsarr)) {
        return RET_FAIL;
    }

    //jsarr.append(topic);
    QJsonObject jsobj;
    QByteArray mac_array;
    for(int i = 0; i < EARBUD_PAYLOAD_LENGTH_MAC; ++i) {
        mac_array.append(hexdata[sizeof(earbud_vbus_notify_header_t) + i]);
    }
    QString mac_str = hexArray2StringPlusSpace(mac_array);
    jsobj.insert(topic, mac_str);
    jsarr.append(jsobj);

    return 0;
}

int earbud_construct_cmd_read_version(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_VERSION, earside);
    return 0;
}

int earbud_parse_notify_read_version(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "读耳机版本";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_VERSION, topic, jsarr)) {
        return RET_FAIL;
    }

    //jsarr.append(topic);
    QJsonObject jsobj;
    QString verStr;
    for(int i = 0; i < EARBUD_PAYLOAD_LENGTH_VERSION; ++i) {
        verStr.append(hexdata[sizeof(earbud_vbus_notify_header_t) + i]);
    }
    jsobj.insert(topic, verStr);
    jsarr.append(jsobj);

    return 0;
}

int earbud_construct_cmd_read_channel(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_CHANNEL, earside);
    return 0;
}

int earbud_parse_notify_read_channel(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "读耳机声道";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_CHANNEL, topic, jsarr)) {
        return RET_FAIL;
    }

    //jsarr.append(topic);
    QJsonObject jsobj;
    jsobj.insert(topic, hexdata[sizeof(earbud_vbus_notify_header_t)] == EARSIDE_LEFT ? "左" : "右");
    jsarr.append(jsobj);

    return 0;
}

int earbud_construct_cmd_read_temperature(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_TEMPERATURE, earside);
    return 0;
}

int earbud_parse_notify_read_temperature(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "读NTC温度";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_TEMPERATURE, topic, jsarr)) {
        return RET_FAIL;
    }

    uint32_t idx = sizeof(earbud_vbus_notify_header_t);
    int temperature = (uint32_t)hexdata[idx] | (uint32_t)hexdata[idx + 1] << 8 | (uint32_t)hexdata[idx + 2] << 16 | (uint32_t)hexdata[idx + 3] << 24;
    QJsonObject jsobj;
    jsobj.insert(topic, temperature);
    jsarr.append(jsobj);

    return 0;
}

int earbud_construct_cmd_set_license_key(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_SET_LICENSE_KEY, earside);
    return 0;
}

int earbud_parse_notify_set_license_key(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "写授权key";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_SET_LICENSE_KEY, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, hexdata[sizeof(earbud_vbus_notify_header_t)] == RET_OK ? VALUE_STR_SUCCESS : VALUE_STR_FAIL);
    jsarr.append(jsobj);

    return 0;
}

int earbud_construct_cmd_get_license_result(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_GET_LICENSE_RESULT, earside);
    return 0;
}

int earbud_parse_notify_get_license_result(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "获取授权结果";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_GET_LICENSE_RESULT, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, hexdata[sizeof(earbud_vbus_notify_header_t)] == RET_OK ? VALUE_STR_SUCCESS : VALUE_STR_FAIL);
    jsarr.append(jsobj);

    return 0;
}

int earbud_construct_cmd_get_license_key(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_GET_LICENSE_KEY, earside);
    return 0;
}

int earbud_parse_notify_get_license_key(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "获取当前授权key";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_GET_LICENSE_KEY, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, hexdata[sizeof(earbud_vbus_notify_header_t)] == RET_OK ? VALUE_STR_SUCCESS : VALUE_STR_FAIL);
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

int earbud_construct_cmd_captouch_start_int(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_CAPTOUCH_START_INT, earside);
    return 0;
}

int earbud_parse_notify_captouch_start_int(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "启动容触中断测试";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_CAPTOUCH_START_INT, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(earbud_vbus_notify_header_t)]));
    jsarr.append(jsobj);

    return 0;
}

int earbud_construct_cmd_captouch_get_int_result(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_CAPTOUCH_GET_INT_RESULT, earside);
    return 0;
}

int earbud_parse_notify_captouch_get_int_result(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "容触中断测试结果";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_CAPTOUCH_GET_INT_RESULT, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(earbud_vbus_notify_header_t)]));
    jsarr.append(jsobj);

    return 0;
}

int earbud_construct_cmd_captouch_read_version(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_CAPTOUCH_READ_VERSION, earside);
    return 0;
}
int earbud_parse_notify_captouch_read_version(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "读容触版本";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_CAPTOUCH_READ_VERSION, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, hexdata[sizeof(earbud_vbus_notify_header_t)] );
    jsarr.append(jsobj);

    return 0;
}

int earbud_construct_cmd_captouch_start_calib(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_CAPTOUCH_START_CALIB, earside);
    return 0;
}
int earbud_parse_notify_captouch_start_calib(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "启动容触校准";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_CAPTOUCH_START_CALIB, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(earbud_vbus_notify_header_t)]));
    jsarr.append(jsobj);

    return 0;
}

int earbud_construct_cmd_captouch_get_calib_result(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_CAPTOUCH_GET_CALIB_RESULT, earside);
    return 0;
}
int earbud_parse_notify_captouch_get_calib_result(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "容触校准结果";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_CAPTOUCH_GET_INT_RESULT, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(earbud_vbus_notify_header_t)]));
    jsarr.append(jsobj);

    return 0;
}

int earbud_construct_cmd_captouch_read_value(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_CAPTOUCH_READ_VALUE, earside);
    return 0;
}
int earbud_parse_notify_captouch_read_value(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "读abov电容值";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_CAPTOUCH_READ_VALUE, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    uint8_t idx = sizeof(earbud_vbus_notify_header_t);
    if(0 != hexdata[idx]) {
        jsobj.insert(topic, "无效值");
        jsarr.append(jsobj);
        return RET_FAIL;
    }

    jsarr.append(topic);
    ++idx;
    uint16_t diff1 = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
    jsobj.insert("按键diff值", diff1);

    idx += 2;
    uint16_t diff2 = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
    jsobj.insert("入耳diff值", diff2);

    idx += 2;
    uint16_t cap1 = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
    jsobj.insert("按键容值", cap1);

    idx += 2;
    uint16_t cap2 = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
    jsobj.insert("入耳容值", cap2);

    idx += 2;
    uint16_t capref = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
    jsobj.insert("参考容值", capref);

    jsarr.append(jsobj);

    return 0;
}

int earbud_construc_cmd_optic_communicate(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_OPTIC_COMMUNICATION, earside);
    return 0;
}

int earbud_parse_notify_optic_communicate(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "光感通信测试";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_OPTIC_COMMUNICATION, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(earbud_vbus_notify_header_t)]));
    jsarr.append(jsobj);

    return 0;
}

int earbud_construc_cmd_optic_int_start(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_OPTIC_INT_START, earside);
    return 0;
}

int earbud_parse_notify_optic_int_start(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "光感INT测试开始";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_OPTIC_INT_START, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(earbud_vbus_notify_header_t)]));
    jsarr.append(jsobj);

    return 0;
}

int earbud_construc_cmd_optic_int_end(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_OPTIC_INT_END, earside);
    return 0;
}

int earbud_parse_notify_optic_int_end(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "光感INT测试结束";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_OPTIC_INT_END, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(earbud_vbus_notify_header_t)]));
    jsarr.append(jsobj);

    return 0;
}

int earbud_construc_cmd_optic_laser_start(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_OPTIC_LASER_START, earside);
    return 0;
}

int earbud_parse_notify_optic_laser_start(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "光感激光测试开始";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_OPTIC_LASER_START, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(earbud_vbus_notify_header_t)]));
    jsarr.append(jsobj);

    return 0;
}

int earbud_construc_cmd_optic_laser_end(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_OPTIC_LASER_END, earside);
    return 0;
}

int earbud_parse_notify_optic_laser_end(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "光感激光测试结束";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_OPTIC_LASER_END, topic, jsarr)) {
        return RET_FAIL;
    }

    jsarr.append(topic);

    QJsonObject jsobj;
    uint8_t idx = sizeof(earbud_vbus_notify_header_t);
    uint16_t v = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
    QString k;
    k.sprintf("PXS = %d", (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8 );
    jsobj.insert(k,  v > 5 ? VALUE_STR_SUCCESS : VALUE_STR_FAIL);
    jsarr.append(jsobj);

    return 0;
}

int earbud_construc_cmd_optic_full_scale(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_OPTIC_FULL_SCALE, earside);
    return 0;
}

int earbud_parse_notify_optic_full_scale(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "光感满量程测试";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_OPTIC_FULL_SCALE, topic, jsarr)) {
        return RET_FAIL;
    }

    jsarr.append(topic);

    QJsonObject jsobj;
    uint8_t idx = sizeof(earbud_vbus_notify_header_t);
    uint16_t v = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
    QString k;
    k.sprintf("PXS = %d", (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8 );
    jsobj.insert(k,  v >= 900 && v <= 1023 ? VALUE_STR_SUCCESS : VALUE_STR_FAIL);
    jsarr.append(jsobj);

    return 0;
}

int earbud_construc_cmd_optic_bg_noise_start(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_OPTIC_BG_NOISE_START, earside);
    return 0;
}

int earbud_parse_notify_optic_bg_noise_start(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "光感底噪测试开始";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_OPTIC_BG_NOISE_START, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(earbud_vbus_notify_header_t)]));
    jsarr.append(jsobj);

    return 0;
}

int earbud_construc_cmd_optic_bg_noise_end(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_OPTIC_BG_NOISE_END, earside);
    return 0;
}

int earbud_parse_notify_optic_bg_noise_end(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "光感底噪测试结束";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_OPTIC_BG_NOISE_END, topic, jsarr)) {
        return RET_FAIL;
    }

    jsarr.append(topic);

    QJsonObject jsobj;
    uint8_t idx = sizeof(earbud_vbus_notify_header_t);
    uint16_t v = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
    QString k;
    k.sprintf("底噪值 = %d", v);
    jsobj.insert(k,  v <= 15 ? VALUE_STR_SUCCESS : VALUE_STR_FAIL );
    jsarr.append(jsobj);

    return 0;
}

int earbud_construc_cmd_force_start_detect(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_FORCE_START_DETECT, earside);
    return 0;
}

int earbud_parse_notify_force_start_detect(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "钛深启动压感检测";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_FORCE_START_DETECT, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(earbud_vbus_notify_header_t)]));
    jsarr.append(jsobj);

    return RET_OK;
}

int earbud_construc_cmd_force_get_fw_ver(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_FORCE_GET_FW_VER, earside);
    return 0;
}

int earbud_parse_notify_force_get_fw_ver(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "钛深获取压感固件版本号";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_FORCE_GET_FW_VER, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, hexdata[sizeof(earbud_vbus_notify_header_t)] );
    jsarr.append(jsobj);

    return RET_OK;
}

int earbud_construc_cmd_force_get_assemble(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_FORCE_GET_ASSEMBLE, earside);
    return 0;
}

int earbud_parse_notify_force_get_assemble(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "钛深获取压感装配情况";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_FORCE_GET_ASSEMBLE, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    QString v_str;
    switch (hexdata[sizeof(earbud_vbus_notify_header_t)]) {
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

int earbud_construc_cmd_force_get_noise_peak(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_FORCE_GET_NOISE_PEAK, earside);
    return 0;
}

int earbud_parse_notify_force_get_noise_peak(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "钛深获取压感噪声峰值";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_FORCE_GET_NOISE_PEAK, topic, jsarr)) {
        return RET_FAIL;
    }

    uint8_t idx = sizeof(earbud_vbus_notify_header_t);
    uint16_t v = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
    QJsonObject jsobj;
    jsobj.insert(topic, v);
    jsarr.append(jsobj);

    return RET_OK;
}

int earbud_construc_cmd_force_get_burst_pressure(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_FORCE_GET_BURST_PRESSURE, earside);
    return 0;
}

int earbud_parse_notify_force_get_burst_pressure(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "钛深获取压感按压突发值";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_FORCE_GET_BURST_PRESSURE, topic, jsarr)) {
        return RET_FAIL;
    }

    uint8_t idx = sizeof(earbud_vbus_notify_header_t);
    uint16_t v = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
    QJsonObject jsobj;
    jsobj.insert(topic, v);
    jsarr.append(jsobj);

    return RET_OK;
}

int earbud_construc_cmd_force_get_semph(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_FORCE_GET_SEMPH, earside);
    return 0;
}

int earbud_parse_notify_force_get_semph(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "钛深获取压感按压信号量";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_FORCE_GET_SEMPH, topic, jsarr)) {
        return RET_FAIL;
    }

    uint8_t idx = sizeof(earbud_vbus_notify_header_t);
    uint16_t v = (uint8_t)hexdata[idx] | (uint8_t)hexdata[idx + 1] << 8;
    QJsonObject jsobj;
    jsobj.insert(topic, v);
    jsarr.append(jsobj);

    return RET_OK;
}

int earbud_construc_cmd_enter_age_mode(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 9, 0, 5, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_ENTER_AGE_MODE, earside);
    cmd.append(0x01);
    return RET_OK;
}

int earbud_parse_notify_enter_age_mode(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "进入老化模式";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_ENTER_AGE_MODE, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, earbud_get_result_string(hexdata[sizeof(earbud_vbus_notify_header_t)]));
    jsarr.append(jsobj);

    return RET_OK;
}

#define EARBUD_CMD_LENGTH_CHGBOX_ENTER_COM_MODE    9
#define EARBUD_NOTIFY_LENGTH_CHGBOX_ENTER_COM_MODE    9
int earbud_construc_cmd_chgbox_enter_com_mode(QByteArray &cmd, uint8_t earside)
{
    Q_UNUSED(earside);
    QString str = "FE F0 01 00 00 00 00 00 00";
    if(RET_FAIL == string2HexArray(str, cmd) ) {
        return RET_FAIL;
    }

    return RET_OK;
}

int earbud_parse_notify_chgbox_enter_com_mode(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "盒子进入通信模式";
    if(EARBUD_NOTIFY_LENGTH_CHGBOX_ENTER_COM_MODE > hexdata.count() ) {
        jsarr.append(topic);
        addInfo2Array(jsarr, KEY_STR_EXCEPTION, "回复数据长度错误！", true);
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, ((uint8_t)hexdata[0] == 0xFE && (uint8_t)hexdata[1] == 0x0F && (uint8_t)hexdata[4] == 0x01) ? VALUE_STR_SUCCESS :VALUE_STR_FAIL );
    jsarr.append(jsobj);

    return RET_OK;
}

//#define EARBUD_CMD_LENGTH_CHGBOX_EXIT_COM_MODE    9
#define EARBUD_NOTIFY_LENGTH_CHGBOX_EXIT_COM_MODE    9

int earbud_construc_cmd_chgbox_exit_com_mode(QByteArray &cmd, uint8_t earside)
{
    Q_UNUSED(earside);
    QString str = "FE F0 00 00 00 00 00 00 00";
    if(RET_FAIL == string2HexArray(str, cmd) ) {
        return RET_FAIL;
    }

    return RET_OK;
}

int earbud_parse_notify_chgbox_exit_com_mode(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "盒子退出通信模式";
    if(EARBUD_NOTIFY_LENGTH_CHGBOX_EXIT_COM_MODE > hexdata.count() ) {
        jsarr.append(topic);
        addInfo2Array(jsarr, KEY_STR_EXCEPTION, "回复数据长度错误！", true);
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, ((uint8_t)hexdata[0] == 0xFE && (uint8_t)hexdata[1] == 0x0F && (uint8_t)hexdata[4] == 0x0) ? VALUE_STR_SUCCESS :VALUE_STR_FAIL );
    jsarr.append(jsobj);

    return RET_OK;
}

bool is_notify_from_earbud_chgbox_com_mode(const QByteArray &hexdata)
{
    if( EARBUD_NOTIFY_LENGTH_CHGBOX_EXIT_COM_MODE <= hexdata.count() && 0xFE == (uint8_t)hexdata[0] && 0x0F == (uint8_t)hexdata[1] ) {
        return true;
    }
    return false;
}

QString earbud_chgbox_com_mode_get_notify_key(const QByteArray &hexdata)
{
    QString key;
    if(EARBUD_NOTIFY_LENGTH_CHGBOX_EXIT_COM_MODE <= hexdata.count() ) {
        key.sprintf("%02X%02X%02X", (uint8_t)hexdata[0], (uint8_t)hexdata[1], (uint8_t)hexdata[4] );
    }

    return key;
}

int earbud_construc_cmd_set_vbus_baud_rate(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 9, 5, 0, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_SET_VBUS_BAUD_RATE, earside);

    // "0x05" means 9600 baud rate of vbus.
    cmd.append(0x05);
    return RET_OK;
}

int earbud_parse_notify_set_vbus_baud_rate(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "设置通信波特率";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_SET_VBUS_BAUD_RATE, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, VALUE_STR_SUCCESS);
    jsarr.append(jsobj);

    return RET_OK;
}

int earbud_construc_cmd_enter_standby(QByteArray &cmd, uint8_t earside)
{
    earside = 0;
    earbud_construct_cmd_header(cmd, 0, 8, 4, 0, 0X01, 0X11, EARBUD_CMD_ENTER_STANDBY, earside);
    return RET_OK;
}

int earbud_construc_cmd_restart(QByteArray &cmd, uint8_t earside)
{
    earside = 0;
    earbud_construct_cmd_header(cmd, 0, 8, 4, 0, 0X01, 0X11, EARBUD_CMD_RESTART, earside);
    return RET_OK;
}


#define EARBUD_CMD_LENGTH_STANDBY_OR_RESTART  11
int earbud_parse_rsp_standby_or_restart(const QByteArray hexdata, QJsonArray &jsarr)
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

int earbud_construc_cmd_power_off(QByteArray &cmd, uint8_t earside)
{
    Q_UNUSED(earside);
    QString str = "FE FC 00 07 05 5A 03 00 27 00 01";
    if(RET_FAIL == string2HexArray(str, cmd) ) {
        return RET_FAIL;
    }

    return RET_OK;
}

int earbud_parse_rsp_power_off(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "关机";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_POWER_OFF, topic, jsarr)) {
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, VALUE_STR_SUCCESS);
    jsarr.append(jsobj);
    return RET_OK;
}

int earbud_construc_cmd_enter_dut(QByteArray &cmd, uint8_t earside)
{
    Q_UNUSED(earside);
    QString str = "FE FC 00 09 05 5A 05 00 01 0A E3 F2 01";
    if(RET_FAIL == string2HexArray(str, cmd) ) {
        return RET_FAIL;
    }

    return RET_OK;
}

int earbud_construc_cmd_exit_dut(QByteArray &cmd, uint8_t earside)
{
    Q_UNUSED(earside);
    QString str = "FE FC 00 09 05 5A 05 00 01 0A E3 F2 00";
    if(RET_FAIL == string2HexArray(str, cmd) ) {
        return RET_FAIL;
    }

    return RET_OK;
}

int earbud_parse_rsp_enter_exit_dut(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "进入/退出DUT";
    if(hexdata.count() < EARBUD_CMD_LENGTH_DUT) {
        jsarr.append(topic);
        addInfo2Array(jsarr, KEY_STR_EXCEPTION, "回复数据长度错误！", true);
        return RET_FAIL;
    }

    QJsonObject jsobj;
    jsobj.insert(topic, hexdata[EARBUD_CMD_LENGTH_DUT - 1] == 0? VALUE_STR_SUCCESS : VALUE_STR_FAIL );
    jsarr.append(jsobj);
    return RET_OK;
}

int earbud_construc_cmd_read_gsensor(QByteArray &cmd, uint8_t earside)
{
    Q_UNUSED(earside);
    QString str = "FE FC 00 06 05 5A 02 00 01 00";
    if(RET_FAIL == string2HexArray(str, cmd) ) {
        return RET_FAIL;
    }

    return RET_OK;
}
#define EARBUD_RSP_LENGTH_READ_GSENSOR  18
int earbud_parse_rsp_read_gsensor(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "读G-Sensor数据";
    if(hexdata.count() < EARBUD_RSP_LENGTH_READ_GSENSOR) {
        jsarr.append(topic);
        addInfo2Array(jsarr, KEY_STR_EXCEPTION, "回复数据长度错误！", true);
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

int earbud_construc_cmd_read_bat_power(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd_header(cmd, 0, 9, 0, 5, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_BAT_POWER, earside);
    cmd.append(0x64);
    return RET_OK;
}
int earbud_parse_notify_read_bat_power(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "电量";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_LENGTH_READ_BAT_POWER, topic, jsarr)) {
        return RET_FAIL;
    }

    uint8_t idx = sizeof(earbud_vbus_notify_header_t);
    QString v;
    v.sprintf("%%%02d", (uint8_t)hexdata[idx]);
    QJsonObject jsobj;
    jsobj.insert(topic, v);
    jsarr.append(jsobj);

    return RET_OK;
}

