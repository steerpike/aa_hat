#pragma strict_types

#include "../hat_def.h"

// -------------------------------------------------- External Prototypes ---

void report(object o, string s, int channel);

// ------------------------------------------------------- Implementation ---

void hatcheck_coins(object o) {
  int mon;
  mon = (int) o->query_coin_value();
  if(mon > 1000)
    report(o, mon+" coins might be too much.", BALANCE_CHANNEL);
}
