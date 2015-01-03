/*
 * File:   yl_memory_file_test.c
 * Author: user
 *
 * Created on Jan 1, 2015, 8:28:58 AM
 */

#include "../../yl_memory_file/yl_memory_file.h"

#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

/*
 * CUnit Test Suite
 */

int init_suite(void) {
   return 0;
}

int clean_suite(void) {
   return 0;
}

void test_create_destroy() {
   static const int default_buffer_size = 1024;

   struct yl_memory_file *memory_file = ylmf_create_with_buffer_size(default_buffer_size);
   CU_ASSERT_PTR_NOT_NULL_FATAL(memory_file);

   unsigned buffer_size = ylmf_get_buffer_size(memory_file);
   CU_ASSERT_EQUAL_FATAL(buffer_size, default_buffer_size);

   ylmf_destroy(&memory_file);
   CU_ASSERT_PTR_NULL_FATAL(memory_file);
}

void test_buffer_size() {
   static const int default_buffer_size = 1024;

   struct yl_memory_file *memory_file = ylmf_create_with_buffer_size(default_buffer_size);
   if (NULL != memory_file) {
      CU_PASS("Successfully created memory file");

      unsigned buffer_size = ylmf_get_buffer_size(memory_file);
      CU_ASSERT_EQUAL_FATAL(buffer_size, default_buffer_size);

      static const int new_buffer_size = 4 * 1024;
      ylmf_set_buffer_size(memory_file, new_buffer_size);

      buffer_size = ylmf_get_buffer_size(memory_file);
      CU_ASSERT_EQUAL_FATAL(buffer_size, new_buffer_size);

      ylmf_destroy(&memory_file);
      CU_ASSERT_PTR_NULL_FATAL(memory_file);
   } else {
      CU_FAIL_FATAL("Failed to create memory file");
   }
}

static void do_test_simple_read_write(struct yl_memory_file *memory_file,
        const char *write_buffer, unsigned write_buffer_size) {
   unsigned buffer_size = write_buffer_size;
   char *read_buffer = (char *) malloc(buffer_size);

   int i;
   long long write_offset = 0, read_offset = 0;
   for (i = 0; i < 1; ++i) {
      ylmf_write(memory_file, write_buffer, write_buffer_size, &write_offset);
      unsigned available = ylmf_get_buffer_available(memory_file);
      CU_ASSERT_EQUAL(available, write_buffer_size);

      unsigned read_count = ylmf_read(memory_file, read_buffer, buffer_size, &read_offset);
      CU_ASSERT_EQUAL(read_count, write_buffer_size);

      int compare_result = memcmp(read_buffer, write_buffer, read_count);
      CU_ASSERT_EQUAL_FATAL(compare_result, 0);

      available = ylmf_get_buffer_available(memory_file);
      CU_ASSERT_EQUAL(available, 0);
   }
   
   free(read_buffer);
}

void test_buffer_simple_read_write() {
   static const char write_buffer[] = "My test buffer";
   static const unsigned write_buffer_size = sizeof (write_buffer) / sizeof (write_buffer[0]);
   static const unsigned default_buffer_size = sizeof (write_buffer) / sizeof (write_buffer[0]);

   struct yl_memory_file *memory_file = ylmf_create_with_buffer_size(default_buffer_size);
   if (NULL != memory_file) {
      CU_PASS("Successfully created memory file");

      do_test_simple_read_write(memory_file, write_buffer, write_buffer_size);

      ylmf_destroy(&memory_file);
      CU_ASSERT_PTR_NULL_FATAL(memory_file);
   } else {
      CU_FAIL_FATAL("Failed to create memory file");
   }
}

