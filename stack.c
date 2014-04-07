/*
 * html - a simple html parser lacking a better name 
 * The contents of this file is licensed under the MIT License,
 * see the file COPYING or http://opensource.org/licenses/MIT
 */

#include <stdlib.h>
#include "stack.h"

void *stack_push(Stack **stack, void *item) {
	struct Stack *n;
	
	if(!stack)
		return NULL;
	if(!(n = malloc(sizeof(Stack))))
		return NULL;
	
	n->item = item;
	n->next = *stack;
	*stack = n;
	return item;
}

void *stack_pop(Stack **stack) {
	struct Stack *p;
	void *item;
	
	if(!stack)
		return NULL;
	if(!(p = *stack))
		return NULL;
	
	*stack = p->next;
	item = p->item;
	free(p);
	
	return item;
}

void *stack_peek(Stack **stack) {
	if(!stack)
		return NULL;
	if(!*stack)
		return NULL;
	
	return (*stack)->item;
}

int stack_find(struct Stack **stack, int (func)(void *, void *), void *data) {
	struct Stack *s;
	if(!stack)
		return 0;
	for(s = *stack; s; s = s->next) {
		if(func(s->item, data))
			return 1;
	}
	return 0;
}
