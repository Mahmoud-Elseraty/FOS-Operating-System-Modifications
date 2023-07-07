/*
 * dyn_block_management.c
 *
 *  Created on: Sep 21, 2022
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"
struct MemBlock *last_checked=NULL;

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//===========================
// PRINT MEM BLOCK LISTS:
//===========================

void print_mem_block_lists()
{
	cprintf("\n=========================================\n");
	struct MemBlock* blk ;
	struct MemBlock* lastBlk = NULL ;
	cprintf("\nFreeMemBlocksList:\n");
	uint8 sorted = 1 ;
	LIST_FOREACH(blk, &FreeMemBlocksList)
	{
		if (lastBlk && blk->sva < lastBlk->sva + lastBlk->size)
			sorted = 0 ;
		cprintf("[%x, %x)-->", blk->sva, blk->sva + blk->size) ;
		lastBlk = blk;
	}
	if (!sorted)	cprintf("\nFreeMemBlocksList is NOT SORTED!!\n") ;

	lastBlk = NULL ;
	cprintf("\nAllocMemBlocksList:\n");
	sorted = 1 ;
	LIST_FOREACH(blk, &AllocMemBlocksList)
	{
		if (lastBlk && blk->sva < lastBlk->sva + lastBlk->size)
			sorted = 0 ;
		cprintf("[%x, %x)-->", blk->sva, blk->sva + blk->size) ;
		lastBlk = blk;
	}
	if (!sorted)	cprintf("\nAllocMemBlocksList is NOT SORTED!!\n") ;
	cprintf("\n=========================================\n");

}

//********************************************************************************//
//********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//===============================
// [1] INITIALIZE AVAILABLE LIST:
//===============================
void initialize_MemBlocksList(uint32 numOfBlocks)
{
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] initialize_MemBlocksList
	// Write your code here, remove the panic and write your code
	//panic("initialize_MemBlocksList() is not implemented yet...!!");
	assert(numOfBlocks <= MAX_MEM_BLOCK_CNT);
	LIST_INIT(&(AvailableMemBlocksList));
	for(int i=0;i<numOfBlocks;i++){
		LIST_INSERT_HEAD(&AvailableMemBlocksList,&MemBlockNodes[i]);
	}
}
//{1,2,6,4,1,2,4,7}
//{3}
//SIZE=4

//===============================
// [2] FIND BLOCK:
//===============================
struct MemBlock *find_block(struct MemBlock_List *blockList, uint32 va)
{
	//panic("find_block() is not implemented yet...!!");
	struct MemBlock *tmp;
	struct MemBlock *Found=NULL;

	LIST_FOREACH(tmp,blockList){
		if(tmp->sva ==va )
			Found=tmp;
	}
	return Found;
}



//=========================================
// [3] INSERT BLOCK IN ALLOC LIST [SORTED]:
//=========================================
void insert_sorted_allocList(struct MemBlock *blockToInsert)
{
	if (LIST_SIZE(&AllocMemBlocksList) == 0)
	{
		LIST_INSERT_HEAD(&AllocMemBlocksList, blockToInsert);
		return;
	}

	struct MemBlock *it;
	LIST_FOREACH(it, &AllocMemBlocksList)
	{
		if (blockToInsert->sva < it->sva)
		{
			LIST_INSERT_BEFORE(&AllocMemBlocksList, it, blockToInsert);
			return;
		}
	}

	LIST_INSERT_TAIL(&AllocMemBlocksList, blockToInsert);

}


