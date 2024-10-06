#ifndef U128_H
#define U128_H

#include <builtin/builtin.h>
#include <endian/endian.h>
#include <stdint.h>

#ifndef INLINE
#if __GNUC__ && !__GNUC_STDC_INLINE__
#define INLINE extern inline
#else
#define INLINE inline
#endif
#endif

#if PSNIP_ENDIAN_ORDER == PSNIP_ENDIAN_BIG
#define U128_UPPER_IDX 0
#define U128_LOWER_IDX 1
#elif PSNIP_ENDIAN_ORDER == PSNIP_ENDIAN_LITTLE
#define U128_LOWER_IDX 0
#define U128_UPPER_IDX 1
#else
#error Could not determine endianness of system
#endif

typedef struct u128 {
  uint64_t halves[2];
} u128;

INLINE uint64_t u128_get_lower(u128 result) {
  return result.halves[U128_LOWER_IDX];
}

INLINE uint64_t u128_get_upper(u128 result) {
  return result.halves[U128_UPPER_IDX];
}

INLINE void u128_set_lower(u128 *result, uint64_t lower) {
  result->halves[U128_LOWER_IDX] = lower;
}

INLINE void u128_set_upper(u128 *result, uint64_t upper) {
  result->halves[U128_UPPER_IDX] = upper;
}

INLINE u128 u128_init_from_u64(uint64_t upper, uint64_t lower) {
  u128 result;
  u128_set_upper(&result, upper);
  u128_set_lower(&result, lower);

  return result;
}

INLINE void u128_addto(u128 *result, u128 lhs, u128 rhs) {
  uint64_t lower = u128_get_lower(lhs) + u128_get_lower(rhs);
  uint64_t carry = lower < u128_get_lower(lhs) ? 1 : 0;
  uint64_t upper = u128_get_upper(lhs) + u128_get_upper(rhs) + carry;

  u128_set_lower(result, lower);
  u128_set_upper(result, upper);
}

INLINE u128 u128_add(u128 lhs, u128 rhs) {
  u128 result;
  u128_addto(&result, lhs, rhs);

  return result;
}

INLINE void u128_subtractfrom(u128 *result, u128 lhs, u128 rhs) {
  uint64_t lower = u128_get_lower(lhs) - u128_get_lower(rhs);
  uint64_t borrow = lower > u128_get_lower(lhs) ? 1 : 0;
  uint64_t upper = u128_get_upper(lhs) - u128_get_upper(rhs) - borrow;

  u128_set_lower(result, lower);
  u128_set_upper(result, upper);
}

INLINE u128 u128_subtract(u128 lhs, u128 rhs) {
  u128 result;
  u128_subtractfrom(&result, lhs, rhs);

  return result;
}

INLINE void u128_increment(u128 *number) {
  u128 u128_1 = u128_init_from_u64(0, 1);
  *number = u128_add(*number, u128_1);
}

INLINE int8_t u128_compare(u128 lhs, u128 rhs) {
  if (u128_get_upper(lhs) == u128_get_upper(rhs)) {
    if (u128_get_lower(lhs) == u128_get_lower(rhs)) {
      return 0;
    } else {
      return u128_get_lower(lhs) < u128_get_lower(rhs) ? -1 : 1;
    }
  }

  return u128_get_upper(lhs) < u128_get_upper(rhs) ? -1 : 1;
}

#define U64_BIT (CHAR_BIT * sizeof(uint64_t))

INLINE u128 u128_shift_left(u128 number, uint8_t shift) {
  if (shift == 64) {
    return u128_init_from_u64(u128_get_lower(number), 0);
  } else if (shift == 0) {
    return number;
  } else if (shift < 64) {
    return u128_init_from_u64((u128_get_upper(number) << shift) +
                                  (u128_get_lower(number) >> (U64_BIT - shift)),
                              u128_get_lower(number) << shift);
  } else if (shift < 128) {
    return u128_init_from_u64(u128_get_lower(number) << (shift - U64_BIT), 0);
  }

  /* All other cases (including shift >= 128) handled here. */
  return u128_init_from_u64(0, 0);
}

#endif /* U128_H */
