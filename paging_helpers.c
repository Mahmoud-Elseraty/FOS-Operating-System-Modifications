/*
 * paging_helpers.c
 *
 *  Created on: Sep 30, 2022
 *      Author: HP
 */
#include "memory_manager.h"

/*[2.1] PAGE TABLE ENTRIES MANIPULATION */
inline void pt_set_page_permissions(uint32* page_directory, uint32 virtual_address, uint32 permissions_to_set, uint32 permissions_to_clear)
{
	//TODO: [PROJECT MS2] [PAGING HELPERS] pt_set_page_permissions
	// Write your code here, remove the panic and write your code
	//panic("pt_set_page_permissions() is not implemented yet...!!");
	uint32 *ptr_to_pg_table=NULL;
	int flag = get_page_table(page_directory,virtual_address,&ptr_to_pg_table);
	if(ptr_to_pg_table==NULL)
		panic("Invalid va");
	else{
		ptr_to_pg_table[PTX(virtual_address)]=ptr_to_pg_table[PTX(virtual_address)] | permissions_to_set ;
		ptr_to_pg_table[PTX(virtual_address)]=ptr_to_pg_table[PTX(virtual_address)] & (~permissions_to_clear) ;
		tlb_invalidate((void*)NULL,(void*)virtual_address);
	}


}

inline int pt_get_page_permissions(uint32* page_directory, uint32 virtual_address )
{
	//TODO: [PROJECT MS2] [PAGING HELPERS] pt_get_page_permissions
		// Write your code here, remove the panic and write your code
		//panic("pt_get_page_permissions() is not implemented yet...!!");
		uint32 *ptr_pg_table;
			int not_found = get_page_table(page_directory,virtual_address,&ptr_pg_table);

			    if(not_found){
			    	return -1;
			    }
			    else
			    {
			    	uint32 returned_perm=0;
			    	returned_perm=ptr_pg_table[PTX(virtual_address)]<<20;
			    	returned_perm=returned_perm>>20;
			    	return returned_perm;
	            }
}

inline void pt_clear_page_table_entry(uint32* page_directory, uint32 virtual_address)
{
	//TODO: [PROJECT MS2] [PAGING HELPERS] pt_clear_page_table_entry
		// Write your code here, remove the panic and write your code
		//panic("pt_clear_page_table_entry() is not implemented yet...!!");
		uint32 *ptr_pg_table;
		int not_found = get_page_table(page_directory,virtual_address,&ptr_pg_table);

		    if(not_found){
		    	panic("Invalid page table");
		    }
		    else
		    {
		    	ptr_pg_table[PTX(virtual_address)]=0;

		    	tlb_invalidate((void*) NULL,(void*)virtual_address);
		    }
}

/***********************************************************************************************/

/*[2.2] ADDRESS CONVERTION*/
inline int virtual_to_physical(uint32* page_directory, uint32 virtual_address)
{
	//TODO: [PROJECT MS2] [PAGING HELPERS] virtual_to_physical
	// Write your code here, remove the panic and write your code
	//panic("virtual_to_physical() is not implemented yet...!!");
	uint32 *ptr_pg_table;
		int not_found = get_page_table(page_directory,virtual_address,&ptr_pg_table);

		    if(not_found){
		    	return -1;
		    }
		    else
		    {

		    	uint32 loc=ptr_pg_table[PTX(virtual_address)]>>12;
		    	loc=loc<<12;
                   return loc;
		    }


}

/***********************************************************************************************/

/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/

///============================================================================================
/// Dealing with page directory entry flags

inline uint32 pd_is_table_used(uint32* page_directory, uint32 virtual_address)
{
	return ( (page_directory[PDX(virtual_address)] & PERM_USED) == PERM_USED ? 1 : 0);
}

inline void pd_set_table_unused(uint32* page_directory, uint32 virtual_address)
{
	page_directory[PDX(virtual_address)] &= (~PERM_USED);
	tlb_invalidate((void *)NULL, (void *)virtual_address);
}

inline void pd_clear_page_dir_entry(uint32* page_directory, uint32 virtual_address)
{
	page_directory[PDX(virtual_address)] = 0 ;
	tlbflush();
}
