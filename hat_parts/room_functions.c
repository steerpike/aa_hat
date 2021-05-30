#pragma strict_types

#include <map.h>

#include "../hat_def.h"

// -------------------------------------------------- External Prototypes ---

void hatcheck_item_list(object o, int notify_ok);

mapping basic_check(object room); // TODO port this over from hat.c?
int format_check_file(object o, string f); // TODO why is this here, and not everywhere?
void check_inherits(object o); // TODO why is this here, and not everywhere?

// -------------------------------------------------- Internal Prototypes ---

int check_exits(object room);
int check_doors(object room);
int check_assay(object room);
int check_forageable(object room);
int is_workroom(object room);
void determine_light(object room, object o, mapping items);

// ----------------------------------------------------- Global Variables ---

// TODO recode the light check, maybe?
int artificial, light;
object hatroom;

// ------------------------------------------------------- Implementation ---

void hatcheck_room(object o) {
  string *inheritance, xf, yf, fname;
  mapping items;

  fname = explode(file_name(o), "#")[0];
  if(!format_check_file(o, fname+".c")) {
    call_out("hatcheck_all_files", 1);
    return;
  }

  check_inherits(o);

  if((!o->query_exits() || !sizeof((string*)o->query_exits())) && !o->enum_doors() && strstr(lower_case(explode(explode(file_name(o), "#")[0],"/")[<1]), "base") != -1) {
    report(o, "Seems to be some type of base file...skipping.", HAT_CHANNEL);
    call_out("hatcheck_all_files", 1);
    return;
  }

  items = basic_check(o);

  if(!items) {
    call_out("hatcheck_all_files", 1);
    return;
  }

  if(sizeof(items) && sizeof(items) < MIN_ITEMS)
    report(o, "The room needs more add_senses, it is too plain.", QC_CHANNEL);

  check_short(o, TEXT_CHECK_LIMITS, ROOM_SHORT_LIMITS);
  check_long(o,
    TEXT_EXCEPTION_ENDING_SPACE | TEXT_CHECK_LIMITS,
    ROOM_LONG_LIMITS);
  check_set_sense(o, "search", TEXT_NOT_MANDATORY);
  check_set_sense(o, "smell", (o->query_is_underwater_room()?TEXT_NOT_MANDATORY:0));
  check_set_sense(o, "sound", 0);

  inheritance = inherit_list(o);

  if(member_array("room/room.c", inheritance) == -1)
    report(o, "It looks like you forgot to inherit room/room.", QC_CHANNEL);
  else if(inheritance[1] == "room/room.c") {
    if(!is_workroom(o))
      report(o, "You probably need to use a base file room here.", QC_CHANNEL);
  }

  if(function_exists("foo", o))
    report(o, "You cannot have a function named foo() in a room.", QC_CHANNEL);

  if(!is_workroom(o)) {
    xf = function_exists("query_x_coord", o);
    yf = function_exists("query_y_coord", o);
    if(!xf || !yf)
      report(o, "Missing X and Y coordinates (should go in a base file).", QC_CHANNEL);
    else if(xf == file_name() || yf == file_name()) {
      if(inheritance[1] == "room/room.c")
        report(o, "The query_x_coord and query_y_coord must go into a base file.", QC_CHANNEL);
      else
        report(o, "The query_x_coord and query_y_coord must go into a base file.", QC_CHANNEL);
    }
  }

  check_add_senses(o, 0);
}

// TODO code checking for add_message()
void check_messages(object room) {
  
}

// TODO code checking for block_stand_messages and block_stand
void check_body_position_messages(object room) {
  
}

// TODO code checking for add_window
void check_windows(object room) {
  
}

int is_mapenter(object room) {
  return member(inherit_list(room), MAP_ROOM+".c") != -1;
}

void hatcheck_room_2(object room) {
  if(check_exits(room) + check_doors(room) == 0 && !is_mapenter(room))
    report(room, "There are no exits here.", QC_CHANNEL);

  light = (int) room->query_light();
  artificial = 0;
  call_other(HAT_ROOM, "???", 0);
  hatroom = find_object(HAT_ROOM);

  determine_light(room, first_inventory(room), 0);
}

