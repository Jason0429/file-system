// based on cs3650 starter code

#include "slist.h"

#include <alloca.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// if we cons with an inum, we need to figure out what inum to pass into
// explode
slist_t *s_cons(const char *text, slist_t *rest) {
  slist_t *xs = malloc(sizeof(slist_t));
  xs->data = strdup(text);
  xs->refs = 1;
  xs->next = rest;
  return xs;
}

void s_free(slist_t *xs) {
  if (xs == 0) {
    return;
  }

  xs->refs -= 1;

  if (xs->refs == 0) {
    s_free(xs->next);
    free(xs->data);
    free(xs);
  }
}

// not sure how it would work if we add inums
slist_t *s_explode(const char *text, char delim) {
  if (*text == 0) {
    return 0;
  }

  int plen = 0;
  while (text[plen] != 0 && text[plen] != delim) {
    plen += 1;
  }

  int skip = 0;
  if (text[plen] == delim) {
    skip = 1;
  }

  slist_t *rest = s_explode(text + plen + skip, delim);
  char *part = alloca(plen + 2);
  memcpy(part, text, plen);
  part[plen] = 0;

  return s_cons(part, rest);
}

slist_t *s_reverse(slist_t *cons) {
  slist_t *reversed;

  slist_t *curr = cons;
  while (curr != NULL) {
    reversed = s_cons(curr->data, reversed);
    curr = curr->next;
  }

  return reversed;
}

void s_print(slist_t *list) {
  slist_t *curr = list;
  printf("[ ");
  while (curr != NULL) {
    printf("\"%s\" ", curr->data);
    curr = curr->next;
  }
  printf("]\n");
}

int s_size(slist_t *list) {
  slist_t *curr = list;
  int size = 0;
  while (curr != NULL) {
    size++;
    curr = curr->next;
  }
  return size;
}