#pragma strict_types

#include <daemons.h>
#include <colour.h>

#include "../hat_def.h"

// -------------------------------------------------- External Prototypes ---

void report(object o, string s); // from ../hat.c
varargs void inform(object o, string s, int highlight); // from ../hat.c

// ------------------------------------------------------- Implementation ---

int abs(int i) { return i>0?i:-i; }

int is_article(string s) {
  if(!s) return 0;
  return member(({"a", "an", "some", "the"}), lower_case(s)) != -1;
}

status starts_with_article(string s) {
  if(!s) return 0;
  return is_article(explode(s, " ")[0]);
}

int is_capitalized(string s) {
  if(!s) return 0;
  return (s[0] >= 'A') && (s[0] <= 'Z');
}

int is_language(string s) { return (int)LANG_D->query_exists(s); }

int format_check_file(object o, string f) {
  string l;

  if(file_size(f) <= 0)
    return 1;

  l = read_file(f, 1, 1);
  if(strlen(l) >= 2)
    if(l[<2] == 13 && l[<1] == 10) {
      report(o, "Please re-upload the file in ASCII format.");
      return 0;
    }
  return 1;
}

string fix_path(string path) {
  if(path == ".")
    return (string)this_player()->query_path();
  else
    return (string)this_player()->get_custom_path(this_player()->query_path(), path);
}
