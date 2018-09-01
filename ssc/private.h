/* private.h
 * Library-private stuff
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

//A small AVL tree for handling collisions in small hash tables
typedef struct
{
	uint8_t key, left, right, height;
} SscMiniAvlNode;

int ssc_mini_avl_set_rec(SscMiniAvlNode *mem, int x, int key, int *tray);

int ssc_mini_avl_unset_rec(SscMiniAvlNode *mem, int x, int key, int *tray);

//A space efficient map that maps a byte to a pointer
typedef struct
{
	void *ptr;
	uint16_t metainf;
} SscByteMap;

void ssc_byte_map_init(SscByteMap *m);

void ssc_byte_map_set(SscByteMap *m, uint8_t key, void *value);

void *ssc_byte_map_get(SscByteMap *s, uint8_t key);
