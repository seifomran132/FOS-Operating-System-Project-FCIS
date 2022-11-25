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

	LIST_INIT(&AvailableMemBlocksList);
	LIST_INSERT_HEAD(&AvailableMemBlocksList, &MemBlockNodes[0]);
	int i;
	for (i = 1; i < numOfBlocks; i++) {
		MemBlockNodes[i].size = 0;
		MemBlockNodes[i].sva = 0;
		LIST_INSERT_TAIL(&AvailableMemBlocksList, &MemBlockNodes[i]);
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

	struct MemBlock* iterator;
	struct MemBlock maximized;
	struct MemBlock* bblock;
	maximized.size = 999999999;
	bblock = &maximized;

	int entrance = 0;
	int k = 0;
	LIST_FOREACH(iterator, &FreeMemBlocksList)
	{

		if (iterator->size == size)
		{
			entrance = 1;
			LIST_REMOVE(&FreeMemBlocksList, iterator);
			return iterator;
		}
		else if (iterator->size > size && iterator->size < bblock->size) {
			entrance = 1;
			k = 1;
			bblock = iterator;
		}

	}
	if (entrance == 1 && k == 1)
	{

		struct MemBlock* fstBlock;
		fstBlock = LIST_LAST(&AvailableMemBlocksList);
		fstBlock->size = size;
		fstBlock->sva = bblock->sva;
		uint32 newSize = bblock->size - size;
		uint32 newSva = bblock->sva + size;
		bblock->size = newSize;
		bblock->sva = newSva;
		LIST_REMOVE(&AvailableMemBlocksList, fstBlock);
		return fstBlock;


	}
	else if (entrance != 1)
	{
		return NULL;
	}

	return iterator;
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
		if (ptrFreeLooper->sva > lastAllocBlockSVA) {

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

	if (ptrFreeLooper == NULL) {
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

	int count = LIST_SIZE(&FreeMemBlocksList);
	if (count == 0) {
		LIST_INSERT_HEAD(&FreeMemBlocksList, blockToInsert);
	}
	else {
		struct MemBlock* iterator;
		LIST_FOREACH(iterator, &FreeMemBlocksList) {

			struct MemBlock* nextBlock = iterator->prev_next_info.le_next;
			uint32 blockToInsertLimit = blockToInsert->sva + blockToInsert->size;
			uint32 iteratorLimit = iterator->sva + iterator->size;

			if (nextBlock != NULL && iteratorLimit == blockToInsert->sva && blockToInsertLimit == nextBlock->sva) {
				uint32 fstSize = iterator->size;
				uint32 scdSize = blockToInsert->size;
				uint32 trdSize = LIST_NEXT(iterator)->size;

				uint32 totalSize = fstSize + scdSize + trdSize;
				iterator->size = totalSize;

				blockToInsert->size = 0;
				nextBlock->size = 0;
				blockToInsert->sva = 0;
				nextBlock->sva = 0;


				LIST_REMOVE(&FreeMemBlocksList, nextBlock);
				LIST_INSERT_HEAD(&AvailableMemBlocksList, nextBlock);
				LIST_INSERT_HEAD(&AvailableMemBlocksList, blockToInsert);

				break;
			}
			if (iteratorLimit == blockToInsert->sva) {
				iterator->size = iterator->size + blockToInsert->size;
				blockToInsert->size = 0;
				blockToInsert->sva = 0;
				LIST_INSERT_HEAD(&AvailableMemBlocksList, blockToInsert);
				break;
			}
			else if (blockToInsertLimit == iterator->sva) {

				iterator->sva = blockToInsert->sva;
				iterator->size = iterator->size + blockToInsert->size;
				blockToInsert->size = 0;
				blockToInsert->sva = 0;
				LIST_INSERT_HEAD(&AvailableMemBlocksList, blockToInsert);
				break;
			}

			else {
				struct MemBlock* listhead = LIST_FIRST(&FreeMemBlocksList);
				if (nextBlock == NULL && blockToInsert->sva > listhead->sva) {
					LIST_INSERT_TAIL(&FreeMemBlocksList, blockToInsert);
					break;
				}
				else if (blockToInsert->sva < listhead->sva) {
					LIST_INSERT_HEAD(&FreeMemBlocksList, blockToInsert);
					break;
				}
				else if (
					nextBlock != NULL &&
					blockToInsertLimit > iterator->sva &&
					blockToInsertLimit < nextBlock->sva
					) {
					LIST_INSERT_AFTER(&FreeMemBlocksList, iterator, blockToInsert);
					break;
				}

			}

		}

	}
}

