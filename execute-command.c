// UCLA CS 111 Lab 1 command execution
#include "alloc.h"
#include "command.h"
#include "command-internals.h"
#include "vector.h"

#include <unistd.h>
#include <error.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "stdio.h"
#include <stdlib.h>

#define UNUSED(x) (void) (x);

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

vector_t dependencies = NULL;
vector_t no_dependencies = NULL;

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
  if (!time_travel)
  {
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
  else
  {
    add_command(c);
  }
}

int execute_sequence_command(command_t command)
{
  // account for empty second part of sequence
  execute_command(command->u.command[0],false);
  if(command->u.command[1])  //allow semicolon to act as a unary operator
  {
    execute_command(command->u.command[1],false);
    command->status = command->u.command[1]->status;
  }
  else
    command->status = command->u.command[0]->status; 
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

  return command->status;
}

// Recursively retrieve a vector of all files used within the command (also includes options)
vector_t get_files(command_t command)
{
  vector_t files = create_vector();
  switch(command->type)
  {
    case AND_COMMAND:
    case SEQUENCE_COMMAND:
    case OR_COMMAND:
    case PIPE_COMMAND:
    {
      // Left / Right commands
      vector_t filesLeft = get_files(command->u.command[0]);
      vector_t filesRight = get_files(command->u.command[1]);
      add_vectors(files, filesLeft);
      add_vectors(files, filesRight);
      delete_vector(filesLeft);
      delete_vector(filesRight);
      break;
    }
    case SIMPLE_COMMAND:
    {
      // in a simple command, all the arguments are considered "files"
      // start at the first *argument* and continue, then include I/O
      char** arg;
      for(arg = command->u.word + 1; *arg != NULL; arg++)
      {
        vector_append(files, *arg);
      }
      vector_append(files, command->input);
      vector_append(files, command->output);
      break;
    }
    case SUBSHELL_COMMAND:
    {
      vector_t subFiles = get_files(command->u.subshell_command);
      add_vectors(files, subFiles);
      delete_vector(subFiles);
      break;
    }
  }
  return files;
}

/**
 * Finds any already processed command nodes with intersecting files
 * Returns 1 if the input node now has dependencies, 0 otherwise.
 */
bool get_has_dependencies(node_t commandNode)
{
  // TODO: Do we need all intersects, or just the closest ones?
  // Currently this only adds direct file dependencies
  // Some command nodes may be added twice if we do all in the path w/o checking for dups
  size_t i;
  bool flag = 0;
  for (i = 0; i < dependencies->size; i++)
  {
    if (files_intersect((node_t) dependencies->elems[i], commandNode))
    {
      vector_append(commandNode->before, dependencies->elems[i]);
      flag = 1;
    }
  }
  for (i = 0; i < no_dependencies->size; i++)
  {
    if (files_intersect((node_t) no_dependencies->elems[i], commandNode))
    {
      vector_append(commandNode->before, no_dependencies->elems[i]);
      flag = 1;
    }
  }
  return flag;
}

/**
 * Adds a command to our time travel lists.
 */
void add_command(command_t command)
{
  if (dependencies == NULL)
    dependencies = create_vector();
  if (no_dependencies == NULL)
    no_dependencies = create_vector();

  node_t commandNode = checked_malloc(sizeof(struct node));
  commandNode->command = command;
  commandNode->files = get_files(command);
  commandNode->before = create_vector();
  commandNode->pid = -1;

  if(get_has_dependencies(commandNode))
    vector_append(dependencies, commandNode);
  else
    vector_append(no_dependencies, commandNode);
}

//returns 1 if all the commands this node depend on have finished
//returns 0 otherwise
int freeToRun(node_t commandNode)
{
  //printf("free to run called\n");
  size_t i;

  if(commandNode == NULL)
    return -1;

  // check to see if commands it depends on have even started running.
  for(i = 0; i < commandNode->before->size; i++)
  {  
    //printf("pid of dependency is %i\n",((node_t)commandNode->before->elems[i])->pid);
    //printf("return status of dependency is %i\n",((node_t)commandNode->before->elems[i])->command->status);
    if( ((node_t)commandNode->before->elems[i])->pid == -1  
     || ((node_t)commandNode->before->elems[i])->command->status != 0 )
    {
      return 0;
    }
  }
  return 1;
}

/**
 * Call this function to run all added commands in parallel.
 * TODO: THIS ISN'T FINISHED & DOESN'T WORK RIGHT!
 */
