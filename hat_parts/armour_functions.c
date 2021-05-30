#pragma strict_types

#include "../hat_def.h"

// -------------------------------------------------- Internal Prototypes ---

int max_ac_for_part(object o, string part);
mixed check_armour_part(object o, string part);

// ------------------------------------------------------- Implementation ---

void hatcheck_armour(object o) {
  object env;
  int typeno, nomajor, weight, expected, allalike_and_anyprotection, i;
  string *special, *types, type;

  special = ({"head", "body", "arm", "leg"});
  for(i=0; i<sizeof(special); i++)
    if(call_other(o, "query_special_"+special[i]+"_ac"))
      report(o, "The set_special_"+special[i]+"_ac function is obsolete.", QC_CHANNEL);

  type = (string) o->query_armour_type();
  types = (string *) o->query_ac_types();
  typeno = sizeof(types);
  weight = (int) o->query_weight();

  env = environment(o);
  if(env && living(env) && env->query_npc() && !o->query_worn())
    report(o, "Armour held by monsters should be worn.", QC_CHANNEL);

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

  if(!typeno) {
    if(type == "armour")
      report(o, "Armour that provides no protection seems unintended.", QC_CHANNEL);
    else if(o->query_weight())
      report(o, "Jewellery with weight needs to grant AC (or some other function).", QC_CHANNEL);
  } else {
    for(i=0;i<typeno;i++)
      allalike_and_anyprotection += check_armour_part(o, types[i]);

    if(typeno > 1) {
      nomajor = 0;
      if(o->query_head_ac())
        nomajor++;
      if(o->query_hbody_ac())
        nomajor++;
      if(o->query_arm_ac())
        nomajor++;
      if(o->query_leg_ac())
        nomajor++;
      if(!nomajor && type == "armour")
        report(o, "Composite armour should cover one of head/hbody/arm/leg.", QC_CHANNEL);
    }

    if(type == "armour") {
      if((string)o->query_armour_name() == "special")
        report(o, "Armour should not use SPECIAL in set_armour (rings and jewellery can).", BALANCE_CHANNEL);
      else if(allalike_and_anyprotection)
        report(o, "The set_armour must come after set_ac values.", QC_CHANNEL);
    } else {
      if((string)o->query_armour_name() != "special")
        report(o, "Jewellery should use SPECIAL in set_armour.", BALANCE_CHANNEL);
      if(!weight && allalike_and_anyprotection)
        report(o, "Weightless jewellery must not provide AC.", BALANCE_CHANNEL);
      if(nomajor)
        report(o, "Jewellery can only provide AC to lighter slots (neck/lbody/foot/hand).", BALANCE_CHANNEL);
      return;
    }

    expected = (int) WANDUS->query_recommended_armour_weight(o);
    if(weight != expected)
      report(o, "Wrong weight ("+weight+"). Expected: "+expected+".", BALANCE_CHANNEL);

    expected = (int) WANDUS->query_recommended_armour_value(o);
    if(expected != -1)
      check_recommended_value(o, expected);
    else
      report(o, "Unable to evaluate with a formula. Check with a Balance member.", BALANCE_CHANNEL);
  }
}

int max_ac_for_part(object o, string part) {
  switch(part) {
    case "neck":
    case "lbody":
    case "hand":
    case "foot":
      return 5;

    case "head":
    case "hbody":
    case "arm":
    case "leg":
      return 15;

    default:
      report(o, "Invalid AC type: "+part+".", QC_CHANNEL);
  }
}

mixed check_armour_part(object o, string part) {
  int ac, max, i, j;
  int *crush, *pierce, *slash, *chop;
  status allalike, anyprotection;
  mapping tohit;
  string type;
  
  ac = (int)call_other(o, "query_"+part+"_ac");
  max = max_ac_for_part(o, part);

  if(ac > max)
    report(o, "Alert! Maximum "+part+" AC is "+max+".", BALANCE_CHANNEL);
  else if((ac == 14 || ac == max) && !o->query_unique())
    report(o, "Alert! This should be unique ("+part+" AC = "+ac+").", BALANCE_CHANNEL);

  type = (string) o->query_armour_name();
  if(!type) {
    report(o, "Did you forget to call set_armour?", QC_CHANNEL);
    return 0;
  }

  i = member_array(type, ({"thin_cloth", "thick_cloth", "padded",
    "soft_leather", "leather", "rf_leather", "stud_leather", "ringmail",
    "scalemail", "chainmail", "splintmail", "bandedmail", "platemail",
    "special"}));
  if(i == -1) {
    report(o, "There is something wrong with set_armour!", QC_CHANNEL);
    return 0;
  }

  tohit = (mapping) o->query_to_hit_armour();
  allalike = 1;
  anyprotection = 0;
  crush = tohit["crush"];
  pierce = tohit["pierce"];
  slash = tohit["slash"];
  chop = tohit["chop"];

  j = member_array(part, ({"head","neck","hbody","lbody","arm","hand","leg","foot"}));

  if(type != "special" &&
  (crush[j]  != pierce[j] ||
   crush[j]  != slash[j]  ||
   crush[j]  != chop[j]   ||
   pierce[j] != slash[j]  ||
   pierce[j] != chop[j]   ||
   slash[j]  != chop[j])) {
    allalike = 0;
  }

  if(crush[j] || pierce[j] || slash[j] || chop[j])
    anyprotection = 1;

  if(!anyprotection)
    report(o, "This armour provides no "+part+" AC, is this intended?", QC_CHANNEL);

  if(allalike)
    return anyprotection;
}
