/*
 * chunk_operations.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include <kern/trap/fault_handler.h>
#include <kern/disk/pagefile_manager.h>
#include "kheap.h"
#include "memory_manager.h"


/******************************/
/*[1] RAM CHUNKS MANIPULATION */
/******************************/

//===============================
// 1) CUT-PASTE PAGES IN RAM:
//===============================
//This function should cut-paste the given number of pages from source_va to dest_va
//if the page table at any destination page in the range is not exist, it should create it
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int cut_paste_pages(uint32* page_directory, uint32 source_va, uint32 dest_va, uint32 num_of_pages)
{
	uint32 start_s_va=ROUNDDOWN(source_va,PAGE_SIZE);
	uint32 end_s_va=start_s_va+(num_of_pages*PAGE_SIZE);
	uint32	start_d_va=ROUNDUP(dest_va,PAGE_SIZE);
	uint32 end_d_va=start_d_va+(num_of_pages*PAGE_SIZE);
	uint32 va=start_d_va;
	while(va!=end_d_va){
			uint32 *ptr_pg_table=NULL;
			int re = get_page_table(page_directory,va,&ptr_pg_table);
			if(re==TABLE_IN_MEMORY){
				uint32 loc=ptr_pg_table[PTX(va)];
				if(loc!=0)
				 return-1;
			}
		va+=PAGE_SIZE;
	}
	  for(int i=0;i<num_of_pages;i++){
		  uint32 perm =pt_get_page_permissions(page_directory,start_s_va);
		  struct FrameInfo *ptr=NULL;
		  unmap_frame(page_directory,start_s_va);
		  allocate_frame(&ptr);
		  map_frame(page_directory,ptr,start_d_va,perm);
		  start_s_va+=PAGE_SIZE;
		  start_d_va+=PAGE_SIZE;
	  }
	  return 0;
}

//===============================
// 2) COPY-PASTE RANGE IN RAM:
//===============================
//This function should copy-paste the given size from source_va to dest_va
//if the page table at any destination page in the range is not exist, it should create it
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int copy_paste_chunk(uint32* page_directory, uint32 source_va, uint32 dest_va, uint32 size)
{
//TODO: [PROJECT MS2] [CHUNK OPERATIONS] copy_paste_chunk
// Write your code here, remove the panic and write your code
//panic("copy_paste_chunk() is not implemented yet...!!");
	uint32 start_sva=ROUNDDOWN(source_va,PAGE_SIZE);
	uint32 end_sva=ROUNDUP(start_sva+size,PAGE_SIZE);
	uint32 start_dva=ROUNDDOWN(dest_va,PAGE_SIZE);
	uint32 end_dva=ROUNDUP(start_dva+size,PAGE_SIZE);
	//-----------EXISTING PAGES HANDLING------------
	for(uint32 i=start_dva;i<end_dva;i+=PAGE_SIZE){
		uint32 *page_table=NULL;
		struct FrameInfo*sva_frame=get_frame_info(page_directory,i,&page_table);
		if(sva_frame !=NULL){
			uint32 perms=pt_get_page_permissions(page_directory,i);
			uint32 check=PERM_WRITEABLE & perms;
			if(check!=PERM_WRITEABLE)
				return -1;
		}
	}
	//-----------NON-EXISTING PAGES HANDLING------------

	for(uint32 i=start_dva,j=start_sva;i<end_dva;i+=PAGE_SIZE,j+=PAGE_SIZE){
		uint32 *page_table=NULL;
		struct FrameInfo* tmp=get_frame_info(page_directory,i,&page_table);
		if(tmp==NULL){
		uint32 perm =pt_get_page_permissions(page_directory,j );
		perm=perm | PERM_WRITEABLE;
		struct FrameInfo *ptr=NULL;
		allocate_frame(&ptr);
		map_frame(page_directory,ptr,i,perm);
		}
	}
	//-----------COPYING------------

	for(uint32 i=dest_va,j=source_va;i<dest_va+size;i++,j++){
		uint8 * src_data = (void*)(j);
		uint8 * dest_data= (void*)(i);
		*dest_data = *src_data;
	}
	return 0;
	}

