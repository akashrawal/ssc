/* bst.c
 * Binary search tree for string lookups
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
 
#include "incl.h"

#include <string.h>
 
//Tree node
typedef struct _BstNode BstNode;
struct _BstNode
{
	void *value;
	BstNode *left, *right;
	int height;
	char key[];
};

#define bst_node_height(node) ((node) ? (node)->height : 0)
#define ssc_max(a, b) ((a) > (b) ? (a) : (b))
#define bst_node_calc_height(node) \
	(ssc_max(bst_node_height((node)->left), bst_node_height((node)->right)) + 1)
#define bst_node_calc_balance_factor(node) \
	(bst_node_height((node)->right) - bst_node_height((node)->left))

static BstNode *bst_node_new(const char *key, void *value)
{
	size_t key_len = strlen(key);
	
	BstNode *node = (BstNode *) mdsl_alloc(sizeof(BstNode) + key_len + 1);
	node->value = value;
	node->left = node->right = NULL;
	node->height = 1;
	memcpy(node->key, key, key_len + 1);
	
	return node;
}

static void bst_node_destroy_rec(BstNode *node)
{
	if (! node)
		return;
	
	bst_node_destroy_rec(node->left);
	bst_node_destroy_rec(node->right);
	free(node);
}

static BstNode *bst_node_rotate_left(BstNode *node)
{
	BstNode *old_root, *new_root, *ltree, *ctree, *rtree;
	old_root = node;
	new_root = node->right;
	ltree = node->left;
	ctree = node->right->left;
	rtree = node->right->right;
	
	node = new_root;
	node->left = old_root;
	node->left->left = ltree;
	node->left->right = ctree;
	node->right = rtree;
	
	node->left->height = bst_node_calc_height(node->left);
	node->height = bst_node_calc_height(node);
	
	return node;
}

static BstNode *bst_node_rotate_right(BstNode *node)
{
	BstNode *old_root, *new_root, *ltree, *ctree, *rtree;
	old_root = node;
	new_root = node->left;
	ltree = node->left->left;
	ctree = node->left->right;
	rtree = node->right;
	
	node = new_root;
	node->right = old_root;
	node->left = ltree;
	node->right->left = ctree;
	node->right->right = rtree;
	
	node->right->height = bst_node_calc_height(node->right);
	node->height = bst_node_calc_height(node);
	
	return node;
}

//The data structure
struct _SscBst
{
	MdslRC parent;
	BstNode *root;
};

mdsl_rc_define(SscBst, ssc_bst);

SscBst *ssc_bst_new()
{
	SscBst *bst = (SscBst *) malloc(sizeof(SscBst));
	
	mdsl_rc_init(bst);
	bst->root = NULL;
	
	return bst;
}

static BstNode *insert_rec(BstNode *node, BstNode *new_node)
{
	int cmpval;
	BstNode **branch_ptr, *new_branch;
	
	//Base case
	if (node == NULL)
		return new_node;
	
	//Which branch to go, left or right?
	cmpval = strcmp(new_node->key, node->key);
	if (cmpval == 0)
		return NULL;
	else if (cmpval < 0)
		branch_ptr = &(node->left);
	else
		branch_ptr = &(node->right);
	
	//Do insertion recursively
	new_branch = insert_rec(*branch_ptr, new_node);
	if (new_branch == NULL)
		return NULL;
	*branch_ptr = new_branch;
	node->height = bst_node_calc_height(node);
	
	//Rotate and balance tree
	if (bst_node_calc_balance_factor(node) < -1)
	{
		if (bst_node_calc_balance_factor(node->left) == 1)
			node->left = bst_node_rotate_left(node->left);
		node = bst_node_rotate_right(node);
	}
	else if (bst_node_calc_balance_factor(node) > 1)
	{
		if (bst_node_calc_balance_factor(node->right) == -1)
			node->right = bst_node_rotate_right(node->right);
		node = bst_node_rotate_left(node);
	}
	
	return node;
}

MdslStatus ssc_bst_insert(SscBst *bst, const char *key, void *value)
{	
	if (! value)
		ssc_error("Null values are prohibited");
	
	BstNode *new_node = bst_node_new(key, value);
	BstNode *new_root = insert_rec(bst->root, new_node);
	if (! new_root)
	{
		free(new_node);
		return MDSL_FAILURE;
	}
	bst->root = new_root;
	
	return MDSL_SUCCESS;
}

void *ssc_bst_lookup(SscBst *bst, const char *key)
{
	BstNode *node;
	int cmpval;
	
	node = bst->root;
	
	while (node)
	{
		//Which branch to go, left or right?
		cmpval = strcmp(key, node->key);
		if (cmpval == 0)
			return node->value;
		else if (cmpval < 0)
			node = node->left;
		else
			node = node->right;
	}
	
	return NULL;
}

static void ssc_bst_destroy(SscBst *bst)
{
	bst_node_destroy_rec(bst->root);
	free(bst);
}

#ifdef SSC_TEST_BST

static void bst_node_prettyprint_rec(BstNode *node, int depth)
{
	int i;
#define tab for (i = 0; i < depth; i++) printf(" ")
#define tabx for (i = 0; i <= depth; i++) printf(" ")
	
	//Base case 
	if (node == NULL)
	{
		tab; 
		printf("NULL\n");
		return;
	}
		
	tab; printf("node\n");
	tab; printf("{\n");
	tabx; printf("\"%s\", h=%d\n", node->key, node->height);
	tabx; printf("left=");
	tabx; bst_node_prettyprint_rec(node->left, depth + 1);
	tabx; printf("right=");
	tabx; bst_node_prettyprint_rec(node->right, depth + 1);
	tab; printf("}\n");
}

static void bst_node_prettyprint(BstNode *node)
{
	bst_node_prettyprint_rec(node, 0);
}

static void ssc_bst_check_rec(BstNode *node)
{
	int balance_factor;
	int errors = 0;
	//Base case
	if (! node)
		return;
	
	//Check children
	ssc_bst_check_rec(node->left);
	ssc_bst_check_rec(node->right);
	
	//Check order
	if (node->left)
		if (strcmp(node->left->key, node->key) >= 0)
		{
			ssc_warn("Nodes not in order");
			errors++;
		}
	if (node->right)
		if (strcmp(node->right->key, node->key) <= 0)
		{
			ssc_warn("Nodes not in order");
			errors++;
		}
	
	//Check heights
	if (node->height != bst_node_calc_height(node))
	{
		ssc_warn("Height of the node is insane");
		errors++;
	}
	
	//Check balance
	balance_factor = bst_node_calc_balance_factor(node);
	if (balance_factor > 1 || balance_factor < -1)
	{
		ssc_warn("Unbalanced tree");
		errors++;
	}
	
	if (errors)
	{
		printf("Problematic node: \n");
		bst_node_prettyprint(node);
		ssc_error("Found %d errors in above node", errors);
	}
}

static void ssc_bst_check(SscBst *bst)
{
	ssc_bst_check_rec(bst->root);
}

#define TEST_ENTRIES 256
#define TEST_IDLEN 32
#define CHARSET_LEN 37
static char charset[] = "qwertyuiopasdfghjklzxcvbnm0123456789_";
static char idfiers[TEST_ENTRIES][TEST_IDLEN + 1];

static void create_idfier(char *buffer)
{
	int i;
	
	for (i = 0; i < TEST_IDLEN; i++)
	{
		buffer[i] = charset[rand() % CHARSET_LEN];
	}
	buffer[i] = 0;
}

int main()
{
	int i;
	SscBst *bst;
	
	while(1)
	{
		bst = ssc_bst_new();
		
		//Add a few identifiers
		for (i = 0; i < TEST_ENTRIES; i++)
		{
			create_idfier(idfiers[i]);
			
			ssc_bst_insert(bst, idfiers[i], bst);
			
			//Check for sanity
			ssc_bst_check(bst);
			
			//Check for insertion
			if (! ssc_bst_lookup(bst, idfiers[i]))
				ssc_error("Inserted element %s not there in list", 
					idfiers[i]);
		}
		
		//Test for presence
		for (i = 0; i < TEST_ENTRIES; i++)
		{
			if (! ssc_bst_lookup(bst, idfiers[i]))
				ssc_error("Inserted element %s not there in list", 
					idfiers[i]);
		}
		
		ssc_bst_unref(bst);
	}
	
	return 0;
}

#endif //SSC_TEST_BST

