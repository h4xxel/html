#ifndef __STACK_H_
#define __STACK_H_

typedef struct Stack Stack;
struct Stack {
	void *item;
	struct Stack *next;
};

void *stack_push(Stack **stack, void *item);
void *stack_pop(Stack **stack);
void *stack_peek(Stack **stack);
int stack_find(struct Stack **stack, int (func)(void *, void *), void *data);

#endif
