#include "BuddyAllocator.h"
#include <iostream>
#include <math.h>
using namespace std;

BlockHeader* split (BlockHeader* b){
  int bs = b->block_size;
  b->block_size /= 2;
  b->next =  nullptr;

  BlockHeader* sh = (BlockHeader*) (char*)b + b->block_size;
  BlockHeader* temp = new (sh) BlockHeader (b->block_size);
  return sh;
}

BuddyAllocator::BuddyAllocator (int _basic_block_size, int _total_memory_length){
  total_memory_size = _total_memory_length;
  basic_block_size = _basic_block_size;
	start = new char [total_memory_size];
  int l = log2 (total_memory_size/basic_block_size);
  for(int i  = 0; i < l;++i){
    FreeList.push_back (LinkedList());
  }
  FreeList.push_back(LinkedList((BlockHeader*)start));
  BlockHeader* h = new (start) BlockHeader (total_memory_size);
}

BuddyAllocator::~BuddyAllocator (){
	delete[] start;
  // delete all the nodes from the list 
}

char* BuddyAllocator::alloc(int _length) {
  int x = _length + sizeof(BlockHeader);
  int index =  ceil(log2 (ceil (x / (double) basic_block_size)));
  //int blockSizeReturn = (1 << index) * basic_block_size;
  if(FreeList[index].head != nullptr){ // found the block that we are looking for 
    BlockHeader* b = FreeList[index].remove();
    return  (char*) (b+1);
  }
  int indexCorrect = index;
  for(; index < FreeList.size() + 1; index++){ // otherwise check the vector for a block of that size
    if(FreeList[index].head){
      break;
    }
  }

  if(index >= FreeList.size()){
    // no bigger block found then return a null
    return nullptr;
  }

  // a bigger block was found
  while (index > indexCorrect){
    BlockHeader* b = FreeList[index].remove(); // get that block of memory
    BlockHeader* shb = split (b); // split b into two blocks of equal size where b already has a pointer there
    --index;
    FreeList[index].insert(b); // insert b and shb to the space before our index since that will be half the size
    FreeList[index].insert(shb);
  }
  return (char*) (FreeList[index].remove() + 1);  // remove the next pointer to the list
}

int BuddyAllocator::free(char* _a) {
  // Given the address, frees up the memory region starting 
  // at that address (i.e., consider it as free again for 
  // future alloc() calls)

  // get the start of the memory taken up by the block
  // BlockHeader* s_a = (BlockHeader*) _a -1;
  // s_a->next = nullptr;

  BlockHeader* startingAddress = (BlockHeader*) ( _a - sizeof(BlockHeader));
  BlockHeader* bh = startingAddress;

  for(int i =0; i < FreeList.size();++i){
    BlockHeader* b = FreeList[i].head;

    while(b){
      if(b == startingAddress){
        return -1;
      }
      b = b->next;
    }
  }

  // get the index for the list 
  int x = startingAddress->block_size;
  int index = (ceil(log2 (ceil ((double) x / basic_block_size))));

  FreeList[index].insert(bh); // insert them into the list


  // start merging the blocks of memory 
  int newSize = bh->block_size;
  bool merging = true;
  while(merging){
    BlockHeader* buddyAddress = getbuddy(startingAddress);
    if(arebuddies(startingAddress, buddyAddress)){
      startingAddress = merge(startingAddress,buddyAddress);
      bh = (BlockHeader*) startingAddress;
    }else {merging = false;}
  }

  return 0;
  // if alloc removes a free memory block from the FreeList, this should do the opposite and add that memory block to the LL
  // delete[] _a;
  // return 0;
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

