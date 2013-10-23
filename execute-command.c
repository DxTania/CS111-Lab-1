// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <unistd.h>
#include <error.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "stdio.h"

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
      execute_sequence_command(c);
      break;
    case PIPE_COMMAND:
      execute_pipe_command(c);
      break;
    case SUBSHELL_COMMAND:
      execute_subshell_command(c);
      break;
  }
}

int execute_sequence_command(command_t command)
{
  execute_command(command->u.command[0],false);
  execute_command(command->u.command[1],false);
  command->status = command->u.command[1]->status;
  return command->status;
}

int execute_subshell_command(command_t command)
{
  execute_command(command->u.subshell_command,false);
  command->status = command->u.subshell_command->status;
  return command->status;
}

int execute_pipe_command(command_t command) //a|b
{
  int fd[2];
  int status, status2;
  pid_t p,p2;
  //if( pipe(fd) != 0)
  //  error(1,0,"Could not create pipe fd");
  if( (p = fork()) < 0)
    error(1,0,"Fork failed on pipe command");
  
  
  if(p == 0) // do our work here
  {
    if( pipe(fd) != 0) //grandfather need not know this
      error(1,0,"Could not create pipe fd");
     
    if( (p2 = fork()) < 0)
      error(1,0,"Second fork on pipe command failed");
    
    if(p2 == 0) // grandchild. will execute a
    {
      dup2(fd[1],1);
      
      if(fd[1] != 0 && fd[1] != 0 )
        close(fd[1]);
      if(fd[0] != 0 && fd[0] != 1)
        close(fd[0]);
 
      //printf("This goes to fd[1]\n");
      execute_command(command->u.command[0],false);
      _exit(command->u.command[0]->status);
      error(1,0,"Failed to fork at simple command");
    }
    else  //child. will execute b
    {
      dup2(fd[0],0);
      if(fd[0] != 0 && fd[0] != 1)
        close(fd[0]);
      if(fd[1] != 0 && fd[1] != 0 )
        close(fd[1]);
      execute_command(command->u.command[1],false);
      //printf("I have executed b\n");
      if(waitpid(p2,&status2,0) < 0)
        error(1,0,"Child wait failed");
      if(WEXITSTATUS(status2) < 0)
        error(1,0,"First process of pipe failed");
      
      _exit(command->u.command[1]->status);
    }
  }
  else //wait for child process to return
  { 
    if(waitpid(p,&status,0) < 0)
      error(1,0,"Outermost grandparent wait failed");
    //printf("I am the father and I waited forsons\n");
    return (command->status = WEXITSTATUS(status));
  } 
}

int execute_and_or_command(command_t command)
{
  pid_t p;
  int status;
  command_t left = command->u.command[0];
  command_t right = command->u.command[1];

  p = fork();
  if( p == 0 )
  {
    execute_command(left,false);
    _exit(left->status);
  }
  else
  {
    //why fork and wait immediately? Why not just use the parent process to do everything?
    if(waitpid(p,&status,0) < 0)
      error(1,0,"Waitpid on and/or failed");
  
    if( (command->type == AND_COMMAND && left->status == 0) ||
        (command->type == OR_COMMAND  && left->status != 0))
    {
      execute_command(right,false);
      command->status = right->status;
    }
  }
  return command->status;
}

int execute_simple_command(command_t command)
{
  int status;

  //TODO: check for redirections!
  pid_t p = fork();
  if (p == 0) 
  {
    int inputFile, outputFile;
    FILE * inStream = NULL;
    FILE * outStream = NULL;
    if( command->input != NULL )
    {
      if( (inStream = fopen(command->input,"r")) == NULL )
        error(1,0,"Input file not found: %s",command->input);
      
      inputFile = fileno(inStream);
      dup2(inputFile,0);
      //should i close the file descriptor here? will it work?
      if(inputFile != 0 && inputFile != 1 && inputFile != 2)
        close(inputFile);
    }
    
    if( command->output != NULL )
    {
      if( (outStream = fopen(command->output,"w")) == NULL )
        error(1,0,"Output file not found: %s",command->output);

      outputFile = fileno(outStream);
      dup2(outputFile,1);
      //should I close the file descriptor here? will it work?
      if(outputFile != 0 && outputFile != 1 && outputFile != 2)
        close(outputFile);
    }

    execvp(command->u.word[0], command->u.word);
    error(1,0,"Failed to fork at simple command");
  }
  else
  {
    if(waitpid(p,&status,0) < 0)
      error(1,0,"WaitPid on Simple command failed!");
    //printf("Executed Simple command\n");
    command->status = WEXITSTATUS(status);
  }  
// must implement making u.word a char* array before this works properly (createSimpleCommand in read-command.c)
  return command->status;
}

