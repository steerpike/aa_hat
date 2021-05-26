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

// -------------------------------------------------- Internal Prototypes ---

void check_weaponrange(object o, string wtype, int weight, int diff, int pac,
  int minw, int maxw, int mindiff, int maxdiff, int minpac, int maxpac);

// ------------------------------------------------------- Implementation ---

int usage_factor(int durability) {
  switch(durability) {
    case 0       : return 4;
    case 1  .. 25: return 5;
    case 26 .. 50: return 6;
    case 51 .. 75: return 7;
    case 76 .. 90: return 8;
    case 91 .. 99: return 9;
    case 100     : return 10;
    default      : return -1;
  }
}

int thrown_weapon_value(object o) {
  int base, balance, damage, usage;

  base = (int)WANDUS->query_recommended_weapon_value(o);
  balance = (int)o->query_thrown_balance();
  damage = (int)o->query_thrown_damage();
  usage = usage_factor((int)o->query_durability());

  return base + balance * damage * usage / 20;
}

void hatcheck_weapon(object o) {
  object env, special;
  string wtype;
  int cursed, wc, pac, weight, rec_weight, expected, diff;

  cursed = (int) o->query_cursed();
  wc = (int) o->query_wc();
  diff = (int) o->query_difficulty();
  pac = (int) o->query_parry_ac();
  weight = (int) o->query_weight();
  wtype = (string) o->query_weapon_type();
  rec_weight = (wc/5)+1;

  env = environment(o);
  if(env && living(env) && env->query_npc() && !o->query_wielded())
    report(o, "Weapons held by monsters should be wielded.");

  if(wc <= 0)
    report(o, "Weapons must have 1 or more damage.");
  else if(pac <= 0)
    report(o, "Weapons must have 1 or more parry.");
  else if(wc >= 19 || pac >= 19) {
    if(!o->query_unique())
      report(o, "Weapons with damage or parry greater than 18 must be unique.");
  } else {
    // TODO probably need to breakoff thrown weapons at some point
    if(member(inherit_list(o), "obj/thrown-weapon.c") != -1) {
      //expected = thrown_weapon_value(o);
      inform(o, "The hat cannot evaluate thrown weapon values currently.");
    } else {
      expected = (int)WANDUS->query_recommended_weapon_value(o);
      check_recommended_value(o, expected);
    }
  }

  if(wc > diff+1)
    report(o, "Weapon difficulty is too low, considering the damage.");
  else if(wc < diff-1)
    report(o, "Weapon difficulty is too high, considering the damage.");

  if(!weight)
    report(o, "Weapons must be 1 or more weight.");
  else if(weight > rec_weight+3)
    report(o, "This weapon is too heavy.");
  else if(weight < rec_weight-3)
    report(o, "This weapon is too light.");

  switch(wtype) {
    case "Longsword":
      check_weaponrange(o, "a longsword",
        weight, diff, pac, 3, 4, 10, 18, 8,19);
      break;

    case "Shortsword":
      check_weaponrange(o, "a shortsword",
        weight, diff, pac, 2, 3, 6, 15, 7, 18);
      break;

    case "Two Handed Sword":
      check_weaponrange(o, "a two handed sword",
        weight, diff, pac, 4, 7, 14, 20, 4, 16);
      break;

    case "Two Handed Axe":
      check_weaponrange(o, "a two handed axe",
        weight, diff, pac, 5, 7, 16, 20, 1, 13);
      break;

    case "Axe":
      check_weaponrange(o, "a one handed axe",
        weight, diff, pac, 2, 4, 8, 16, 1, 13);
      break;

    case "Staff":
      if(member(inherit_list(o), "class/ranger/obj/bowstave.c") != -1)
        check_weaponrange(o, "a bow",
          weight, diff, pac, 1, 3, 6, 16, 1, 7);
      else if(o->query_twohanded())
        check_weaponrange(o, "a two handed staff",
          weight, diff, pac, 5, 6, 10, 15, 5, 17);
      else
        check_weaponrange(o, "a one handed staff",
          weight, diff, pac, 2, 4, 6, 10, 4, 16);
      break;

    case "Club":
      if(o->query_twohanded())
        check_weaponrange(o, "a two handed club",
          weight, diff, pac, 5, 8, 12, 20, 1, 10);
      else
        check_weaponrange(o, "a one handed club",
          weight, diff, pac, 2, 4, 6, 14, 1, 10);
      break;

    case "Curved Blade":
      check_weaponrange(o, "a curved blade",
        weight, diff, pac, 2, 4, 6, 17, 6, 18);
      break;

    case "Rapier":
      check_weaponrange(o, "a rapier",
        weight, diff, pac, 2, 3, 4, 15, 7, 20);
      break;

    case "Polearm":
      check_weaponrange(o, "a polearm",
        weight, diff, pac, 3, 7, 12, 20, 1, 10);
      break;

    case "Knife":
      check_weaponrange(o, "a knife",
        weight, diff, pac, 1, 2, 4, 13, 4, 15);
      break;

    case "Flail":
      check_weaponrange(o, "a flail",
        weight, diff, pac, 2, 4, 7, 17, 3, 14);
      break;

    case "Spear":
      if(o->query_twohanded())
        check_weaponrange(o, "a two handed spear",
          weight, diff, pac, 4, 6, 10, 17, 1, 14);
      else
        check_weaponrange(o, "a one handed spear",
          weight, diff, pac, 2, 3, 5, 15, 3, 13);
      break;

    case "Markmanship":
      check_weaponrange(o, "a markmanship weapon",
        weight, diff, pac, 1, 3, 6, 16, 1, 7);
      break;

    case "Exotic":
      check_weaponrange(o, "an exotic",
        weight, diff, pac, 1, 8, 1, 20, 1, 20);
      break;

    default:
      report(o, "Unknown set_weapon type!");
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

void check_weaponrange(object o, string wtype, int weight, int diff, int pac,
  int minw, int maxw, int mindiff, int maxdiff, int minpac, int maxpac) {

  if(weight < minw)
    report(o, "The minimum weight for "+wtype+" is "+minw+".");
  else if(weight > maxw)
    report(o, "The maximum weight for "+wtype+" is "+maxw+".");

  if(diff < mindiff)
    report(o, "The minimum difficulty for "+wtype+" is "+mindiff+".");
  else if(diff > maxdiff)
    report(o, "The maximum difficulty for "+wtype+" is "+maxdiff+".");

  if(pac < minpac)
    report(o, "The minimum parry class for "+wtype+" is "+minpac+".");
  else if(pac > maxpac)
    report(o, "The maximum parry class for "+wtype+" is "+maxpac+".");
}
