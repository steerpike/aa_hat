#pragma strict_types

#include "../hat_def.h"

// ------------------------------------------------------- Implementation ---

void hatcheck_torch(object o) {
  check_name(o, 0);
  check_short(o, TEXT_CHECK_LIMITS, ITEM_SHORT_LIMITS);
  check_long(o, TEXT_CHECK_LIMITS, ITEM_LONG_LIMITS);
  // /obj/lightsource ends in a \n for query_identify, for some reason.
  check_set_sense(o, "identify", TEXT_EXCEPTION_ENDING_NL);
  check_set_sense(o, "search", TEXT_NOT_MANDATORY);
  check_set_sense(o, "smell", 0);
  check_set_sense(o, "sound", 0);
  check_set_sense(o, "taste", TEXT_NOT_MANDATORY);
  check_set_sense(o, "touch", 0);
  check_add_senses(o, 0);
  check_material(o, 1);
}
