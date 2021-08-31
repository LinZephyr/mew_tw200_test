#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QByteArray>
#include <QJsonArray>

int string2HexArray(const QString &str, QByteArray &hexArr);
QString hexArray2StringPlusSpace(const QByteArray &hexArr);
QString jsonValue2String(const QJsonValue &jval);

unsigned char calcCRC8(unsigned char crc8, const unsigned char * ptr, unsigned char len);
bool checkCRC8(const unsigned char crc8, const unsigned char * ptr, unsigned char len);

uint32_t calc_length_little_end(uint32_t len1, uint32_t len2);
uint32_t calc_length_big_end(uint32_t len1, uint32_t len2);


#endif // UTILS_H
