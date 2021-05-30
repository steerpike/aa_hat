#ifndef HAT_CONFIG_PROTOTYPES
#define HAT_CONFIG_PROTOTYPES

// From common_functions.c
void report(object o, string s, int channel);
void check_name(object o, int flags);
int check_short(object o, int flags, mapping extra);
void check_long(object o, int flags, mapping extra);
void check_identify(object o, int flags);
void check_set_sense(object o, string thing, int flags);
void check_add_senses(object o, int i);
void check_material(object o, int mandatory);
void check_recommended_value(object o, int rec);
varargs int text_check(object o, string what, string text, int flags, mapping extra);

#endif