//===============================
// 3) SHARE RANGE IN RAM:
//===============================
//This function should share the given size from dest_va with the source_va
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int share_chunk(uint32* page_directory, uint32 source_va,uint32 dest_va, uint32 size, uint32 perms)
{
	//TODO: [PROJECT MS2] [CHUNK OPERATIONS] share_chunk
	// Write your code here, remove the panic and write your code
	//panic("share_chunk() is not implemented yet...!!");
	uint32 start_sva=ROUNDDOWN(source_va,PAGE_SIZE);
	uint32 end_sva=ROUNDUP(source_va+size,PAGE_SIZE);
	uint32 start_dva=ROUNDDOWN(dest_va,PAGE_SIZE);
	uint32 end_dva=ROUNDUP(dest_va+size,PAGE_SIZE);


	uint32 va=start_dva;
	while(va!=end_dva){
		uint32 *ptr_pg_table=NULL;
		int re = get_page_table(page_directory,va,&ptr_pg_table);
		if(re==TABLE_IN_MEMORY){
			uint32 loc=ptr_pg_table[PTX(va)];
			if(loc!=0)
				return-1;
		}
		va+=PAGE_SIZE;
	}
  for(uint32 i= start_dva , j=start_sva; i<end_dva ; i+=PAGE_SIZE,j+=PAGE_SIZE ) {
	 // unmap_frame(page_directory,start_s_va);
	  uint32 *ptr_pg_table=NULL;
	  struct FrameInfo *ptr=get_frame_info(page_directory,j,&ptr_pg_table);
	  map_frame(page_directory,ptr,i,perms);
  }
  return 0;

}

//===============================
// 4) ALLOCATE CHUNK IN RAM:
//===============================
//This function should allocate in RAM the given range [va, va+size)
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int allocate_chunk(uint32* page_directory, uint32 va, uint32 size, uint32 perms)
{
	//TODO: [PROJECT MS2] [CHUNK OPERATIONS] allocate_chunk
	// Write your code here, remove the panic and write your code
	//panic("allocate_chunk() is not implemented yet...!!");
	uint32 sva=ROUNDDOWN(va,PAGE_SIZE);
	uint32 eva=ROUNDUP(va+size,PAGE_SIZE);
	for(int i=sva;i<eva;i+=PAGE_SIZE){
		uint32 *ptr_page_table=NULL;
		struct FrameInfo* tmp=get_frame_info(page_directory,i,&ptr_page_table);
		if(tmp!=NULL)
			return -1;
	}

	for(int i=sva;i<eva;i+=PAGE_SIZE){
		struct FrameInfo *ptr_frame_info;
		int ret = allocate_frame(&ptr_frame_info);
		ret=map_frame(page_directory,ptr_frame_info,i,perms);
		ptr_frame_info->va=i;
	}
	return 0;
}

/*BONUS*/
//=====================================
// 5) CALCULATE ALLOCATED SPACE IN RAM:
//=====================================
void calculate_allocated_space(uint32* page_directory, uint32 sva, uint32 eva, uint32 *num_tables, uint32 *num_pages)
{
	//TODO: [PROJECT MS2 - BONUS] [CHUNK OPERATIONS] calculate_allocated_space
	// Write your code here, remove the panic and write your code
	//	panic("calculate_allocated_space() is not implemented yet...!!");//////////////
	uint32* ptr_table=NULL;
	sva=ROUNDDOWN(sva,PAGE_SIZE);
	eva=ROUNDUP(eva,PAGE_SIZE);
	uint32 Ntables=0;
	uint32 Npages=0;
	uint32 *checker=NULL;
	for(uint32 i= sva ; i<eva ; i+=PAGE_SIZE ){
		int ret = get_page_table(page_directory,i,&ptr_table);
		if (ret== TABLE_IN_MEMORY && checker!=ptr_table)
			Ntables++;
		checker=ptr_table;
	}
	for(uint32 i= sva ; i<eva ; i+=PAGE_SIZE ){
		struct FrameInfo * ptr=get_frame_info(page_directory,i,&ptr_table);
		if (ptr!=NULL)
			Npages++;
	}
	*num_pages=Npages;
	*num_tables=Ntables;
	cprintf ("NUMTABLES= %d\nNUMPAGES= %d\n",*num_tables,*num_pages);

}

