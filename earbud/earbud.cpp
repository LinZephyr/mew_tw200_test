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
    earbud_vbus_notify_header_t *header = (earbud_vbus_notify_header_t *)hexdata.data();
    if(!earbud_vbus_notify_length_field_is_valid(hexdata, payload_len)) {
        jsarr.append(topic);
        addInfo2Array(jsarr, KEY_STR_EXCEPTION, "回复数据长度错误！", true);
        return RET_FAIL;
    }

    topic += (EARSIDE_LEFT == header->race_earside) ? " (左)" : " (右)";
    return RET_OK;
}

QString earbud_get_notify_key(const QByteArray &hexdata)
{
    QString key;
    earbud_vbus_notify_header_t *notify = (earbud_vbus_notify_header_t *)hexdata.data();
    key.sprintf("%02X%02X%02X", (uint8_t)notify->race_id1, (uint8_t)notify->race_id2, (uint8_t)notify->race_cmd);
    return key;
}

void earbud_construct_cmd(QByteArray &cmd_arr, uint8_t vlen1, uint8_t vlen2, uint8_t rlen1, uint8_t rlen2, uint8_t race_id1, uint8_t race_id2, uint8_t cmd, uint8_t earside)
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
    assert(earbud_vbus_cmd_length_field_is_valid(cmd_arr, 0));
}

int earbud_construct_cmd_read_mac(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_MAC, earside);
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
    earbud_construct_cmd(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_VERSION, earside);
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
    earbud_construct_cmd(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_CHANNEL, earside);
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
    earbud_construct_cmd(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_TEMPERATURE, earside);
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
    earbud_construct_cmd(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_SET_LICENSE_KEY, earside);
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
    earbud_construct_cmd(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_GET_LICENSE_RESULT, earside);
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
    earbud_construct_cmd(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_GET_LICENSE_KEY, earside);
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
    earbud_construct_cmd(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_CAPTOUCH_START_INT, earside);
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
    earbud_construct_cmd(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_CAPTOUCH_GET_INT_RESULT, earside);
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
    earbud_construct_cmd(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_CAPTOUCH_READ_VERSION, earside);
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
    earbud_construct_cmd(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_CAPTOUCH_START_CALIB, earside);
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
    earbud_construct_cmd(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_CAPTOUCH_GET_CALIB_RESULT, earside);
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
    earbud_construct_cmd(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_CAPTOUCH_READ_VALUE, earside);
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





















