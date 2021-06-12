#pragma strict_types

#include <xfun.h>
#include <daemons.h>

#include "../hat_def.h"

int last_time_news_checked;
// ([ unix time: "news strings" ]);
mapping news;

int query_last_time_news_checked() { return last_time_news_checked; }
void set_last_time_news_checked(int i) { last_time_news_checked = i; }

mapping query_hatnews() { return ([]) + news; }

void init() {
  add_action("do_hatnews", "hatnews");
  add_action("do_addhatnews", "addhatnews");
}

void load_news() {
  int i;
  string *data, *news_items;

  news = ([]);
  if(file_size(HAT_NEWS) > 0) {
    news_items = explode(read_file(HAT_NEWS), "\n");
    for(i=0; i<sizeof(news_items); i++) {
      data = explode(news_items[i], "|");
      news += ([ to_int(data[0]): implode(data[1..], "|") ]);
    }
    if(file_date(HAT_NEWS) > query_last_time_news_checked())
      COLOURUTIL_D->write_c((string)COLOURUTIL_D->igreen("There is new 'hatnews' (or 'hatnews <num>/all' for details.\n"));
  }
}

void check_news(int num_of_items) {
  int i, *keys, time_threshold;
  string date, *news_items;

  if(!num_of_items) {
    time_threshold = query_last_time_news_checked();
    num_of_items = sizeof(news);
  }
  
  //set_last_time_news_checked(time());

  news_items = ({});
  keys = sort_array(m_indices(news), #'<); //'
  for(i=0; i<sizeof(keys) && i<num_of_items; i++) {
    if(keys[i] > time_threshold) {
      date = explode((string)XFUN->short_time(keys[i]), " ")[0];
      news_items += ({ date + ": " + news[keys[i]] });
    }
  }

  if(!sizeof(news_items))
    write("No new hat news.\n");
  else
    this_player()->more(news_items);
}

int do_addhatnews(string arg) {
  // TODO make a better security routine
  if((string)this_player()->query_real_name() != "maker")
    return 0;

  write_file(HAT_NEWS, time()+"|"+arg+"\n");
  return 1;
}

int do_hatnews(string arg) {
  if(arg == "all")
    check_news(sizeof(news));
  else
    check_news(to_int(arg));
  return 1;
}
