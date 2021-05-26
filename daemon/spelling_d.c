#pragma strict_types

#define TO this_object()

#define SPELLING                                                        \
  "/"+implode(explode(explode(file_name(TO), "#")[0], "/")[0..<2], "/")+    \
  "/SPELLING"

static variables inherit "obj/basic/create";

mapping words;

void parse_words();

void create() {
  ::create();

  if(is_clone()) {
    destruct(this_object());
    return;
  }
  
  parse_words();
}

void parse_words() {
  int i, size;
  string *lines, *items;
  
  words = ([]);
  
  lines = explode(read_file(SPELLING), "\n");
  
  for(i=0, size=sizeof(lines); i<size; i++) {
    items = explode(lines[i], ":");
    words[items[0]] = items[1];
  }
}

string query_spelling(string word) {
  string lword;

  lword = lower_case(word);

  if(member(words, word))
    return words[word];
  return word;
}

int query_is_mispelled(string word) { return member(words, lower_case(word)); }

mapping query_words()  { return words ? words + ([]) : ([]); }
