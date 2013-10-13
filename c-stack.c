#include "c-stack.h"
#include "alloc.h"
#include "stdio.h"
#include "command.h"
#include <error.h>

int* g_operator_stack = NULL;
unsigned int  g_iOperator_stack_capacity = 16;
unsigned int  g_iOperator_stack_size = 0;

command_t* g_operand_stack = NULL;
unsigned int g_iOperand_stack_capacity = 16;
unsigned int g_iOperand_stack_size = 0;

extern int g_lineNumber;

void init_stacks()
{
  if(g_operator_stack != NULL || g_operand_stack != NULL)
  {
    error(1,0,"One or more of the stacks are already initalized!\n");
    return;
  }

  g_operator_stack = checked_malloc(sizeof(char*)*g_iOperator_stack_capacity);
  g_operand_stack = checked_malloc( sizeof(command_t)*g_iOperand_stack_capacity);
}

int operator_stack_top()
{
  if(g_iOperator_stack_size <= 0)
  {
    //error(1,0,"Operator stack is empty! cannot view top of it\n",g_lineNumber);
    return -1;
  }

  return g_operator_stack[g_iOperator_stack_size - 1];
}

command_t operand_stack_top()
{
  if(g_iOperand_stack_size <= 0)
  {
    //error(1,0,"Operand stack is empty! cannot view top of it\n");
    return NULL;
  }
  
  return g_operand_stack[g_iOperand_stack_size - 1];
}

void push_operator(int type )
{
 //Do capacity check and double capacity if needed.
 
  if( g_iOperator_stack_capacity == g_iOperator_stack_size)
  {
    //grow the stack
    g_operator_stack = checked_realloc(g_operator_stack,g_iOperator_stack_capacity*2);
    g_iOperator_stack_capacity *=2;
  } 

  g_operator_stack[g_iOperator_stack_size] = type;
  g_iOperator_stack_size++;
}

void push_operand(command_t operand)
{
  //do capcity check and double capacity if needed.

  if( g_iOperand_stack_capacity == g_iOperand_stack_size )
  {
    //grow the stack
    g_operand_stack = checked_realloc(g_operand_stack,g_iOperand_stack_capacity*2);
    g_iOperand_stack_capacity *=2;
  }
  
  g_operand_stack[g_iOperand_stack_size] = operand;
  g_iOperand_stack_size++;
}

int pop_operator()
{
  if(g_iOperator_stack_size <= 0)
  {
    error(1,0,"%i: Syntax Error: Tried to pop operator when the stack is empty!\n",g_lineNumber);
    return 0;
  }
  
  g_iOperator_stack_size--;
  return g_operator_stack[g_iOperator_stack_size];
}

command_t pop_operand()
{
  if(g_iOperand_stack_size <= 0)
  {
    error(1,0,"%i: Syntax Error: Tried to pop operand when the stack is empty!\n",g_lineNumber);
    return NULL;
  }

  g_iOperand_stack_size--;
  return g_operand_stack[g_iOperand_stack_size];
}


