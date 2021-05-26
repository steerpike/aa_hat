#pragma strict_types

#include <daemons.h>
#include <colour.h>
#include <xfun.h>

#include "../hat_def.h"

// -------------------------------------------------- External Prototypes ---

 // from ../hat.c
int query_hat_ansi();

// from basic_functions.c
int is_article(string word);
int is_capitalized(string s);

// from monster_functions.c
void set_sum_value(int i);

// -------------------------------------------------- Internal Prototypes ---

int query_hat_ansi();
varargs int text_check(object o, string what, string text, int flags, mapping extra);
varargs void out_line(string s, int highlight, int indent);

// --------------------------------------------------------- Global Variables

int emergency_stop, error_count, is_busy;
object last_reported;
mapping reports;

// ------------------------------------------------------- Implementation ---

void reset_error_count() { error_count = 0; }
void increment_error_count() { error_count++; }
int query_error_count() { return error_count; }

object query_last_reported() { return last_reported; }

void set_reporting_on_object(object o) {
  string file, spam, item_short, name;

  if(o && last_reported != o) {
    file = explode(file_name(o), "#")[0]+".c";

    item_short = (string)o->query_short();
    if(!item_short)
      item_short = (string)o->short();
    
    name = (string)o->query_name();
    
    if(item_short && item_short[0..1] == "0 ")
      item_short = item_short[2..<1];

    spam = "Checking: " + (item_short ? item_short+" " : (name?name+" ":"") +"[Invisible]") + "\n(" + file + ")";

    out_line(spam, 0, 10);
  }
  last_reported = o;
}

void enable_emergency_stop() { emergency_stop = 1; }
void disable_emergency_stop() { emergency_stop = 0; }
int query_emergency_stop() { return emergency_stop; }

void start_busy() { is_busy = 1; }
void stop_busy() { is_busy = 0; }
int query_busy() { return is_busy; }

void clear_reports() { reports = ([]); }
mapping query_reports() { return reports ? reports + ([]) : ([]); }

void reset_variables() {
  set_sum_value(0);
  reset_error_count();
  set_reporting_on_object(0);
  disable_emergency_stop();
  clear_reports();
}

string hatlog_dir() {
  return HAT_LOG + (string)this_player()->query_real_name() + "/";
}

string hatlog_file() { return hatlog_dir() + "hat.log"; }

int setup_hatlog() {
  string dir, name;

  name = (string)this_player()->query_real_name();
  dir = hatlog_dir();

  if(file_size(dir) != -2) {
    if(file_size(dir) != -1 || !mkdir(dir)) {
      writef("Error creating directory: "+dir);
      return 0;
    }
    if(!write_file(dir+"ACCESS", name+":R")) {
      writef("Error setting up ACCESS for "+capitalize(name)+" in "+dir);
      return 0;
    }
  }
  return 1;
}

void hat_log(string s) {
  if(setup_hatlog())
    log_file(hatlog_file(), s);
}

varargs void out_line(string s, int highlight, int indent) {
  string colour;
  
  if(!indent)
    indent = 2;
  
  if(highlight && query_hat_ansi()) {
    s = "- " +s;
    colour = ({COLOUR_INTENSE_RED, COLOUR_INTENSE_YELLOW})[highlight-1];
    COLOURUTIL_D->write_c(COLOURUTIL_D->get_cf_string(colour+s+COLOUR_RESET, 0, indent));
  } else
    write(get_f_string(s, 0, indent));
  hat_log(get_f_string(s, 0, indent));
}

int already_reported(object o, string error) {
  string file;

  file = explode(file_name(o), "#")[0];

  if(!member(reports, file))
    return 0;

  if(member(reports[file], error) == -1)
    return 0;

  // this file has already had this particular report
  return 1;
}

void add_report(object o, string error) {
  string file;

  file = explode(file_name(o), "#")[0];

  if(!member(reports, file))
    reports += ([ file : ({ error }) ]);

  if(member(reports[file], error) == -1)
    reports[file] += ({error});
}

varargs void inform(object o, string s, int is_error) {
  if(o != query_last_reported())
    set_reporting_on_object(o);

  if(!already_reported(o, s)) {
    add_report(o, s);
    if(is_error) {
      out_line(s, 1); // 1 is red
      increment_error_count();
    } else
      out_line(s, 2);  // 2 is yellow
  }
}

void report(object o, string s) {
  inform(o, s, 1);
}