void speed_of_light(bool time_travel)
{
  if (!time_travel)
    return;

    size_t i;
    node_t commandNode;//, dependency;
    pid_t pid;
    //int status, depsize = dependencies->size;
    //bool skip = false;

    //printf("no dependences size is %i\n",(int) no_dependencies->size);
    // First fork processes for each command that has no dependencies
    for (i = 0; i < no_dependencies->size; i++)
    {
      //printf("no_dependencies->elems[i]->command->status is %i\n",((node_t)no_dependencies->elems[i])->command->status);
      pid = fork();
      commandNode = (node_t) no_dependencies->elems[i];
      if (pid == 0)
      {
        execute_command(commandNode->command, 0);
        _exit(commandNode->command->status);
      }
      else
        commandNode->pid = pid;
    }
    
    //wait for all commands to finish?
    //this prevents some dependent commands from executing until all
    //non_dependent comands are done.
    for(i = 0;i < no_dependencies->size ; i++)
    {
      int status;
      commandNode = (node_t) no_dependencies->elems[i];
      if(waitpid(commandNode->pid,&status,0) < 0)
        error(1,0,"speed of light error\n");
      else
        commandNode->command->status = WEXITSTATUS(status);
    }

    //for(i = 0 ; i < no_dependencies->size;i++)
      //printf("Checkin no_dependencies->elems[i]->command->status is %i\n",((node_t)no_dependencies->elems[i])->command->status);
    
    //printf("dependencies->size is %i\n",(int)dependencies->size);
    int depSize = (int) dependencies->size;
    while( depSize > 0)
    {      
      for(i = 0; i < dependencies->size; i++)
      {

        commandNode = (node_t) dependencies->elems[i];
        //printf("current commands status %i and pid %i\n",commandNode->command->status,commandNode->pid);
        if(commandNode->pid > 0 || commandNode->command->status == 0)
          continue;
       
        int iFree = freeToRun((node_t)dependencies->elems[i]);
        //printf("free is %i. depsize is %i\n",iFree,depSize);
        if( iFree == 0 )
          continue;
        
        //printf("forking with command index %i %s\n",(int)i,commandNode->command->u.word[0]);
        depSize--;
        pid = fork();
        if(pid == 0)
        {
          execute_command(commandNode->command,0);
          _exit(commandNode->command->status);
        }
        else
        {
          commandNode->pid = pid;
          //dependencies->elems[i] = NULL;
          //waitpid(
        }
      }
      
      for(i = 0; i < dependencies->size;i++)
      { 
        //printf("iterating");
        commandNode = (node_t) dependencies->elems[i];
        if(commandNode == NULL || commandNode->command->status == 0 || commandNode->pid < 0)
          continue;
        //printf("trying to check if item %i with pid %i is finished\n",(int)i,commandNode->pid);
        int status;
        int waitpidReturnVal = waitpid(commandNode->pid,&status,WNOHANG);
        //printf("waitpid passed\n");
        if(waitpidReturnVal < 0)
          error(1,0,"Error waiting on child");
        else if( waitpidReturnVal > 0 )
        {
          //printf("element %i returned %i\n",(int)i,WEXITSTATUS(status));
          commandNode->command->status = WEXITSTATUS(status);
        }
        //printf("insideloop\n");
        //printf("dependenceis is %i depSize is %i\n",(int) dependencies->size,depSize);
      }
      //printf("looping\n");
    }
    
    for(i = 0;i < dependencies->size ; i++)
    {
      int status;
      commandNode = (node_t) dependencies->elems[i];
      if(commandNode == NULL || commandNode->command == NULL || commandNode->command->status ==0)
        continue;

      if(waitpid(commandNode->pid,&status,0) < 0)
        error(1,0,"speed of light error\n");
      else
        commandNode->command->status = WEXITSTATUS(status);
    }
}

// Prints out the dependences for each command with dependences
// and just the command for command without dependencies
void print()
{
  size_t i = 0, j =0;
  if(dependencies != NULL)
  {
  printf("dependencies:\n");
  for (i = 0; i < dependencies->size; i++) {
    node_t commandNode = (node_t) dependencies->elems[i];
    printf("-------------------------------------\n");
    printf("dependencies for following command: ");
    print_command(commandNode->command);
    printf("===\n");
    for(j = 0; j < commandNode->before->size; j++)
    {
      node_t dependency = (node_t) commandNode->before->elems[j];
      print_command(dependency->command);
    }
    printf("-------------------------------------\n");
    printf("\n");
  }
  }
  
  if( no_dependencies != NULL)
  {
  printf("no dependencies:\n");
  for (i = 0; i < no_dependencies->size; i++) {
    node_t commandNode = (node_t) no_dependencies->elems[i];
    print_command(commandNode->command);
    printf("-------------------------------------\n");
    printf("\n");
  }
  }
}
