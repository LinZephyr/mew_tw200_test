#ifndef EARBUD_INTERNAL_H
#define EARBUD_INTERNAL_H

#include <QDebug>

#define EARBUD_MSG_LEAD1 0xFE
#define EARBUD_MSG_LEAD2 0xFC

#define EARBUD_MSG_INDICATOR 0x05

#define EARBUD_MSG_TYPE_CMD 0x5A
#define EARBUD_MSG_TYPE_RSP 0x5B
// indication don't need response from the other side.
#define EARBUD_MSG_TYPE_IND 0x5C
#define EARBUD_MSG_TYPE_NOTIFY 0x5D

#define EARBUD_MSG_USR_ID1 0
#define EARBUD_MSG_USR_ID2 0x20
#define EARBUD_MSG_ID_LEN 2


#endif // EARBUD_INTERNAL_H
