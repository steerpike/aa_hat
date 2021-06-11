// Official QC tool for checking projects for issues:

#pragma strict_types

#include <daemons.h>
#include <wizards.h>
/*
#include <armour_lib.h>
#include <containers.h>
#include <colour.h>
#include <map.h>
#include <xfun.h>
*/

#include "hat_def.h"

// ---------------------------------------------------------- Inheritance ---

static variables inherit "obj/armour";

static variables inherit (HAT_PARTS + "basic_functions");
static variables inherit (HAT_PARTS + "common_functions");

static variables inherit (HAT_PARTS + "check_armour_functions");
static variables inherit (HAT_PARTS + "check_coins_functions");
static variables inherit (HAT_PARTS + "check_container_functions");
static variables inherit (HAT_PARTS + "check_drink_functions");
static variables inherit (HAT_PARTS + "check_food_functions");
static variables inherit (HAT_PARTS + "check_gem_functions");
static variables inherit (HAT_PARTS + "check_key_functions");
static variables inherit (HAT_PARTS + "check_monster_functions");
static variables inherit (HAT_PARTS + "check_room_functions");
static variables inherit (HAT_PARTS + "check_shield_functions");
static variables inherit (HAT_PARTS + "check_torch_functions");
static variables inherit (HAT_PARTS + "check_treasure_functions");
static variables inherit (HAT_PARTS + "check_weapon_functions");

inherit (HAT_PARTS + "hat_news");

// -------------------------------------------------- External Prototypes ---

int query_total_value();
void hatcheck_item_list(object o, int notify_ok);
void hatcheck_add_thing(object room, int i);
void hatcheck_room_2(object room);
void hatcheck_room_3(object room, mapping items);
void determine_light(object room, object o, mapping items);

void increment_qc_count();
int query_qc_count();

void increment_balance_count();
int query_balance_count();

// -------------------------------------------------- Internal Prototypes ---

void hatcheck_finished();

// ----------------------------------------------------- Global Variables ---

int already, allno, allchecked, hat_visible, hat_light, hat_ansi;
string commands, *all_files;

// ------------------------------------------------------- Implementation ---

void create() {
  armour::create();

  hat_visible = 1;
  hat_light = 1;
  hat_ansi = 1;
  load_news();
  set_light(1);
  set_name("hat");
  set_alias(({"wizard hat", "\nqc_wizard_hat"}));
  set_long("Use 'help hat' to see a list of commands.");

  set_identify("The hat is meant to help people write better code.");

  set_smell("The hat smells magical.");
  set_taste("The hat tastes magical.");
  set_touch("The hat feels magical.");

  set_material("cloth");
}

void init() {
  armour::init();
  hat_news::init();

  if(environment() != this_player())
    return;

  if(this_player()->query_level() < APPRENTICE) {
    write("The hat disappears in a flash of light.\n");
    destruct(this_object());
    return;
  }

  add_action("do_help", "help");
  add_action("do_hatcheck", "hatcheck");
  add_action("do_hatlight", "hatlight");
  add_action("do_hatansi", "hatansi");
  add_action("do_hatshow", "hatshow");
  add_action("do_hatmap", "hatmap");
  add_action("do_hatfind", "hatfind");
  add_action("do_hatstop", "hatstop");
  add_action("do_hatlog", "hatlog");
}

void hatlog_more() {
  this_player()->more(hatlog_file());
}

void hatlog_cat() {
  cat(hatlog_file());
}

void hatlog_clear() {
  rm(hatlog_file());
  rm(hatlog_dir()+"ACCESS");
  rmdir(hatlog_dir());
}

int do_hatlog(string what) {
  if(what == "more") {
    hatlog_more();
    return 1;
  }

  if(what == "cat") {
    hatlog_cat();
    return 1;
  }
  
  if(what == "clear") {
    hatlog_clear();
    write("hatlog cleared!\n");
    return 1;
  }

  notify_fail("'hatlog what?' (more, cat, clear)\n");
  return 0;
}

