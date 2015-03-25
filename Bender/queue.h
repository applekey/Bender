#ifndef QUEUE_H
#define QUEUE_H
#include <stdio.h>
#include <boost/assert.hpp>
#include <boost/thread/thread.hpp>
#include "FlyCapture2.h"

typedef struct Node{
	struct Node* next;
	FlyCapture2::Image img;
} node;

void shutdown (node *linkedList);
void addImg (node **linkedList, FlyCapture2::Image newImg);
FlyCapture2::Image getImg (node **linkedList);
//global mutex lock
extern boost::mutex listLock;

#endif