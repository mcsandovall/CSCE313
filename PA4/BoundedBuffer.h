#ifndef BoundedBuffer_h
#define BoundedBuffer_h

#include <stdio.h>
#include <queue>
#include <string>
#include "Semaphore.h"

using namespace std;

class BoundedBuffer
{
private:
	int cap; // max number of items in the buffer
	queue<vector<char> > q;	/* the queue of items in the buffer. Note
	that each item a sequence of characters that is best represented by a vector<char> because: 
	1. An STL std::string cannot keep binary/non-printables
	2. The other alternative is keeping a char* for the sequence and an integer length (i.e., the items can be of variable length), which is more complicated.*/

	// add necessary synchronization variables (e.g., sempahores, mutexes) and variables
	Semaphore * fullSlots;
	Semaphore * emptySlots;
	Semaphore * mutex;


public:
	BoundedBuffer() {} // dummy constructor for the response buffer

	BoundedBuffer(int _cap){
		cap = _cap;
		// start the semaphores
		fullSlots = new Semaphore(0);
		emptySlots = new Semaphore(cap);
		mutex = new Semaphore(1);
	}

	~BoundedBuffer(){
		// empty out the queue
		// delete the semaphores 
		delete fullSlots, emptySlots, mutex;
	}

	void push(vector<char> data){
		// follow the class lecture pseudocode
				
		//1. Perform necessary waiting (by calling wait on the right semaphores and mutexes),
		//2. Push the data onto the queue
		//3. Do necessary unlocking and notification

		emptySlots->P();
		mutex->P();
		q.push(data);
		mutex->V();
		fullSlots->V();
	}

	vector<char> pop(){
		//1. Wait using the correct sync variables 
		//2. Pop the front item of the queue. 
		//3. Unlock and notify using the right sync variables
		//4. Return the popped vector
		vector<char> data;
		fullSlots->P();
		mutex->P();
		data = q.front();
		q.pop();
		mutex->V();
		emptySlots->V();

		return data;
	}
};

#endif /* BoundedBuffer_ */
