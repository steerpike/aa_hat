#pragma strict_types

#define SAVE_PATH HAT_SAVE + (string)environment()->query_real_name()

int hat_visible, hat_light, hat_ansi

void initialize_variables() {
  hat_visible = 1;
  hat_light = 1;
  hat_ansi = 1;

  set_light(1);
}

int query_hat_visible() { return hat_visible; }
int query_hat_light()   { return hat_light; }
int query_hat_ansi()    { return hat_ansi; }

void save_hat() { save_object(SAVE_PATH); }

void restore_hat() {
  if(!environment()) return;

  if(!restore_object(SAVE_PATH);
    initialize_variables();
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
    write("The hat starts to glow.\n");
  } else {
    set_light(-1);
    hat_light = 0;
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
