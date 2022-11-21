#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

//==================================================================//
//==================================================================//
//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)//
//==================================================================//
//==================================================================//

void initialize_dyn_block_system()
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] initialize_dyn_block_system
	// your code is here, remove the panic and write your code
	//kpanic_into_prompt("initialize_dyn_block_system() is not implemented yet...!!");

	//[1] Initialize two lists (AllocMemBlocksList & FreeMemBlocksList) [Hint: use LIST_INIT()]

	LIST_INIT(&AllocMemBlocksList);
	LIST_INIT(&FreeMemBlocksList);

#if STATIC_MEMBLOCK_ALLOC
	//DO NOTHING
#else
	/*[2] Dynamically allocate the array of MemBlockNodes
	 * 	remember to:
	 * 		1. set MAX_MEM_BLOCK_CNT with the chosen size of the array
	 * 		2. allocation should be aligned on PAGE boundary
	 * 	HINT: can use alloc_chunk(...) function
	 */
	uint32 heapSize = (KERNEL_HEAP_MAX - KERNEL_HEAP_START);
	uint32 numOfElements = (heapSize / PAGE_SIZE);

	MAX_MEM_BLOCK_CNT = numOfElements;

	MemBlockNodes = (struct MemBlock*)KERNEL_HEAP_START;

	uint32 totalSizeRequired = numOfElements * sizeof(struct MemBlock);
	allocate_chunk(ptr_page_directory, KERNEL_HEAP_START, totalSizeRequired, (PERM_PRESENT | PERM_WRITEABLE));


#endif
	//[3] Initialize AvailableMemBlocksList by filling it with the MemBlockNodes
	cprintf("numOfElements %x \n", numOfElements);
	cprintf("KERNEL_HEAP_MAX %x \n", KERNEL_HEAP_MAX);
	cprintf("KERNEL_HEAP_START %x \n", KERNEL_HEAP_START);
	cprintf("PAGE_SIZE %x \n", PAGE_SIZE);
	cprintf("MAX_MEM_BLOCK_CNT: %d \n", MAX_MEM_BLOCK_CNT);


	initialize_MemBlocksList(MAX_MEM_BLOCK_CNT);
//	uint32 i;
//	for (i = 0; i < MAX_MEM_BLOCK_CNT; i++) {
//		MemBlockNodes[i].size = 0;
//		MemBlockNodes[i].sva = 0;
//		LIST_INSERT_TAIL(&AvailableMemBlocksList, &MemBlockNodes[i]);
//		cprintf("av %d \n", LIST_SIZE(&(AvailableMemBlocksList)));
//	}
//
//	cprintf("i %x \n", i);
	//initialize_MemBlocksList(MAX_MEM_BLOCK_CNT);
	//cprintf("av %d /n",LIST_SIZE(&(AvailableMemBlocksList)));


	//[4] Insert a new MemBlock with the remaining heap size into the FreeMemBlocksList
	struct MemBlock *allocatedForFree = LIST_FIRST(&AvailableMemBlocksList);
	LIST_REMOVE(&AvailableMemBlocksList, allocatedForFree);

	uint32 restedSize = (KERNEL_HEAP_MAX - KERNEL_HEAP_START) - totalSizeRequired - sizeof(struct MemBlock);
	cprintf("rested size: %d \n", restedSize);

	uint32 freeStartSVA = KERNEL_HEAP_START + totalSizeRequired - sizeof(struct MemBlock);

	allocatedForFree->size = restedSize;
	allocatedForFree->sva = ROUNDUP(KERNEL_HEAP_START + totalSizeRequired, PAGE_SIZE);

	insert_sorted_with_merge_freeList(allocatedForFree);


	cprintf("MAX BLOCKS: %d  LIST SIZE: %d \n", MAX_MEM_BLOCK_CNT, LIST_SIZE(&AvailableMemBlocksList));
	cprintf("Test after insert \n");
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

	struct MemBlock* ptr = NULL;
	void*ptrAllocation;
	if(isKHeapPlacementStrategyFIRSTFIT())
	{
		//ptr=alloc_block_FF(size);
		//allocation
	}
	if(isKHeapPlacementStrategyBESTFIT())
	{
		//ptr=alloc_block_BF(size);
		//allocation
	}

	return ptr;
}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kfree
	// Write your code here, remove the panic and write your code
	panic("kfree() is not implemented yet...!!");



}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kheap_virtual_address
	// Write your code here, remove the panic and write your code
	panic("kheap_virtual_address() is not implemented yet...!!");

	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details
	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kheap_physical_address
	// Write your code here, remove the panic and write your code
	panic("kheap_physical_address() is not implemented yet...!!");

	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details
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

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT MS2 - BONUS] [KERNEL HEAP] krealloc
	// Write your code here, remove the panic and write your code
	panic("krealloc() is not implemented yet...!!");
}
