#include "tests.h"
#include "x86_desc.h"
#include "x86_page.h" //LYS
#include "lib.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* Paging Test - LYS
 * 
 * Asserts VM 0 goes to video memory page, VM 4MB~8MB-1
 * Inputs: None
 * Outputs: PASS/FAIL, printing to screen about memory info
 * Side Effects: None
 * Coverage: Init and Enable paging
 * Files: x86_page.h/S
 */
int page_test(){
	TEST_HEADER;

	int vm, prev=0, cur=0;
	int result = PASS;
	for (vm = 0; vm < 0x800001; vm++){  //for vm from 0 to 8MB+1
		if (PD[vm>>22].P == 0) continue;
		//big page
		if (PD[vm>>22].PS == 1) {
			printf("A big page at Virtual memory: %d, Map to Physical memory: %d\n", vm, (PD[vm>>22].PBase_Addr)<<22);
			vm += 0x400000-1; //4MB-1
			continue;
		}
		//small page
		PTE_t *PT_ = &PT[PD[vm>>22].PTBase_Addr];
		int PT_addr = (vm>>12)&0x003FF000;
		if (PT_[PT_addr].P == 1) {
			printf("A small page at Virtual memory: %d, Map to Physical memory: %d\n", vm, (PT_[PT_addr].PBase_Addr)<<12);
			vm += 0x1000-1; //4KB-1
			continue;
		}
	}

	return result;
}



// add more tests here

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
	TEST_OUTPUT("page_test", page_test());
	while (1);
}
