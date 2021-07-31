#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QByteArray>

int string2HexArray(const QString &str, QByteArray &hexArr);
QString hexArray2String(const QByteArray &hexArr);
unsigned char calcCRC8(unsigned char crc8, const unsigned char * ptr, unsigned char len);
bool checkCRC8(const unsigned char crc8, const unsigned char * ptr, unsigned char len);
#endif // UTILS_H
