
#ifndef C_STACK
#define C_STACK

#include "command.h"

extern unsigned int g_iOperator_stack_size;
extern unsigned int g_iOperand_stack_size;

void init_stacks();

void push_operator(int type);
int pop_operator();
int operator_stack_top();

void push_operand(command_t operand);
command_t pop_operand();
command_t operand_stack_top();

#endif //C_STACK
