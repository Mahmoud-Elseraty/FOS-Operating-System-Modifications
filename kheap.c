#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"
//==================================================================//
//==================================================================//
//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)//
//==================================================================//
//==================================================================//
uint32 STARTADDR =0;
void initialize_dyn_block_system()
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] initialize_dyn_block_system
	// your code is here, remove the panic and write your code
	//kpanic_into_prompt("initialize_dyn_block_system() is not implemented yet...!!");

	//[1] Initialize two lists (AllocMemBlocksList & FreeMemBlocksList) [Hint: use LIST_INIT()]
	LIST_INIT(&FreeMemBlocksList);
	LIST_INIT(&AllocMemBlocksList);
#if STATIC_MEMBLOCK_ALLOC
	//DO NOTHING
#else
	/*[2] Dynamically allocate the array of MemBlockNodes
	 * 	remember to:
	 * 		1. set MAX_MEM_BLOCK_CNT with the chosen size of the array
	 * 		2. allocation should be aligned on PAGE boundary
	 * 	HINT: can use alloc_chunk(...) function
	 */
	MAX_MEM_BLOCK_CNT=NUM_OF_KHEAP_PAGES;
	MemBlockNodes=(struct MemBlock*)KERNEL_HEAP_START;
	uint32 HEAP_SIZE=NUM_OF_KHEAP_PAGES * PAGE_SIZE;
	uint32 blockSize=NUM_OF_KHEAP_PAGES * sizeof(*MemBlockNodes);
	int check=allocate_chunk(ptr_page_directory,KERNEL_HEAP_START,blockSize,PERM_WRITEABLE);
#endif
	//[3] Initialize AvailableMemBlocksList by filling it with the MemBlockNodes
	//uint32 HEAP_SIZE=NUM_OF_KHEAP_PAGES * PAGE_SIZE;
	//uint32 blockSize=NUM_OF_KHEAP_PAGES * sizeof(*MemBlockNodes);
	initialize_MemBlocksList(MAX_MEM_BLOCK_CNT);
	struct MemBlock *Rem=LIST_LAST(&AvailableMemBlocksList);
	LIST_REMOVE(&AvailableMemBlocksList,LIST_LAST(&AvailableMemBlocksList));
	//Rem->size=HEAP_SIZE - ROUNDUP(NUM_OF_KHEAP_PAGES* sizeof(*MemBlockNodes),PAGE_SIZE);
	Rem->size=HEAP_SIZE - ROUNDUP(blockSize,PAGE_SIZE);
	Rem->sva=KERNEL_HEAP_START + ROUNDUP(blockSize,PAGE_SIZE);
	LIST_INSERT_HEAD(&FreeMemBlocksList,Rem);
	//[4] Insert a new MemBlock with the remaining heap size into the FreeMemBlocksList

}


void* kmalloc(unsigned int size)
{

	//TODO: [PROJECT MS2] [KERNEL HEAP] kmalloc
	// your code is here, remove the panic and write your code
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");

	//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
	//refer to the project presentation and documentation for details
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

	//change this "return" according to your answer
	size=ROUNDUP(size,PAGE_SIZE);
	if(isKHeapPlacementStrategyFIRSTFIT()){
		struct MemBlock* Returned=alloc_block_FF(size);
		if(Returned!=NULL){
			allocate_chunk(ptr_page_directory,Returned->sva,Returned->size,PERM_WRITEABLE);
			insert_sorted_allocList(Returned);
			return (void*)Returned->sva;
		}return NULL;

	}else if(isKHeapPlacementStrategyBESTFIT()){
		struct MemBlock* Returned=alloc_block_BF(size);
		if(Returned!=NULL){
			allocate_chunk(ptr_page_directory,Returned->sva,Returned->size,PERM_WRITEABLE);
			insert_sorted_allocList(Returned);
			return (void*)Returned->sva;
		}
		return NULL;

	}else{
		struct MemBlock* Returned=alloc_block_NF(size);
		if(Returned!=NULL){
			allocate_chunk(ptr_page_directory,Returned->sva,Returned->size,PERM_WRITEABLE);
			insert_sorted_allocList	(Returned);
			return (void*)Returned->sva;
		}
		return NULL;
	}
}


