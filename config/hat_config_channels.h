#ifndef HAT_CONFIG_REPORT_CHANNELS
#define HAT_CONFIG_REPORT_CHANNELS

#include <colour.h>

#define CHANNELS ({      \
  COLOUR_WHITE,          \
  COLOUR_INTENSE_YELLOW, \
  COLOUR_INTENSE_RED,    \
  COLOUR_INTENSE_CYAN,   \
  COLOUR_CYAN            \
})

#define HAT_CHANNEL     0
#define ERROR_CHANNEL   1
#define QC_CHANNEL      2
#define BALANCE_CHANNEL 3
#define WORLD_CHANNEL   4

#endif