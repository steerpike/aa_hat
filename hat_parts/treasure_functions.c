#pragma strict_types

#include "../hat_def.h"

// ------------------------------------------------------- Implementation ---

void hatcheck_treasure(object o) {
  int value, weight;

  value = (int)o->query_value();
  weight = (int)o->query_weight();

  if(!weight) {
    if(value)
      report(o, "All items of value must be 1 or more weight.", QC_CHANNEL);
    if(!value)
      report(o, "This is weightless, if that is intended it will need special approval.", QC_CHANNEL);
  }

  if(weight && !value)
    report(o, "All items with weight must have some value.", QC_CHANNEL);

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
