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
varargs void out_line(string s, int highlight, int indent);

// --------------------------------------------------------- Global Variables

int emergency_stop, qc_count, balance_count, world_count, is_busy;
object last_reported;
mapping reports;

// ------------------------------------------------------- Implementation ---

void reset_qc_count() { qc_count = 0; }
void increment_qc_count() { qc_count++; }
int query_qc_count() { return qc_count; }

void reset_balance_count() { balance_count = 0; }
void increment_balance_count() { balance_count++; }
int query_balance_count() { return balance_count; }

void reset_world_count() { world_count = 0; }
void increment_world_count() { world_count++; }
int query_world_count() { return world_count; }

object query_last_reported() { return last_reported; }

void set_reporting_on_object(object o) {
  string file, spam, item_short, name;

  if(o && last_reported != o) {
    file = explode(file_name(o), "#")[0]+".c";

    item_short = (string)o->query_short();
    if(!item_short)
      item_short = (string)o->short();

    name = (string)o->query_name();

    if(item_short && item_short[0..0] == "0")
      item_short = item_short[2..<1];

    spam = "Checking: " + (item_short ? item_short+" " : (name?name+" ":"") +"[Invisible]") + "\n(" + file + ")";

    out_line(spam, HAT_CHANNEL, 10);
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
  reset_qc_count();
  reset_balance_count();
  reset_world_count();
  set_reporting_on_object(0);
  disable_emergency_stop();
  clear_reports();
}

string hatlog_dir() {
  return HAT_LOG + (string)this_player()->query_real_name() + "/";
}

string hatlog_file() { return hatlog_dir() + "hat.log"; }

string hatlog_sense_file() { return hatlog_dir() +"senses.log"; }

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

void hat_log_senses(string s) {
  if(setup_hatlog())
    log_file(hatlog_sense_file(), s);
}

varargs void out_line(string s, int channel, int indent) {
  string colour;

  colour = CHANNELS[channel];
  COLOURUTIL_D->write_c(COLOURUTIL_D->get_cf_string(colour+s+COLOUR_RESET, 0, indent));
  hat_log(get_f_string(s, 0, indent));
}

int already_reported(object o, string error) {
  string file;

  if(o)
    file = explode(file_name(o), "#")[0];
  else
    file = 0;

  if(!member(reports, file))
    return 0;

  if(member(reports[file], error) == -1)
    return 0;

  // this file has already had this particular report
  return 1;
}

void add_report(object o, string error) {
  string file;

  if(o)
    file = explode(file_name(o), "#")[0];
  else
    file = 0;

  if(!member(reports, file))
    reports += ([ file : ({ error }) ]);

  if(member(reports[file], error) == -1)
    reports[file] += ({error});
}

void inform(object o, string s, int channel) {
  int indent;
  indent = 2;

  if(channel == HAT_CHANNEL) // Nin wanted 10 indent for HAT_CHANNEL
    indent = 10;

  if(o != query_last_reported())
    set_reporting_on_object(o);

  s = "- " +s;

  if(!already_reported(o, s)) {
    add_report(o, s);
    if(channel == QC_CHANNEL)
      increment_qc_count();
    else if(channel == BALANCE_CHANNEL)
      increment_balance_count();
    else if(channel == WORLD_CHANNEL)
      increment_world_count();
    out_line(s, channel, indent);
  }
}

void report(object o, string s, int channel) {
  inform(o, s, channel);
}

void check_inherits(object o) {
  string *s;
  int i;

  s = sort_array(inherit_list(o),#'>); //'
  for(i=0; i < sizeof(s)-1; i++)
    if(s[i] == s[i+1])
      report(o, "Alert! "+s[i]+" is inherited twice.", QC_CHANNEL);
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

  if(name[0..0] == "\n")
    report(o,"The set_name should not begin with \"\\n\", should it be an alias instead?.", QC_CHANNEL);

  if(text_short && strstr(lower_case(text_short), lower_case(name)) == -1)
    report(o, "The set_name \""+name+"\" should appear in the short or needs adjustment.", QC_CHANNEL);

  if(!(flags & TEXT_EXCEPTION_NAME_IN_ALIAS) && pointerp(ids) && member(ids, name) != -1)
    report(o, "The set_name \""+name+"\" should not also be an alias.", QC_CHANNEL);

  if(strlen(name) > 13 && living(o) && strstr(name, " ", 0) == -1 && !BANISH_D->query_name_banished(lower_case(name)))
    report(o, "The set_name \""+name+"\" needs to be banished still.", QC_CHANNEL);
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
      report(o, "The set_long doesn't seem to be working right.", QC_CHANNEL);
    } else
      report(o, "The set_long seems to be the default.", QC_CHANNEL);
    return;
  }

  if(!text_check(o, "the set_long", text_long,
  flags | TEXT_ALLOW_REDIRECT,
  extra ))
    return;

  if(!o->short())
    report(o, "Has a set_long but also needs a set_short too.", QC_CHANNEL);
}

