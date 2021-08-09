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
#define RACE_TYPE_NOTIFICATION 0x5D

#define RACE_USR_ID1 0
#define RACE_USR_ID2 0x20
#define RACE_ID_LEN 2

#define EARBUD_MAC_LENGTH 6
#define EARBUD_VERSION_LENGTH 7

#define EARBUD_CMD_READ_MAC 0x2E
#define EARBUD_CMD_READ_VERSION 0x23




QString earbud_make_key(uint8_t usr_id1, uint8_t usr_id2, uint8_t cmd);
static parse_func_list_t earbud_parse_func_list = {
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_MAC), earbud_parse_read_mac_notification},
    {earbud_make_key(RACE_USR_ID1, RACE_USR_ID2, EARBUD_CMD_READ_VERSION), earbud_parse_read_version_notification},
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
typedef earbud_vbus_cmd_header_t earbud_vbus_rsp_header_t;

uint32_t calc_length_little_end(uint32_t len1, uint32_t len2)
{
    return len1 << 8 | len2;
}

uint32_t calc_length_big_end(uint32_t len1, uint32_t len2)
{
    return len2 << 8 | len1;
}


bool earbud_vbus_is_cmd_length_field_valid(const QByteArray &hexdata, uint32_t payload_len)
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

bool earbud_vbus_rsp_length_field_is_valid(const QByteArray &hexdata, uint32_t payload_len)
{
    earbud_vbus_rsp_header_t *header = (earbud_vbus_rsp_header_t *)hexdata.data();
    unsigned int hex_bytes = (unsigned int)hexdata.count();

    if(hex_bytes < sizeof(earbud_vbus_rsp_header_t) + payload_len) {
        return false;
    }

    uint32_t vlen = calc_length_little_end(header->vbus_len1, header->vbus_len2);
    uint32_t rlen = calc_length_big_end(header->race_len1, header->race_len2);
    return hex_bytes >= vlen + 4 && vlen == rlen + 4;
}



QString earbud_get_rsp_key(const QByteArray &hexrsp)
{
    QString key;
    if((uint32_t)hexrsp.count() >= sizeof(earbud_vbus_rsp_header_t)) {
        earbud_vbus_rsp_header_t *rsp = (earbud_vbus_rsp_header_t *)hexrsp.data();
        key.sprintf("%02X%02X%02X", (uint8_t)rsp->race_id1, (uint8_t)rsp->race_id2, (uint8_t)rsp->race_cmd);
    }

    return key;
}

int earbud_construct_read_mac_cmd(QByteArray &cmd, uint8_t earside)
{
    earbud_vbus_cmd_header_t header;

    header.vbus_lead1 = EARBUD_LEAD1;
    header.vbus_lead2 = EARBUD_LEAD2;
    header.vbus_len1 = 0;
    header.vbus_len2 = 8;
    header.race_channel = RACE_CHANNEL;
    header.race_type = RACE_TYPE_CMD;
    header.race_len1 = 0;
    header.race_len2 = 4;
    header.race_id1 = RACE_USR_ID1;
    header.race_id2 = RACE_USR_ID2;
    header.race_cmd = EARBUD_CMD_READ_MAC;
    header.race_earside = earside;

    cmd.append((const char*)&header, sizeof(header));
    assert(earbud_vbus_is_cmd_length_field_valid(cmd, 0));
    return 0;
}

int earbud_parse_read_mac_notification(const QByteArray hexdata, QJsonArray &jsarr)
{
    earbud_vbus_rsp_header_t *header = (earbud_vbus_rsp_header_t *)hexdata.data();
    QString topic = "读mac地址";

    if((uint32_t)hexdata.count() >= sizeof(earbud_vbus_rsp_header_t) && header->race_type != RACE_TYPE_NOTIFICATION) {
        return RET_FAIL;
    }

    if(!earbud_vbus_rsp_length_field_is_valid(hexdata, EARBUD_MAC_LENGTH)) {
        jsarr.append(topic);
        addInfo2Array(jsarr, ERR_KEY_STR, "回复数据长度域错误！", true);
        return RET_FAIL;
    }

    topic += (EARSIDE_LEFT == header->race_earside) ? "读MAC地址__左" : "读mac地址__右";
    jsarr.append(topic);
    QJsonObject jsobj;
    QByteArray mac_array;
    for(int i = 0; i < EARBUD_MAC_LENGTH; ++i) {
        mac_array.append(hexdata[sizeof(earbud_vbus_rsp_header_t) + i]);
    }
    QString mac_str = hexArray2StringPlusSpace(mac_array);
    jsobj.insert("MAC", mac_str);
    jsarr.append(jsobj);


    return 0;
}

int earbud_construct_read_version_cmd(QByteArray &cmd, uint8_t earside)
{
    earbud_vbus_cmd_header_t header;

    header.vbus_lead1 = EARBUD_LEAD1;
    header.vbus_lead2 = EARBUD_LEAD2;
    header.vbus_len1 = 0;
    header.vbus_len2 = 8;
    header.race_channel = RACE_CHANNEL;
    header.race_type = RACE_TYPE_CMD;
    header.race_len1 = 0;
    header.race_len2 = 4;
    header.race_id1 = RACE_USR_ID1;
    header.race_id2 = RACE_USR_ID2;
    header.race_cmd = EARBUD_CMD_READ_VERSION;
    header.race_earside = earside;

    cmd.append((const char*)&header, sizeof(header));
    assert(earbud_vbus_is_cmd_length_field_valid(cmd, 0));
    return 0;
}

int earbud_parse_read_version_notification(const QByteArray hexdata, QJsonArray &jsarr)
{
    earbud_vbus_rsp_header_t *header = (earbud_vbus_rsp_header_t *)hexdata.data();
    QString topic = "读耳机版本";

    if((uint32_t)hexdata.count() >= sizeof(earbud_vbus_rsp_header_t) && header->race_type != RACE_TYPE_NOTIFICATION) {
        return RET_FAIL;
    }

    if(!earbud_vbus_rsp_length_field_is_valid(hexdata, EARBUD_MAC_LENGTH)) {
        jsarr.append(topic);
        addInfo2Array(jsarr, ERR_KEY_STR, "回复数据长度域错误！", true);
        return RET_FAIL;
    }

    topic += (EARSIDE_LEFT == header->race_earside) ? "__左" : "__右";
    jsarr.append(topic);
    QJsonObject jsobj;
    QString verStr;
    for(int i = 0; i < EARBUD_VERSION_LENGTH; ++i) {
        verStr.append(hexdata[sizeof(earbud_vbus_rsp_header_t) + i]);
    }
    jsobj.insert("MAC", verStr);
    jsarr.append(jsobj);

    return 0;
}

