#include <stdio.h>
#include <string.h>

#include <syscfg/syscfg.h>
#include <sysinit/sysinit.h>
#include <testutil/testutil.h>

TEST_SUITE(test_ringbuf_suite) {
}

#if MYNEWT_VAL(SELFTEST)
int
main(int argc, char **argv)
{
  sysinit();

  test_ringbuf_suite();

  return tu_any_failed;
}
#endif