void check_identify(object o, int flags) {
  if(!o->query_unique() && o->query_value() < 500)
    flags |= TEXT_NOT_MANDATORY;

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
    if(keys[i] != lower_case(keys[i]))
      report(o, "The noun \""+keys[i]+"\" in "+what+" should be lower case.", QC_CHANNEL);

    description = map[keys[i]];
    if(!member(descriptions, description)) {
      text_check(o, "the "+what+" for \""+keys[i]+"\"", description, TEXT_ALLOW_REDIRECT);
      descriptions += ([description]);
      hat_log_senses(get_f_string(description,0,0));
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
    report(o, "The material of this object is unknown.", QC_CHANNEL);
  else {
    if(mat[0] == "?")
      report(o, "The set_material is missing, it might be "+mat[1]+"?", QC_CHANNEL);
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
          report(o, "The set_material \""+mat[i]+"\" is not standard.", QC_CHANNEL);
      }
    }
  }
}

void check_recommended_value(object o, int rec) {
  int val;
  val = (int) o->query_value();
  if(!val && rec)
    report(o, "Has no value. Expected: "+rec+".", BALANCE_CHANNEL);
  else if(val > rec)
    report(o, "Has a high value ("+val+"). Expected: "+rec+".", BALANCE_CHANNEL);
  else if(val < rec)
    report(o, "Has a low value ("+val+"). Expected: "+rec+".", BALANCE_CHANNEL);
}

