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

	uint32 * ptrPageTable = NULL;

	get_page_table(page_directory, virtual_address, &ptrPageTable);

	if (ptrPageTable != NULL) {
		ptrPageTable[PTX(virtual_address)] = ptrPageTable[PTX(virtual_address)] | permissions_to_set;
		ptrPageTable[PTX(virtual_address)] = ptrPageTable[PTX(virtual_address)] & (~permissions_to_clear);
	}
	else {
		panic("Page table does not exist");
	}
	tlb_invalidate((void*)NULL, (void*)virtual_address);
}

inline int pt_get_page_permissions(uint32* page_directory, uint32 virtual_address )
{
	//TODO: [PROJECT MS2] [PAGING HELPERS] pt_get_page_permissions
	// Write your code here, remove the panic and write your code
	//panic("pt_get_page_permissions() is not implemented yet...!!");
	uint32 *ptr_page_table = NULL;
	int result = get_page_table(page_directory, virtual_address, &ptr_page_table);
	if (result  == TABLE_NOT_EXIST)
	{
		return -1;
	}
	else if (result  == TABLE_IN_MEMORY)
	{
		uint32 table_entry = ptr_page_table[PTX(virtual_address)];
		int n = (((1 << 12) - 1) & table_entry);
		return n;
	}
	return 0;
}

inline void pt_clear_page_table_entry(uint32* page_directory, uint32 virtual_address)
{
	//TODO: [PROJECT MS2] [PAGING HELPERS] pt_clear_page_table_entry
	// Write your code here, remove the panic and write your code
	//panic("pt_clear_page_table_entry() is not implemented yet...!!");
	uint32 *ptr_page_table = NULL;
	int result = get_page_table(page_directory, virtual_address, &ptr_page_table);
	if (result  == TABLE_NOT_EXIST)
	{
		panic("Page Table Not Exist");
	}
	else if (result  == TABLE_IN_MEMORY)
	{
		unmap_frame(page_directory, virtual_address);
	}
	tlb_invalidate((void *)NULL, (void *)virtual_address);
}

/***********************************************************************************************/

/*[2.2] ADDRESS CONVERTION*/
inline int virtual_to_physical(uint32* page_directory, uint32 virtual_address)
{
	//TODO: [PROJECT MS2] [PAGING HELPERS] virtual_to_physical
	// Write your code here, remove the panic and write your code
	//panic("virtual_to_physical() is not implemented yet...!!");

	uint32 *pageTable = NULL;
	get_page_table(page_directory, virtual_address, &pageTable);

	if(pageTable != NULL) {
		uint32 pageEntry = pageTable[PTX(virtual_address)];
		int offset = (((1 << 12) - 1) & pageEntry);
		int physicalAddress = (pageEntry) - offset;

		return physicalAddress;
	}
	else {
		return -1;
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
