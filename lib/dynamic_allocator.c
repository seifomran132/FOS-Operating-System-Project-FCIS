/*
 * dyn_block_management.c
 *
 *  Created on: Sep 21, 2022
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"


 //==================================================================================//
 //============================== GIVEN FUNCTIONS ===================================//
 //==================================================================================//

 //===========================
 // PRINT MEM BLOCK LISTS:
 //===========================

void print_mem_block_lists()
{
	cprintf("\n=========================================\n");
	struct MemBlock* blk;
	struct MemBlock* lastBlk = NULL;
	cprintf("\nFreeMemBlocksList:\n");
	uint8 sorted = 1;
	LIST_FOREACH(blk, &FreeMemBlocksList)
	{
		if (lastBlk && blk->sva < lastBlk->sva + lastBlk->size)
			sorted = 0;
		cprintf("[%x, %x)-->", blk->sva, blk->sva + blk->size);
		lastBlk = blk;
	}
	if (!sorted)	cprintf("\nFreeMemBlocksList is NOT SORTED!!\n");

	lastBlk = NULL;
	cprintf("\nAllocMemBlocksList:\n");
	sorted = 1;
	LIST_FOREACH(blk, &AllocMemBlocksList)
	{
		if (lastBlk && blk->sva < lastBlk->sva + lastBlk->size)
			sorted = 0;
		cprintf("[%x, %x)-->", blk->sva, blk->sva + blk->size);
		lastBlk = blk;
	}
	if (!sorted)	cprintf("\nAllocMemBlocksList is NOT SORTED!!\n");
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

// For NF
uint32 lastAllocBlockSVA = 0;

void initialize_MemBlocksList(uint32 numOfBlocks)
{
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] initialize_MemBlocksList
	// Write your code here, remove the panic and write your code
	//panic("initialize_MemBlocksList() is not implemented yet...!!");


	cprintf("From init: %d \n", &MemBlockNodes);

	LIST_INIT(&AvailableMemBlocksList);
	cprintf("Test 1\n");
	LIST_INSERT_HEAD(&AvailableMemBlocksList, &MemBlockNodes[0]);
	cprintf("Test 2\n");
	int i;
	for (i = 1; i < numOfBlocks; i++) {
		MemBlockNodes[i].size = 0;
		MemBlockNodes[i].sva = 0;
		LIST_INSERT_TAIL(&AvailableMemBlocksList, &MemBlockNodes[i]);
		//cprintf("Test 3 %d\n", i);
	}


}

//===============================
// [2] FIND BLOCK:
//===============================
struct MemBlock* find_block(struct MemBlock_List* blockList, uint32 va)
{
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] find_block
	// Write your code here, remove the panic and write your code
	//panic("find_block() is not implemented yet...!!");
	cprintf("Enter Find\n");

	struct MemBlock* node = NULL;
	LIST_FOREACH(node, blockList)
	{
		cprintf("Block: %d \n", node->sva);
		if (node->sva == va)
		{
			struct MemBlock* tmp = node;
			return tmp;
		}
	}

	return NULL;
}

//=========================================
// [3] INSERT BLOCK IN ALLOC LIST [SORTED]:
//=========================================
void insert_sorted_allocList(struct MemBlock* blockToInsert)
{
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] insert_sorted_allocList
	// Write your code here, remove the panic and write your code
	//panic("insert_sorted_allocList() is not implemented yet...!!");


	int AllocListSize = LIST_SIZE(&AllocMemBlocksList);

	// Update lastAllocBlockSVA for NF
	lastAllocBlockSVA = blockToInsert->sva;

	if (AllocListSize == 0) {
		//insert empty

		LIST_INSERT_HEAD(&AllocMemBlocksList, blockToInsert);
	}

	else {

		struct MemBlock* LocatedElm;

		LIST_FOREACH(LocatedElm, &AllocMemBlocksList) {

			if (
				LocatedElm->prev_next_info.le_next != NULL &&
				LocatedElm->sva < blockToInsert->sva &&
				blockToInsert->sva < LocatedElm->prev_next_info.le_next->sva
				) {
				// insert between two blocks

				LIST_INSERT_AFTER(&AllocMemBlocksList, LocatedElm, blockToInsert);
			}
			else if (blockToInsert->sva > LIST_LAST(&AllocMemBlocksList)->sva) {

				// Insert After in tail

				LIST_INSERT_TAIL(&AllocMemBlocksList, blockToInsert);
			}
			else if (blockToInsert->sva < LIST_FIRST(&AllocMemBlocksList)->sva) {
				// Insert In Head

				LIST_INSERT_HEAD(&AllocMemBlocksList, blockToInsert);
			}

		}
	}
}

//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
struct MemBlock* alloc_block_FF(uint32 size)
{
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] alloc_block_FF
	// Write your code here, remove the panic and write your code
	//panic("alloc_block_FF() is not implemented yet...!!");

	struct MemBlock* ptrFreeLooper = LIST_FIRST(&FreeMemBlocksList);
	LIST_FOREACH(ptrFreeLooper, &FreeMemBlocksList)
	{
		//case no free available
		if (ptrFreeLooper->size < size)
		{
			if (ptrFreeLooper == FreeMemBlocksList.lh_last)
			{
				ptrFreeLooper = NULL;
				break;
			}
			else {
				continue;
			}
		}
		//case grater size
		if (ptrFreeLooper->size > size)
		{
			struct MemBlock* ptrToBeKept;
			//init block to be returned
			ptrToBeKept = LIST_LAST(&AvailableMemBlocksList);
			ptrToBeKept->size = size;
			ptrToBeKept->sva = ptrFreeLooper->sva;
			//remove returned block from avai
			LIST_REMOVE(&AvailableMemBlocksList, ptrToBeKept);
			//updating remaining free block size
			ptrFreeLooper->size = ptrFreeLooper->size - size;
			ptrFreeLooper->sva = ptrFreeLooper->sva + size;
			//update returned block size
			return ptrToBeKept;
		}
		//case if size is equal to given size
		if (ptrFreeLooper->size == size)
		{
			LIST_REMOVE(&FreeMemBlocksList, ptrFreeLooper);
			break;
		}
	}
	return ptrFreeLooper;
}

//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
struct MemBlock* alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] alloc_block_BF
	// Write your code here, remove the panic and write your code
	//panic("alloc_block_BF() is not implemented yet...!!");

	struct MemBlock* ptrFreeLooper;
	struct MemBlock ptr;
	struct MemBlock* var;
	int entrance = 0;
	int flag = 0;
	ptr.size = 5555555;
	var = &ptr;
	//foreach to get into free list
	LIST_FOREACH(ptrFreeLooper, &FreeMemBlocksList)
	{
		if (ptrFreeLooper->size > size && ptrFreeLooper->size < var->size) {
			var = ptrFreeLooper;
			entrance = 1;
			flag = 1;
		}

		if (ptrFreeLooper->size == size)
		{
			LIST_REMOVE(&FreeMemBlocksList, ptrFreeLooper);
			entrance = 1;
			return ptrFreeLooper;
		}

	}
	//if entrance
	if (entrance != 0 && flag == 1)
	{

		struct MemBlock* ptrToBeKept;
		//init block to be returned
		ptrToBeKept = LIST_LAST(&AvailableMemBlocksList);
		ptrToBeKept->size = size;
		ptrToBeKept->sva = var->sva;
		//remove returned block from ava
		LIST_REMOVE(&AvailableMemBlocksList, ptrToBeKept);
		//updating remaining free block size
		var->size = var->size - size;
		var->sva = var->sva + size;
		return ptrToBeKept;


	}
	else if (entrance == 0)
	{
		return NULL;
	}

	return ptrFreeLooper;
}


//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
struct MemBlock* alloc_block_NF(uint32 size)
{
	//TODO: [PROJECT MS1 - BONUS] [DYNAMIC ALLOCATOR] alloc_block_NF
	// Write your code here, remove the panic and write your code
	//panic("alloc_block_NF() is not implemented yet...!!");

	struct MemBlock* ptrFreeLooper;
		LIST_FOREACH(ptrFreeLooper, &FreeMemBlocksList)
		{
			if(ptrFreeLooper->sva > lastAllocBlockSVA) {

				//case grater size
				if (ptrFreeLooper->size > size)
				{
					struct MemBlock* ptrToBeKept;
					//init block to be returned
					ptrToBeKept = LIST_LAST(&AvailableMemBlocksList);
					ptrToBeKept->size = size;
					ptrToBeKept->sva = ptrFreeLooper->sva;
					//remove returned block from avai
					LIST_REMOVE(&AvailableMemBlocksList, ptrToBeKept);
					//updating remaining free block size
					ptrFreeLooper->size = ptrFreeLooper->size - size;
					ptrFreeLooper->sva = ptrFreeLooper->sva + size;
					//update returned block size
					lastAllocBlockSVA = ptrToBeKept->sva;
					return ptrToBeKept;
				}
				//case if size is equal to given size
				if (ptrFreeLooper->size == size)
				{
					lastAllocBlockSVA = ptrFreeLooper->sva;
					LIST_REMOVE(&FreeMemBlocksList, ptrFreeLooper);
					break;
				}



			}


		}

		// In case it did not found any free block it start from the beginning of the list

		if(ptrFreeLooper == NULL) {
			LIST_FOREACH(ptrFreeLooper, &FreeMemBlocksList) {
				//case grater size
				if (ptrFreeLooper->size > size)
				{

					struct MemBlock* ptrToBeKept;
					//init block to be returned
					ptrToBeKept = LIST_LAST(&AvailableMemBlocksList);
					ptrToBeKept->size = size;
					ptrToBeKept->sva = ptrFreeLooper->sva;
					//remove returned block from avai
					LIST_REMOVE(&AvailableMemBlocksList, ptrToBeKept);
					//updating remaining free block size
					ptrFreeLooper->size = ptrFreeLooper->size - size;
					ptrFreeLooper->sva = ptrFreeLooper->sva + size;
					//update returned block size
					lastAllocBlockSVA = ptrToBeKept->sva;
					return ptrToBeKept;
				}
				//case if size is equal to given size
				if (ptrFreeLooper->size == size)
				{
					lastAllocBlockSVA = ptrFreeLooper->sva;
					LIST_REMOVE(&FreeMemBlocksList, ptrFreeLooper);
					break;
				}

			}
		}
		return ptrFreeLooper;

}

//===================================================
// [8] INSERT BLOCK (SORTED WITH MERGE) IN FREE LIST:
//===================================================
void insert_sorted_with_merge_freeList(struct MemBlock* blockToInsert)
{
	//cprintf("BEFORE INSERT with MERGE: insert [%x, %x)\n=====================\n", blockToInsert->sva, blockToInsert->sva + blockToInsert->size);
	//print_mem_block_lists() ;

	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] insert_sorted_with_merge_freeList
	// Write your code here, remove the panic and write your code
	//panic("insert_sorted_with_merge_freeList() is not implemented yet...!!");

	int freeListSize = LIST_SIZE(&FreeMemBlocksList);

	// First Case: If the free list is empty
	if (freeListSize == 0) {
		LIST_INSERT_HEAD(&FreeMemBlocksList, blockToInsert);
	}
	// If Not Empty
	else {
		struct MemBlock* freeBlock;
		LIST_FOREACH(freeBlock, &FreeMemBlocksList) {
			// CASE: Merge with prev and Next
			if (
				freeBlock->prev_next_info.le_next != NULL &&
				freeBlock->sva + freeBlock->size == blockToInsert->sva &&
				blockToInsert->sva + blockToInsert->size == freeBlock->prev_next_info.le_next->sva
				) {
				// Adding Sizes
				uint32 firstBlockSize = freeBlock->size;
				uint32 totalSize = firstBlockSize + blockToInsert->size + LIST_NEXT(freeBlock)->size;
				freeBlock->size = totalSize;

				struct MemBlock* prtToBeRemoved = freeBlock->prev_next_info.le_next;
				// Zeroing
				blockToInsert->size = 0;
				blockToInsert->sva = 0;
				prtToBeRemoved->size = 0;
				prtToBeRemoved->sva = 0;

				// Remove the next to freeBlock
				LIST_REMOVE(&FreeMemBlocksList, prtToBeRemoved);
				//Insert Elements to AvailableMemBlockList
				LIST_INSERT_HEAD(&AvailableMemBlocksList, prtToBeRemoved);
				LIST_INSERT_HEAD(&AvailableMemBlocksList, blockToInsert);
				break;
			}
			// Merge with prev
			if (freeBlock->sva + freeBlock->size == blockToInsert->sva) {

				// Adding Sizes
				freeBlock->size += blockToInsert->size;

				// Zeroing
				blockToInsert->size = 0;
				blockToInsert->sva = 0;

				// Insert to Available
				LIST_INSERT_HEAD(&AvailableMemBlocksList, blockToInsert);
				break;
			}
			// Merge with Next
			else if (blockToInsert->sva + blockToInsert->size == freeBlock->sva) {

				// Adding sizes
				freeBlock->sva = blockToInsert->sva;
				freeBlock->size += blockToInsert->size;

				// Zeroing
				blockToInsert->size = 0;
				blockToInsert->sva = 0;

				// Insert to Available
				LIST_INSERT_HEAD(&AvailableMemBlocksList, blockToInsert);
				break;
			}

			// NO MERGE CASES
			else {

				// Insert at tail
				if (freeBlock->prev_next_info.le_next == NULL && blockToInsert->sva > LIST_FIRST(&FreeMemBlocksList)->sva) {
					LIST_INSERT_TAIL(&FreeMemBlocksList, blockToInsert);
					break;
				}
				// Insert at head
				else if (blockToInsert->sva < LIST_FIRST(&FreeMemBlocksList)->sva) {
					LIST_INSERT_HEAD(&FreeMemBlocksList, blockToInsert);
					break;
				}
				// Insert between 2 blocks without merge
				else if (
					freeBlock->prev_next_info.le_next != NULL &&
					blockToInsert->sva + blockToInsert->size > freeBlock->sva &&
					blockToInsert->sva + blockToInsert->size < freeBlock->prev_next_info.le_next->sva
					) {
					LIST_INSERT_AFTER(&FreeMemBlocksList, freeBlock, blockToInsert);
					break;
				}

			}

		}

	}

	//cprintf("\nAFTER INSERT with MERGE:\n=====================\n");
	//print_mem_block_lists();

}

