#ifndef MEW_TYPE_DEFINE_H
#define MEW_TYPE_DEFINE_H
#include <QString>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>

#define RET_OK 0
#define RET_FAIL -1

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

typedef QString func_uuid_t;
typedef int (*parse_rsp_data_func_t)(const QByteArray hexdata, QJsonArray &jsarr);
typedef QMap<func_uuid_t, parse_rsp_data_func_t> parse_func_map_t;
typedef std::pair<func_uuid_t, parse_rsp_data_func_t> parse_func_pair_t;
typedef std::initializer_list<parse_func_pair_t> parse_func_list_t;

//#define MAKE_STRING_UINT8_UINT8(U8_1, U8_2) QString::number(((uint16_t)U8_1 << 8)|(uint16_t)U8_2, 16)

#define ERR_KEY_STR "异常"
#define addInfo2Array(jsarr, k, v, print) do \
    {\
        QJsonObject tmpobj;\
        tmpobj.insert(k, v);\
        jsarr.append(tmpobj);\
        if(print) {\
            qWarning() << tmpobj;\
        }\
    }\
while(0)

#endif // MEW_TYPE_DEFINE_H
