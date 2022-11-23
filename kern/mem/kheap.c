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
	 //Set MAX_MEM_BLOCK_CNT
	uint32 heapSize = (KERNEL_HEAP_MAX - KERNEL_HEAP_START);
	uint32 numOfElements = (heapSize / PAGE_SIZE);
	MAX_MEM_BLOCK_CNT = numOfElements;

	//Allocation of MemBlockNodes
	uint32 tmp = KERNEL_HEAP_START;
	struct MemBlock* initial_block_node = (struct MemBlock*)tmp;
	MemBlockNodes = initial_block_node;

	uint32 totalSizeRequired = numOfElements * sizeof(struct MemBlock);
	allocate_chunk(ptr_page_directory, KERNEL_HEAP_START, totalSizeRequired, (PERM_PRESENT | PERM_WRITEABLE));


#endif
	//[3] Initialize AvailableMemBlocksList by filling it with the MemBlockNodes
	initialize_MemBlocksList(MAX_MEM_BLOCK_CNT);
	cprintf("MAX 1: %d \n", MAX_MEM_BLOCK_CNT);
	//[4] Insert a new MemBlock with the remaining heap size into the FreeMemBlocksList
	struct MemBlock* allocatedForFree = LIST_FIRST(&AvailableMemBlocksList);
	LIST_REMOVE(&AvailableMemBlocksList, allocatedForFree);
	cprintf("MAX 2: %d \n", MAX_MEM_BLOCK_CNT);
	//The Block which will be inserted in Free list
	uint32 restedSize = (KERNEL_HEAP_MAX - KERNEL_HEAP_START) - totalSizeRequired - sizeof(struct MemBlock);
	allocatedForFree->size = restedSize;
	allocatedForFree->sva = ROUNDUP(KERNEL_HEAP_START + totalSizeRequired, PAGE_SIZE);
	insert_sorted_with_merge_freeList(allocatedForFree);
	cprintf("MAX 3: %d \n", MAX_MEM_BLOCK_CNT);
}
void* kmalloc(unsigned int size)
{
	struct MemBlock* ptr;
	void *ptrAllocation=NULL;
	unsigned int Roundedsize=ROUNDUP(size,PAGE_SIZE);

	if(isKHeapPlacementStrategyFIRSTFIT())
	{
		//passing to FF
		ptr=alloc_block_FF(Roundedsize);
		//check returned Memblock data
		if(ptr == NULL){return NULL;}
		//allocation
		allocate_chunk(ptr_page_directory,ptr->sva,ptr->size,3);
		//Insert in Allocated
		insert_sorted_allocList(ptr);
		//assigning by casting
		ptrAllocation=&(*(uint32*)ptr->sva);
	}

	if(isKHeapPlacementStrategyBESTFIT())
	{
		//passing to BF
		ptr=alloc_block_BF(Roundedsize);
		//check Memblock data
		if(ptr == NULL){return NULL;}
		//allocation
		allocate_chunk(ptr_page_directory,ptr->sva,ptr->size,3);
		//assigning by casting
		ptrAllocation=&(*(uint32*)ptr->sva);
	}

	return ptrAllocation;
}

void kfree(void* virtual_address)
{
	cprintf("Enter KFREE %d\n", virtual_address);
	//TODO: [PROJECT MS2] [KERNEL HEAP] kfree
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");

	uint32 blockAddress = ROUNDDOWN((uint32)virtual_address, PAGE_SIZE);
	struct MemBlock * wantedBlock = find_block(&AllocMemBlocksList, blockAddress);

	if(wantedBlock != NULL) {
		uint32 blockEnd = wantedBlock->sva + wantedBlock->size;
		uint32 startingAddress = ROUNDDOWN(wantedBlock->sva, PAGE_SIZE);

		cprintf("Block Start: %d Block Size: %d Block End: %d \n", wantedBlock->sva, wantedBlock->size, blockEnd);
		while (startingAddress < blockEnd) {

			uint32 *framePT = NULL;
			struct FrameInfo* frameToFree = get_frame_info(ptr_page_directory, startingAddress, &framePT);
			free_frame(frameToFree);
			unmap_frame(ptr_page_directory, startingAddress);
			cprintf("Address: %d freed\n", startingAddress);
			startingAddress += PAGE_SIZE;
		}
		cprintf("Trace 1\n");
		LIST_REMOVE(&AllocMemBlocksList, wantedBlock);
		cprintf("Trace 2\n");
		insert_sorted_with_merge_freeList(wantedBlock);
		cprintf("Trace 3\n");
	}
	else {
		cprintf("NULL\n");
	}
	cprintf("Kfree End\n");
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
	//panic("krealloc() is not implemented yet...!!");


	struct MemBlock* NewVirAdd ;
	struct MemBlock* NxtPa   ;
	struct FrameInfo* CheckEmptness;


	 NewVirAdd=virtual_address;
	 NxtPa = virtual_address + PAGE_SIZE;
	 uint32 *TabPa =NULL;
	 uint32 *dir=NULL;
	//NxtPa =NxtPa->prev_next_info.le_next;
	 CheckEmptness= get_frame_info(dir,NxtPa->sva,&TabPa);

			if(CheckEmptness==NULL){
				if (NewVirAdd->size+NxtPa->size>=new_size){
					NewVirAdd->size=new_size;
				}
			}
			if (virtual_address==NULL){
				NewVirAdd=kmalloc(new_size);
			}
			else if (new_size==0){
				kfree(virtual_address);
				NewVirAdd=NULL;
			}
			else {
				NewVirAdd=alloc_block_BF(new_size);
			}

	return NewVirAdd ;

}
