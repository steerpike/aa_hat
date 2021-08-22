#pragma strict_types

#include <xfun.h>

#include "../hat_def.h"

// -------------------------------------------------- External Prototypes ---

int is_capitalized(string s);
int is_language(string s);

// -------------------------------------------------- Internal Prototypes ---

void check_stat(object o, int lvl, int stat, string statname);
void monster_stat(object o);
void monster_chats(object o);
void monster_spells(object o);
void monster_race(object o);
void monster_ac(object o);
void monster_wc(object o);
void monster_hp(object o);
void monster_align(object o);
void check_name_say(object o);

// ----------------------------------------------------- Global Variables ---

object holder; // TODO how does holder work?
int sum_value, total_value; // TODO how does value work?

// ------------------------------------------------------- Implementation ---

void set_holder(object o) { holder = o; }
object query_holder() { return holder; }

void set_total_value(int i) { total_value = i; }
void add_total_value(int i) { total_value += i; }
int query_total_value() { return total_value; }

void set_sum_value(int i) { sum_value = i; }
void add_sum_value(int i) { sum_value += i; }
int query_sum_value(int i) { return sum_value; }

void hatcheck_monster(object o) {
  int lvl;
  string r_name;

  r_name = (string)o->query_real_name();

  if(!r_name) {
    report(o, "Alert! Nameless monster.", QC_CHANNEL);
    return;
  }
  
  if(sizeof(explode(r_name, " ")) > 2) {
    report(o, "The set_name in monsters may only be one word, see 'man set_cap_name'.", QC_CHANNEL);
    return;
  }

  if(XFUN->query_player_exists(r_name))
    report(o, "There is a player named "+r_name+".", QC_CHANNEL);

  lvl = (int) o->query_level();

  if(lvl < 1 || lvl > 25) {
    report(o, "A level "+lvl+" monster, this is probably unintended.", QC_CHANNEL);
    return;
  }

  monster_wc(o);
  monster_ac(o);
  monster_hp(o);
  //monster_exp(o);
  monster_stat(o);
  monster_align(o);
  monster_spells(o);
  monster_race(o);
  // TODO cycle through persona states and check all their chats
  // and while we're doing that, check their reactions as well,
  // and maybe even their destinations for PATH_D?
  if(o->query_is_persona())
    report(o, "Monster is using persona code, skipping chat checks.", HAT_CHANNEL);
  else
    monster_chats(o);

  check_name_say(o);

  // TODO when using set_cap_name, we run into quirks. I can possibly fix
  // this by adding a get_livings() in the living hashtable
  check_name(o, TEXT_DENY_MULTIPLE_WORDS);
  check_short(o, TEXT_CHECK_LIMITS, MONSTER_SHORT_LIMITS);
  check_long(o, TEXT_CHECK_LIMITS, MONSTER_LONG_LIMITS);
  check_identify(o, TEXT_NOT_MANDATORY);
  check_set_sense(o, "smell", TEXT_NOT_MANDATORY);
  check_set_sense(o, "sound", TEXT_NOT_MANDATORY);
  check_set_sense(o, "taste", TEXT_NOT_MANDATORY);
  check_set_sense(o, "touch", TEXT_NOT_MANDATORY);
  check_add_senses(o, 0);

  set_sum_value(1); // TODO why is this 1?
  set_total_value(0); // TODO how does the value sytem work?
  set_holder(o);
}

 // TODO uhh, use a balance daemon
int *query_treasure_max() {
  return ({0, 25, 50, 100, 150, 200, 250, 300, 350, 400, 450, 550, 650, 750, 800, 900, 1000, 1150, 1300, 1500, 1700, 1950, 2250, 2600, 3000, 3450});
}

void report_ac(object o, int ac, string part, int min, int max, int lvl) {
  report(o, capitalize(part)+" AC ("+ac+") is too "+(ac<min?"low":"high")+" for a level "+lvl+" monster. Expected: "+min+"-"+max+".", BALANCE_CHANNEL);
}

void check_stat(object o, int lvl, int stat, string statname) {
  if(stat < 0)
    report(o, statname+"="+stat+"? This is probably unintended.", BALANCE_CHANNEL);
  else if(stat < lvl-3)
    report(o, statname+"="+stat+" is low for a level "+lvl+" monster.", BALANCE_CHANNEL);
  else if(stat > lvl+3)
    report(o, statname+"="+stat+" is high for a level "+lvl+" monster.", BALANCE_CHANNEL);
}