int do_help(string what) {
  if(what != "hat")
    return 0;

  write("Hat commands:\n" +
    "hatcheck = Searches for possible errors (item/path).\n" +
    "hatlight = Toggles the light of the hat (1 light level).\n" +
    "hatansi  = Toggles ansi colours.\n" +
    "hatshow  = Toggles the visibility of the hat in your inventory.\n" +
    "hatmap   = Create a map of the current area.\n" +
    "hatfind  = Find objects (use a keyword, such as 'trinket').\n" +
    "hatstop  = Cancel a 'hatcheck' in progress.\n" +
    "hatlog   = more, cat, or clear your hatlog.\n");
  return 1;
}

string short() {
  if(!hat_visible)
    return 0;

  if(hat_light)
    return "a wizard hat (glowing)";

  return "a wizard hat";
}

int do_hatlight() {
  if(!hat_light) {
    set_light(1);
    hat_light = 1;
    write("The hat starts to glow.\n");
  } else {
    set_light(-1);
    hat_light = 0;
    write("The hat stops glowing.\n");
  }
  return 1;
}

int do_hatansi() {
  if(!hat_ansi) {
    hat_ansi = 1;
    write("Errors will be highlighted.\n");
  } else {
    hat_ansi = 0;
    write("Errors will not be highlighted.\n");
  }
  return 1;
}

int do_hatmap() {
  if(!AREA_MAP->do_area_map(environment(this_player()), 5, 7, 100, this_player()))
    write("Unable to map the area.\n");
  return 1;
}

int do_hatfind(string s) {
  string *list;
  int i;

  if(!s) {
    notify_fail("Hatfind what?\n");
    return 0;
  }

  list = (string*)FIND_D->find(s);
  if(!sizeof(list)) {
    write("'"+s+"' not found.\n");
    return 1;
  }

  for(i = 0; i < sizeof(list); i++)
    list[i] = capitalize(s) + ": " + list[i] + ((find_object(list[i])) ? " (loaded)" : "");

  this_player()->more(list);
  return 1;
}

// TODO either put this in room_functions or common_functions
mapping basic_check(object room) {
  mapping items;

  if(!room) {
    write("It seems you are nowhere.\n");
    return 0;
  }

  if(!room->query_room()) {
    write("Unable to find your location.\n");
    return 0;
  }

  items = (mapping)room->query_items();

  if(!items || !sizeof(items)) {
    report(room, "The room has no add_senses and these are required.", QC_CHANNEL);
    return ([]);
  }

  return items;
}

void hatcheck_item(object o) {
  string err, *inheritance, fname, mname, *aliases;
  int done, i;
  object master;

  done = already = 0;

  fname = explode(file_name(o), "#")[0];
  if(!format_check_file(o, fname+".c")) {
    call_out("hatcheck_all_files", 1);
    return;
  }

  check_inherits(o);

  err = catch(master = load_object(fname));
  if(err)
    report(o, "Cannot find the master object...", HAT_CHANNEL);
  // TODO add is_clone() check for livings after adding efun find_livings()
  // to simul_efun.c

  if(living(o)) {
    if(o->query_is_player()) {
      call_out("hatcheck_all_files", 1);
      return;
    }
    hatcheck_monster(o);
    hatcheck_item_list(first_inventory(o), 0);
    return;
  }

  if(!o->query_short() && !o->short()) {
    report(o, "Seems to be invisible...skipping.", HAT_CHANNEL);
    call_out("hatcheck_all_files", 1);
    return;
  }

  if(query_sum_value())
    add_total_value((int) o->query_value());

  if(o->query_container()) {
    hatcheck_container(o);
    hatcheck_item_list(first_inventory(o), 0);
    return;
  }

  if(first_inventory(o))
    report(o, "Not living/container, but contains something.", QC_CHANNEL);

  if(o->query_is_armour()) {
    hatcheck_armour(o);
    done = 1;
  }
  if(o->query_is_weapon()) {
    if(explode(file_name(o), "#")[0] == "/obj/torch")
      hatcheck_torch(o);
    else
      hatcheck_weapon(o);
    done = 1;
  }
  if(o->query_is_food()) {
    hatcheck_food(o);
    done = 1;
  }
  if(o->query_is_drink()) {
    hatcheck_drink(o);
    done = 1;
  }
  if(o->query_is_shield()) {
    hatcheck_shield(o);
    done = 1;
  }
  if(o->query_is_coins()) {
    hatcheck_coins(o);
    done = 1;
  }
  if(o->query_is_gem()) {
    hatcheck_gem(o);
    done = 1;
  }
  if(o->query_is_potion())
    done = 1;
  if(!done) {

    inheritance = inherit_list(o);
    for(i=0; i<sizeof(inheritance); i++) {
      if(inheritance[i] == "obj/treasure.c" ||
        inheritance[i] == "obj/rope.c" ||
        inheritance[i] == "obj/book.c") {
        hatcheck_treasure(o);
        done = 1;
      }
      if(inheritance[i] == "obj/key.c") {
        hatcheck_key(o);
        done = 1;
      }
    }
  }

  if(o->short() && !done)
    report(o, "Unrecognized object type...skipping.", HAT_CHANNEL);
  call_out("hatcheck_all_files", 1);
}

