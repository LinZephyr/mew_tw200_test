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

#define EARBUD_PAYLOAD_MAC_LENGTH 6
#define EARBUD_PAYLOAD_VERSION_LENGTH 7
#define EARBUD_PAYLOAD_CHANNEL_LENGTH 1

#define EARBUD_CMD_READ_MAC 0x2E
#define EARBUD_CMD_READ_VERSION 0x23
#define EARBUD_CMD_READ_CHANNEL 0x69



QString earbud_make_key(uint8_t usr_id1, uint8_t usr_id2, uint8_t cmd);
static parse_func_list_t earbud_parse_func_list = {
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_MAC), earbud_parse_read_mac_notify},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_VERSION), earbud_parse_read_version_notify},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_CHANNEL), earbud_parse_read_channel_notify},
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
        addInfo2Array(jsarr, ERR_KEY_STR, "回复数据长度检查错误！", true);
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

int earbud_construct_read_mac_cmd(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_MAC, earside);
    return 0;
}

int earbud_parse_read_mac_notify(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "读耳机MAC地址";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_MAC_LENGTH, topic, jsarr)) {
        return RET_FAIL;
    }

    jsarr.append(topic);
    QJsonObject jsobj;
    QByteArray mac_array;
    for(int i = 0; i < EARBUD_PAYLOAD_MAC_LENGTH; ++i) {
        mac_array.append(hexdata[sizeof(earbud_vbus_notify_header_t) + i]);
    }
    QString mac_str = hexArray2StringPlusSpace(mac_array);
    jsobj.insert("MAC", mac_str);
    jsarr.append(jsobj);

    return 0;
}

int earbud_construct_read_version_cmd(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_VERSION, earside);
    return 0;
}

int earbud_parse_read_version_notify(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "读耳机版本";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_VERSION_LENGTH, topic, jsarr)) {
        return RET_FAIL;
    }

    jsarr.append(topic);
    QJsonObject jsobj;
    QString verStr;
    for(int i = 0; i < EARBUD_PAYLOAD_VERSION_LENGTH; ++i) {
        verStr.append(hexdata[sizeof(earbud_vbus_notify_header_t) + i]);
    }
    jsobj.insert("耳机版本", verStr);
    jsarr.append(jsobj);

    return 0;
}

int earbud_construct_read_channel_cmd(QByteArray &cmd, uint8_t earside)
{
    earbud_construct_cmd(cmd, 0, 8, 0, 4, RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_CHANNEL, earside);
    return 0;
}

int earbud_parse_read_channel_notify(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "读耳机声道";
    if(RET_FAIL == earbud_vbus_notify_check_format(hexdata, EARBUD_PAYLOAD_CHANNEL_LENGTH, topic, jsarr)) {
        return RET_FAIL;
    }

    jsarr.append(topic);
    QJsonObject jsobj;
    jsobj.insert("耳机声道", hexdata[sizeof(earbud_vbus_notify_header_t)] == EARSIDE_LEFT ? "左" : "右");
    jsarr.append(jsobj);

    return 0;
}










