#include "utils.h"
#include <QStringList>

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
            return -1;
        }
        hexArr.append(data);
    }
    return 0;
}

QString hexArray2String(const QByteArray &hexArr)
{
    QString str;
    for(int i = 0; i < hexArr.count(); i++){
        QString s;
        s.sprintf("%02X ", (uchar)hexArr.at(i));
        str += s;
    }
    return str;
}

#define CRC_KEY 7
unsigned char CRC8(unsigned char crc, unsigned char * ptr, unsigned char len)
{
    unsigned char i;
    while(len--!=0)
    {
        for(i=0x80; i!=0; i/=2)
        {
            if((crc & 0x80) != 0)
            {
                crc *= 2;
                crc ^= CRC_KEY;
            }
            else
                crc *= 2;

            if((*ptr & i)!=0)
                crc ^= CRC_KEY;
        }
        ptr++;
    }
    return(crc);
}