void kfree(void* virtual_address)
{
//	TODO: [PROJECT MS2] [KERNEL HEAP] kfree
//	 Write your code here, remove the panic and write your code
//	panic("kfree() is not implemented yet...!!");
	//show_list_content12(&AllocMemBlocksList);
	struct MemBlock* Found=find_block(&AllocMemBlocksList,(uint32)virtual_address);
	//cprintf("VA= %x SIZE\n %d \n",virtual_address,LIST_SIZE(&AllocMemBlocksList));
	if(Found!=NULL){
		for(uint32 i=ROUNDDOWN(Found->sva,PAGE_SIZE);i<ROUNDUP(Found->sva + Found->size,PAGE_SIZE);i+=PAGE_SIZE){
			uint32 *ptr_table=NULL;
			struct FrameInfo* current=get_frame_info(ptr_page_directory,i,&ptr_table);
			unmap_frame(ptr_page_directory,i);
			//free_frame(current);
		}
		LIST_REMOVE(&(AllocMemBlocksList),Found);
		insert_sorted_with_merge_freeList(Found);
	}
	//else
		//cprintf("IAM NOT FOUND \n",LIST_SIZE(&AllocMemBlocksList));
}


unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kheap_virtual_address
	// Write your code here, remove the panic and write your code
	/// panic("kheap_virtual_address() is not implemented yet...!!");
	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details
	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
//	struct FrameInfo *ptr_frame_info;
//	ptr_frame_info = to_frame_info(physical_address) ;
//	cprintf("%x \n",to_frame_number(ptr_frame_info)<<12);
//	return to_frame_number(ptr_frame_info)<<12;
	struct FrameInfo *tmp=to_frame_info(physical_address);
	return tmp->va;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kheap_physical_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");
	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details
	uint32 ret=virtual_to_physical(ptr_page_directory,virtual_address);
	return virtual_to_physical(ptr_page_directory,virtual_address);
}


void kfreeall()
{
	panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
	panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
	panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

int check_chunk(uint32 sva,uint32 size){
	sva=ROUNDDOWN(sva,PAGE_SIZE);
	uint32 eva=ROUNDUP(sva+size,PAGE_SIZE);
	uint32 *page_table;
	for(uint32 i=sva;i<eva;i+=PAGE_SIZE){
		struct FrameInfo* ptr=get_frame_info(ptr_page_directory,i,&page_table);
		if(ptr!=NULL)
			return 0;
	}
	return 1;
}
void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT MS2 - BONUS] [KERNEL HEAP] krealloc
	// Write your code here, remove the panic and write your code
	//panic("krealloc() is not implemented yet...!!");
	if(virtual_address==NULL)
		return kmalloc(new_size);
	if(new_size==0){
		kfree(virtual_address);
		return NULL;
	}
	//---------------------
	struct MemBlock* found=find_block(&AllocMemBlocksList,(uint32)virtual_address);
	uint32 sva=ROUNDDOWN((uint32)virtual_address+found->size,PAGE_SIZE);
	uint32 eva=ROUNDUP((uint32)virtual_address+new_size,PAGE_SIZE);
	if(found->size < new_size && check_chunk(sva,new_size-found->size)){
		allocate_chunk(ptr_page_directory,sva,new_size-found->size,PERM_WRITEABLE);
		cprintf("BOMV1  \n");
	}
	else if(found->size < new_size && !check_chunk(sva,new_size-found->size)){
		kfree((void*)found->sva);
		cprintf("BOMV2 \n");
		return kmalloc(new_size);
	}

	return (void*) virtual_address;
}

