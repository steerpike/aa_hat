#pragma strict_types

#include <containers.h>

#include "../hat_def.h"

// -------------------------------------------------- External Prototypes ---

void report(object o, string s);
void inform(object o, string s);
void check_name(object o, int flags);
int check_short(object o, int flags, mapping extra);
void check_long(object o, int flags, mapping extra);
void check_identify(object o, int flags);
void check_set_sense(object o, string thing, int flags);
void check_add_senses(object o, int i);
void check_material(object o, int mandatory);
void check_recommended_value(object o, int rec);

// ------------------------------------------------------- Implementation ---

void hatcheck_container(object o) {
  int capacity, value, expected;

  if(!o->query_weight())
    report(o, "Containers must be 1 or more weight.");

  capacity = (int) o->query_max_weight();
  if(!capacity) {
    report(o, "This container has zero capacity.");
  } else {
    if(!o->query_wearable())
      expected = capacity * CONTAINER_VALUE_NON_WEARABLE;
    else
      expected = capacity * CONTAINER_VALUE_WEARABLE;
    check_recommended_value(o, expected);
  }

  check_name(o, 0);
  check_short(o, TEXT_CHECK_LIMITS, ITEM_SHORT_LIMITS);
  check_long(o, TEXT_EXCEPTION_ENDING_NL | TEXT_CHECK_LIMITS, ITEM_LONG_LIMITS);
  check_identify(o, 0);
  check_set_sense(o, "search", TEXT_NOT_MANDATORY);
  check_set_sense(o, "smell", TEXT_NOT_MANDATORY);
  check_set_sense(o, "sound", TEXT_NOT_MANDATORY);
  check_set_sense(o, "taste", TEXT_NOT_MANDATORY);
  check_set_sense(o, "touch", 0);
  check_add_senses(o, 0);
  check_material(o, 1);
}
