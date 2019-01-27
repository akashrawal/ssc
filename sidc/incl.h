/* incl.h
 * Headers
 * 
 * Copyright 2015-2019 Akash Rawal
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
#include <ssc/ssc.h>

//TODO: Factor out this into unit test
//Which main() function to use (for testing)
#define SSC_TEST_NONE
//#define SSC_TEST_BST

//Include all headers
#include "bst.h"
#include "symbol.h"
#include "parselib.h"
#include "codegen.h"
#include "structure.h"
#include "interface.h"
#include "sequencer.h"
