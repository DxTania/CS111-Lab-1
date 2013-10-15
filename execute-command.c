// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <unistd.h>
#include <error.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <stdio.h>

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
      execute_simple_command(c);
      break;
    case SEQUENCE_COMMAND:
      break;
    case PIPE_COMMAND:
      break;
    case SUBSHELL_COMMAND:
      execute_subshell_command(c);
      break;
  }
}

pid_t execute_and_or_command(command_t command)
{
  pid_t pid;
  int status;
  command_t left = command->u.command[0];
  command_t right = command->u.command[1];

  pid = fork();
  if (pid == 0) {
    execute_command(left, false);
    _exit(left->status);
  } else if (left->type == SIMPLE_COMMAND) {
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status)) {
      error(1, 0, "Child never exited.\n");
    }
    left->status = WEXITSTATUS(status);
  }

  if((command->type == AND_COMMAND && left->status == 0) ||
    (command->type == OR_COMMAND && left->status != 0)) { // left not/successful, execute right side
    pid = fork();
    if (pid == 0) {
      execute_command(right, false);
      _exit(right->status);
    } else if (right->type == SIMPLE_COMMAND) {
      waitpid(pid, &status, 0);
      if (!WIFEXITED(status)) {
        error(1, 0, "Child never exited.\n");
      }
      right->status = WEXITSTATUS(status);
    }
    command->status = right->status;
  }

  return pid;
}

pid_t execute_subshell_command(command_t command)
{
  pid_t pid;
  int status;

  // SEGMENTATION FAULT!!!
  command_t subshell = command->u.subshell_command;

  printf("Trying to execute subshell command %s\n", subshell->u.word[0]);
  pid = fork();
  if (pid == 0) {
    execute_command(subshell, false);
    _exit(subshell->status);
  } else {
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status)) {
      error(1, 0, "Child never exited.\n");
    }
    command->status = WEXITSTATUS(status);
  }

  return pid;
}

int execute_simple_command(command_t command)
{
  int status;
  pid_t pid = fork();
  if (pid == 0) {
    execvp(command->u.word[0], command->u.word);
    _exit(1);
  } else {
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status)) {
      error(1, 0, "Child never exited.\n");
    }
    command->status = WEXITSTATUS(status);
  }
  return command->status;
}

