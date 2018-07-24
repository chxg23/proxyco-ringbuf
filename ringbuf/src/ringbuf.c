#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "ringbuf/ringbuf.h"

int
rb_init(struct ringbuf *rb, void *buf, uint16_t buf_len, uint16_t len)
{
  memset(rb, 0, sizeof(*rb));
  rb->buf = buf;
  rb->buf_end = buf + buf_len;
  rb->len = len;
  return 0;
}

int
rb_append(struct ringbuf *rb, void *data, uint16_t len)
{
  uint8_t *start;
  uint8_t *end;
  uint8_t *dst;

  assert(len == rb->len);

  if (rb->tail) {
    dst = rb->tail + len;
  } else {
    dst = rb->buf;
  }
  end = dst + len;

  /* If this entry would take us past the end of the buffer, then wrap it
   * around to the beginning of the buffer. */
  if (end > rb->buf_end) {
    rb->buf_end_cur = dst;
    dst = rb->buf;
    end = dst + len;
    if (rb->head >= rb->buf_end_cur) {
      rb->head = rb->buf;
    }
  }

  /* If destination is prior to the start, and would overwrite the start of
   * the buffer, move head head forward until we don't overwrite it anymore. */
  start = rb->head;
  if (start && dst < start + len && start < end) {
    while (start < end) {
      start = start + len;
      if (start == rb->buf_end_cur) {
        start = rb->buf;
        break;
      }
    }
    rb->head = start;
  }

  /* Copy the entry into the ring buffer. */
  memcpy(dst, data, len);

  rb->tail = dst;
  if (!rb->head) {
    rb->head = dst;
  }

  return 0;
}

int
rb_flush_to(struct ringbuf *rb, uint8_t *entry)
{
  assert(entry);
  assert(entry >= rb->buf && (!rb->buf_end_cur || entry < rb->buf_end));

  rb->head = entry;
  return 0;
}

int
rb_flush(struct ringbuf *rb)
{
  rb->buf_end_cur = NULL;
  rb->head = NULL;
  rb->tail = NULL;
  return 0;
}

void
rb_iter_start(struct ringbuf *rb, struct ringbuf_iter *iter)
{
  iter->start = rb->head;
  iter->cur = rb->head;
  iter->end = rb->tail;
}

uint8_t *
rb_iter_next(struct ringbuf *rb, struct ringbuf_iter *iter)
{
  uint8_t *data;

  if (iter->start > iter->end) {
    data = iter->cur;
    iter->cur = iter->cur + rb->len;

    if (iter->cur >= rb->buf_end_cur) {
      iter->cur = rb->buf;
      iter->start = rb->buf;
    }
  } else {
    data = iter->cur;
    if (!data) {
      return NULL;
    }

    if (data == iter->end + rb->len) {
      data = NULL;
    } else {
      iter->cur = iter->cur + rb->len;
    }
  }
  return data;
}