/*BONUS*/
//=====================================
// 6) CALCULATE REQUIRED FRAMES IN RAM:
//=====================================
// calculate_required_frames:
// calculates the new allocation size required for given address+size,
// we are not interested in knowing if pages or tables actually exist in memory or the page file,
// we are interested in knowing whether they are allocated or not.
uint32 calculate_required_frames(uint32* page_directory, uint32 sva, uint32 size)
{
	// Write your code here, remove the panic and write your code
		//panic("calculate_required_frames() is not implemented yet...!!");
		uint32 s_va=ROUNDDOWN(sva,PAGE_SIZE);
		uint32 e_va=ROUNDUP(sva+size,PAGE_SIZE*1024);
		uint32 SIZEX=e_va-s_va;
		uint32 SIZEY=ROUNDUP(sva+size,PAGE_SIZE)-s_va;
		uint32 numt=0;uint32 nump=0;
		uint32 x=(SIZEX)/(PAGE_SIZE*1024);
		uint32 y=(SIZEY)/(PAGE_SIZE);
		uint32 R1=SIZEX%(PAGE_SIZE*1024)==0? x:x+1;
		uint32 R2=SIZEY%(PAGE_SIZE)==0? y:y+1;
		uint32 res = (R1+R2);
		calculate_allocated_space(page_directory,s_va,ROUNDUP(sva+size,PAGE_SIZE),&numt,&nump);
		return (res-numt-nump);
}
//=================================================================================//
//===========================END RAM CHUNKS MANIPULATION ==========================//
//=================================================================================//

/*******************************/
/*[2] USER CHUNKS MANIPULATION */
/*******************************/

//======================================================
/// functions used for USER HEAP (malloc, free, ...)
//======================================================

//=====================================
// 1) ALLOCATE USER MEMORY:
//=====================================
void allocate_user_mem(struct Env* e, uint32 virtual_address, uint32 size)
{
	// Write your code here, remove the panic and write your code
	panic("allocate_user_mem() is not implemented yet...!!");

}

//=====================================
// 2) FREE USER MEMORY:
//=====================================
int check_pg_table(uint32 *pg_table){
	for(uint32 i=0;i<1024;i++){
		if(pg_table[i]!=0)
			return 0;
	}
	return 1;
}
void free_user_mem(struct Env* e, uint32 virtual_address, uint32 size)
{
	//TODO: [PROJECT MS3] [USER HEAP - KERNEL SIDE] free_user_mem
	// Write your code here, remove the panic and write your code
	//panic("free_user_mem() is not implemented yet...!!");

	//This function should:
	//1. Free ALL pages of the given range from the Page File
	//2. Free ONLY pages that are resident in the working set from the memory
	//3. Removes ONLY the empty page tables (i.e. not used) (no pages are mapped in the table)
	uint32 SVA=ROUNDDOWN(virtual_address,PAGE_SIZE);
	uint32 EVA=ROUNDUP(virtual_address+size,PAGE_SIZE);
	int pg_checker;
	//env_page_ws_print(e);
	for(uint32 i=SVA;i<EVA;i+=PAGE_SIZE){
		uint32* ptr_table=NULL;
		pf_remove_env_page(e,i);
		env_page_ws_invalidate(e,i);
		unmap_frame(e->env_page_directory,i);
		int ret=get_page_table(e->env_page_directory,i,&ptr_table);
		if(ptr_table!=NULL){
			pg_checker=check_pg_table(ptr_table);
			if(pg_checker){
				e->env_page_directory[PDX(i)]=0;
				unmap_frame(e->env_page_directory,(uint32)ptr_table);
				kfree((uint32*)ptr_table);
			}
		}
	}
	//env_page_ws_print(e);

}

//=====================================
// 2) FREE USER MEMORY (BUFFERING):
//=====================================
void __free_user_mem_with_buffering(struct Env* e, uint32 virtual_address, uint32 size)
{
	// your code is here, remove the panic and write your code
	panic("__free_user_mem_with_buffering() is not implemented yet...!!");

	//This function should:
	//1. Free ALL pages of the given range from the Page File
	//2. Free ONLY pages that are resident in the working set from the memory
	//3. Free any BUFFERED pages in the given range
	//4. Removes ONLY the empty page tables (i.e. not used) (no pages are mapped in the table)
}

//=====================================
// 3) MOVE USER MEMORY:
//=====================================
void move_user_mem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
{
	//TODO: [PROJECT MS3 - BONUS] [USER HEAP - KERNEL SIDE] move_user_mem
	//your code is here, remove the panic and write your code
	panic("move_user_mem() is not implemented yet...!!");

	// This function should move all pages from "src_virtual_address" to "dst_virtual_address"
	// with the given size
	// After finished, the src_virtual_address must no longer be accessed/exist in either page file
	// or main memory

	/**/
}

//=================================================================================//
//========================== END USER CHUNKS MANIPULATION =========================//
//=================================================================================//