void monster_stat(object o) {
  int str, dex, intl, con, wis, lvl;

  lvl = (int)o->query_level();

  str = (int) o->query_str();
  dex = (int) o->query_dex();
  intl = (int) o->query_int();
  con = (int) o->query_con();
  wis = (int) o->query_wis();

  if(lvl >= 17 && str == lvl && intl == lvl && con == lvl && dex == lvl && wis == lvl)
    report(o, "Level 17 and above cannot have str/dex/int/con/wis equal.", BALANCE_CHANNEL);

  check_stat(o, lvl, str, "STR");
  check_stat(o, lvl, dex, "DEX");
  check_stat(o, lvl, intl, "INT");
  check_stat(o, lvl, con, "CON");
  check_stat(o, lvl, wis, "WIS");
}

void check_chats(object o, string *chats, int a_chat) {
  int i;
  string chat, language, speech, adj_num;

  for(i=0; i<sizeof(chats); i++) {
    adj_num = (string)XFUN->number_to_adjective_string(i+1);
    chat = chats[i];
    language = 0;
    speech = 0;
    if(sscanf(chat, "%~s %s: %~s", speech) && member(({"says", "asks", "exclaims"}), speech) != -1) {
      report(o, "The "+adj_num+" load_"+(a_chat?"a_":"")+"chat is speech and should use \"@<language>@\" syntax.", QC_CHANNEL);
    } else{ 
      sscanf(chat, "@%s@%s", language, chat);
      if(language && !is_language(language)) {
        if(language == "")
          report(o, "The "+adj_num+" load_"+(a_chat?"a_":"")+"chat language is empty.", QC_CHANNEL);
        else if(language == "common")
          report(o, "The "+adj_num+" load_"+(a_chat?"a_":"")+"chat language \"common\" should be \"common language\".", QC_CHANNEL);
        else
          report(o, "The "+adj_num+" load_"+(a_chat?"a_":"")+"chat language \""+language+"\" is not supported.", QC_CHANNEL);
      }
    }
    text_check(o, "the "+adj_num+" load_"+(a_chat?"a_":"")+"chat text", chat, TEXT_ALLOW_REDIRECT | TEXT_ALLOW_CONTRACTION);
  }
}

void monster_chats(object o) {
  string *chats, *a_chats;
  int chance;

  chats = (string *) o->query_chats();
  a_chats = (string *) o->query_a_chats();
  chance = (int)o->query_chat_chance();

  if(chats) {
    if(chance > 10 || chance < 5)
      report(o, "The chat chance ("+chance+") is "+(chance>10?"high":"low")+". Expected: 5-10.", QC_CHANNEL);
    if(!pointerp(chats))
      report(o, "Error: load_chat is not an array!", QC_CHANNEL);
    else
      check_chats(o, chats, 0);
    if(sizeof(chats) < 3)
      report(o, "Monsters require 3 load_chats.", QC_CHANNEL);
  } else
    report(o, "Monsters require 3 load_chats.", QC_CHANNEL);

  // TODO a_chats are implemented in /obj/monster.c, but should be moved to
  // /lib/monster/chat.c, and at this time add query_a_chat_chance(), so
  // i can access it.
  if(a_chats) {
    if(!pointerp(a_chats))
      report(o, "Error: load_a_chat is not an array!", QC_CHANNEL);
    else
      check_chats(o, a_chats, 1);
    if(sizeof(a_chats) < 3)
      report(o, "Monsters require 3 load_a_chats (unkillable monsters do not need a_chats).", QC_CHANNEL);
  } else
    report(o, "Monsters require 3 load_a_chats (unkillable monsters do not need a_chats).", QC_CHANNEL);
}

