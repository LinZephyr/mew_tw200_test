#include "chgbox.h"
#include "utils/utils.h"

#include <QtDebug>
#include <QJsonArray>

#define CHG_BOX_DEBUG

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

int parse_chgbox_basic_ft_rsp(const QByteArray hexdata, QJsonObject &jsobj, QString &str)
{
    //Q_UNUSED(hexdata);
#ifdef CHG_BOX_DEBUG
    //qDebug() << "sizeof chgbox_basic_ft_rsp_t : " << sizeof (chgbox_basic_ft_rsp_t);
    //qDebug("%s, hexdata size=%d", __FUNCTION__, hexdata.count() );
#endif
    if(CHGBOX_BASIC_FT_RSP_LEN > hexdata.count()) {
        qWarning() << "????????????????????????????????????????????????";
        return RET_FAIL;
    }

    chgbox_basic_ft_rsp_t rsp;
    QJsonArray jarr;

    memcpy((void *)&rsp, (void*)hexdata.data(), CHGBOX_BASIC_FT_RSP_LEN);
    str.clear();
    str.append("??????????????? : \n{");

    {
        QString key = "LED1";
        QString val = 1 == rsp.D0.bits.led1 ? "ON" : "OFF";
        QJsonObject tmpobj;
        tmpobj.insert(key, val);
        jarr.append(tmpobj);
        str.append("\t" + key + " ??? " + val + "\n");
    }

    {
        QString key = "LED2";
        QString val = 1 == rsp.D0.bits.led2 ? "ON" : "OFF";
        QJsonObject tmpobj;
        tmpobj.insert(key, val);
        jarr.append(tmpobj);
        str.append("\t" + key + " ??? " + val + "\n");
    }

    {
        QString key = "LED3";
        QString val = 1 == rsp.D0.bits.led3 ? "ON" : "OFF";
        QJsonObject tmpobj;
        tmpobj.insert(key, val);
        jarr.append(tmpobj);
        str.append("\t" + key + " ??? " + val + "\n");
    }

    {
        QString key = "HALL";
        QString val = 1 == rsp.D0.bits.hall ? "ON" : "OFF";
        QJsonObject tmpobj;
        tmpobj.insert(key, val);
        jarr.append(tmpobj);
        str.append("\t" + key + " ??? " + val + "\n");
    }

    {
        QString key = "NTC(mV)";
        uint16_t val = (uint16_t)rsp.NTC_part2 | ((uint16_t)rsp.NTC_part1 << 8);
        QJsonObject tmpobj;
        tmpobj.insert(key, val);
        jarr.append(tmpobj);
        str.append("\t" + key + " ??? " + QString::number(val) + "\n");
    }

    {
        QString key = "????????????(mV)";
        uint16_t val = (uint16_t)rsp.bat_vol_part2 | ((uint16_t)rsp.bat_vol_part1 << 8);
        QJsonObject tmpobj;
        tmpobj.insert(key, val);
        jarr.append(tmpobj);
        str.append("\t" + key + " ??? " + QString::number(val) + "\n");
    }

    {
        QString key = "??????????????????(mA)";
        uint16_t val = rsp.box_chg_cur * 10;
        QJsonObject tmpobj;
        tmpobj.insert(key, val);
        jarr.append(tmpobj);
        str.append("\t" + key + " ??? " + QString::number(val) + "\n");
    }

    {
        QString key = "????????????(mA)";
        uint16_t val = rsp.l_earbud_cur;
        QJsonObject tmpobj;
        tmpobj.insert(key, val);
        jarr.append(tmpobj);
        str.append("\t" + key + " ??? " + QString::number(val) + "\n");
    }

    {
        QString key = "????????????(mA)";
        uint16_t val = rsp.r_earbud_cur;
        QJsonObject tmpobj;
        tmpobj.insert(key, val);
        jarr.append(tmpobj);
        str.append("\t" + key + " ??? " + QString::number(val) + "\n");
    }

    {
        QString key = "???????????????";
        uint16_t val = rsp.fw_ver;
        QJsonObject tmpobj;
        tmpobj.insert(key, val);
        jarr.append(tmpobj);
        str.append("\t" + key + " ??? " + QString::number(val) + "\n");
    }

    str.append("}\n");

    jsobj.insert("???????????????", jarr);

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

int construct_chgbox_ft_w_sn_cmd(QByteArray SN, QByteArray &hexcmd)
{
    if(CHGBOX_FT_SN_LEN > SN.count()) {
        qWarning() << "???????????????SN???????????????";
        return RET_FAIL;
    }

    hexcmd.append(CHGBOX_FT_CMD_LEAD);
    hexcmd.append(CHGBOX_FT_W_SN_CMD_FC);
    hexcmd.append(SN);
    hexcmd.append(CRC8(0, (uint8_t *)hexcmd.data(), CHGBOX_FT_W_SN_CMD_LEN - 1));

    return RET_OK;
}

int parse_chgbox_ft_w_sn_rsp(const QByteArray hexdata, QJsonObject &jsobj, QString &str)
{
    if(CHGBOX_FT_W_SN_RSP_LEN > hexdata.count()) {
        qWarning() << "??????????????????SN???????????????????????????";
        return RET_FAIL;
    }

    chgbox_ft_w_sn_rsp_t rsp;
    QJsonArray jarr;

    memcpy((void *)&rsp, (void*)hexdata.data(), CHGBOX_FT_W_SN_RSP_LEN);
    str.clear();
    str.append("??????????????????SN : \n{");

    {
        QString key = "????????????";
        QString val;
        switch(rsp.SN[0]) {
            case BAT_BRAND_DEFAULT:
                val = "??????";
                break;
            case BAT_BRAND_PENGHUI:
                val = "??????";
                break;
            case BAT_BRAND_YIWEI:
                val = "??????";
                break;
            case BAT_BRAND_LISHEN:
                val = "??????";
                break;
            default:
                val = "??????";
                break;
        }
        QJsonObject tmpobj;
        tmpobj.insert(key, val);
        jarr.append(tmpobj);
        str.append("\t" + key + " ??? " + val + "\n");
    }

    {
        QString key = "????????????";
        QString val;
        switch(rsp.SN[1]) {
            case MANUFACTURE_LIANYUN:
                val = "L";
                break;
            case MANUFACTURE_H:
                val = "H";
                break;
            case MANUFACTURE_I:
                val = "I";
                break;
            case MANUFACTURE_MEW:
                val = "?????????";
                break;
            default:
                val = "?????????";
                break;
        }
        QJsonObject tmpobj;
        tmpobj.insert(key, val);
        jarr.append(tmpobj);
        str.append("\t" + key + " ??? " + val + "\n");
    }

    {
        QString key = "SN";
        QByteArray tmparr(hexdata.data() + 2, 10);
        QString val = hexArray2String(tmparr);

        QJsonObject tmpobj;
        tmpobj.insert(key, val);
        jarr.append(tmpobj);
        str.append("\t" + key + " ??? " + val + "\n");
    }

    str.append("}\n");

    jsobj.insert("??????????????????SN", jarr);

    return RET_OK;
}


