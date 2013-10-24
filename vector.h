#ifndef VECTOR_H
#define VECTOR_H

struct vector
{
  void **elems;
  size_t size;
  size_t memSize;
};

typedef struct vector *vector_t;

struct node
{
  command_t command;
  vector_t files; // files that command uses
  vector_t before; // command nodes we must wait on
  vector_t after;
  pid_t pid; // current process running the command
};

typedef struct node *node_t;

vector_t create_vector();
void delete_vector(vector_t v);
void vector_append(vector_t v, void* elem);
void add_vectors(vector_t v1, vector_t v2);
bool files_intersect(node_t n1, node_t n2);

#endif //VECTOR