//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
struct MemBlock *alloc_block_FF(uint32 size)
{
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] alloc_block_FF
	// Write your code here, remove the panic and write your code
	//panic("alloc_block_FF() is not implemented yet...!!");
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] alloc_block_FF
		// Write your code here, remove the panic and write your code

		struct MemBlock *tmp; // tmp

		LIST_FOREACH(tmp,&FreeMemBlocksList)
		{

				// Case 1 (Divide and Return)
				if((tmp->size)>size)

				{

					// Allocate needed space in Available
					struct MemBlock *returned = LIST_FIRST(&AvailableMemBlocksList);
					LIST_REMOVE(&AvailableMemBlocksList,returned);

					// Define size and SVA of needed space
					returned->size = size;
					returned->sva = tmp->sva;

					// Define size and SVA of residual (not needed) space
					tmp->size = tmp->size-size;
					tmp->sva =tmp->sva+size;

					// Removing residual space from Available
					return returned;

				}

				// Case 2 (Exact size is found)
				else if((tmp->size) == size)
				{
					LIST_REMOVE(&FreeMemBlocksList, tmp);

					return tmp;
				}
				else
					continue;
		}
				return NULL;
}
//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
struct MemBlock *alloc_block_BF(uint32 size)
{
		struct MemBlock *tmp;
		struct MemBlock *bfound=NULL;
		//{2*kilo, 8*kilo, 1*kilo, 9*kilo, 4*kilo, 2*Mega, 1*Mega}
		int first_found=0;
		LIST_FOREACH(tmp,&FreeMemBlocksList){
			if((tmp->size)>=size&&!first_found){
				first_found=1;
				bfound=tmp;
			}
			if(tmp->size>=size && tmp->size < bfound->size){
				bfound =tmp;
			}
		}
		 if(bfound==NULL)
			 return NULL;
		 else if(bfound->size>size){
			 struct MemBlock *zomba=LIST_FIRST(&AvailableMemBlocksList);
			LIST_REMOVE(&AvailableMemBlocksList,zomba);
			 zomba->size=bfound->size;
			 zomba->sva=bfound->sva;
			 LIST_FOREACH(tmp,&FreeMemBlocksList){
				 	 if(tmp->size == bfound->size){
				 		 tmp->size=(bfound->size)-size;
				 		 tmp->sva+=size;
					 break;
				}}
			 zomba->size =size;
			 return zomba;
		 }
		 else{
			 LIST_FOREACH(tmp,&FreeMemBlocksList){
				 if(tmp->size==bfound->size){
					 LIST_REMOVE(&FreeMemBlocksList,tmp);
				 }
			 }

			 return bfound;
		 }
}
//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
//{3*kilo, 3*kilo, 1*kilo, 10*kilo, 4*kilo, 2*Mega, 1*Mega, 1*kilo} ;
struct MemBlock *ptr=NULL;
struct MemBlock *alloc_block_NF(uint32 size)
{
	//TODO: [PROJECT MS1 - BONUS] [DYNAMIC ALLOCATOR] alloc_block_NF
	// Write your code here, remove the panic and write your code
	//panic("alloc_block_NF() is not implemented yet...!!");
	struct MemBlock *tmp=NULL;
	if(last_checked==NULL){
		//FIRST ITERATION, OR NO VALUE FOUND
		last_checked=alloc_block_FF(size);

		return last_checked;
	}
	else{
		int counter=0;
		LIST_FOREACH(tmp,&FreeMemBlocksList){
			if(last_checked->sva > tmp->sva)
				continue;
			else{
				if((tmp->size)>size){
				struct MemBlock *returned = LIST_FIRST(&AvailableMemBlocksList);
				LIST_REMOVE(&AvailableMemBlocksList,returned);
				returned->size = size;
				returned->sva = tmp->sva;
				tmp->size = tmp->size-size;
				tmp->sva =tmp->sva+size;
				last_checked=tmp;
				return returned;
			}
			else if((tmp->size) == size){
				last_checked=LIST_NEXT(tmp);
				LIST_REMOVE(&FreeMemBlocksList, tmp);
				return tmp;
				}
			if(tmp == LIST_LAST(&FreeMemBlocksList)){
					//Return to start, just play it like FF
					last_checked=alloc_block_FF(size);
					return last_checked;
				}
			}
		}
		cprintf("our ptr = %x \n",(last_checked->sva));
	return NULL;
	}
}


//===================================================
// [8] INSERT BLOCK (SORTED WITH MERGE) IN FREE LIST:
//===================================================
void insert_sorted_with_merge_freeList(struct MemBlock *blockToInsert)
{
	//print_mem_block_lists() ;

	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] insert_sorted_with_merge_freeList
	// Write your code here, remove the panic and write your code
	//panic("insert_sorted_with_merge_freeList() is not implemented yet...!!");

	//insert in head (case 1)
	if (LIST_EMPTY(&FreeMemBlocksList) || (LIST_FIRST(&FreeMemBlocksList)->sva > blockToInsert->sva && (LIST_FIRST(&FreeMemBlocksList)->sva != (blockToInsert->sva + blockToInsert->size))))
	{
			LIST_INSERT_HEAD(&FreeMemBlocksList,blockToInsert);
	}
	//insert in tail making sure it doesnt need merge(extension to case 1 not written)
	else if ((LIST_LAST(&FreeMemBlocksList)->sva < blockToInsert->sva) && (LIST_LAST(&FreeMemBlocksList)->sva+LIST_LAST(&FreeMemBlocksList)->size != blockToInsert->sva))
	{
		LIST_INSERT_TAIL(&FreeMemBlocksList,blockToInsert);
	}
	else
	{
		struct MemBlock *tmp;
		LIST_FOREACH(tmp, &FreeMemBlocksList)
		{
				struct MemBlock *hamada = LIST_NEXT(tmp);
			//merging with both (case 5)
			 if (((tmp->sva + tmp->size) == blockToInsert->sva) && ((LIST_LAST(&FreeMemBlocksList) != tmp) && (hamada->sva == (blockToInsert->sva + blockToInsert->size))))
			{
				tmp->size=tmp->size + blockToInsert->size + hamada->size;
				LIST_REMOVE(&FreeMemBlocksList,hamada);
				//blockToInsert->size = blockToInsert->size +LIST_NEXT(tmp)->size;
				hamada->size = 0;
				hamada->sva = 0;
				LIST_INSERT_TAIL(&AvailableMemBlocksList,hamada);
				LIST_INSERT_TAIL(&AvailableMemBlocksList,blockToInsert);
				blockToInsert->size = 0;
				blockToInsert->sva = 0;
				break;
			}
			//merging with free block before (case 3)
			 else if ((tmp->sva + tmp->size) == blockToInsert->sva)
			{
				tmp->size = tmp->size+blockToInsert->size;
				LIST_INSERT_TAIL(&AvailableMemBlocksList,blockToInsert);
				blockToInsert->size = 0;
				blockToInsert->sva = 0;
				break;
			}
			//merging with the free block after (case 4)
			else if (tmp->sva  == (blockToInsert->sva + blockToInsert->size))
			{
				tmp->size = tmp->size+blockToInsert->size;
				tmp->sva = blockToInsert->sva;
				blockToInsert->size = 0;
				blockToInsert->sva = 0;
				LIST_INSERT_TAIL(&AvailableMemBlocksList,blockToInsert);
				break;
			}
			//not merging and inserting freely in freemem (case 2)
			else if (tmp->sva > blockToInsert->sva)
			{
				LIST_INSERT_BEFORE(&FreeMemBlocksList,tmp,blockToInsert);
				break;
			}
		}
	}
}
