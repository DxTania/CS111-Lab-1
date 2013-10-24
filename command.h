// UCLA CS 111 Lab 1 command interface
#ifndef COMMAND_H
#define COMMAND_H

#include <stdbool.h>
#include <sys/types.h>
typedef struct command *command_t;
typedef struct command_stream *command_stream_t;

/* Create a command stream from GETBYTE and ARG.  A reader of
   the command stream will invoke GETBYTE (ARG) to get the next byte.
   GETBYTE will return the next input byte, or a negative number
   (setting errno) on failure.  */
command_stream_t make_command_stream (int (*getbyte) (void *), void *arg);

/* Read a command from STREAM; return it, or NULL on EOF.  If there is
   an error, report the error and exit instead of returning.  */
command_t read_command_stream (command_stream_t stream);

/* Print a command to stdout, for debugging.  */
void print_command (command_t);

/* Execute a command.  Use "time travel" if the flag is set.  */
void execute_command (command_t, bool);

int execute_simple_command(command_t command);
int execute_pipe_command(command_t command);
int execute_subshell_command(command_t command);
int execute_sequence_command(command_t command);
int execute_and_or_command(command_t command);

int isRedirectionCommand(char* word);
int isValidWordCharacter(char* word);
void checkForConsecutiveRedir(char* word);

/* Return the exit status of a command, which must have previously
   been executed.  Wait for the command, if it is not already finished.  */
int command_status (command_t);

void evalStack();
void evalStackUntilLeftP();

void add_command(command_t command);
void speed_of_light(bool time_travel);
void print();

int getRidOfExtraWhitespaces(char* word);

command_t createSimpleCommand(char* currentWord);

command_t createCommand(command_t operand1, command_t operand2, 
  int operatorNumber);

#endif // COMMAND_H
