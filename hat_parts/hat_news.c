#pragma strict_types

#include <xfun.h>
#include <daemons.h>

#include "../hat_def.h"

void check_if_news() {
  if(file_date(HAT_NEWS) > query_last_time_news_checked())
    COLOURUTIL_D->write_c((string)COLOURUTIL_D->igreen("There is new 'hatnews'.\n"));
}

void init() {
  add_action("do_hatnews", "hatnews");
  add_action("do_addhatnews", "addhatnews");

  call_out("check_if_news", 2);
}

mapping get_news() {
  int i;
  string *data, *news_items;
  mapping news;

  news = ([]);
  if(file_size(HAT_NEWS) > 0) {
    news_items = explode(read_file(HAT_NEWS), "\n");
    for(i=0; i<sizeof(news_items); i++) {
      data = explode(news_items[i], "|");
      news += ([ to_int(data[0]): implode(data[1..], "|") ]);
    }
  }
  return news;
}

void check_news(int num_of_items) {
  int i, *keys, time_threshold;
  string date, *news_items;
  mapping news;

  news = get_news();

  if(!num_of_items) {
    time_threshold = query_last_time_news_checked();
    num_of_items = sizeof(news);
  } else if (num_of_items == -1) {
    num_of_items = sizeof(news);
  }
  
  set_last_time_news_checked(time());
  save_hat();

  news_items = ({});
  keys = sort_array(m_indices(news), #'<); //'
  for(i=0; i<sizeof(keys) && i<num_of_items; i++) {
    if(keys[i] > time_threshold) {
      date = explode((string)XFUN->short_time(keys[i]), " ")[0];
      news_items += ({ get_f_string(date + ": " + news[keys[i]], 0, strlen(date)+2) });
    }
  }

  if(!sizeof(news_items))
    write("No new hat news.\n");
  else
    this_player()->more(news_items);
}

int do_addhatnews(string arg) {
  // TODO make a better security routine
  if((string)this_player()->query_real_name() != "maker" &&
  (string)this_player()->query_real_name() != "ninjutsu")
    return 0;

  write_file(HAT_NEWS, time()+"|"+arg+"\n");
  writef("Added news: \""+arg+"\"");
  return 1;
}

int do_hatnews(string arg) {
  if(arg == "all")
    check_news(-1);
  else
    check_news(to_int(arg));
  return 1;
}
