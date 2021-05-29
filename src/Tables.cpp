#include "Tables.h"

namespace TS
{
    // ****
    // TODO: DO NOT use global variables?
    // ****
    TPAT_map PAT_map{};
    TPMT_map PMT_map{};
    TPES_map PES_map{};

    TPAT_map& get_PAT_map() { return PAT_map; }
    TPMT_map& get_PMT_map() { return PMT_map; }
    TPES_map& get_PES_map() { return PES_map; }
}
