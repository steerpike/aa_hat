#ifndef HAT_CONFIG_TEXT_FLAGS
#define HAT_CONFIG_TEXT_FLAGS

#define TEXT_FLAGS ({              \
  "TEXT_NOT_MANDATORY",            \
  "TEXT_REQUIRE_ENDING_NL",        \
  "TEXT_DENY_ENDING_PUNC",         \
  "TEXT_DENY_ARTICLE",             \
  "TEXT_REQUIRE_ARTICLE",          \
  "TEXT_ALLOW_REDIRECT",           \
  "TEXT_ALLOW_CONTRACTION",        \
  "TEXT_DENY_MULTIPLE_WORDS",      \
  "TEXT_EXCEPTION_ENDING_SPACE",   \
  "TEXT_EXCEPTION_ENDING_NL",      \
  "TEXT_EXCEPTION_ALLOW_ARTICLE",  \
  "TEXT_EXCEPTION_INLINE_NL",      \
  "TEXT_EXCEPTION_NAME_IN_ALIAS",  \
  "TEXT_CHECK_LIMITS"})

#define TEXT_NOT_MANDATORY            (1 <<  0) // Allow text to be 0 or ""
#define TEXT_REQUIRE_ENDING_NL        (1 <<  1) // Ending newline needed
#define TEXT_DENY_ENDING_PUNC         (1 <<  2) // Ending punctuation is bad
#define TEXT_DENY_ARTICLE             (1 <<  3) // A-An-Some-The is bad
#define TEXT_REQUIRE_ARTICLE          (1 <<  4) // A-An-Some-The needed
#define TEXT_ALLOW_REDIRECT           (1 <<  5) // Redirection is okay
#define TEXT_ALLOW_CONTRACTION        (1 <<  6) // Contractions are okay
#define TEXT_DENY_MULTIPLE_WORDS      (1 <<  7) // Only single word allowed
#define TEXT_EXCEPTION_ENDING_SPACE   (1 <<  8) // Ignore mudlib's whitespace
#define TEXT_EXCEPTION_ENDING_NL      (1 <<  9) // Ignore mudlib's newline
#define TEXT_EXCEPTION_INLINE_NL      (1 << 10) // Ignore mudlib's inline nl
#define TEXT_EXCEPTION_ALLOW_ARTICLE  (1 << 11) // Article is auto inserted
#define TEXT_EXCEPTION_NAME_IN_ALIAS  (1 << 12) // Ignore the name in alias
#define TEXT_CHECK_LIMITS             (1 << 13) // Check the length of text

#endif
