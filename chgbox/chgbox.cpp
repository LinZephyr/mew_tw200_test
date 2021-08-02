#include "chgbox.h"
#include "utils/utils.h"

#include <QtDebug>
#include <QJsonArray>
#include <QMap>

#define CHG_BOX_DEBUG

static parse_func_list_t chgbox_parse_func_list = {
    {MAKE_STRING_UINT8_UINT8(CHGBOX_FT_RSP_LEAD, CHGBOX_BASIC_FT_RSP_FC),  parse_chgbox_basic_ft_rsp},
    {MAKE_STRING_UINT8_UINT8(CHGBOX_FT_RSP_LEAD, CHGBOX_FT_W_SN_RSP_FC),  parse_chgbox_ft_w_sn_rsp},
    {MAKE_STRING_UINT8_UINT8(CHGBOX_FT_RSP_LEAD, CHGBOX_FT_R_SN_RSP_FC),  parse_chgbox_ft_r_sn_rsp},
};

int chgbox_initialize_parse_func_list(parse_func_map_t &map)
{
    for(parse_func_list_t::const_iterator it = chgbox_parse_func_list.begin(); it != chgbox_parse_func_list.end(); ++it) {
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

typedef struct {
    uint8_t LEAD;
    uint8_t FC;
    union {
        uint8_t byte;
        struct {
            uint8_t led1 : 1;
            uint8_t led2 : 1;
            uint8_t led3 : 1;
            uint8_t chgmode : 1;
            uint8_t resv : 4;
        }bits;
    } D0;
}chgbox_basic_ft_cmd_t;

typedef struct {
    uint8_t LEAD;
    uint8_t FC;
    union {
        uint8_t byte;
        struct {
            uint8_t led1 : 1;
            uint8_t led2 : 1;
            uint8_t led3 : 1;
            uint8_t hall : 1;
            uint8_t key  : 1;
            uint8_t      : 0;
        }bits;
    } D0;
    uint8_t NTC_part1;  // D1
    uint8_t NTC_part2;  // D2
    uint8_t bat_vol_part1;  // D3
    uint8_t bat_vol_part2;  // D4
    uint8_t box_chg_cur;    // D5
    uint8_t l_earbud_cur;   // D6, left
    uint8_t r_earbud_cur;   // D7, right
    uint8_t fw_ver;         // D8

}chgbox_basic_ft_rsp_t;

QByteArray construct_chgbox_basic_ft_cmd(bool led1_on, bool led2_on, bool led3_on, bool chg_mode_wl)
{
#ifdef CHG_BOX_DEBUG
    //qDebug() << "sizeof chgbox_factory_test_cmd_t : " << sizeof (chgbox_factory_test_cmd_t);
#endif
    QByteArray arr;
    chgbox_basic_ft_cmd_t cmd;
    cmd.LEAD = CHGBOX_FT_CMD_LEAD;
    cmd.FC = CHGBOX_BASIC_FT_CMD_FC;

    if(led1_on) {
        cmd.D0.bits.led1 = 1;
    }

    if(led2_on) {
        cmd.D0.bits.led2 = 1;
    }

    if(led3_on) {
        cmd.D0.bits.led3 = 1;
    }

    if(chg_mode_wl) {
        cmd.D0.bits.chgmode = 1;
    }
    cmd.D0.bits.resv = 0;

    arr.append(cmd.LEAD);
    arr.append(cmd.FC);
    arr.append(cmd.D0.byte);

    return arr;

}

int parse_chgbox_basic_ft_rsp(const QByteArray hexdata, QJsonArray &jsarr)
{
    //Q_UNUSED(hexdata);
#ifdef CHG_BOX_DEBUG
    //qDebug() << "sizeof chgbox_basic_ft_rsp_t : " << sizeof (chgbox_basic_ft_rsp_t);
    //qDebug("%s, hexdata size=%d", __FUNCTION__, hexdata.count() );
#endif

    QString k;
    QString v_str;
    uint16_t v_uint;
    chgbox_basic_ft_rsp_t rsp;
    QString topic = "充电仓基本厂测";

    jsarr.append(topic);

    if (CHGBOX_BASIC_FT_RSP_LEN > hexdata.count()) {
        addInfo2Array(jsarr, ERR_KEY_STR, "回复数据长度不够！", true);
        return RET_FAIL;
    }

    memcpy((void *)&rsp, (void*)hexdata.data(), CHGBOX_BASIC_FT_RSP_LEN);

    k = "LED1";
    v_str = 1 == rsp.D0.bits.led1 ? "ON" : "OFF";
    addInfo2Array(jsarr, k, v_str, false);

    k = "LED2";
    v_str = 1 == rsp.D0.bits.led2 ? "ON" : "OFF";
    addInfo2Array(jsarr, k, v_str, false);

    k = "LED3";
    v_str = 1 == rsp.D0.bits.led3 ? "ON" : "OFF";
    addInfo2Array(jsarr, k, v_str, false);

    k = "HALL";
    v_str = 1 == rsp.D0.bits.hall ? "ON" : "OFF";
    addInfo2Array(jsarr, k, v_str, false);

    k = "NTC(mV)";
    v_uint = (uint16_t)rsp.NTC_part2 | ((uint16_t)rsp.NTC_part1 << 8);
    addInfo2Array(jsarr, k, v_uint, false);

    k = "电池电压(mV)";
    v_uint = (uint16_t)rsp.bat_vol_part2 | ((uint16_t)rsp.bat_vol_part1 << 8);
    addInfo2Array(jsarr, k, v_uint, false);

    k = "电池充电电流(mA)";
    v_uint = rsp.box_chg_cur * 10;
    addInfo2Array(jsarr, k, v_uint, false);

    k = "左耳电流(mA)";
    v_uint = rsp.l_earbud_cur;
    addInfo2Array(jsarr, k, v_uint, false);

    k = "右耳电流(mA)";
    v_uint = rsp.r_earbud_cur;
    addInfo2Array(jsarr, k, v_uint, false);

    k =  "程序版本号";
    v_uint = rsp.fw_ver;
    addInfo2Array(jsarr, k, v_uint, false);

    return RET_OK;
}

#define BAT_BRAND_DEFAULT   0xFF
#define BAT_BRAND_PENGHUI   0
#define BAT_BRAND_YIWEI     1
#define BAT_BRAND_LISHEN    2

#define MANUFACTURE_LIANYUN     0x31
#define MANUFACTURE_H           0x32
#define MANUFACTURE_I           0x33
#define MANUFACTURE_MEW         0x34

typedef struct {
    uint8_t lead;
    uint8_t fc;
    uint8_t SN[10];
    uint8_t crc;
} chgbox_ft_w_sn_cmd_t;

typedef struct {
    uint8_t lead;
    uint8_t fc;
    uint8_t SN[10];
    uint8_t crc;
} chgbox_ft_w_sn_rsp_t;

int construct_chgbox_ft_w_sn_cmd(const QByteArray SN, QByteArray &hexcmd)
{
    if(CHGBOX_FT_SN_LEN > SN.count()) {
        qWarning() << "充电仓厂测SN命令长度不够！";
        return RET_FAIL;
    }
    hexcmd.clear();

    hexcmd.append(CHGBOX_FT_CMD_LEAD);
    hexcmd.append(CHGBOX_FT_W_SN_CMD_FC);
    hexcmd.append(SN);
    hexcmd.append(calcCRC8(0, (uint8_t *)hexcmd.data(), CHGBOX_FT_W_SN_CMD_LEN - 1));

    return RET_OK;
}

int chgbox_ft_get_sn(const QByteArray &hexdata, QJsonArray &jsarr, bool rw_flag)
{
    chgbox_ft_w_sn_rsp_t rsp;
    int len = CHGBOX_SN_FLAG_W == rw_flag ? CHGBOX_FT_W_SN_RSP_LEN : CHGBOX_FT_R_SN_RSP_LEN;
    QString k, v;
    uint8_t crc_recv = (uint8_t)(hexdata.data()[len - 1]);
    uint8_t crc_calc = calcCRC8(0, (const uint8_t *)hexdata.data(),  len - 1);

    if(crc_recv != crc_calc) {
        QString errval;
        errval.sprintf("SN CRC8校验码错误！收到CRC8:0x%X, 计算CRC8:0x%X", crc_recv, crc_calc);
        addInfo2Array(jsarr, ERR_KEY_STR, errval, true);
        return RET_FAIL;
    }

    memcpy((void *)&rsp, (void*)hexdata.data(), len);

    k = "电池品牌";
    switch(rsp.SN[0]) {
        case BAT_BRAND_DEFAULT:
            v = "默认";
            break;
        case BAT_BRAND_PENGHUI:
            v = "鹏辉";
            break;
        case BAT_BRAND_YIWEI:
            v = "亿伟";
            break;
        case BAT_BRAND_LISHEN:
            v = "力神";
            break;
        default:
            v = "默认";
            break;
    }
    addInfo2Array(jsarr, k, v, false);

    k = "生产厂家";
    switch(rsp.SN[1]) {
        case MANUFACTURE_LIANYUN:
            v = "L";
            break;
        case MANUFACTURE_H:
            v = "H";
            break;
        case MANUFACTURE_I:
            v = "I";
            break;
        case MANUFACTURE_MEW:
            v = "美恩微";
            break;
        default:
            v = "美恩微";
            break;
    }
    addInfo2Array(jsarr, k, v, false);

    k = "SN";
    QByteArray tmparr(hexdata.data() + 2, CHGBOX_FT_SN_LEN);
    v = hexArray2String(tmparr);
    addInfo2Array(jsarr, k, v, false);

    return RET_OK;
}

int parse_chgbox_ft_w_sn_rsp(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "充电仓厂测写SN";
    jsarr.append(topic);

    if(CHGBOX_FT_W_SN_RSP_LEN > hexdata.count()) {
        addInfo2Array(jsarr, ERR_KEY_STR, "回复数据长度不够！", true);
        return RET_FAIL;
    }

    return chgbox_ft_get_sn(hexdata, jsarr, CHGBOX_SN_FLAG_W);
}

int construct_chgbox_ft_r_sn_cmd(QByteArray &hexcmd)
{
    hexcmd.clear();
    hexcmd.append(CHGBOX_FT_CMD_LEAD);
    hexcmd.append(CHGBOX_FT_R_SN_CMD_FC);
    return RET_OK;
}

int parse_chgbox_ft_r_sn_rsp(const QByteArray hexdata, QJsonArray &jsarr)
{
    QString topic = "充电仓厂测读SN";
    jsarr.append(topic);

    if(CHGBOX_FT_W_SN_RSP_LEN > hexdata.count()) {
        addInfo2Array(jsarr, ERR_KEY_STR, "回复数据长度不够！", true);
        return RET_FAIL;
    }

    return chgbox_ft_get_sn(hexdata, jsarr, CHGBOX_SN_FLAG_W);
}



