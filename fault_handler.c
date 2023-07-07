/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include "../cpu/sched.h"
#include "../disk/pagefile_manager.h"
#include "../mem/memory_manager.h"

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE)
{
	assert(LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE ;
}
void setPageReplacmentAlgorithmCLOCK(){_PageRepAlgoType = PG_REP_CLOCK;}
void setPageReplacmentAlgorithmFIFO(){_PageRepAlgoType = PG_REP_FIFO;}
void setPageReplacmentAlgorithmModifiedCLOCK(){_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;}
/*2018*/ void setPageReplacmentAlgorithmDynamicLocal(){_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;}
/*2021*/ void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps){_PageRepAlgoType = PG_REP_NchanceCLOCK;  page_WS_max_sweeps = PageWSMaxSweeps;}

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE){return _PageRepAlgoType == LRU_TYPE ? 1 : 0;}
uint32 isPageReplacmentAlgorithmCLOCK(){if(_PageRepAlgoType == PG_REP_CLOCK) return 1; return 0;}
uint32 isPageReplacmentAlgorithmFIFO(){if(_PageRepAlgoType == PG_REP_FIFO) return 1; return 0;}
uint32 isPageReplacmentAlgorithmModifiedCLOCK(){if(_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1; return 0;}
/*2018*/ uint32 isPageReplacmentAlgorithmDynamicLocal(){if(_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL) return 1; return 0;}
/*2021*/ uint32 isPageReplacmentAlgorithmNchanceCLOCK(){if(_PageRepAlgoType == PG_REP_NchanceCLOCK) return 1; return 0;}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt){_EnableModifiedBuffer = enableIt;}
uint8 isModifiedBufferEnabled(){  return _EnableModifiedBuffer ; }

void enableBuffering(uint32 enableIt){_EnableBuffering = enableIt;}
uint8 isBufferingEnabled(){  return _EnableBuffering ; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length;}
uint32 getModifiedBufferLength() { return _ModifiedBufferLength;}

//===============================
// FAULT HANDLERS
//===============================

//Handle the table fault
void table_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}

//Handle the page fault

void placement(struct Env *curenv, uint32 fault_va)
{

	int flag =0;
	struct FrameInfo *ptr_frame_info = NULL; // dummy frame_info
	uint32 *ptr_pt1= NULL;
	int perm = 0 ;
	allocate_frame(&ptr_frame_info);
	perm =PERM_USER | PERM_PRESENT |PERM_WRITEABLE ;
	map_frame(curenv->env_page_directory, ptr_frame_info,fault_va,perm);
	int page_exist = pf_read_env_page(curenv,(void *)fault_va);
	if (page_exist == E_PAGE_NOT_EXIST_IN_PF)
	{
		if ((fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX) || (fault_va >= USTACKBOTTOM && fault_va < USTACKTOP))
		{
			flag =1;
		}
		else // place fault_va in allocated space
			panic("ILLEGAL MEMORY ACCESS");
	}
	/* Reflect changes in WS
	Update page file with new allocated page (fault_va and frame_info) */

	if (flag == 0)
	{
		ptr_frame_info = get_frame_info(curenv->env_page_directory,fault_va,&ptr_pt1);
		int page_updated = pf_update_env_page(curenv, fault_va, ptr_frame_info);
	}
	if (env_page_ws_get_size(curenv) < curenv->page_WS_max_size) {
		for(int i=curenv->page_last_WS_index;1;i++,i%=curenv->page_WS_max_size){
			if(env_page_ws_is_entry_empty(curenv,i)){
				env_page_ws_set_entry(curenv,i,fault_va);
				curenv->page_last_WS_index=i;
				break;
			}
		}
	}else{
		env_page_ws_set_entry(curenv,curenv->page_last_WS_index,fault_va);
		curenv->page_last_WS_index ++;
		curenv->page_last_WS_index %=curenv->page_WS_max_size;
	}


	//env_page_ws_print(curenv);
}

uint32 leavingIndex=0;

struct WorkingSetElement clock_Algo(struct Env *curenv)
{
	struct WorkingSetElement tmp ;
	for(uint32 i = curenv->page_last_WS_index;i<=curenv->page_WS_max_size;i++)
	{

		i %=curenv->page_WS_max_size ;
		curenv->page_last_WS_index= i ;
		tmp =curenv->ptr_pageWorkingSet[i];
		uint32 tmpVa = (uint32) tmp.virtual_address;
		int tmpPerm = pt_get_page_permissions((curenv->env_page_directory),tmpVa);//actual permission
		int tmpsopP=pt_get_page_permissions((curenv->env_page_directory),tmpVa) & (~PERM_USED);//looking for usedbit =0
		if(tmpPerm==tmpsopP)
		{
			leavingIndex=i;
			return tmp;
		}
		else
		{
			pt_set_page_permissions(curenv->env_page_directory,tmp.virtual_address,0,PERM_USED);
		}

	}
	return tmp;
}


void page_fault_handler(struct Env * curenv, uint32 fault_va)
{

	unsigned int WS_Size = 0;
	WS_Size = env_page_ws_get_size(curenv);
	int worksetSize = 0;
	worksetSize = curenv->page_WS_max_size;
	// Scenario 1 (Placement)
	if (WS_Size < worksetSize) // WS_Max defined in environment def.h
	{
		placement(curenv,fault_va);

	}
	else
	{
		struct WorkingSetElement leaving = clock_Algo(curenv);
		int leavingPerm;
		int leavingMod;
		leavingPerm = pt_get_page_permissions(curenv->env_page_directory,leaving.virtual_address);
		leavingMod = pt_get_page_permissions(curenv->env_page_directory,leaving.virtual_address) | PERM_MODIFIED;
		if (leavingPerm == leavingMod)
		{
			uint32 *ptr_table=NULL;
			struct FrameInfo *ptr_frame_info = NULL; // dummy frame_info //getframe info
			ptr_frame_info=get_frame_info(curenv->env_page_directory,leaving.virtual_address,&ptr_table);
			uint32 *ptr_pt1= NULL;
			int ret = pf_update_env_page(curenv,leaving.virtual_address,ptr_frame_info);
			free_frame(ptr_frame_info);
		}

		unmap_frame(curenv->env_page_directory,leaving.virtual_address);
		placement(curenv,fault_va);

	}

}

void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	// Write your code here, remove the panic and write your code
	panic("__page_fault_handler_with_buffering() is not implemented yet...!!");


}
