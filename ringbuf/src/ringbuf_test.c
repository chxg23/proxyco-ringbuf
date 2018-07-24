#ifdef TEST

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <assert.h>

#include "ringbuf/ringbuf.h"

#define STRINGIFY(x) #x
#define TEST_CASE(x) { \
  fprintf(stderr, "Running " STRINGIFY(x) ": "); \
  x(); \
  fprintf(stderr, "PASSED\n"); \
}

#define RB_BUF_SIZE 256
#define RB_ENTRY_SIZE 4

static struct ringbuf rb;
uint8_t rb_buf[RB_BUF_SIZE];
uint8_t rb_entry[RB_ENTRY_SIZE];

static void setup()
{
  int i;
  int rc;

  rc = rb_init(&rb, rb_buf, RB_BUF_SIZE, RB_ENTRY_SIZE);
  assert(rc == 0);

  memset(rb_entry, 0xff, sizeof(rb_entry));

  for (i = 0; i < 66; i++) {
    rb_entry[0] = i;
    rc = rb_append(&rb, rb_entry, sizeof(rb_entry));
    assert(rc == 0);
  }
}

/* Append and iterate. */
static void test_case_1()
{
  int rc;
  uint8_t i, j;
  uint8_t *entry;

  struct ringbuf_iter iter;

  /* Buffer is sized for 64 entries, expect the first 2 to be overwritten. */
  i = 2;
  rb_iter_start(&rb, &iter);
  while (1) {
    entry = rb_iter_next(&rb, &iter);
    if (entry == NULL) {
      break;
    }

    assert(entry[0] == i);
    for (j = 1; j < RB_ENTRY_SIZE; j++) {
      assert(entry[j] == 0xff);
    }
    i++;
  }

  assert(i == 66);
}

/* Flush. */
static void test_case_2()
{
  int rc;
  uint8_t i;
  uint8_t *entry;

  struct ringbuf_iter iter;

  rc = rb_flush(&rb);
  assert(rc == 0);

  i = 0;
  rb_iter_start(&rb, &iter);
  while (1) {
    entry = rb_iter_next(&rb, &iter);
    if (entry == NULL) {
      break;
    }
    i++;
  }

  assert(i == 0);
}

/* Flush and recycle. */
static void test_case_3()
{
  int i;
  int rc;

  test_case_2();

  for (i = 0; i < 66; i++) {
    rb_entry[0] = i;
    rc = rb_append(&rb, rb_entry, sizeof(rb_entry));
    assert(rc == 0);
  }

  test_case_1();
}

/* Flush leading entries. */
static void test_case_4()
{
  int rc;
  uint8_t i, j;
  uint8_t *entry;

  struct ringbuf_iter iter;

  rc = rb_flush_to(&rb, rb.head + sizeof(rb_entry) * 2);
  assert(rc == 0);

  i = 4;
  rb_iter_start(&rb, &iter);
  while (1) {
    entry = rb_iter_next(&rb, &iter);
    if (entry == NULL) {
      break;
    }

    assert(entry[0] == i);
    for (j = 1; j < RB_ENTRY_SIZE; j++) {
      assert(entry[j] == 0xff);
    }
    i++;
  }

  assert(i == 66);
}

int main(int argc, char **argv)
{
  setup();

  TEST_CASE(test_case_1);
  TEST_CASE(test_case_2);
  TEST_CASE(test_case_3);
  TEST_CASE(test_case_4);

  return 0;
}

#endif
