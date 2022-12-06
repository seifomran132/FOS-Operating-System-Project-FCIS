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
	//TODO: [PROJECT MS2] [CHUNK OPERATIONS] cut_paste_pages
	// Write your code here, remove the panic and write your code
	//panic("cut_paste_pages() is not implemented yet...!!");

	uint32* sourcePageTable = NULL;
	get_page_table(page_directory, source_va, &sourcePageTable);
	uint32* destPageTable = NULL;
	get_page_table(page_directory, dest_va, &destPageTable);

	if (sourcePageTable != NULL) {
		//Round Addresses
		uint32 roundedAddress = ROUNDDOWN(source_va, PAGE_SIZE);
		uint32 roundedDestAddress = ROUNDDOWN(dest_va, PAGE_SIZE);

		// Save Addresses in variables to iterate
		uint32 srcAddressItr = roundedAddress;
		uint32 destAddressItr = roundedDestAddress;

		// Checking If Dest Address Exists
		uint32 destAddressForChecking = roundedDestAddress;
		for (int i = 0; i < num_of_pages; i++) {
			uint32* ptrToPTDEST = NULL;
			struct FrameInfo* destFrameInfo = get_frame_info(page_directory, destAddressForChecking, &ptrToPTDEST);
			if (destFrameInfo != NULL) {
				return -1;
			}
			else {
				destAddressForChecking += PAGE_SIZE;
			}

		}

		// Create Dest Page Table If Not Exist
		if (destPageTable == NULL) {
			create_page_table(page_directory, dest_va);
		}

		for (int i = 0; i < num_of_pages; i++) {

			// Getting Permissions
			int permsOfPage = pt_get_page_permissions(page_directory, srcAddressItr);

			// Mapping dest to PA
			uint32* ptrToPT = NULL;
			struct FrameInfo* myFrameInfo = get_frame_info(page_directory, srcAddressItr, &ptrToPT);
			int mapres = map_frame(page_directory, myFrameInfo, destAddressItr, permsOfPage);
			// Unmap Source
			unmap_frame(page_directory, srcAddressItr);
			// Increment Iterators
			srcAddressItr += PAGE_SIZE;
			destAddressItr += PAGE_SIZE;

		}
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

	uint32* sourcePageTable = NULL;
	get_page_table(page_directory, source_va, &sourcePageTable);
	uint32* destPageTable = NULL;
	get_page_table(page_directory, dest_va, &destPageTable);

	if (sourcePageTable != NULL) {


		// Check if destination exists with read only
		uint32 checkerIt = source_va;
		while (checkerIt < dest_va + size) {
			uint32* destTablePtr = NULL;
			struct FrameInfo* destFrameInfo = get_frame_info(page_directory, checkerIt, &destTablePtr);
			if (destFrameInfo != NULL) {

				int destPerms = pt_get_page_permissions(page_directory, checkerIt);
				int WritableFlag = destPerms & PERM_WRITEABLE;
				if (WritableFlag != 2) {
					return -1;
				}
				else {
					checkerIt += PAGE_SIZE;
				}

			}
			else {
				checkerIt += PAGE_SIZE;
			}
		}

		// If page table does not exist
		if (destPageTable == NULL) {
			create_page_table(page_directory, dest_va);
		}

		// Save Addresses in variables to iterate
		uint32 srcAddressItr = source_va;
		uint32 destAddressItr = dest_va;
		while (destAddressItr < dest_va + size) {

			uint32* srcTablePtr = NULL;
			struct FrameInfo* srcFrame = get_frame_info(page_directory, srcAddressItr, &srcTablePtr);

			uint32* destTablePtr = NULL;
			struct FrameInfo* destFrame = get_frame_info(page_directory, destAddressItr, &destTablePtr);

			if (destFrame != NULL) {

				// Mapping with the permissions
				int srcPerms = pt_get_page_permissions(page_directory, srcAddressItr);
				int mapres = map_frame(page_directory, destFrame, destAddressItr, srcPerms);

			}
			else {
				// Allocating frame
				int ret = allocate_frame(&destFrame);

				// Getting Permissions
				int srcPerms = pt_get_page_permissions(page_directory, srcAddressItr);
				int userPerm = srcPerms | PERM_WRITEABLE;

				// Mapping Destination to the frame
				int mapres = map_frame(page_directory, destFrame, destAddressItr, userPerm);
			}

			srcAddressItr += PAGE_SIZE;
			destAddressItr += PAGE_SIZE;
		}

		// Copying the data
		uint32 contIt = source_va;
		uint32 destContIt = dest_va;
		while (destContIt < dest_va + size) {
			unsigned char* destFrameContent = (unsigned char*)(destContIt);
			unsigned char* srcFrameContent = (unsigned char*)(contIt);
			*destFrameContent = *srcFrameContent;
			destContIt += 1;
			contIt += 1;
		}


	}

	return 0;

}

