/* libtest.c
 * Test support library
 * 
 * Copyright 2015 Akash Rawal
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

#include "libtest.h"

static void test_caller_ctx_return_fn(SscCallerCtx *p_ctx, MmcMsg *reply)
{
	TestCallerCtx *ctx = (TestCallerCtx *) p_ctx;
	ctx->reply = reply;
}

void test_caller_ctx_init(TestCallerCtx *ctx)
{
	memset(ctx, 0, sizeof(TestCallerCtx));
	ctx->parent.return_fn = test_caller_ctx_return_fn;
}
