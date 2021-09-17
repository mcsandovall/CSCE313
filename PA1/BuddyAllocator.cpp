#include "BuddyAllocator.h"
#include <iostream>
#include <math.h>
using namespace std;
BlockHeader* split(BlockHeader* b){
  int bs = b->block_size;
  b->block_size /= 2;
  b->next = nullptr;

  BlockHeader* sh = (BlockHeader*) ((char*)b + b->block_size);
  sh->block_size = b->block_size;
  sh->next = nullptr;
  b->next = sh;
  return sh;
}

bool arebuddies (BlockHeader* block1, BlockHeader* block2){
  return block2->isfree && (block1->block_size == block2->block_size);
}

BuddyAllocator::BuddyAllocator (int _basic_block_size, int _total_memory_length){
	total_memory_size = _total_memory_length;
  basic_block_size = _basic_block_size;
  start = new BlockHeader [total_memory_size];
  int l = ceil(log2(total_memory_size/basic_block_size));
  for(int i  = 0; i < l; ++i){
    FreeList.push_back(LinkedList());
  }
  FreeList.push_back(LinkedList(start));
  BlockHeader* h = new (start) BlockHeader(total_memory_size);
  h->isfree = true;
}

BuddyAllocator::~BuddyAllocator (){
	delete[] start;
}

char* BuddyAllocator::alloc(int _length) {
  int x = _length + sizeof(BlockHeader);
  int index = (int) ceil(log2(ceil((double) x /  basic_block_size)));
  if (FreeList[index].head != nullptr){ // found the blcok that i am looking for
    BlockHeader* b = FreeList[index].remove();
    return (char*) (b+1);
  }

  int indexCorrect = index;
  for(; index < FreeList.size();++index){
    if(FreeList[index].head){
      break;
    }
  }
  if(index >= FreeList.size()){ // no bigger block found
    return nullptr;
  }

  // a bigger block found
  while(index >= indexCorrect){
    BlockHeader* b = FreeList[index].remove(); // gets the head of the index
    BlockHeader* shb = split(b);
    FreeList[--index].insert(b);
    FreeList[--index].insert(shb);
  }
  return (char*) (FreeList[index].remove() + 1);
}

int BuddyAllocator::free(char* _a) {
  /* Same here! */
  delete _a;
  return 0;
}

void BuddyAllocator::printlist (){
  cout << "Printing the Freelist in the format \"[index] (block size) : # of blocks\"" << endl;
  for (int i=0; i<FreeList.size(); i++){
    cout << "[" << i <<"] (" << ((1<<i) * basic_block_size) << ") : ";  // block size at index should always be 2^i * bbs
    int count = 0;
    BlockHeader* b = FreeList [i].head;
    // go through the list from head to tail and count
    while (b){
      count ++;
      // block size at index should always be 2^i * bbs
      // checking to make sure that the block is not out of place
      if (b->block_size != (1<<i) * basic_block_size){
        cerr << "ERROR:: Block is in a wrong list" << endl;
        exit (-1);
      }
      b = b->next;
    }
    cout << count << endl;  
  }
}
