/* deleted.c
 * Deleted code that might see reuse
 * 
 * Copyright 2015-2018 Akash Rawal
 * This file is part of Modular Middleware.
 * 
 * Modular Middleware is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Modular Middleware is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Modular Middleware.  If not, see <http://www.gnu.org/licenses/>.
 */

//Stack, not tested
typedef struct 
{
	BstNode *node;
	int isright;
} StackEl;

typedef struct
{
	int top;
	int alloc_len;
	StackEl *data;
} Stack;

static void stack_init(Stack *stack)
{
	stack->top = 0;
	stack->alloc_len = 16;
	stack->data = (StackEl *) mdsl_alloc
		(sizeof(StackEl) * stack->alloc_len);
}

static void stack_push(Stack *stack, StackEl element)
{
	stack->data[stack->top] = element;
	stack->top++;
	if (stack->top == stack->alloc_len)
	{
		stack->alloc_len *= 2;
		stack->data = (StackEl *) mmc_realloc
			(stack->data, sizeof(StackEl) * stack->alloc_len);
	}
}

static StackEl stack_pop(Stack *stack)
{
	stack->top--;
	return stack->data[stack->top];
}

static void stack_free(Stack *stack)
{
	free(stack->data);
}
