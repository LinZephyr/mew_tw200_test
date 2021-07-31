#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QByteArray>

int string2HexArray(const QString &str, QByteArray &hexArr);
QString hexArray2String(const QByteArray &hexArr);
unsigned char CRC8(unsigned char crc, unsigned char * ptr, unsigned char len);

#endif // UTILS_H