void monster_spells(object o) {
  int i, lvl, type, chance, dam, nospells, maxchance, maxdam;
  string adj_num;
  mixed *spells;

  lvl = (int)o->query_level();

  maxchance = ({0,15,15,15,15,15,15,15,15,15,15,20,20,25,25,25,30,35,50,50,55,60,60,70,70,80})[lvl];

  maxdam = ({0,4,9,13,17,21,26,30,35,39,42,37,40,35,37,40,35,35,25,30,30,30,36,32,38,35})[lvl];

  spells = (mixed *) o->query_spells();
  if(spells)
    nospells = sizeof(spells[0]);

  if(!spells && lvl > 16) {
    report(o, "Level 17 and above must have an add_spell.", BALANCE_CHANNEL);
    return;
  }

  for (i=0; i < nospells; i++) {
    adj_num = (string)XFUN->number_to_adjective_string(i+1);
    type = spells[0][i];
    chance = spells[2][i];
    dam = spells[1][i];

    if(member(({"fire", "cold", "poison", "saving", "physical", "other"}), type) == -1)
      report(o, "Unknown spell type ("+type+") on "+adj_num+" spell).", QC_CHANNEL);

    if(chance > maxchance)
      report(o, "Spell chance too high ("+chance+") on "+adj_num+" spell. Maximum: "+maxchance+".", BALANCE_CHANNEL);

    if(chance <= 0)
      report(o, "Spell chance is zero on "+adj_num+" spell. This is probably unintended.", QC_CHANNEL);

    if(dam > maxdam)
      report(o, "Spell damage too high ("+dam+") on "+adj_num+" spell. Maximum: "+maxdam+".", BALANCE_CHANNEL);

    if(dam <= 0)
      report(o, "Spell damage is zero on "+adj_num+" spell. This is probably unintended.", QC_CHANNEL);
  }
}

void monster_race(object o) {
  string race, *aliases;

  race = (string) o->query_race();

  if(!race) {
    report(o, "The set_race is missing!", QC_CHANNEL);
    return;
  }

  if(strlen(race) > 15) {
    report(o, "The set_race is too long ("+strlen(race)+" chars). Maximum: 15.", QC_CHANNEL);
    return;
  }

  if(!o->query_gender() && (string)o->query_race() == "human") {
    report(o, "A neuter human? Check the gender and the race here.", QC_CHANNEL);
    return;
  }

  if(!RACE_D->query_known_race(o)) {
    switch(race) {
    case "bird":
      if((string) o->query_arm_str() != "wing" &&
        RACE_D->query_has_wings(o))
        report(o, "Does "+lower_case((string) o->short())+" have wings?", QC_CHANNEL);
      break;
    case "equipment":
    case "pan":
    case "statue":
    case "utensil":
      report(o, capitalize(race)+" race? Use \"animate\".", QC_CHANNEL);
      break;
    case "avian":
    case "buzzard":
      report(o, capitalize(race)+" race? Use \"bird\".", QC_CHANNEL);
      break;
    case "calf":
      report(o, capitalize(race)+" race? Use \"cow\".", QC_CHANNEL);
      break;
    case "drow":
      report(o, capitalize(race)+" race? Use \"dark elf\".", QC_CHANNEL);
      break;
    case "incubus":
    case "succubus":
      report(o, capitalize(race)+" race? Use \"demon\".", QC_CHANNEL);
      break;
    case "earthquaker":
      report(o, capitalize(race)+" race? Use \"golem\".", QC_CHANNEL);
      break;
    case "butterfly":
    case "centipede":
    case "cockroach":
    case "grasshopper":
    case "larva":
    case "tick":
    case "wasp":
      report(o, capitalize(race)+" race? Use \"insect\".", QC_CHANNEL);
      break;
    case "floral":
    case "fungus":
    case "squash":
    case "vegetable":
      report(o, capitalize(race)+" race? Use \"plant\".", QC_CHANNEL);
      break;
    case "banshee":
    case "ghoul":
      report(o, capitalize(race)+" race? Use \"undead\".", QC_CHANNEL);
      break;
    case "animal":
    case "beast":
    case "creature":
    case "feline":
    case "hag":
    case "horror":
    case "humanoid":
    case "mammal":
      report(o, capitalize(race)+" race? That is too broad.", QC_CHANNEL);
      break;
    default:
      report(o, capitalize(race)+" race? Is this intended?", QC_CHANNEL);
      break;
    }
  } else {
    if((int) o->query_arm_ac() != -1 && (string) o->query_arm_str() == "arm" && !RACE_D->query_has_arms(o))
      report(o, "Does "+lower_case((string) o->short())+" have arms?", QC_CHANNEL);
    if((int) o->query_leg_ac() != -1 && (string) o->query_leg_str()  == "leg" && !RACE_D->query_has_legs(o))
      report(o, "Does "+lower_case((string) o->short())+" have legs?", QC_CHANNEL);
    if((string) o->query_arm_str() != "wing" && RACE_D->query_has_wings(o))
    report(o, "Does "+lower_case((string) o->short())+" have wings?", QC_CHANNEL);
  }

  aliases = (string*)o->query_alias();
  if(aliases && sizeof(aliases) && member(aliases, race) != -1)
    report(o, "The race \""+race+"\" does not need to be an alias.", QC_CHANNEL);
}

