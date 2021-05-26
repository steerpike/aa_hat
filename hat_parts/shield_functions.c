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
void check_recommended_value(object o, int rec);

// ------------------------------------------------------- Implementation ---

void hatcheck_shield(object o) {
  int block, weight, expected, *shield_prices;
  object env;

  shield_prices = ({10, 25, 50, 75, 100, 125, 150, 200, 250, 300, 350, 400, 450, 525, 600, 675, 750, 825, 900, 1000});

  env = environment(o);
  if(env && living(env) && env->query_npc() && !o->query_worn())
    report(o, "Shields held by monsters should be worn.");

  block = (int) o->query_block_ac();
  if(block <= 0)
    report(o, "Shields must have 1 or more block AC.");
  else if(block > 20)
    report(o, "Maximum block AC is 20.");
  else
    check_recommended_value(o, shield_prices[block-1]);

  if(block >= 19 && !o->query_unique())
    report(o, "Shields with greater than 18 block AC must be unique.");

  weight = (int) o->query_weight();
  if(!weight)
    report(o, "Shields must be 1 or more weight.");
  else {
    expected = to_int(to_float(block)/4.0+0.999);
    if(this_object()->abs(expected - weight) >= 2)
      report(o, "Weight ("+weight+") is off. Expected: "+expected+".");
  }

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