// TODO how does this work?
void hatcheck_item_list(object o, int notify_ok) {
  int lvl;

  while(o) {
    if(query_emergency_stop()) {
      write("Hatcheck cancelled.\n");
      stop_busy();
      return;
    }
    if(EVAL_COST_LIMIT) {
      call_out("hatcheck_item_list", 2, o, notify_ok);
      return;
    } else {
      hatcheck_item(o);
      o = next_inventory(o);
    }
  }
  // TODO, port this value stuff into monster_functions.c
  if(query_sum_value()) {
    add_total_value((int)query_holder()->query_money());
    lvl = (int) query_holder()->query_level();
    if(lvl > 0 && lvl < 26) {
      if(query_total_value() > query_treasure_max()[lvl] * 3 / 2)
        report(query_holder(), "Excess treasure (" + query_total_value() +
               "). Level " + lvl + " max is " + (query_treasure_max()[lvl]*3/2) +
               " with the extra 50% allowance.", BALANCE_CHANNEL);
    }
    set_sum_value(0);
  }
  call_out("hatcheck_all_files", 1);
}

void hatcheck_file(string path) {
  object o, o2;
  string err, name;
  int lvl;
  
  lvl = (int)this_player()->query_level();
  name = (string)this_player()->query_real_name();
  
  // TODO pretty flimsy check
  if(lvl < 550 && path[3..strlen(name)+2] != name) {
    writef("You may hatcheck objects, but if you would like to hatcheck "
          "files they must be in your own directory.");
    stop_busy();
    enable_emergency_stop();
    return;
  }

  o = find_object(path);
  if(!o) {
    err = catch(load_object(path));
    if(err) {
      report(0, "Error loading: "+path, ERROR_CHANNEL);
      increment_qc_count();
      call_out("hatcheck_all_files", 1);
      return;
    }
    o = find_object(path);
  }

  if(!o) {
    report(0, "Error locating: "+path, ERROR_CHANNEL);
    increment_qc_count();
    call_out("hatcheck_all_files", 1);
    return;
  }

  if(strstr(lower_case(explode(explode(file_name(o), "#")[0],"/")[<1]), "base") != -1) {
    report(o, "Seems to be some type of base file...skipping.", HAT_CHANNEL);
    call_out("hatcheck_all_files", 1);
    return;
  }
  
  if(o->query_room()) {
    set_reporting_on_object(o);
    hatcheck_room(o);
  } else if(!is_clone(o) && (living(o) || o->query_is_weapon() ||
  o->query_is_armour() || o->query_is_shield() || o->query_is_food() ||
  o->query_is_drink() || o->query_is_gem() ||
  member(inherit_list(o), "obj/treasure.c"))) {
    err = catch(o2 = clone_object(path));
    if(err || !o2) {
      report(o, "Error cloning: "+path+"...skipping\n", HAT_CHANNEL);
      call_out("hatcheck_all_files", 1);
      return;
    }
    else {
      set_reporting_on_object(o2);
      hatcheck_item(o2);
      destruct(o2);
    }
  } else {
    set_reporting_on_object(o);
    hatcheck_item(o);
  }
}

