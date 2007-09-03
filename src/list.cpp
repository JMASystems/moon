/*
 * list.cpp: a non-sucky linked list implementation
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "list.h"


List::Node::Node ()
{
	next = 0;
	prev = 0;
}


List::List ()
{
	head = 0;
	tail = 0;
}


List::Node *
List::First ()
{
	return head;
}


List::Node *
List::Last ()
{
	return tail;
}


bool
List::IsEmpty ()
{
	return !head;
}


int
List::Length ()
{
	List::Node *node;
	int n = 0;
	
	node = head;
	while (node) {
		node = node->next;
		n++;
	}
	
	return n;
}


void
List::Clear (bool freeNodes)
{
	if (freeNodes) {
		List::Node *n, *nn;
		
		n = head;
		while (n) {
			nn = n->next;
			delete n;
			n = nn;
		}
	}
	
	head = 0;
	tail = 0;
}


List::Node *
List::Append (List::Node *node)
{
	node->prev = tail;
	node->next = 0;
	
	if (tail)
		tail->next = node;
	else
		head = node;
	
	tail = node;
	
	return node;
}


List::Node *
List::Prepend (List::Node *node)
{
	node->next = head;
	node->prev = 0;
	
	if (head)
		head->prev = node;
	else
		tail = node;
	
	head = node;
	
	return node;
}


List::Node *
List::Insert (List::Node *node, int index)
{
	List::Node *n = head;
	int i = 0;
	
	if (head) {
		while (n->next && i < index) {
			n = n->next;
			i++;
		}
		
		if (i == index) {
			if (n == tail)
				tail = node;
			
			n->prev->next = node;
			node->prev = n->prev;
			node->next = n;
			n->prev = node;
		} else {
			tail = n->next = node;
			node->prev = n;
			node->next = 0;
		}
	} else {
		head = tail = node;
		node->next = 0;
		node->prev = 0;
	}
	
	return node;
}


List::Node *
List::Replace (List::Node *node, int index)
{
	List::Node *n;
	
	if (!(n = Index (index)))
		return 0;
	
	node->next = n->next;
	node->prev = n->prev;
	
	if (n->prev)
		n->prev->next = node;
	else
		head = node;
	
	if (n->next)
		n->next->prev = node;
	else
		tail = node;
	
	n->next = 0;
	n->prev = 0;
	
	return n;
}

List::Node *
List::Find (NodeFinder find, void *data)
{
	List::Node *n = head;
	
	if (!find)
		return 0;
	
	while (n) {
		if (find (n, data))
			return n;
		
		n = n->next;
	}
	
	return 0;
}


void
List::Remove (NodeFinder find, void *data)
{
	List::Node *n;
	
	if ((n = Find (find, data))) {
		Unlink (n);
		delete n;
	}
}


void
List::Unlink (List::Node *node)
{
	if (head == node)
		head = node->next;
	else
		node->prev->next = node->next;
	
	if (tail == node)
		tail = node->prev;
	else
		node->next->prev = node->prev;
	
	node->prev = 0;
	node->next = 0;
}


List::Node *
List::Index (int index)
{
	List::Node *n = head;
	int i = 0;
	
	if (index < 0)
		return 0;
	
	while (n && i < index) {
		n = n->next;
		i++;
	}
	
	if (i == index)
		return n;
	
	return 0;
}

int
List::IndexOf (List::Node *node)
{
	List::Node *n = head;
	int i = 0;
	
	while (n && n != node) {
		n = n->next;
		i++;
	}
	
	return n == node ? i : -1;
}


int
List::IndexOf (NodeFinder find, void *data)
{
	List::Node *n = head;
	int i = 0;
	
	if (!find)
		return -1;
	
	while (n) {
		if (find (n, data))
			return i;
		
		n = n->next;
		i++;
	}
	
	return -1;
}



//#define TEST_PROGRAM
#ifdef TEST_PROGRAM

#include <stdio.h>

class IntNode : public List::Node {
public:
	int id;
	
	IntNode (int i) { id = i; }
};


static int
IntNodeCompare (List::Node *n0, List::Node *n1)
{
	IntNode *in0 = (IntNode *) n0;
	IntNode *in1 = (IntNode *) n1;
	
	return in0->id - in1->id;
}


int main (int argc, char **argv)
{
	List::Node *node;
	List *list;
	
	list = new List ();
	
	node = list->Append (new IntNode (1));
	printf ("appended node with id = %d\n", ((IntNode *) node)->id);
	node = list->Append (new IntNode (3));
	printf ("appended node with id = %d\n", ((IntNode *) node)->id);
	node = list->Append (new IntNode (4));
	printf ("appended node with id = %d\n", ((IntNode *) node)->id);
	node = list->Insert (new IntNode (2), 1);
	printf ("inserted node with id = %d at index = %d\n",
		((IntNode *) node)->id, list->IndexOf (node));
	node = list->Prepend (new IntNode (0));
	printf ("prepended node with id = %d\n", ((IntNode *) node)->id);
	
	printf ("\nlist contains (in order):\n");
	for (node = list->First (); node != NULL; node = node->next)
		printf ("node id = %d\n", ((IntNode *) node)->id);
	
	printf ("\nlist contains (in reverse order):\n");
	for (node = list->Last (); node != NULL; node = node->prev)
		printf ("node id = %d\n", ((IntNode *) node)->id);
	
	list->Clear (true);
	delete list;
	
	return 0;
}

#endif /* TEST_PROGRAM */