//===============================
// 3) SHARE RANGE IN RAM:
//===============================
//This function should share the given size from dest_va with the source_va
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int share_chunk(uint32* page_directory, uint32 source_va, uint32 dest_va, uint32 size, uint32 perms)
{
	//TODO: [PROJECT MS2] [CHUNK OPERATIONS] share_chunk
	// Write your code here, remove the panic and write your code
	//panic("share_chunk() is not implemented yet...!!");

	uint32 roundedSrcStart = ROUNDDOWN(source_va, PAGE_SIZE);
	uint32 roundedSrcEnd = ROUNDUP(source_va + size, PAGE_SIZE);
	uint32 roundedDestStart = ROUNDDOWN(dest_va, PAGE_SIZE);
	uint32 roundedDestEnd = ROUNDUP(dest_va + size, PAGE_SIZE);

	uint32* sourcePageTable = NULL;
	get_page_table(page_directory, roundedSrcStart, &sourcePageTable);
	uint32* destPageTable = NULL;
	get_page_table(page_directory, roundedDestStart, &destPageTable);



	if (sourcePageTable != NULL) {

		// Check if destination exists
		uint32 checkerIt = roundedDestStart;
		while (checkerIt < roundedDestEnd) {
			uint32* destTablePtr = NULL;
			struct FrameInfo* destFrameInfo = get_frame_info(page_directory, checkerIt, &destTablePtr);
			if (destFrameInfo != NULL) {
				return -1;
			}
			else {
				checkerIt += PAGE_SIZE;
			}
		}


		// Create Dest Page Table If Not Exist
		if (destPageTable == NULL) {
			create_page_table(page_directory, roundedDestStart);
		}

		// Save Addresses in variables to iterate
		uint32 srcAddressItr = roundedSrcStart;
		uint32 destAddressItr = roundedDestStart;
		while (destAddressItr < roundedDestEnd) {

			uint32* srcTablePtr = NULL;
			struct FrameInfo* srcFrame = get_frame_info(page_directory, srcAddressItr, &srcTablePtr);

			//uint32 *destTablePtr = NULL;
			//struct FrameInfo *destFrame = get_frame_info(page_directory, destAddressItr, &destTablePtr);
			//int ret = allocate_frame(&destFrame);

			int mapres = map_frame(page_directory, srcFrame, destAddressItr, perms);

			srcAddressItr += PAGE_SIZE;
			destAddressItr += PAGE_SIZE;
		}
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

	uint32 roundedStart = ROUNDDOWN(va, PAGE_SIZE);
	uint32 roundedEnd = ROUNDUP(va + size, PAGE_SIZE);

	uint32* ourPageTable = NULL;
	get_page_table(page_directory, roundedStart, &ourPageTable);

	// Check If Destination Exists
	uint32 checkerIt = roundedStart;
	while (checkerIt < roundedEnd) {

		uint32* destTablePtr = NULL;
		struct FrameInfo* destFrameInfo = get_frame_info(page_directory, checkerIt, &destTablePtr);
		if (destFrameInfo != NULL) {
			cprintf("Address Exists \n");
			return -1;
		}

		checkerIt += PAGE_SIZE;
	}

	if (ourPageTable == NULL) {
		create_page_table(page_directory, roundedStart);
	}

	uint32 allocIt = roundedStart;
	while (allocIt < roundedEnd) {

		// Mapping dest to PA
		uint32* ptrToPT = NULL;
		struct FrameInfo* myFrameInfo = get_frame_info(page_directory, allocIt, &ptrToPT);
		int ret = allocate_frame(&myFrameInfo);
		int mapres = map_frame(page_directory, myFrameInfo, allocIt, perms);
		//cprintf("Allocated: %d\n", allocIt);
		allocIt += PAGE_SIZE;
	}
	return 0;
}

/*BONUS*/
//=====================================
// 5) CALCULATE ALLOCATED SPACE IN RAM:
//=====================================
void calculate_allocated_space(uint32* page_directory, uint32 sva, uint32 eva, uint32* num_tables, uint32* num_pages)
{
	//TODO: [PROJECT MS2 - BONUS] [CHUNK OPERATIONS] calculate_allocated_space
	// Write your code here, remove the panic and write your code
	//panic("calculate_allocated_space() is not implemented yet...!!");

	uint32 NumOfTables = 0;
	uint32 NumOfPages = 0;

	// Round up and down the addresses
	uint32 roundedBase = ROUNDDOWN(sva, PAGE_SIZE);
	uint32 roundedEnd = ROUNDUP(eva, PAGE_SIZE);

	uint32 iteratorAddress = roundedBase;

	uint32* savedPagedTable = NULL;
	while (iteratorAddress <= roundedEnd) {
		uint32* ourPageTable = NULL;
		get_page_table(page_directory, iteratorAddress, &ourPageTable);

		if (ourPageTable != NULL && savedPagedTable != ourPageTable) {
			NumOfTables++;
			savedPagedTable = ourPageTable;
		}
		if (ourPageTable != NULL) {

			uint32* ptrToFTable = NULL;
			struct FrameInfo* frameToBeChecked = get_frame_info(page_directory, iteratorAddress, &ptrToFTable);
			if (frameToBeChecked != 0) {
				NumOfPages++;
			}

		}
		iteratorAddress += PAGE_SIZE;
	}
	*num_pages = NumOfPages;
	*num_tables = NumOfTables;
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
	//TODO: [PROJECT MS2 - BONUS] [CHUNK OPERATIONS] calculate_required_frames
	// Write your code here, remove the panic and write your code
	//panic("calculate_required_frames() is not implemented yet...!!");

	uint32 totalNumOfPages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	uint32 NumOfTables = 0;
	uint32 NumOfPages = 0;
	uint32 roundedEnd = ROUNDUP(sva + size, PAGE_SIZE);

	// Round up and down the addresses
	uint32 roundedBase = ROUNDDOWN(sva, PAGE_SIZE);


	uint32 ptChecker = roundedBase;
	uint32 tableIt = -1;
	uint32* ourPageTable = NULL;
	while (ptChecker < roundedEnd) {
		get_page_table(page_directory, ptChecker, &ourPageTable);
		if (tableIt != PDX(ptChecker)) {
		}
		if (ourPageTable == NULL) {

			if (tableIt == PDX(ptChecker)) {

			}
			else {
				NumOfTables += 1;

			}

		}

		tableIt = PDX(ptChecker);
		ptChecker += PAGE_SIZE;
	}
	uint32 addrStart = roundedBase;
	uint32 addrEnd = roundedEnd;
	while (addrStart < addrEnd) {
		uint32* ptrToFTable = NULL;
		struct FrameInfo* frameToBeChecked = get_frame_info(page_directory, addrStart, &ptrToFTable);
		if (frameToBeChecked != NULL) {
		}
		else {
			NumOfPages++;
		}

		addrStart += PAGE_SIZE;
	}
	int total = NumOfPages + NumOfTables;
	return total;
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
void free_user_mem(struct Env* e, uint32 virtual_address, uint32 size)
{
	//TODO: [PROJECT MS3] [USER HEAP - KERNEL SIDE] free_user_mem
	// Write your code here, remove the panic and write your code
	panic("free_user_mem() is not implemented yet...!!");

	//This function should:
	//1. Free ALL pages of the given range from the Page File
	//2. Free ONLY pages that are resident in the working set from the memory
	//3. Removes ONLY the empty page tables (i.e. not used) (no pages are mapped in the table)





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