void monster_ac(object o) {
  int body, head, arm, leg, lvl, min, max;

  lvl = (int)o->query_level();

  // Ideally, we can pull these values from some balance daemon in the future
  min = ({0,3,4,5,6,8,10,11,12,13,14,16,17,18,19,20,21,25,26,27,29,31,34,37,41,45})[lvl];
  max = to_int(to_float(min) * 1.5 + 0.5);

  body = (int) o->query_body_ac();
  if(body != -1) {
    if(body < min || body > max)
      report_ac(o, body, "body", min, max, lvl);
  } else
    report(o, "Body AC is required for combat to work.", QC_CHANNEL);

  head = (int) o->query_head_ac();
  if(head != -1) {
    if(head < min || head > max)
      report_ac(o, head, "head", min, max, lvl);
  }

  arm = (int) o->query_arm_ac();
  if(arm != -1) {
    if(arm < min || arm > max)
      report_ac(o, arm, "arm", min, max, lvl);
  }

  leg = (int) o->query_leg_ac();
  if(leg != -1) {
    if(leg < min || leg > max)
      report_ac(o, leg, "leg", min, max, lvl);
  }

  if(lvl >= 17) {
    if(head != -1 && head == body &&
    arm != -1 && arm == body &&
    leg != 01 && leg == body)
      report(o, "Level 17 and above cannot have the same AC on each slot.", BALANCE_CHANNEL);
  }
}

void monster_wc(object o) {
  int min, max, wc, lvl;

  lvl = (int)o->query_level();

  wc = (int) o->query_wc();

  min = ({0,3,4,5,6,7,9,10,12,13,14,16,17,18,19,20,21,23,24,25,27,29,32,36,41,47})[lvl];

  max = ({0,5,6,8,9,11,14,15,18,20,21,24,26,27,29,30,32,35,36,38,41,44,48,54,62,71})[lvl];

  if(!wc)
    report(o, "Alert! WC of zero, probably not intended.", QC_CHANNEL);
  else if(wc < min || wc > max)
    report(o, "WC ("+wc+") too "+(wc<min?"low":"high")+". Expected: "+(min)+"-"+(max)+".", BALANCE_CHANNEL);
}

void monster_hp(object o) {
  int hp, min, max, lvl;

  lvl = (int)o->query_level();

  min = ({0, 70, 80, 90, 100, 110, 120, 130, 145, 160, 175, 190, 205, 220, 235, 255, 280, 310, 340, 390, 480, 630, 930, 1430, 2430, 4430})[lvl];

  max = to_int(to_float(min) * 1.5 + 0.5);

  hp = (int) o->query_hp();
  if(hp < min || hp > max)
    report(o, (hp<min?"Low":"High")+" hitpoints ("+hp+"). Expected: "+min+"-"+max+".", BALANCE_CHANNEL);
}

/* An exp query on monsters is returning incorrect level 13/14 values. Bug?
void monster_exp(object o) {
  int exp, min, max, lvl;

  lvl = (int)o->query_level();

  min = ({

    })[lvl];

  max = to_int(to_float(min) * 1.5 + 0.5);

  exp = (int) o->query_exp();
  if(exp < min || exp > max)
    report(o, (exp<min?"Low":"High")+" experience ("+exp+"). Expected: "+min+"-"+max+".", BALANCE_CHANNEL);
}
*/

void monster_align(object o) {
  int align, min, max, lvl;

  lvl = (int)o->query_level();
  align = (int) o->query_alignment();
  
  max = lvl*50;
  min = -max;

  if(align < min || align > max)
    report(o, "Alignment ("+align+") is too "+(align<min?"evil":"good")+" for a level "+lvl+" monster. Max: "+(align<min?min:max)+".", BALANCE_CHANNEL);

  if(align <= -500 && !EVILSENSE->query_is_registered(o))
    report(o, "Register with evilsense. Alignment: "+align+" ('man evilsense')", QC_CHANNEL);
}


void check_name_say(object o) {
  if(!intp(o->query_name_say()))
    report(o, "The query_name_say should return 1 rather than a string.", QC_CHANNEL);
}