/*
 * Copyright 2024 Google LLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "heap.h"
#include "printf.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "ubsan.h"
#ifndef INT_MIN
#define INT_MIN          (-__INT_MAX__ - 1)
#endif
#ifndef INT_MAX
#define INT_MAX          __INT_MAX__
#endif

#ifndef LONG_MIN
#define LONG_MIN         (-__LONG_MAX__ - 1)
#endif
#ifndef LONG_MAX
#define LONG_MAX         __LONG_MAX__
#endif


static void test_ubsan_add_overflow(void)
{
	volatile int val = INT_MAX;
  printf("\nKUBSan test: integer overflow in add operation\n");
  printf("Overflowing integer variable with INT_MAX value\n");
  
	val += 2;
}

static void test_ubsan_sub_overflow(void)
{
	volatile int val = INT_MIN;
	volatile int val2 = 2;

  printf("\nKUBSan test: integer underflow in sub operation\n");
  printf("Underflowing integer variable with INT_MIN value\n");

	val -= val2;

}

static void test_ubsan_mul_overflow(void)
{
	volatile int val = INT_MAX / 2;

  
  printf("\nKUBSan test: integer overflow in mul operation\n");
  printf("Overflowing integer variable with INT_MAX value\n");

	val *= 3;
}

static void test_ubsan_negate_overflow(void)
{
	volatile int val = INT_MIN;

  printf("\nKUBSan test: integer negate overflow\n");
  printf("Overflowing by negating integer variable with INT_MIN value\n");

	val = -val;
}

static void test_ubsan_divrem_overflow(void)
{
	volatile int val = 16;
	volatile int val2 = 0;

  printf("\nKUBSan test: integer division by zero\n");
  printf("Dividing by zero variable with %d value\n", val);

	val /= val2;
}

static void test_ubsan_truncate_signed(void)
{
	volatile long val = LONG_MAX;
	volatile int val2 = 0;

  printf("\nKUBSan test: signed variable truncation\n");
  printf("Truncating long variable with LONG_MAX value assigning its value to a int variable\n");

	val2 = val;

  
}

static void test_ubsan_shift_out_of_bounds(void)
{
	volatile int neg = -1, wrap = 4;
	volatile int val1 = 10;
	volatile int val2 = INT_MAX;

  printf("\nKUBSan test: OOB shift\n");
  printf("Performing negative exponent right shift and left overflow shift in INT_MAX variable\n");

	val1 <<= neg;

	val2 <<= wrap;
}


static void test_ubsan_out_of_bounds(void)
{
	int i = 4, j = 4, k = -1;
	volatile struct {
		char above[4]; /* Protect surrounding memory. */
		int arr[4];
		char below[4]; /* Protect surrounding memory. */
	} data;

  printf("\nKUBSan test: OOB write\n");
  printf("Attempting to write data in OOB space of a %d positions array\n", i);

	data.arr[j] = i;
	data.arr[k] = i;
}

enum ubsan_test_enum {
	UBSAN_TEST_ZERO = 0,
	UBSAN_TEST_ONE,
	UBSAN_TEST_MAX,
};

static void test_ubsan_load_invalid_value(void)
{
	volatile char *dst, *src;
	bool val, val2, *ptr;
	enum ubsan_test_enum eval, eval2, *eptr;
	unsigned char c = 0xff;

  printf("\nKUBSan test: Invalid load value\n");
  printf("Attempting to load a value with wrong type pointers\n");

	dst = (char *)&val;
	src = &c;
	*dst = *src;

	ptr = &val2;
	val2 = val;

	dst = (char *)&eval;
	src = &c;
	*dst = *src;

	eptr = &eval2;
	eval2 = eval;
}
