#include "utils.h"
#include "mew_type_define.h"

#include <QStringList>
#include <QJsonObject>
#include <QDebug>

int string2HexArray(const QString &str, QByteArray &hexArr)
{
    bool ok = false;
    unsigned char data = 0xFF;
    QStringList list = str.split(" ");

    for(int i = 0; i < list.count(); i++){
        if(list.at(i) == " ")
            continue;
        if(list.at(i).isEmpty())
            continue;
        data = (char)list.at(i).toInt(&ok, 16);
        if(!ok){
            return RET_FAIL;
        }
        hexArr.append(data);
    }
    return RET_OK;
}

QString hexArray2StringPlusSpace(const QByteArray &hexArr)
{
    QString str;
    for(int i = 0; i < hexArr.count(); i++){
        QString s;
        s.sprintf("%02X ", (uchar)hexArr.at(i));
        str += s;
    }
    return str;
}

QString jsonValue2String(const QJsonValue &jval)
{
    //qDebug() << "v.type = " << jval.type();
    QString str;
    if(jval.type() == QJsonValue::Double ) {
        str = QString::number(jval.toDouble());
    }
    else if(jval.type() == QJsonValue::String) {
        str = jval.toString();
    }

    return str;
}

#define CRC_KEY 7
unsigned char calcCRC8(unsigned char crc8, const unsigned char * ptr, unsigned char len)
{
    unsigned char i;
    while(len--!=0)
    {
        for(i=0x80; i!=0; i/=2)
        {
            if((crc8 & 0x80) != 0)
            {
                crc8 *= 2;
                crc8 ^= CRC_KEY;
            }
            else
                crc8 *= 2;

            if((*ptr & i)!=0)
                crc8 ^= CRC_KEY;
        }
        ptr++;
    }
    return(crc8);
}

bool checkCRC8(const unsigned char crc8, const unsigned char * ptr, unsigned char len)
{
    return crc8 == calcCRC8(0, ptr, len);
}

uint32_t calc_length_little_end(uint32_t len1, uint32_t len2)
{
    return len1 << 8 | len2;
}

uint32_t calc_length_big_end(uint32_t len1, uint32_t len2)
{
    return len2 << 8 | len1;
}







