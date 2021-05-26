#include "../hat_def.h"
#include <daemons.h>

inherit (HAT_PARTS+"common_functions");
inherit COLOURUTIL_D;

int flags;
string expect;
string testing;
string result;

void report(object o, string s) {
  result = s;
}

void perform_text_check(string testing, int flags) {
  text_check(0, "Foo", testing, flags);

  if(result != expect)
    throw(result);
}

void reset_test() {
  flags = 0;
  expect = 0;
  testing = 0;
  result = 0;
}

// This function will run all other function that start with "test_"
void do_tests() {
  int i, size;
  string err, test, *tests;

  tests = functionlist(this_object());
  for(i = 0, size = sizeof(tests); i<size; i++) {
    test = tests[i];
    if(test[0..4] == "test_") {
      reset_test();
      write_cf("Running: "+iwhite(test));
      call_other(this_object(), test);
      err = catch(call_other(this_object(), "perform_text_check", testing, flags));
      if(err)
        write_cf("... " + ired(" failed: ")+red(err));
      else
        write_cf("... " + igreen(" passed."));
    }
  }
}

// These are the actual tests:

void test_text_mandatory_with_no_text() {
  flags = 0;
  testing = 0;
  // we expect an error, because text is mandatory
  expect = "Foo is missing.";
}

void test_text_not_mandatory_with_no_text() {
  flags = TEXT_NOT_MANDATORY;
  testing = 0;
  // we dont expect a report, because the text is not mandatory
  expect = 0;
}

void test_text_mandatory_with_empty_text() {
  flags = 0;
  testing = "";
  expect = "Foo is missing.";
}

void test_text_not_mandatory_with_empty_text() {
  flags = TEXT_NOT_MANDATORY;
  testing = "";
  expect = 0;
}
