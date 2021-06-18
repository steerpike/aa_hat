#pragma strict_types

#include "../hat_def.h"

#define SAVE_PATH HAT_SAVE + (string)environment()->query_real_name()

int hat_visible, hat_light, hat_ansi, last_time_news_checked;
static int hat_is_lit;

void initialize_variables() {
  hat_visible = 1;
  hat_light = 0;
  hat_ansi = 1;
}

int query_hat_visible() { return hat_visible; }
int query_hat_light()   { return hat_light; }
int query_hat_ansi()    { return hat_ansi; }

int query_last_time_news_checked() { return last_time_news_checked; }
void set_last_time_news_checked(int i) { last_time_news_checked = i; }

void save_hat() { save_object(SAVE_PATH); }

void restore_hat() {
  if(!environment()) return;

  if(!restore_object(SAVE_PATH))
    initialize_variables();

  if(query_hat_light() && !hat_is_lit) {
    set_light(1);
    hat_is_lit = 1;
  }
}

void init() {
  add_action("do_hatshow", "hatshow");
  add_action("do_hatlight", "hatlight");
  add_action("do_hatansi", "hatansi");

  restore_hat();
}

int do_hatshow() {
  if(!hat_visible) {
    write("The hat turns visible.\n");
    hat_visible = 1;
  } else {
    write("The hat turns invisible.\n");
    hat_visible = 0;
  }
  save_hat();
  return 1;
}

int do_hatlight() {
  if(!hat_light) {
    set_light(1);
    hat_light = 1;
    hat_is_lit = 1;
    write("The hat starts to glow.\n");
  } else {
    set_light(-1);
    hat_light = 0;
    hat_is_lit = 0;
    write("The hat stops glowing.\n");
  }
  save_hat();
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
  save_hat();
  return 1;
}