void hatcheck_room_3(object room, mapping items) {
  object hatroom, *o;
  int outdoors, base_light, i;

  outdoors = (int) room->query_outdoors();
  base_light = light-artificial;

  if(base_light < 0)
    report(room, "A very dark ("+base_light+") room? This is uncommon.", QC_CHANNEL);
  if(base_light > 1) {
    report(room, "A very bright ("+base_light+") room? This is uncommon.", QC_CHANNEL);
    if(base_light == 2)
      report(room, "Did you put set_light(1) in the base room and in here?", QC_CHANNEL);
  }
  if(base_light <= 0) {
    if(outdoors == 2)
      report(room, "This room has windows of some kind, but has no light?", QC_CHANNEL);
    else
      if(outdoors == 3 || outdoors == 4)
        report(room, "This is an outdoor room, but has no light?", QC_CHANNEL);
  } else
    if(outdoors == 0 || outdoors == 1)
      if(!items["torch"] && !items["torches"] && !items["lamp"] &&
        !items["lamps"] && !items["fire"] && !items["fireplace"] &&
        !items["lantern"] && !items["lanterns"] && !items["candle"] &&
        !items["candles"] && !items["light"])
        report(room, "An indoor room with light, but no light sources are described in the room?", QC_CHANNEL);
  if(outdoors < 0 || outdoors > 4)
    report(room, "Error: illegal value ("+outdoors+") in set_outdoors.", QC_CHANNEL);

  // TODO assess the outdoorsness for foraging and whatnot
  if(outdoors == 3 || outdoors == 4) {
    if(!check_forageable(room) && !(room->query_is_underwater_room() || room->query_is_water_room()))
      report(room, "Rangers cannot forage here. See 'man query_ranger_terrain'.", QC_CHANNEL);
    if(!check_assay(room) && !(room->query_is_underwater_room() || room->query_is_water_room()))
      report(room, "Artificers cannot assay here. See 'man query_artificer_terrain_type'.", QC_CHANNEL);
    // TODO /room/room.c should call terrain::create(), so that all rooms
    // get "none" by default. then the hat can stop checking for terrain
    if(!room->is_maproom() && !room->query_terrain())
      report(room, "Outdoor rooms should use set_terrain.", QC_CHANNEL);
  }
  
  if(room->is_maproom() && file_name(room)[0..10] != "/room/map/m" && room->query_terrain())
    report(room, "Mapenter rooms should not set_terrain.", QC_CHANNEL);

  hatcheck_item_list(first_inventory(room), 1);
}

int direction_sort(string str) {
  switch(str) {
    case "north":     return 1;
    case "northeast": return 2;
    case "east":      return 3;
    case "southeast": return 4;
    case "south":     return 5;
    case "southwest": return 6;
    case "west":      return 7;
    case "northwest": return 8;
    case "up":        return 9;
    case "down":      return 10;
    case "enter":     return 11;
    case "out":       return 12;
    default:          return 0;
  }
}

string reverse_direction(string str) {
  switch(str) {
    case "north":     return "south";
    case "northeast": return "southwest";
    case "east":      return "west";
    case "southeast": return "northwest";
    case "south":     return "north";
    case "southwest": return "northeast";
    case "west":      return "east";
    case "northwest": return "southeast";
    case "up":        return "down";
    case "down":      return "up";
    case "enter":     return "out";
    case "out":       return "enter";
    default:          return 0;
  }
}

int check_exit(object room, object dest, string direction) {
  string backdir, backroom;

  if(!call_other(dest, "query_room"))
    report(room, "The \""+direction+"\" exit leads to a non-room.", QC_CHANNEL);
  else {
    if(dest == room)
      report(room, "The \""+direction+"\" exit leads right back here.", QC_CHANNEL);
    else {
      backdir = reverse_direction(direction);
      if(!backdir) {
        if(!is_workroom(room))
          report(room, "The \""+direction+"\" direction is non-standard.", QC_CHANNEL);
      } else if(!is_mapenter(room)) {
        backroom = (string) dest->query_exit(backdir);
        if(!backroom)
          report(room, "If you go \""+direction+"\" you cannot come \""+backdir+"\" to return.", QC_CHANNEL);
        else
          if(find_object(backroom) != room)
            report(room, "If you go \""+direction+"\" the \""+backdir+"\" leads to another room instead.", QC_CHANNEL);
      }
    }
  }
}

// TODO ditch the sorting crap
int check_exits(object room) {
  string *exits, direction, err;
  object dest;
  int i, last_dir, this_dir, dir_ok;

  exits = (string *) room->query_exits();
  if(!sizeof(exits))
    return 0;

  last_dir = 0;
  dir_ok = 1;
  for(i=0; i<sizeof(exits); i += 2) {
    direction = exits[i+1];
    this_dir = direction_sort(direction);
    if(this_dir)
      if(this_dir > last_dir)
        last_dir = this_dir;
      else
        dir_ok = 0;

    err = catch(load_object(exits[i]));

    if(err)
      report(room, "The \""+direction+"\" exit leads somewhere that does not load.", QC_CHANNEL);
    else {
      dest = find_object(exits[i]);
      check_exit(room, dest, direction);
    }
  }
  // TODO remove once sort_exits is installed
  if(!dir_ok)
    report(room, "Put exits in order: N, NE, E, SE, S, SW, W, NW, up, down, enter, out.", QC_CHANNEL);

  return 1;
}

