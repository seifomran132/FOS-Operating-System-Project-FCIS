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
#include "../mem/kheap.h"

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

void page_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//TODO: [PROJECT MS3] [FAULT HANDLER] page_fault_handler
	// Write your code here, remove the panic and write your code
	//panic("page_fault_handler() is not implemented yet...!!");

	//refer to the project presentation and documentation for details

	cprintf("Enter @%x, %x\n", fault_va, ROUNDDOWN(fault_va, PAGE_SIZE));
	uint32 currSize = env_page_ws_get_size(curenv);
	uint32 maxSize = curenv->page_WS_max_size;

	if(currSize < maxSize) {
		cprintf("curr %d \t max %d\n", currSize, maxSize);
		// Placement
		uint32 *pageTablePtr = NULL;
		struct FrameInfo *allocatedFrame = get_frame_info(curenv->env_page_directory, fault_va, &pageTablePtr);;
		allocate_frame(&allocatedFrame);
		map_frame(curenv->env_page_directory, allocatedFrame, fault_va, PERM_WRITEABLE | PERM_USER);

		int pfr = pf_read_env_page(curenv, ((void *) fault_va));
		if(pfr == E_PAGE_NOT_EXIST_IN_PF){

			if((fault_va > USER_HEAP_START && fault_va < USER_HEAP_MAX) || (fault_va < USTACKTOP && fault_va > USTACKBOTTOM)) {
				// DO NOTHING

			}
			else {
				panic("ILLEGAL MEMORY ACCESS");
			}

		}
		else {


			cprintf("Exists\n");
		}
		//env_page_ws_set_entry(curenv, curenv->page_last_WS_index, fault_va);

		//env_page_ws_print(curenv);
		//env_page_ws_set_entry(curenv, curenv->page_last_WS_index, fault_va);
		//curenv->page_last_WS_index++;

		int found = 0;
		for(int i = 0; i < curenv->page_WS_max_size; i++) {
			int wspva = env_page_ws_get_virtual_address(curenv, i);
			if(wspva == ROUNDDOWN(fault_va, PAGE_SIZE)) {
				env_page_ws_set_entry(curenv, i, fault_va);
				curenv->page_last_WS_index++;
				found = 1;
				break;
			}
		}
		if(found == 0) {
			//env_page_ws_print(curenv);
			//cprintf("entry: %d, max: %d\n", curenv->page_last_WS_index, curenv->page_WS_max_size);
			env_page_ws_set_entry(curenv, curenv->page_last_WS_index, fault_va);
			curenv->page_last_WS_index = (curenv->page_last_WS_index + 1) % curenv->page_WS_max_size;
		}
		//uint32 *ptrToPageTable = NULL;
//		allocatedFrame = get_frame_info(curenv->env_page_directory, fault_va, &ptrToPageTable);
		//pf_update_env_page(curenv, ROUNDDOWN(fault_va, PAGE_SIZE), allocatedFrame);

	}
	else {
		// Replacement
		uint32 victimVA = 0;

		while(1) {

			int f00 = 0;
			for(int i = 0; i < curenv->page_WS_max_size; i++) {
				uint32 pageAddress = env_page_ws_get_virtual_address(curenv, i);
				int isModified = (pt_get_page_permissions(curenv->env_page_directory, pageAddress) & PERM_MODIFIED) ? 1 : 0;
				int isUsed = (pt_get_page_permissions(curenv->env_page_directory, pageAddress) & PERM_USED) ? 1 : 0;

				if(isUsed == 0 && isModified == 0) {
					victimVA = pageAddress;
					f00 = 1;
					break;
				}
			}
			if (f00 == 1) {
				break;
			}
			else {
				int f01 = 0;
				for(int i = 0; i < curenv->page_WS_max_size; i++) {
					uint32 pageAddress = env_page_ws_get_virtual_address(curenv, i);
					int isUsed = (pt_get_page_permissions(curenv->env_page_directory, pageAddress) & PERM_USED) ? 1 : 0;

					if(isUsed == 0) {
						victimVA = pageAddress;
						f01 = 1;
						break;
					}
					else {
						pt_set_page_permissions(curenv->env_page_directory, pageAddress, 0, PERM_USED);
					}
				}
				if (f01 == 1) {
					break;
				}
			}


		}



		for(int i = 0; i < curenv->page_WS_max_size; i++) {
			int wspva = env_page_ws_get_virtual_address(curenv, i);

			if (wspva == victimVA) {
				int isVictimModified = (pt_get_page_permissions(curenv->env_page_directory, victimVA) & PERM_MODIFIED) ? 1 : 0;

				if(isVictimModified == 1) {
					if((victimVA > USER_HEAP_START && victimVA < USER_HEAP_MAX) || (victimVA < USTACKTOP && victimVA > USTACKBOTTOM)) {
						uint32 *pt = NULL;
						struct FrameInfo *updatedPage = get_frame_info(curenv->env_page_directory, victimVA, &pt);
						pf_update_env_page(curenv, victimVA, updatedPage);
					}

				}

				env_page_ws_invalidate(curenv, victimVA);
				uint32 *pageTablePtr = NULL;
				struct FrameInfo *allocFrame = get_frame_info(curenv->env_page_directory, victimVA, &pageTablePtr);
				int mapres = map_frame(curenv->env_page_directory, allocFrame, fault_va, PERM_WRITEABLE | PERM_USER);
				env_page_ws_set_entry(curenv, i, fault_va);
				curenv->page_last_WS_index = (i + 1) % curenv->page_WS_max_size;
			}
		}

	}
	cprintf("Out\n");

}
void __page_fault_handler_with_buffering(struct Env * curenv, uint32 roundedFaultedVA)
{
	// Write your code here, remove the panic and write your code
	panic("__page_fault_handler_with_buffering() is not implemented yet...!!");


}
