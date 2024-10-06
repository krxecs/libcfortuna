#ifndef TIMESPEC_H
#define TIMESPEC_H

#include <clock/clock.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if !defined(TIMESPEC_STATIC_INLINE)
#if defined(__GNUC__)
#define TIMESPEC__COMPILER_ATTRIBUTES __attribute__((__unused__))
#else
#define TIMESPEC__COMPILER_ATTRIBUTES
#endif

#if defined(HEDLEY_INLINE)
#define TIMESPEC__INLINE HEDLEY_INLINE
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define TIMESPEC__INLINE inline
#elif defined(__GNUC_STDC_INLINE__)
#define TIMESPEC__INLINE __inline__
#elif defined(_MSC_VER) && _MSC_VER >= 1200
#define TIMESPEC__INLINE __inline
#else
#define TIMESPEC__INLINE
#endif

#define TIMESPEC__FUNCTION TIMESPEC__COMPILER_ATTRIBUTES static TIMESPEC__INLINE
#endif

enum { NS_PER_SECOND = 1000000000 };

TIMESPEC__FUNCTION void timespec_subtract(struct PsnipClockTimespec t1,
                                          struct PsnipClockTimespec t2,
                                          struct PsnipClockTimespec *td) {
  td->nanoseconds = t2.nanoseconds - t1.nanoseconds;
  td->seconds = t2.seconds - t1.seconds;
  if (td->seconds > 0 && td->nanoseconds < 0) {
    td->nanoseconds += NS_PER_SECOND;
    td->seconds--;
  } else if (td->seconds < 0 && td->nanoseconds > 0) {
    td->nanoseconds -= NS_PER_SECOND;
    td->seconds++;
  }
}

TIMESPEC__FUNCTION void timespec_add(struct PsnipClockTimespec t1,
                                     struct PsnipClockTimespec t2,
                                     struct PsnipClockTimespec *td) {
  td->nanoseconds = t2.nanoseconds + t1.nanoseconds;
  td->seconds = t2.seconds + t1.seconds;
  if (td->nanoseconds >= NS_PER_SECOND) {
    td->nanoseconds -= NS_PER_SECOND;
    td->seconds++;
  } else if (td->nanoseconds <= -NS_PER_SECOND) {
    td->nanoseconds += NS_PER_SECOND;
    td->seconds--;
  }
}

TIMESPEC__FUNCTION int timespec_cmp(struct PsnipClockTimespec a,
                                    struct PsnipClockTimespec b) {
  if (a.seconds == b.seconds) {
    if (a.nanoseconds == b.nanoseconds)
      return 0;
    else
      return a.nanoseconds < b.nanoseconds ? -1 : 1;
  }

  return a.seconds < b.seconds ? -1 : 1;
}

#endif /* TIMESPEC_H */
