#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

//==================================================================//
//==================================================================//
//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)//
//==================================================================//
//==================================================================//


uint32 framesArr[] = {};
uint32 MBNEND = 0; // To locate MemBlockNodes
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

//	uint32 va_it = KERNEL_HEAP_START;
//	while (va_it < KERNEL_HEAP_START + totalSizeRequired) {
//		uint32 *ptPtr = NULL;
//		get_page_table(ptr_page_directory, va_it, &ptPtr);
//		if(ptPtr != NULL) {
//			uint32 tableEntry = ptPtr[PTX(va_it)];
//			uint32 frameNum = tableEntry >> 12;
//			framesArr[frameNum] = 0;
//			cprintf("FN: %d\n", frameNum);
//
//		}
//		va_it += PAGE_SIZE;
//	}

	//[3] Initialize AvailableMemBlocksList by filling it with the MemBlockNodes
	initialize_MemBlocksList(MAX_MEM_BLOCK_CNT);
	//[4] Insert a new MemBlock with the remaining heap size into the FreeMemBlocksList
	struct MemBlock* allocatedForFree = LIST_FIRST(&AvailableMemBlocksList);
	LIST_REMOVE(&AvailableMemBlocksList, allocatedForFree);
	//The Block which will be inserted in Free list
	uint32 restedSize = (KERNEL_HEAP_MAX - KERNEL_HEAP_START) - totalSizeRequired - sizeof(struct MemBlock);
	allocatedForFree->size = restedSize;
	allocatedForFree->sva = ROUNDUP(KERNEL_HEAP_START + totalSizeRequired, PAGE_SIZE);
	MBNEND = KERNEL_HEAP_START + totalSizeRequired;
	insert_sorted_with_merge_freeList(allocatedForFree);
#endif
}



void* kmalloc(unsigned int size)
{
	uint32 roundedSize = ROUNDUP(size, PAGE_SIZE);
	struct MemBlock * allocatedBlock;
	if(isKHeapPlacementStrategyFIRSTFIT()) {
		allocatedBlock = alloc_block_FF(roundedSize);
		if(allocatedBlock == NULL) {
			return NULL;
		}
		else {
			allocate_chunk(ptr_page_directory, allocatedBlock->sva, size, PERM_WRITEABLE);

		}
	}
	else if(isKHeapPlacementStrategyBESTFIT()) {
		allocatedBlock = alloc_block_BF(roundedSize);
		if(allocatedBlock == NULL) {
			return NULL;
		}
		else {
			allocate_chunk(ptr_page_directory, allocatedBlock->sva, size, PERM_WRITEABLE);
		}
	}
	else if(isKHeapPlacementStrategyNEXTFIT()) {
		allocatedBlock = alloc_block_NF(roundedSize);
		if(allocatedBlock == NULL) {
			return NULL;
		}
		else {
			allocate_chunk(ptr_page_directory, allocatedBlock->sva, size, PERM_WRITEABLE);
		}
	}
	uint32 va_it = allocatedBlock->sva;
	while (va_it < allocatedBlock->sva + size) {
		uint32 *ptPtr = NULL;
		get_page_table(ptr_page_directory, va_it, &ptPtr);
		if(ptPtr != NULL) {
			uint32 tableEntry = ptPtr[PTX(va_it)];
			uint32 frameNum = tableEntry >> 12;
			framesArr[frameNum] = va_it;
			cprintf("FN: %x\n", va_it);

		}
		va_it += PAGE_SIZE;
	}
	insert_sorted_allocList(allocatedBlock);

	return (void *)allocatedBlock->sva;
}

void kfree(void* virtual_address)
{
	cprintf("Enter KFREE %d\n", virtual_address);
	//TODO: [PROJECT MS2] [KERNEL HEAP] kfree
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");

	uint32 blockAddress = (uint32)virtual_address;
	struct MemBlock * wantedBlock = find_block(&AllocMemBlocksList, blockAddress);
	if(wantedBlock != NULL) {
		uint32 blockEnd = wantedBlock->sva + wantedBlock->size;
		uint32 startingAddress = wantedBlock->sva;

		cprintf("Block Start: %d Block Size: %d Block End: %d \n", wantedBlock->sva, wantedBlock->size, blockEnd);
		while (startingAddress < blockEnd) {

			// Deleting from Frames Array
			uint32 *ptPtr = NULL;
			get_page_table(ptr_page_directory, startingAddress, &ptPtr);
			if(ptPtr != NULL) {
				uint32 tableEntry = ptPtr[PTX(startingAddress)];
				uint32 frameNum = tableEntry >> 12;
				framesArr[frameNum] = 0;
				//cprintf("Delete FN: %d\n", frameNum);

			}
			uint32 *framePT = NULL;
			struct FrameInfo* frameToFree = get_frame_info(ptr_page_directory, startingAddress, &framePT);
			unmap_frame(ptr_page_directory, startingAddress);

			startingAddress += PAGE_SIZE;
		}


		LIST_REMOVE(&AllocMemBlocksList, wantedBlock);
		insert_sorted_with_merge_freeList(wantedBlock);
	}
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kheap_virtual_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");

	//cprintf("PA: %d\n", physical_address);

	uint32 MemBNPA = kheap_physical_address((uint32)MemBlockNodes);

	if(physical_address < kheap_physical_address(MBNEND) && physical_address > MemBNPA) {
		return 0;
	}

	//cprintf("MEM: %d KHS: %d PA: %d\n", MemBNPA, kheap_physical_address(MBNEND), physical_address);
	uint32 frameNumber = physical_address >> 12;
	//cprintf("FN: %d\n", framesArr[frameNumber]);
	return framesArr[frameNumber];
	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details
	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kheap_physical_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details

	return virtual_to_physical(ptr_page_directory, virtual_address);

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