void test_buffer_read_write1() {
   static const int default_buffer_size = 64;

   struct yl_memory_file *memory_file = ylmf_create_with_buffer_size(default_buffer_size);
   if (NULL != memory_file) {
      CU_PASS("Successfully created memory file");

      int i;
      long long write_offset = 0, read_offset = 0;
      long long second_write_offset = 0, second_read_offset = 0;
      for (i = 0; i < 1; ++i) {
         static const char first_write_buffer[] = "My first test buffer";
         static const unsigned first_write_buffer_size = sizeof (first_write_buffer) / sizeof (first_write_buffer[0]);

         static const char second_write_buffer[] = "My second test buffer";
         static const unsigned second_write_buffer_size = sizeof (second_write_buffer) / sizeof (second_write_buffer[0]);

         // Write first buffer
         ylmf_write(memory_file, first_write_buffer, first_write_buffer_size, &write_offset);
         unsigned available = ylmf_get_buffer_available(memory_file);
         CU_ASSERT_EQUAL_FATAL(available, first_write_buffer_size);

         //
         // Read first test
         char read_buffer[default_buffer_size];
         unsigned read_count = ylmf_read(memory_file, read_buffer, first_write_buffer_size, &read_offset);
         CU_ASSERT_EQUAL_FATAL(read_count, first_write_buffer_size);
         
         available = ylmf_get_buffer_available(memory_file);
         CU_ASSERT_EQUAL_FATAL(available, 0);
         
         int compare_result = memcmp(read_buffer, first_write_buffer, read_count);
         CU_ASSERT_EQUAL_FATAL(compare_result, 0);
         
         // Write second buffer
         ylmf_write(memory_file, second_write_buffer, second_write_buffer_size, &second_write_offset);
         available = ylmf_get_buffer_available(memory_file);
         CU_ASSERT_EQUAL_FATAL(available, second_write_buffer_size);
         
         //
         // Read second test
         read_count = ylmf_read(memory_file, read_buffer, second_write_buffer_size, &second_read_offset);
         CU_ASSERT_EQUAL(read_count, second_write_buffer_size);

         available = ylmf_get_buffer_available(memory_file);
         CU_ASSERT_EQUAL_FATAL(available, 0);

         compare_result = memcmp(read_buffer, second_write_buffer, read_count);
         CU_ASSERT_EQUAL_FATAL(compare_result, 0);
      }

      ylmf_destroy(&memory_file);
      CU_ASSERT_PTR_NULL_FATAL(memory_file);
   } else {
      CU_FAIL_FATAL("Failed to create memory file");
   }
}

void test_buffer_read_write2() {
   static const int default_buffer_size = 128;

   struct yl_memory_file *memory_file = ylmf_create_with_buffer_size(default_buffer_size);
   if (NULL != memory_file) {
      CU_PASS("Successfully created memory file");

      int i;
      long long write_offset = 0, read_offset = 0;
      long long second_write_offset = 0, second_read_offset = 0;
      for (i = 0; i < 1; ++i) {
         static const char first_write_buffer[] = "My first test buffer";
         static const unsigned first_write_buffer_size = sizeof (first_write_buffer) / sizeof (first_write_buffer[0]);

         static const char second_write_buffer[] = "And this is my second test case for memory file";
         static const unsigned second_write_buffer_size = sizeof (second_write_buffer) / sizeof (second_write_buffer[0]);

         // Write first buffer
         ylmf_write(memory_file, first_write_buffer, first_write_buffer_size, &write_offset);
         unsigned available = ylmf_get_buffer_available(memory_file);
         CU_ASSERT_EQUAL_FATAL(available, first_write_buffer_size);

         // Write second buffer
         ylmf_write(memory_file, second_write_buffer, second_write_buffer_size, &second_write_offset);
         available = ylmf_get_buffer_available(memory_file);
         CU_ASSERT_EQUAL_FATAL(available, first_write_buffer_size + second_write_buffer_size);
         
         //
         // Read first test
         char read_buffer[default_buffer_size];
         unsigned read_count = ylmf_read(memory_file, read_buffer, first_write_buffer_size, &read_offset);
         CU_ASSERT_EQUAL_FATAL(read_count, first_write_buffer_size);
         
         available = ylmf_get_buffer_available(memory_file);
         CU_ASSERT_EQUAL_FATAL(available, second_write_buffer_size);
         
         int compare_result = memcmp(read_buffer, first_write_buffer, read_count);
         CU_ASSERT_EQUAL_FATAL(compare_result, 0);
         
         //
         // Read second test
         read_count = ylmf_read(memory_file, read_buffer, second_write_buffer_size, &second_read_offset);
         CU_ASSERT_EQUAL(read_count, second_write_buffer_size);

         available = ylmf_get_buffer_available(memory_file);
         CU_ASSERT_EQUAL_FATAL(available, 0);

         compare_result = memcmp(read_buffer, second_write_buffer, read_count);
         CU_ASSERT_EQUAL_FATAL(compare_result, 0);
      }

      ylmf_destroy(&memory_file);
      CU_ASSERT_PTR_NULL_FATAL(memory_file);
   } else {
      CU_FAIL_FATAL("Failed to create memory file");
   }
}

int main() {
   CU_pSuite pSuite = NULL;

   /* Initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* Add a suite to the registry */
   pSuite = CU_add_suite("memory_file_test", init_suite, clean_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Add the tests to the suite */
   if ((NULL == CU_add_test(pSuite, "test_create_destroy", test_create_destroy))
           || (NULL == CU_add_test(pSuite, "test_buffer_size", test_buffer_size))
           || (NULL == CU_add_test(pSuite, "test_buffer_simple_read_write", test_buffer_simple_read_write))
           || (NULL == CU_add_test(pSuite, "test_buffer_read_write1", test_buffer_read_write1))
           || (NULL == CU_add_test(pSuite, "test_buffer_read_write2", test_buffer_read_write2))) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}
