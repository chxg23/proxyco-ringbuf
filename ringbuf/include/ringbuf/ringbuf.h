#ifndef RINGBUF_H
#define RINGBUF_H

#include <inttypes.h>

/**
 * Ring buffer for fixed-size entries.
 */

struct ringbuf {
  uint8_t *buf;
  uint8_t *buf_end;
  uint8_t *buf_end_cur;
  /* Pointer to the start of the first entry. */
  uint8_t *head;
  /* Pointer to the start of the last entry. */
  uint8_t *tail;
  /* Size of each entry. */
  uint16_t len;
};

struct ringbuf_iter {
  uint8_t *start;
  uint8_t *cur;
  uint8_t *end;
};

int rb_init(struct ringbuf *rb, void *buf, uint16_t buf_len, uint16_t len);
int rb_append(struct ringbuf *rb, const void *data, uint16_t len);
/* Entry must have been previously returned from rb_iter_next(). */
int rb_flush_to(struct ringbuf *rb, uint8_t *entry);
int rb_flush(struct ringbuf *rb);
void rb_iter_start(struct ringbuf *rb, struct ringbuf_iter *iter);
uint8_t *rb_iter_next(struct ringbuf *rb, struct ringbuf_iter *iter);

// typedef int (*rb_walk_fn_t)(struct ringbuf *, uint8_t *, void *);
// int rb_walk(struct ringbuf *rb, rb_walk_fn_t fn, void *arg);

#endif
