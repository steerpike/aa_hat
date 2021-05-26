#pragma strict_types

#include <xfun.h>

#define TO this_object()

#define CONTRACTIONS                                                        \
  "/"+implode(explode(explode(file_name(TO), "#")[0], "/")[0..<2], "/")+    \
  "/CONTRACTIONS"

inherit "/obj/basic/create";

mapping contractions;

void setup_contractions() {
  int i, size;
  string *words;

  if(!contractions) {
    words = explode(read_file(CONTRACTIONS), "\n");
    size = sizeof(words);
    contractions = ([]);
    for(i=0; i<size; i++)
      contractions += ([words[i]]);
  }
}

int is_contraction(string s) {
  setup_contractions();

  return member(contractions, lower_case(s));
}

void debug_contractions() {
  setup_contractions();

  write(get_f_string((string)XFUN->variable_to_string(contractions)));
}