// return 0 - input was 0 or ""
// return 1 - input was a string, even if its a bad one
// TODO add a return value of 2 it indicate it a bad result
// TODO check for function_exists on redirection, and maybe also return type
varargs int text_check(object o, string what, string text, int flags, mapping extra) {
  int i, j, text_len, num_lines, word_len, words_size, changed_word, changed_words;
  string c, word, *lines, *words, *extra_words, *punctuation;

  debug("Running text_check for "+what+" on \""+text+"\".", "text_check");

  j = sizeof(TEXT_FLAGS);
  debug("checking "+j+" flags for \""+what+"\" ("+file_name(o)+")", "text_check");
  for(i=0; i<j; i++)
    debug(sprintf("  FLAG %2d: %3s - %s\n", i+1, (flags & (1 << i)?"on":"off"), TEXT_FLAGS[i]), "text_check");

  if(!text || text == "") {
    if(!(flags & TEXT_NOT_MANDATORY))
      report(o, capitalize(what)+" is missing.", QC_CHANNEL);
    return 0;
  }

  text_len = strlen(text);
  while(text[<1..<1] == " ") // strip trailing spaces
    text = text[0..<2];
  if(strlen(text) != text_len && !(flags & TEXT_EXCEPTION_ENDING_SPACE && strlen(text) == text_len -1)) {
    report(o, capitalize(what) + " contains trailing spaces.", QC_CHANNEL);
  }

  // TODO track down all the various redirect implementations, and
  // see which ones can handle a return string
  if(text[0..1] == "##") {
    if(!(flags & TEXT_ALLOW_REDIRECT)) {
      report(o, "Redirection is not allowed on "+what, QC_CHANNEL);
      return 1;
    }
    if(!function_exists(text[2..<1], o))
      report(o, capitalize(what) + " is redirecting to \""+text[2..<1]+"\" which does not exist.", QC_CHANNEL);
    return 1;
  }

  if(strstr(text, " ,") != -1)
    report(o, capitalize(what)+" has a space before a comma.", QC_CHANNEL);

  if(strstr(text, "  ") != -1)
    report(o, capitalize(what)+" has double spaces.", QC_CHANNEL);

  if(!(flags & TEXT_EXCEPTION_INLINE_NL) && strstr(text[0..<2], "\n") != -1)
    report(o, capitalize(what)+" should usually not contain \"\\n\".", QC_CHANNEL);

  c = text[<1..<1];

  if(flags & TEXT_REQUIRE_ENDING_NL) {
    if(c != "\n")
      report(o, capitalize(what)+" should end with \"\\n\".", QC_CHANNEL);
  } else {
    if(c == "\n") {
      if(!(flags & TEXT_EXCEPTION_ENDING_NL))
        report(o, capitalize(what)+" should not end in \"\\n\".", QC_CHANNEL);
    }
  }

  words = explode(text, " ") - ({0}) - ({""}) - ({"\n"});

  if(flags & TEXT_REQUIRE_ARTICLE && sizeof(words) && !is_article(words[0]))
    report(o, capitalize(what) + " usually starts with \"a\", \"an\", \"the\" or \"some\".", QC_CHANNEL);
  else if(flags & TEXT_DENY_ARTICLE && !(flags & TEXT_EXCEPTION_ALLOW_ARTICLE) && sizeof(words) && is_article(words[0]))
    report(o, capitalize(what) + " usually does not start with \"a\", \"an\", \"the\" or \"some\".", QC_CHANNEL);

  debug("words are "+(string)XFUN->variable_to_string(words)+".", "text_check");
  // loop through all the words, and strip punctuation from them
  changed_words = 0;
  for(i=0, words_size=sizeof(words); i<words_size; i++) {
    word = words[i];
    extra_words = 0;
    changed_word = 0;
    debug("Parsing word: \""+word+"\"", "text_check");

    for(j=0, word_len=strlen(word); j<word_len; j++) {
      c = word[j..j];
      debug("Parsing char: \""+c+"\"", "text_check");
      if( !(c[0] >= 'a' && c[0] <= 'z') && !(c[0] >= 'A' && c[0] <= 'Z') && c != "'" && c != "-") {
        debug("character \""+c+"\" is not part of word \""+word+"\".", "text_check");
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
        if(changed_word) {
          debug( ([ "words" : words ]), "text_check" );
          debug( "word " + i + " \"" + word + "\".", "text_check" );
          debug( words_size + " words.", "text_check" );
        }
        break;
      }
    }
  }

  if(changed_words)
    debug( (["Words have been changed to" : words]), "text_check" );

  for(i=0; i<sizeof(words); i++) {
    word = words[i];

    // now check words for contractions
    if(!(flags & TEXT_ALLOW_CONTRACTION) && CONTRACTION_D->is_contraction(lower_case(word)))
      report(o, capitalize(what) + " used contraction \""+word+"\".", QC_CHANNEL);

    if(i < words_size-1 && SPELLING_D->query_is_mispelled(word+" "+words[i+1]))
      report(o, capitalize(what) + " has \""+word+" "+words[i+1]+"\" which should be \""+(string)SPELLING_D->query_spelling(word+" "+words[i+1])+"\".", QC_CHANNEL);

    if(SPELLING_D->query_is_mispelled(word))
      report(o, capitalize(what) + " has \""+word+"\" which should be \""+(string)SPELLING_D->query_spelling(word)+"\".", QC_CHANNEL);

    if( (word == "a" || word == "an") && i < words_size-1 && (string)A_AN_D->query_article(words[i+1]) != lower_case(word) )
      report(o, capitalize(what) + " uses article \""+word+"\" before \""+words[i+1]+"\", but should use \""+(lower_case(word)=="a"?"an":"a")+"\".", QC_CHANNEL);

    if( i < words_size-1 && member(({"hand", "hands", "finger", "fingers", "foot", "feet", "arm", "arms", "leg", "legs"}), words[i+1]) != -1 && word == "your")
      report(o, capitalize(what) + " has \""+words[i+1]+"\" (see World policies).", WORLD_CHANNEL);
  }

  if(flags & TEXT_DENY_MULTIPLE_WORDS && sizeof(words) > 1)
    report(o, capitalize(what) + " should only be one word.", QC_CHANNEL);

  //strip the remaining newline at the end if it exists
  c = text[<1..<1];
  if(c == "\n") {
    text = text[0..<2];
    c = text[<1..<1];
  }

  num_lines = sizeof(explode(get_f_string(text), "\n"));
  if(num_lines > 21)
    report(o, capitalize(what) + " has too many lines ("+num_lines+"). Max 21.", QC_CHANNEL);

  if(flags & TEXT_CHECK_LIMITS) {
    debug( ([ "extra"     : extra     ]), "text_check" );
    debug( ([ "num_lines" : num_lines ]), "text_check" );
    debug( ([ "text_len"  : text_len  ]), "text_check" );
    debug( ([ "text"      : text      ]), "text_check" );

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
          j), QC_CHANNEL);

      i = extra["text_min_lines"];
      j = extra["text_max_lines"];
      if((i > 0 && num_lines < i) || (j > 0 && num_lines > j))
        report(o, sprintf("%s is too %s lines (%d). Expected: %d to %d lines.",
          capitalize(what),
          num_lines < i ? "few" : "many",
          num_lines,
          i,
          j), QC_CHANNEL);
    }
  }

  punctuation = ({".", "!", "?"});
  if(flags & TEXT_DENY_ENDING_PUNC) {
    if(member(punctuation, c) != -1)
      report(o, capitalize(what) + " does not need punctuation.", QC_CHANNEL);
  } else {
    if(member(punctuation, c) == -1)
      report(o, capitalize(what) + " requires punctuation.", QC_CHANNEL);
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
