#ifndef MEW_TYPE_DEFINE_H
#define MEW_TYPE_DEFINE_H
#include <QString>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

//#define DUMP_THREAD_ID

#define RET_OK 0
#define RET_FAIL -1

//typedef unsigned char  uint8_t;
//typedef unsigned short uint16_t;
//typedef unsigned int   uint32_t;

typedef QString func_uuid_t;
typedef int (*parse_rsp_data_func_t)(const QByteArray hexdata, QJsonArray &jsarr);
typedef QMap<func_uuid_t, parse_rsp_data_func_t> parse_func_map_t;
typedef std::pair<func_uuid_t, parse_rsp_data_func_t> parse_func_pair_t;
typedef std::initializer_list<parse_func_pair_t> parse_func_list_t;

//#define MAKE_STRING_UINT8_UINT8(U8_1, U8_2) QString::number(((uint16_t)U8_1 << 8)|(uint16_t)U8_2, 16)

#define KEY_STR_EXCEPTION  "异常"
#define VALUE_STR_SUCCESS  "成功"
#define VALUE_STR_FAIL     "失败"
#define VALUE_STR_TESTING   "测试中..."

#define VALUE_STR_LENGTH_NOT_ENOUGH "回复数据长度不够"

#define EARSIDE_LEFT 0
#define EARSIDE_RIGHT 1

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
