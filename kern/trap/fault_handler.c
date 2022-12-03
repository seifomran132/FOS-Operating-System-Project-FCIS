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

void page_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//TODO: [PROJECT MS3] [FAULT HANDLER] page_fault_handler
	// Write your code here, remove the panic and write your code
	//panic("page_fault_handler() is not implemented yet...!!");

	//refer to the project presentation and documentation for details

	//env_page_ws_print(curenv);

	cprintf("Enter %x\n", fault_va);

	uint32 wsSize = env_page_ws_get_size(curenv);
	cprintf("WS Size %d\n", wsSize);

	if(wsSize < curenv->page_WS_max_size) {
		// Placement
		cprintf("Trace 1 %d\n", wsSize);
		// Rounding
		uint32 roundedFaultedVA = ROUNDDOWN(fault_va, PAGE_SIZE);
		// Allocate
		uint32 *ptrPT = NULL;
		struct FrameInfo *faultedFrame = NULL;
		int allocres = allocate_frame(&faultedFrame);
		cprintf("Trace 2 %x\n", roundedFaultedVA);

		// Get
		int epExist = pf_read_env_page(curenv, ((void *)fault_va));
		cprintf("Pages Exist %d\n",epExist);
		if(epExist == E_PAGE_NOT_EXIST_IN_PF) {

			cprintf("Alloc status: %d, frame: %d\n", allocres, faultedFrame != NULL);

			if((fault_va > USER_HEAP_START && fault_va < USER_HEAP_MAX) || (fault_va < USTACKTOP && fault_va > USTACKBOTTOM)) {
				cprintf("STACK PAGE @%x\n",fault_va);
			}
			else {
				cprintf("NOT STACK\n");
				panic("ILLEGAL MEMORY ACCESS %d", fault_va);
			}
			//faultedFrame = get_frame_info(curenv->env_page_directory, roundedFaultedVA, &ptrPT);
			//uint32 updateres = pf_update_env_page(curenv, roundedFaultedVA, faultedFrame);
			//cprintf("Update %d\n", updateres);
		}
		else {
			cprintf("Trace 3 %d\n", wsSize);
		}


		cprintf("Trace 1, %d, %x\n", curenv->page_last_WS_index, fault_va);
//		env_page_ws_print(curenv);
		//env_page_ws_set_entry(curenv, curenv->page_last_WS_index, roundedFaultedVA);

//		int epExist2 = pf_read_env_page(curenv, (void *)roundedFaultedVA);
//		cprintf("Try Find %d\n",epExist2);

		cprintf("Trace 2\n");
		//env_page_ws_print(curenv);
	}
	else {
		cprintf("Rep\n");
		// Replacement
	}
}
void __page_fault_handler_with_buffering(struct Env * curenv, uint32 roundedFaultedVA)
{
	// Write your code here, remove the panic and write your code
	panic("__page_fault_handler_with_buffering() is not implemented yet...!!");


}
