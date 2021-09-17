#ifndef _BuddyAllocator_h_                   // include file only once
#define _BuddyAllocator_h_
#include <iostream>
#include <vector>
#include <assert.h>
#include <stdint.h>
#include <math.h>
using namespace std;
typedef unsigned int uint;

/* declare types as you need */

class BlockHeader{
public:
	// think about what else should be included as member variables
	int block_size;  // size of the block
	BlockHeader* next; // pointer to the next block
	bool isFree;

	// constructor that takes in the size of a block 
	BlockHeader (int size = 0){
		block_size = size;
		next = nullptr;
	}
};

class LinkedList{
	// this is a special linked list that is made out of BlockHeader s. 
public:
	BlockHeader* head;		// you need a head of the list
	int ListSize;
public:
	// default constructor 
	LinkedList (BlockHeader* h = nullptr){
		head = h;
	}
	void insert (BlockHeader* b){	// adds a block to the list
		// get the index of the block 
		if(!head){
			b->next = nullptr;
		}else{
			b->next = head;
		}

		head = b;
		b->isFree = true;
		++ListSize;
	}

	void remove (BlockHeader* b){  // removes a block from the list
		BlockHeader* current = head;
		while(current->next != b){
			current = current->next;
		}
		
		current->next = b->next;
		b->isFree = false;
		--ListSize;
	}

	BlockHeader* remove (){ // return the first item from the list
		assert (head != nullptr);
		BlockHeader* b = head;
		head = head->next;
		--ListSize;
		b->isFree = false;
		return b;
	}

	BlockHeader* getHead(){ return head; }

	// this is a test function 
	void print(){
		BlockHeader* current = head;
		std::cout << "[";
		while(current){
			std::cout << current << " " << current->block_size << "][";
			current = current->next;
		}
		std::cout << "]" << std::endl;
	}
};


class BuddyAllocator{
private:
	/* declare more member variables as necessary */
	vector<LinkedList> FreeList;
	int basic_block_size;
	int total_memory_size;
	char* start; // this is where the memory starts

private:
	/* private function you are required to implement
	 this will allow you and us to do unit test */
	
	BlockHeader* getbuddy (BlockHeader * addr){
		// since the memory starts at address 0 xor the addr size
		// to make this work for all cases we will make all the addreses equal to 0
		// (block address – start) XOR (block size) + start
		return (BlockHeader*) (((addr - (BlockHeader*) start) ^ addr->block_size) + start);
	} 
	// given a block address, this function returns the address of its buddy 
	
	bool arebuddies (BlockHeader* block1, BlockHeader* block2){
		return block2->isFree && block1->block_size == block2->block_size;
	}
	// checks whether the two blocks are buddies are not

	BlockHeader* merge (BlockHeader* block1, BlockHeader* block2){
		// there a bad memeory access when dealing with the free function
		// debug this functions 
		// check the addresses 
		int x = block1->block_size;
		int index = (ceil(log2 (ceil ( x / (double) basic_block_size))));
		
		FreeList[index].remove(block1);
		FreeList[index].remove(block2);

		block1->block_size *= 2;
		block1->next = nullptr;
		
		if (x >= total_memory_size){
			return nullptr;
		}

		x = block1->block_size;
		index = (ceil(log2 (ceil ( x / (double)basic_block_size))));
		FreeList[index].insert(block1);
		
		return block1;
	}
	// this function merges the two blocks returns the beginning address of the merged block
	// note that either block1 can be to the left of block2, or the other way around

	BlockHeader* split (BlockHeader* block){
		block->block_size = block->block_size / 2;

		BlockHeader * newblock = new BlockHeader();
		newblock->block_size = block->block_size; // split == halfed
		newblock->next = block->next;
		block->next = newblock;

		return newblock;
	}
	// splits the given block by putting a new header halfway through the block
	// also, the original header needs to be corrected


public:
	BuddyAllocator (int _basic_block_size, int _total_memory_length);
	/* This initializes the memory allocator and makes a portion of 
	   ’_total_memory_length’ bytes available. The allocator uses a ’_basic_block_size’ as 
	   its minimal unit of allocation. The function returns the amount of 
	   memory made available to the allocator. If an error occurred, 
	   it returns 0. 
	*/ 

	~BuddyAllocator(); 
	/* Destructor that returns any allocated memory back to the operating system. 
	   There should not be any memory leakage (i.e., memory staying allocated).
	*/ 

	char* alloc(int _length); 
	/* Allocate _length number of bytes of free memory and returns the 
		address of the allocated portion. Returns 0 when out of memory. */ 

	int free(char* _a); 
	/* Frees the section of physical memory previously allocated 
	   using ’my_malloc’. Returns 0 if everything ok. */ 
   
	void printlist ();
	/* Mainly used for debugging purposes and running short test cases */
	/* This function should print how many free blocks of each size belong to the allocator
	at that point. The output format should be the following (assuming basic block size = 128 bytes):

	[0] (128): 5
	[1] (256): 0
	[2] (512): 3
	[3] (1024): 0
	....
	....
	 which means that at this point, the allocator has 5 128 byte blocks, 3 512 byte blocks and so on.*/
};

#endif 
