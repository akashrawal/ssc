/* incl.h
 * Headers
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


//Dependencies
#include <mmc/mmc.h>

#define ssc_error(...) mmc_context_error("SSC", __VA_ARGS__)
#define ssc_warn(...) mmc_context_warn("SSC", __VA_ARGS__)
#define ssc_assert(...) mmc_context_assert("SSC", __VA_ARGS__)

//Include all headers
#ifndef SSC_PUBLIC_HEADER
#include "private.h"
#endif
#include "types.h"
#include "serialize.h"
#include "interface.h"
#include "msg.h"
#include "dict.h"

