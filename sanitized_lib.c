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

//#include "common.h"
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

/*
void test_heap_overflow(void) {
  int oob_index = 18;
  int size = 17;
  unsigned char *ptr = malloc(size);
  printf("\nKASan test: heap OOB write\n");
  printf("Writing 1 byte at offset %d in %d-byte heap buffer allocated at %x\n",
         oob_index, size, ptr);
  ptr[oob_index] = 0;
}

char oob_value;

void test_stack_overflow(void) {
  char buffer[17];
  int oob_index = 18;
  printf("\nKASan test: stack OOB read\n");
  printf("Reading 1 byte at offset %d in %d-byte stack buffer at %x\n",
         oob_index, sizeof(buffer), buffer);
  oob_value = buffer[oob_index];
}

int global_array[17];

void test_globals_overflow(void) {
  int oob_index = 18;
  printf("\nKASan test: global OOB write\n");
  printf(
      "Writing an integer at index %d in %d-element global integer array at "
      "%x\n",
      oob_index, sizeof(global_array) / sizeof(int), global_array);
  global_array[oob_index] = 0;
}

char global_char_buffer[17];

void test_memset_overflow(void) {
  int oob_size = 18;
  printf("\nKASan test: memset OOB write in globals\n");
  printf("Memsetting global %d-byte buffer at %x with %d values of 0xaa\n",
         sizeof(global_char_buffer), global_char_buffer, oob_size);
  memset(global_char_buffer, 0xaa, oob_size);
}

void test_memcpy_overflow(void) {
  char buffer[18];
  int oob_size = sizeof(buffer);
  printf("\nKASan test: memcpy OOB read from globals\n");
  printf("Memcopying %d bytes from %d-byte global buffer into local array\n",
         oob_size, sizeof(global_char_buffer));
  memcpy(buffer, global_char_buffer, oob_size);
}


/*====================================================================
                  UBSAN TESTCASES                                     */

//typedef void(*test_ubsan_fp)(void);

/*
#define UBSAN_TEST(config, ...)	do {					\
		pr_info("%s " __VA_ARGS__ "%s(%s=%s)\n", __func__,	\
			sizeof(" " __VA_ARGS__) > 2 ? " " : "",		\
			#config, IS_ENABLED(config) ? "y" : "n");	\
	} while (0)
  */

static void test_ubsan_add_overflow(void)
{
	volatile int val = INT_MAX;
  printf("\nKUBSan test: integer overflow in add operation\n");
  printf("Overflowing integer variable with INT_MAX value\n");
  

	//UBSAN_TEST(CONFIG_UBSAN_INTEGER_WRAP);
	val += 2;
  //printf("%d", val);
}

static void test_ubsan_sub_overflow(void)
{
	volatile int val = INT_MIN;
	volatile int val2 = 2;

  printf("\nKUBSan test: integer underflow in sub operation\n");
  printf("Underflowing integer variable with INT_MIN value\n");

	//UBSAN_TEST(CONFIG_UBSAN_INTEGER_WRAP);
	val -= val2;

}

static void test_ubsan_mul_overflow(void)
{
	volatile int val = INT_MAX / 2;

  
  printf("\nKUBSan test: integer overflow in mul operation\n");
  printf("Overflowing integer variable with INT_MAX value\n");

	//UBSAN_TEST(CONFIG_UBSAN_INTEGER_WRAP);
	val *= 3;
}

static void test_ubsan_negate_overflow(void)
{
	volatile int val = INT_MIN;

  printf("\nKUBSan test: integer negate overflow\n");
  printf("Overflowing by negating integer variable with INT_MIN value\n");

	//UBSAN_TEST(CONFIG_UBSAN_INTEGER_WRAP);
	val = -val;
}

static void test_ubsan_divrem_overflow(void)
{
	volatile int val = 16;
	volatile int val2 = 0;

  printf("\nKUBSan test: integer division by zero\n");
  printf("Dividing by zero variable with %d value\n", val);

	//UBSAN_TEST(CONFIG_UBSAN_DIV_ZERO);
	val /= val2;
}

static void test_ubsan_truncate_signed(void)
{
	volatile long val = LONG_MAX;
	volatile int val2 = 0;

  printf("\nKUBSan test: signed variable truncation\n");
  printf("Truncating long variable with LONG_MAX value assigning its value to a int variable\n");

	//UBSAN_TEST(CONFIG_UBSAN_INTEGER_WRAP);
	val2 = val;

  
}

static void test_ubsan_shift_out_of_bounds(void)
{
	volatile int neg = -1, wrap = 4;
	volatile int val1 = 10;
	volatile int val2 = INT_MAX;

  printf("\nKUBSan test: OOB shift\n");
  printf("Performing negative exponent right shift and left overflow shift in INT_MAX variable\n");

	//UBSAN_TEST(CONFIG_UBSAN_SHIFT, "negative exponent");
	val1 <<= neg;

	//UBSAN_TEST(CONFIG_UBSAN_SHIFT, "left overflow");
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

  /*
	OPTIMIZER_HIDE_VAR(i);
	OPTIMIZER_HIDE_VAR(j);
	OPTIMIZER_HIDE_VAR(k);
  */

	//UBSAN_TEST(CONFIG_UBSAN_BOUNDS, "above");
	data.arr[j] = i;

	//UBSAN_TEST(CONFIG_UBSAN_BOUNDS, "below");
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

	//UBSAN_TEST(CONFIG_UBSAN_BOOL, "bool");
	dst = (char *)&val;
	src = &c;
	*dst = *src;

	ptr = &val2;
	val2 = val;

	//UBSAN_TEST(CONFIG_UBSAN_ENUM, "enum");
	dst = (char *)&eval;
	src = &c;
	*dst = *src;

	eptr = &eval2;
	eval2 = eval;
}

/*
static void test_ubsan_misaligned_access(void)
{
	volatile char arr[5] __aligned(4) = {1, 2, 3, 4, 5};
	volatile int *ptr, val = 6;

  printf("\nKUBSan test: missaligned access\n");
  printf("Attempting to load a value into missaligned array pointer\n");

	//UBSAN_TEST(CONFIG_UBSAN_ALIGNMENT);
	ptr = (int *)(arr + 1);
	*ptr = val;
}
*/

/*

static const test_ubsan_fp test_ubsan_array[] = {
	test_ubsan_add_overflow,
	test_ubsan_sub_overflow,
	test_ubsan_mul_overflow,
	test_ubsan_negate_overflow,
	test_ubsan_truncate_signed,
	test_ubsan_shift_out_of_bounds,
	test_ubsan_out_of_bounds,
	test_ubsan_load_invalid_value,
	test_ubsan_misaligned_access,
};

 /*Excluded because they Oops the module. 
static __used const test_ubsan_fp skip_ubsan_array[] = {
	test_ubsan_divrem_overflow,
};

static int __init test_ubsan_init(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(test_ubsan_array); i++)
		test_ubsan_array[i]();

	return 0;
}
module_init(test_ubsan_init);

static void __exit test_ubsan_exit(void)
{
	/* do nothing 
}
module_exit(test_ubsan_exit);

MODULE_AUTHOR("Jinbum Park <jinb.park7@gmail.com>");
MODULE_DESCRIPTION("UBSAN unit test");
MODULE_LICENSE("GPL v2");
*/