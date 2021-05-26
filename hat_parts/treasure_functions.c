#pragma strict_types

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

// ------------------------------------------------------- Implementation ---

void hatcheck_treasure(object o) {
  int value, weight;

  value = (int)o->query_value();
  weight = (int)o->query_weight();

  if(!weight) {
    if(value)
      report(o, "All items of value must be 1 or more weight.");
    if(!value)
      inform(o, "This is weightless, if that is intended it will need special approval.");
  }

  if(weight && !value)
    report(o, "All items with weight must have some value.");

  check_name(o, 0);
  check_short(o, TEXT_CHECK_LIMITS, ITEM_SHORT_LIMITS);
  check_long(o, TEXT_CHECK_LIMITS, ITEM_LONG_LIMITS);
  check_identify(o, 0);
  check_set_sense(o, "search", TEXT_NOT_MANDATORY);
  check_set_sense(o, "smell", TEXT_NOT_MANDATORY);
  check_set_sense(o, "sound", TEXT_NOT_MANDATORY);
  check_set_sense(o, "taste", TEXT_NOT_MANDATORY);
  check_set_sense(o, "touch", 0);
  check_add_senses(o, 0);
  check_material(o, 1);
}
