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

//#include "kasan.h"

#include "heap.h"
#include "printf.h"
//#include "rt_utils.h"
#include "sanitized_lib.c"
#include "sanitized_lib.h"
#include "ubsan.h"

int main(void) {
  printf("Starting bare-metal UBSan test driver.\n");

  // Needed to invoke KASan globals instrumentation.
  //call_global_ctors();

  initialize_heap();
/*
  initialize_kasan();

  test_heap_overflow();
  test_stack_overflow();
  test_globals_overflow();
  test_memset_overflow();
  test_memcpy_overflow();
*/


  test_ubsan_add_overflow();
  test_ubsan_sub_overflow();
  test_ubsan_mul_overflow();
  test_ubsan_negate_overflow();
  test_ubsan_divrem_overflow();
  test_ubsan_truncate_signed();
  test_ubsan_shift_out_of_bounds();
  test_ubsan_out_of_bounds();

  //UNIMPLEMENTED TESTS

	//test_ubsan_load_invalid_value();
	//test_ubsan_misaligned_access();

  printf("Press ctrl + a then x to exit.\n");

  return 0;
}
