#pragma strict_types

#include <xfun.h>

#include "../hat_def.h"

int last_time_news_checked;

// ([ unix time: "news strings" ]);
mapping news;

int query_last_time_news_checked() { return last_time_news_checked; }

void save_news() {
  save_object(HAT_SAVE + "hat");
}

void load_news() {
  if(!load_object(HAT_SAVE + "hat"))
    news = ([]);
}

void add_news(string arg) {
  // TODO make a better security routine
  if((string)this_player()->query_real_name() != "maker")
    return;

  news += ([ time(): arg ]);
  save_news();
}

void check_news(int num_of_items) {
  int i, *keys, time_threshold;
  string *news_items;

  if(!num_of_items) {
    time_threshold = query_last_time_news_checked();
    num_of_items = sizeof(news);
  }
  
  last_time_news_checked = time();

  news_items = ({});
  keys = sort_array(m_indices(news), #'<); //'
  for(i=0; i<sizeof(keys) && i<num_of_items; i++) {
    if(keys[i] > time_threshold)
      news_items += ({ (string)XFUN->short_time(keys[i]) + news[keys[i]] });
  }

  this_player()->more(news_items);
}
