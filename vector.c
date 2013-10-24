#include "alloc.h"
#include "stdio.h"
#include "command.h"
#include "vector.h"
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>

vector_t create_vector()
{
  vector_t v = checked_malloc(sizeof(vector_t));
  v->size = 0;
  v->memSize = 0;
  v->elems = NULL;
  return v;
}

// Add elem to end of vector, increasing size if necessary
void vector_append(vector_t v, void* elem)
{
  if (elem == NULL)
  {
    return;
  }
  if (v->memSize == 0)
  {
    v->memSize = 10 * sizeof(void*);
    v->elems = checked_malloc(v->memSize);
    memset(v->elems, 0, v->memSize);
  }
  else if (v->size * sizeof(void*) >= v->memSize)
  {
    v->memSize *= 2;
    checked_realloc(v->elems, v->memSize);
  }
  v->elems[v->size] = elem;
  v->size++;
}

// Add elements from v2 to v1
void add_vectors(vector_t v1, vector_t v2)
{
  size_t i;
  for(i = 0; i < v2->size; i++)
    vector_append(v1, v2->elems[i]);
}

void delete_vector(vector_t v)
{
  free(v->elems);
  free(v);
}

// returns true if vectors intersect at all
bool files_intersect(node_t n1, node_t n2)
{
  size_t i, j;
  vector_t f1 = n1->files;
  vector_t f2 = n2->files;
  for(i = 0; i < f1->size; i++)
  {
    for(j = 0; j < f2->size; j++)
    {
      if (strcmp(f1->elems[i], f2->elems[j]) == 0)
      {
        return true;
      }
    }
  }
  return false;
}