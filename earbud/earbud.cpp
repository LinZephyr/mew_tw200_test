#include "earbud.h"
#include "mew_type_define.h"
#include "utils/utils.h"

#include <QDebug>

int init_earbud_parse_func_map(parse_func_map_t &map)
{
    parse_func_list_t* func_arr[3] = {
        get_1wire_parse_func_list(),
        get_commu_mode_parse_func_list(),
        get_race_parse_func_list()
    };

    for(int i = 0; i < 3; ++i) {
        parse_func_list_t *list = func_arr[i];
        for(parse_func_list_t::const_iterator it = list->begin(); it != list->end(); ++it) {
            if(map.find(it->first) != map.end()) {
                QString str;
                str.sprintf("[%s] duplicate map key: %s", __FUNCTION__, it->first.toStdString().data());
                qWarning() << str;
            }
            map.insert(it->first, it->second);
        }
    }

    return 0;
}