// TODO need to apply the same check_exits logic to doors
int check_doors(object room) {
  if(strlen((string) room->query_doors()))
    return 1;
  else
    return 0;
}

int check_assay(object room) {
  string s;

  if(room->query_artificer_terrain_type()) return 1;

  s = lower_case((string) room->short());

  if(strstr(s, "forest", 0) != -1) return 1;
  if(strstr(s, "woods", 0) != -1) return 1;
  if(strstr(s, "wooded", 0) != -1) return 1;
  if(strstr(s, "park", 0) != -1) return 1;
  if(strstr(s, "burnham wood", 0) != -1) return 1;
  if(strstr(s, "plain", 0) != -1) return 1;
  if(strstr(s, "grass", 0) != -1) return 1;
  if(strstr(s, "field", 0) != -1) return 1;
  if(strstr(s, "meadow", 0) != -1) return 1;
  if(strstr(s, "pasture", 0) != -1) return 1;
  if(strstr(s, "shore", 0) != -1) return 1;
  if(strstr(s, "beach", 0) != -1) return 1;
  if(strstr(s, "river", 0) != -1) return 1;
  if(strstr(s, "creek", 0) != -1) return 1;
  if(strstr(s, "lake", 0) != -1) return 1;
  if(strstr(s, "swamp", 0) != -1) return 1;
  if(strstr(s, "marsh", 0) != -1) return 1;
  if(strstr(s, "mud", 0) != -1) return 1;
  if(strstr(s, "desert", 0) != -1) return 1;
  if(strstr(s, "dune", 0) != -1) return 1;
  if(strstr(s, "jungle", 0) != -1) return 1;
  if(strstr(s, "mount", 0) != -1) return 1;
  if(strstr(s, "hill", 0) != -1) return 1;
  if(strstr(s, "slope", 0) != -1) return 1;
  if(strstr(s, "rocky", 0) != -1) return 1;
  if(strstr(s, "steppe", 0) != -1) return 1;
  if(strstr(s, "snow", 0) != -1) return 1;
  if(strstr(s, "glacier", 0) != -1) return 1;
  if(strstr(s, "taiga", 0) != -1) return 1;
  if(strstr(s, "frozen", 0) != -1) return 1;
  if(strstr(s, "tundra", 0) != -1) return 1;

  return 0;
}

int check_forageable(object room) {
  string s;

  if(room->query_ranger_terrain()) return 1;

  s = lower_case((string) room->short());

  if(strstr(s, "forest", 0) != -1) return 1;
  if(strstr(s, "woods", 0) != -1) return 1;
  if(strstr(s, "wooded", 0) != -1) return 1;
  if(strstr(s, "park", 0) != -1) return 1;
  if(strstr(s, "burnham wood", 0) != -1) return 1;
  if(strstr(s, "jungle", 0) != -1) return 1;
  if(strstr(s, "swamp", 0) != -1) return 1;
  if(strstr(s, "marsh", 0) != -1) return 1;
  if(strstr(s, "mud", 0) != -1) return 1;
  if(strstr(s, "shore", 0) != -1) return 1;
  if(strstr(s, "beach", 0) != -1) return 1;
  if(strstr(s, "slope", 0) != -1) return 1;
  if(strstr(s, "mount", 0) != -1) return 1;
  if(strstr(s, "rocky", 0) != -1) return 1;
  if(strstr(s, "hill", 0) != -1) return 1;
  if(strstr(s, "plain", 0) != -1) return 1;
  if(strstr(s, "grass", 0) != -1) return 1;
  if(strstr(s, "field", 0) != -1) return 1;
  if(strstr(s, "meadow", 0) != -1) return 1;
  if(strstr(s, "pasture", 0) != -1) return 1;
  if(strstr(s, "snow", 0) != -1) return 1;
  if(strstr(s, "tundra", 0) != -1) return 1;
  if(strstr(s, "desert", 0) != -1) return 1;
  if(strstr(s, "dune", 0) != -1) return 1;

  return 0;
}

int is_workroom(object room) {
  string name;
  name = file_name(room);
  if(sscanf(name, "/w/%~s/workroom") == 1)
    return 1;
  return 0;
}

void determine_light(object room, object o, mapping items) {
  object nexto;

  if(!items)
    items = (mapping)room->query_items();

  while(o) {
    if(EVAL_COST_LIMIT) {
      call_out("determine_light", 1, room, o, items);
      return;
    } else {
      nexto = next_inventory(o);
      o->move(hatroom);
      artificial += light - (int) room->query_light();
      o->move(room);
      o = nexto;
    }
  }
  if(EVAL_COST_LIMIT)
    call_out("hatcheck_room_3", 0, room, items);
  else
    hatcheck_room_3(room, items);
}
