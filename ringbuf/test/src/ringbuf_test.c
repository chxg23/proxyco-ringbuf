#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <assert.h>

#include <sysinit/sysinit.h>
#include <testutil/testutil.h>

#include "ringbuf/ringbuf.h"

#define RB_BUF_SIZE     (256)
#define RB_ENTRY_SIZE   (4)

#define RB_ENTRY_COUNT  (66)

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

  for (i = 0; i < RB_ENTRY_COUNT; i++) {
    rb_entry[0] = i;
    rc = rb_append(&rb, rb_entry, sizeof(rb_entry));
    assert(rc == 0);
  }
}

/* Append and iterate. */
TEST_CASE(test_append_and_iterate)
{
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

    TEST_ASSERT_FATAL(entry[0] == i);
    for (j = 1; j < RB_ENTRY_SIZE; j++) {
      TEST_ASSERT_FATAL(entry[j] == 0xff);
    }
    i++;
  }

  TEST_ASSERT_FATAL(i == RB_ENTRY_COUNT);
}

/* Flush. */
TEST_CASE(test_flush)
{
  int rc;
  uint8_t i;
  uint8_t *entry;

  struct ringbuf_iter iter;

  rc = rb_flush(&rb);
  TEST_ASSERT_FATAL(rc == 0);

  i = 0;
  rb_iter_start(&rb, &iter);
  while (1) {
    entry = rb_iter_next(&rb, &iter);
    if (entry == NULL) {
      break;
    }
    i++;
  }

  TEST_ASSERT_FATAL(i == 0);
}

/* Flush and recycle. */
TEST_CASE(test_flush_and_recycle)
{
  int i;
  int rc;

  test_flush();

  for (i = 0; i < 66; i++) {
    rb_entry[0] = i;
    rc = rb_append(&rb, rb_entry, sizeof(rb_entry));
    TEST_ASSERT_FATAL(rc == 0);
  }

  test_append_and_iterate();
}

/* Flush leading entries. */
TEST_CASE(test_flush_leading_entries)
{
  int rc;
  uint8_t i, j;
  uint8_t *entry;

  struct ringbuf_iter iter;

  rc = rb_flush_to(&rb, rb.head + sizeof(rb_entry) * 2);
  TEST_ASSERT_FATAL(rc == 0);

  i = 4;
  rb_iter_start(&rb, &iter);
  while (1) {
    entry = rb_iter_next(&rb, &iter);
    if (entry == NULL) {
      break;
    }

    TEST_ASSERT_FATAL(entry[0] == i);
    for (j = 1; j < RB_ENTRY_SIZE; j++) {
      TEST_ASSERT_FATAL(entry[j] == 0xff);
    }
    i++;
  }

  TEST_ASSERT_FATAL(i == RB_ENTRY_COUNT);
}


TEST_SUITE(test_ringbuf_suite) {
  test_append_and_iterate();
  test_flush();
  test_flush_and_recycle();
  test_flush_leading_entries();
}

#if MYNEWT_VAL(SELFTEST)
int
main(int argc, char **argv)
{
  sysinit();

  setup();

  test_ringbuf_suite();

  return tu_any_failed;
}
#endif
