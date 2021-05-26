#pragma strict_types
inherit "room/room";

void create() {
  ::create();
  set_short("nowhere");
  set_long("You are nowhere.");
}