void hatcheck_all_files() {
  int l, i;
  string *files;
  string dir;

  if(find_call_out("hatcheck_all_files") != -1) {
    return;
  }

  while(1) {
    if(allno >= sizeof(all_files)) {
      hatcheck_finished();
      return;
    }
    if(query_emergency_stop()) {
      write("Hatcheck cancelled.\n");
      stop_busy();
      return;
    }
    l = file_size(all_files[allno]);
    if(l == -1) {
      all_files[allno] += "/";
      l = file_size(all_files[allno]);
    }

    if(l == -2) {
      dir = all_files[allno];
      if(dir[<1..<1] != "/")
        dir += "/";
      if(dir == "/w/" || dir == "/d/" || !(dir[0..2] == "/w/" || dir[0..2] == "/d/"))
        writef("Invalid dir for hatcheck: "+dir);
      else {
        files = get_dir(dir, 1);
        for(i=0; i<sizeof(files); i++)
          all_files += ({dir+files[i]});
      }
    } else
      if(l > 0 && strlen(all_files[allno]) > 2) {
        if(all_files[allno][<2..<1] == ".c") {
          allchecked++;
          hatcheck_file(all_files[allno++]);
          return;
        }
      }
    allno++;
  }
}

int do_hatstop() {
  if(is_busy) {
    stop_busy();
    enable_emergency_stop();
    write("Hatcheck stopped.\n");
  } else
    write("No hatcheck in progress to stop.\n");
  return 1;
}

int do_hatcheck(string what) {
  object ob;
  int filesize;
  string wname;

  if(query_busy()) {
    write("Hatcheck is currently busy--use 'hatstop' to cancel.\n");
    return 1;
  }

  reset_variables();

  allno = allchecked = 0;
  all_files = ({});
  start_busy();

  if(!what) {
    all_files = ({explode(file_name(environment(this_player())), "#")[0]+".c"});
    hatcheck_all_files();
  } else {
    ob = present(what, this_player());
    if(!ob)
      ob = present(what, environment(this_player()));
    if(ob) {
      allchecked++;
      set_reporting_on_object(ob);
      hatcheck_item(ob);
    } else {
      what = fix_path(what);
      catch(filesize = file_size(what));
      if(filesize == -2 || filesize > 0) {
        all_files = ({what});
        hatcheck_all_files();
      } else {
        write("Cannot find any "+what+" here.\n");
        stop_busy();
      }
    }
  }
  return 1;
}

void hatcheck_finished() {
  if(!query_qc_count() && !query_balance_count())
    write("Hatcheck complete: no problems detected.\n");
  else {
    if(allchecked) {
      write("Hatcheck complete: checked "+allchecked+" files.\n");
      if(query_qc_count())
        COLOURUTIL_D->write_cf(CHANNELS[QC_CHANNEL]+"Found "+query_qc_count()+" QC issues."+COLOUR_RESET);
      if(query_balance_count())
        COLOURUTIL_D->write_cf(CHANNELS[BALANCE_CHANNEL]+"Found "+query_balance_count()+" Balance issues."+COLOUR_RESET);
    } else
      write("Hatcheck complete: nothing checked--nothing found.\n");
  }
  stop_busy();
}

int do_hatshow() {
  if(!hat_visible) {
    write("The hat turns visible.\n");
    hat_visible = 1;
  } else {
    write("The hat turns invisible.\n");
    hat_visible = 0;
  }
  return 1;
}

void init_arg(string arg) {
  string *args;

  args = explode(arg, "|");

  hat_visible = to_int(args[0]);
  hat_light = to_int(args[1]);
  hat_ansi = to_int(args[2]);
  set_last_time_news_checked(to_int(args[3]));
}

string query_auto_load() {
  if(this_player()->query_level() < APPRENTICE)
    return 0;
  else
    return explode(file_name(this_object()),"#")[0]+":"+
      sprintf("%d|%d|%d|%d",
      hat_visible,
      hat_light,
      hat_ansi,
      query_last_time_news_checked());
}

int drop() { return 1; }

int query_level() {
  return APPRENTICE;
}

string* query_all_files() {
  return all_files;
}

int query_hat_ansi() {
  return hat_ansi;
}