void check_inherits(object o) {
  string *s;
  int i;

  s = sort_array(inherit_list(o),#'>);
  for(i=0; i < sizeof(s)-1; i++)
    if(s[i] == s[i+1])
      report(o, "Alert! "+s[i]+" is inherited twice.");
}

void check_name(object o, int flags) {
  string name, *ids, text_short;

  name = (string)o->query_real_name();
  if(!name)
    name = (string)o->query_name();

  ids = (string *) o->query_alias();
  text_short = (string)o->query_short();
  if(!text_short)
    text_short = (string)o->short();

  if(!text_check(o, "the set_name", name,
  flags | TEXT_DENY_ARTICLE | TEXT_DENY_ENDING_PUNC))
    return;

  if(text_short && strstr(lower_case(text_short), lower_case(name)) == -1)
    report(o, "The set_name \""+name+"\" should appear in the short or needs adjustment.");

  if(!(flags & TEXT_EXCEPTION_NAME_IN_ALIAS) && pointerp(ids) && member(ids, name) != -1)
    report(o, "The set_name \""+name+"\" should not also be an alias.");

  if(strlen(name) > 13 && living(o) && strstr(name, " ", 0) == -1 && !BANISH_D->query_name_banished(lower_case(name)))
    report(o, "The set_name \""+name+"\" needs to be banished still.");
}

varargs int check_short(object o, int flags, mapping extra) {
  string text_short;

  text_short = (string)o->query_short();
  if(!text_short)
    text_short = (string)o->short();

  return text_check(o, "the set_short", text_short,
    flags | TEXT_DENY_ENDING_PUNC,
    extra);
}

varargs void check_long(object o, int flags, mapping extra) {
  string text_long;

  text_long = (string) o->query_long();
  if(!text_long && !o->short())
    return;
  if(text_long == "You see nothing special.\n") {
    if(function_exists("long", o) != function_exists("query_long", o)) {
      report(o, "The set_long doesn't seem to be working right.");
    } else
      report(o, "The set_long seems to be the default.");
    return;
  }

  if(!text_check(o, "the set_long", text_long,
  flags | TEXT_ALLOW_REDIRECT,
  extra ))
    return;

  if(!o->short())
    report(o, "Has a set_long but also needs a set_short too.");
}

void check_identify(object o, int flags) {
  text_check(o, "the set_identify", (string)o->query_identify(), flags);
}

void check_set_sense(object o, string sense, int flags) {
  string s;

  s = (string)call_other(o, "query_"+sense);

  text_check(o, "the set_"+sense, s, flags | TEXT_ALLOW_REDIRECT);
}

void check_add_senses_texts(object o, mapping map, string what) {
  string *keys, description;
  int i, j, size;
  mapping descriptions;

  if(!map || !sizeof(map))
    return;

  keys = m_indices(map);
  size = sizeof(keys);
  descriptions = ([]);

  for(i=0; i<size; i++) {
    description = map[keys[i]];
    if(!member(descriptions, description)) {
      text_check(o, "the "+what+" for \""+keys[i]+"\"", description, TEXT_ALLOW_REDIRECT);
      descriptions += ([description]);
      map -= ([keys[i]]);
    }
  }
}

void check_material(object o, int mandatory) {
  int i;
  string *mat, *allowed_materials;

  mat = (string *) o->query_material();
  if(!mat)
    mat = (string *) MATERIAL_D->guess_material(o);
  if(!mat || !sizeof(mat))
    report(o, "The material of this object is unknown.");
  else {
    if(mat[0] == "?")
      report(o, "The set_material is missing, it might be "+
                mat[1]+"?");
    else {
      allowed_materials = ({"adamantite", "bone", "brass", "bronze",
        "cheese", "clay", "cloth", "copper", "dough", "egg", "electrum",
        "feather", "fur", "glass", "gold", "hide", "ice", "iron", "lead",
        "leather", "meat", "metal", "mithril", "orichalcum", "paper",
        "pearl", "plant", "platinum", "salt", "shell", "shellfish", "silver",
        "sinew", "soil", "steel", "stone", "tin", "trillian", "wax", "wood",
        "zinc"});
      for(i=0; i<sizeof(mat); i++) {
        if(member_array(mat[i], allowed_materials) == -1)
          report(o, "The set_material \""+mat[i]+"\" is not standard.");
      }
    }
  }
}

void check_recommended_value(object o, int rec) {
  int val;
  val = (int) o->query_value();
  if(!val && rec)
    report(o, "Has no value. Expected: "+rec+".");
  else if(val > rec)
    report(o, "Has a high value ("+val+"). Expected: "+rec+".");
  else if(val < rec)
    report(o, "Has a low value ("+val+"). Expected: "+rec+".");
}

// return 0 - input was 0 or ""
// return 1 - input was a string, even if its a bad one
// TODO add a return value of 2 it indicate it a bad result
// TODO check for function_exists on redirection, and maybe also return type
varargs int text_check(object o, string what, string text, int flags, mapping extra) {
  int i, j, text_len, num_lines, word_len, words_size, changed_word, changed_words;
  string c, word, *lines, *words, *extra_words, *punctuation;

  //inform(o, "DEBUG: Running text_check for "+what+" on \""+text+"\".");

  //j = sizeof(TEXT_FLAGS);
  //inform(o, "DEBUG: checking "+j+" flags for \""+what+"\" ("+file_name(o)+")");
  //for(i=0; i<j; i++)
  //  write(sprintf("  FLAG %2d: %3s - %s\n", i+1, (flags & (1 << i)?"on":"off"), TEXT_FLAGS[i]));

  if(!text || text == "") {
    if(!(flags & TEXT_NOT_MANDATORY))
      report(o, capitalize(what)+" is missing.");
    return 0;
  }

  text_len = strlen(text);
  while(text[<1..<1] == " ") // strip trailing spaces
    text = text[0..<2];
  if(strlen(text) != text_len && !(flags & TEXT_EXCEPTION_ENDING_SPACE && strlen(text) == text_len -1)) {
    report(o, capitalize(what) + " contains trailing spaces.");
  }

  // TODO track down all the various redirect implementations, and
  // see which ones can handle a return string
  if(text[0..1] == "##") {
    if(!(flags & TEXT_ALLOW_REDIRECT)) {
      report(o, "Redirection is not allowed on "+what);
      return 1;
    }
    if(!function_exists(text[2..<1], o))
      report(o, capitalize(what) + " is redirecting to \""+text[2..<1]+"\" which does not exist.");
    return 1;
  }

  if(strstr(text, " ,") != -1)
    report(o, capitalize(what)+" has a space before a comma.");

  if(strstr(text, "  ") != -1)
    report(o, capitalize(what)+" has double spaces.");

  if(!(flags & TEXT_EXCEPTION_INLINE_NL) && strstr(text[0..<2], "\n") != -1)
    report(o, capitalize(what)+" should usually not contain \"\\n\".");

  c = text[<1..<1];

  if(flags & TEXT_REQUIRE_ENDING_NL) {
    if(c != "\n")
      report(o, capitalize(what)+" should end with \"\\n\".");
  } else {
    if(c == "\n") {
      if(!(flags & TEXT_EXCEPTION_ENDING_NL))
        report(o, capitalize(what)+" should not end in \"\\n\".");
    }
  }

  words = explode(text, " ") - ({0}) - ({""}) - ({"\n"});

  if(flags & TEXT_REQUIRE_ARTICLE && sizeof(words) && !is_article(words[0]))
    report(o, capitalize(what) + " usually starts with \"a\", \"an\", \"the\" or \"some\".");
  else if(flags & TEXT_DENY_ARTICLE && !(flags & TEXT_EXCEPTION_ALLOW_ARTICLE) && sizeof(words) && is_article(words[0]))
    report(o, capitalize(what) + " usually does not start with \"a\", \"an\", \"the\" or \"some\".");

  //inform(o, "DEBUG: words are "+(string)XFUN->variable_to_string(words)+".");
  // loop through all the words, and strip punctuation from them
  changed_words = 0;
  for(i=0, words_size=sizeof(words); i<words_size; i++) {
    word = words[i];
    extra_words = 0;
    changed_word = 0;
    //inform(o, "DEBUG: Parsing word: \""+word+"\"");

    for(j=0, word_len=strlen(word); j<word_len; j++) {
      c = word[j..j];
      //inform(o, "DEBUG: Parsing char: \""+c+"\"");
      if( !(c[0] >= 'a' && c[0] <= 'z') && !(c[0] >= 'A' && c[0] <= 'Z') && c != "'" && c != "-") {
        //inform(o, "DEBUG: character \""+c+"\" is not part of word \""+word+"\".");
        extra_words = explode(word, c);
        // insert the new words, but preserve order
        if(words_size == 1) {
          words = extra_words;
        } else if(i == 0) {
          words = extra_words + words[1..<1];
        } else if(i == words_size-1) {
          words = words[0..<2] + extra_words;
        } else {
          words = words[0..i-1] + extra_words + words[i+1..<1];
        }

        words_size += sizeof(extra_words) - 1;
        changed_words = 1;
        changed_word = 1;
        //if(changed_word) {
        //  inform(o, "DEBUG: words "+(string)XFUN->variable_to_string(words)+".");
        //  inform(o, "DEBUG: word "+i+" \""+word+"\".");
        //  inform(o, "DEBUG: "+words_size+" words.");
        //}
        break;
      }
    }
  }

  //if(changed_words)
  //  inform(o, "DEBUG: words have been changed to "+(string)XFUN->variable_to_string(words)+".");

  for(i=0; i<sizeof(words); i++) {
    word = words[i];

    // now check words for contractions
    if(!(flags & TEXT_ALLOW_CONTRACTION) && CONTRACTION_D->is_contraction(lower_case(word)))
      report(o, capitalize(what) + " used contraction \""+word+"\".");

    if(i < words_size-1 && SPELLING_D->query_is_mispelled(word+" "+words[i+1]))
      report(o, capitalize(what) + " has \""+word+" "+words[i+1]+"\" which should be \""+(string)SPELLING_D->query_spelling(word+" "+words[i+1])+"\".");

    if(SPELLING_D->query_is_mispelled(word))
      report(o, capitalize(what) + " has \""+word+"\" which should be \""+(string)SPELLING_D->query_spelling(word)+"\".");

    if( (word == "a" || word == "an") && i < words_size-1 && (string)A_AN_D->query_article(words[i+1]) != lower_case(word) )
      report(o, capitalize(what) + " uses article \""+word+"\" before \""+words[i+1]+"\", but should use \""+(lower_case(word)=="a"?"an":"a")+"\".");
  }

  if(flags & TEXT_DENY_MULTIPLE_WORDS && sizeof(words) > 1)
    report(o, capitalize(what) + " should only be one word.");

  //strip the remaining newline at the end if it exists
  c = text[<1..<1];
  if(c == "\n") {
    text = text[0..<2];
    c = text[<1..<1];
  }

  if(flags & TEXT_CHECK_LIMITS) {
    num_lines = sizeof(explode(get_f_string(text), "\n"));
    //inform(o, "DEBUG: extra is "+XFUN->variable_to_string(extra));
    //inform(o, "DEBUG: num_lines ("+num_lines+").");
    //inform(o, "DEBUG: text_len ("+text_len+").");
    //inform(o, "DEBUG: text is \""+text+"\".");

    if(mappingp(extra)) {
      // TODO break up this dang text_check function!
      // we're out of local variables, so...
      // `i` will represent min
      // `j` will represent max
      i = extra["text_min_length"];
      j = extra["text_max_length"];
      if((i > 0 && text_len < i) || (j > 0 && text_len > j))
        report(o, sprintf("%s is too %s (%d chars). Expected: %d to %d chars.",
          capitalize(what),
          text_len < i ? "short" : "long",
          text_len,
          i,
          j));

      i = extra["text_min_lines"];
      j = extra["text_max_lines"];
      if((i > 0 && num_lines < i) || (j > 0 && num_lines > j))
        report(o, sprintf("%s is too %s lines (%d). Expected: %d to %d lines.",
          capitalize(what),
          num_lines < i ? "few" : "many",
          num_lines,
          i,
          j));
    }
  }

  punctuation = ({".", "!", "?"});
  if(flags & TEXT_DENY_ENDING_PUNC) {
    if(member(punctuation, c) != -1)
      report(o, capitalize(what) + " does not need punctuation.");
  } else {
    if(member(punctuation, c) == -1)
      report(o, capitalize(what) + " requires punctuation.");
  }

  return 1;
}

void check_add_senses(object o, int i) {
  int cost;
  string sense, *senses;
  mapping items;

  senses = ({"item","noget","search","smell","sound","taste","touch"});
  sense = senses[i];

  items = (mapping) call_other(o, "query_"+PLURAL_D->to_plural(sense));
  check_add_senses_texts(o, items, "add_"+sense);
  
  if(i == sizeof(senses)-1) {
    // TODO this is a bit of a hack. all the call_out break points should
    // be converted to a callback system, instead of directly calling
    // in a chain
    if(o->query_is_room()) {
      if(EVAL_COST_LIMIT)
        call_out("hatcheck_room_2", 0, o);
      else
        call_other(this_object(), "hatcheck_room_2", o);
    }
  } else if(EVAL_COST_LIMIT)
    call_out("check_add_senses", 2, o, i+1);
  else
    check_add_senses(o, i+1);
}
