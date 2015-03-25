#include "queue.h"

// global mutex lock
boost::mutex listLock;

void shutdown (node *linkedList) {
	node* tmp;
	while (linkedList) {
		tmp = linkedList->next;
		free(linkedList);
		linkedList = tmp;
	}
}

// prepend a new image to linked list
void addImg(node **linkedList, FlyCapture2::Image newImg) {
	node* newNode = (node*) malloc(sizeof(node));
	// add the image into the new node
	newNode->img = newImg;
	newNode->next = NULL;

	// atomic operation - stick this node into the list
	listLock.lock();
	// if the list is empty, then just set it equal to the new node
	if (*linkedList==NULL) {
		*linkedList = newNode;
		return;
	}
	newNode->next = *linkedList;
	*linkedList = newNode;
	listLock.unlock();
}

// remove image from linked list to process
FlyCapture2::Image getImg (node **linkedList) {
	// atomic operation - grab image from head
	// and return it, then delete head
	FlyCapture2::Image ret;
	// If the list is empty, just return the blank image
	if (*linkedList==NULL)
		return ret;
	// If you can't get the lock, then just return the first image you see
	if (!listLock.try_lock()){
		ret = (*linkedList)->img;
		return ret;
	}
	// no need to call lock() if try_lock() succeeded
	ret = (*linkedList)->img;
	node *tmp = (*linkedList)->next;
	// only remove the node if the list has > 1 nodes to start with
	if (tmp!=NULL){
		*linkedList = (*linkedList)->next;
		free(tmp);
	}
	listLock.unlock();
	return ret;
}
