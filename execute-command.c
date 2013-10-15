// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <unistd.h>
#include <error.h>
#include <sys/wait.h>
#include <sys/types.h>

#define UNUSED(x) (void) (x);

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

int
command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, bool time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  UNUSED(time_travel);
  switch(c->type) {
    case AND_COMMAND:
    case OR_COMMAND:
      execute_and_or_command(c);
      break;
    case SIMPLE_COMMAND:
      break;
    case SEQUENCE_COMMAND:
      break;
    case PIPE_COMMAND:
      break;
    case SUBSHELL_COMMAND:
      break;
  }
}

pid_t execute_fork_command(command_t command)
{
  pid_t pid = fork();
  if (pid == 0) {
    execute_command(command, false);
  }
  return pid;
}

pid_t execute_and_or_command(command_t command)
{
  pid_t pid;
  int status, command_status;
  command_t left = command->u.command[0];
  command_t right = command->u.command[1];

  pid = execute_fork_command(left);
  if (left->type == SIMPLE_COMMAND) {
    waitpid(pid, &status, 0);
    left->status = WEXITSTATUS(status);
  }

  command_status = left->status;
  if((command->type == AND_COMMAND && left->status == 0) ||
    (command->type == OR_COMMAND && left->status == != 0)) { // left not/successful, execute right side
    pid = execute_fork_command(right);
    if (right->type == SIMPLE_COMMAND) {
      waitpid(pid, &status, 0);
      right->status = WEXITSTATUS(status);
    }
    command->status = right->status;
  }

  return pid;
}

int execute_simple_command(command_t command)
{
  int status;
  pid_t p = fork();
  if (p == 0) {
    execvp(command->u.word[0], command->u.word);
  }
  // must implement making u.word a char* array before this works properly (createSimpleCommand in read-command.c)
  return status;
}

