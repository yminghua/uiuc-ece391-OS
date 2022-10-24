#include "tests.h"
#include "x86_desc.h"
#include "x86_page.h" //LYS
#include "types.h" //LYS
#include "fileSystem.h"
#include "lib.h"
#include "e391device.h"//drush8: can be cancelled when we doesn't use cp1: pageF test
#include "fileSystem.h"

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
 * Asserts VM 0xB8000 goes to video memory page, VM 4MB~8MB-1 goes to a big page 4MB~8MB-1
 * Inputs: None
 * Outputs: PASS/FAIL, printing to screen about memory info
 * Side Effects: None
 * Coverage: Init and Enable paging
 * Files: x86_page.h/S
 */
int page_test(){
	TEST_HEADER;

	int vm; //prev=0, cur=0
	int result = PASS;
	for (vm = 0; vm < 0x800001; vm++){  //for vm from 0 to 8MB+1
		if (PD[vm>>22].P == 0) continue;
		//big page
		if (PD[vm>>22].PS == 1) {
			printf("A big page at Virtual memory: %x, Map to Physical memory: %x\n", vm, (PD[vm>>22].PBase_Addr)<<22);
			vm += 0x400000-1; //4MB-1
			continue;
		}
		//small page
		PTE_t *PT_ = (PTE_t *)(((uint32_t)(PD[vm>>22].PTBase_Addr))<<12);
		int PT_addr = (vm&0x003FF000)>>12;
		if (PT_[PT_addr].P == 1) {
			printf("A small page at Virtual memory: %x, Map to Physical memory: %x\n", vm, (PT_[PT_addr].PBase_Addr)<<12);
			vm += 0x1000-1; //4KB-1
			continue;
		}
	}
	return result;
}

/* syscall Test - Drush8
 * 
 * Asserts that syscall can be called and return properly.
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: syscall and idt as well as assambly linkage
 * Files: idt.c/h, ece391device.c/h(only in cp1)
 */
int syscall_test(){
	TEST_HEADER;
	asm volatile(
			"int $0x80;"
			:
			:
			: "memory","cc"
	);  //call the syscall.

	return PASS; //pass and teturn back, yehh
}

/* K&R Test - Drush8
 * 
 * Asserts that keyboard and RTC interrupt works well.
 * Inputs: None
 * Outputs: PASS
 * Side Effects: None
 * Coverage: interrupt handler
 * Files: idt.c/h, ece391device.c/h(only in cp1)
 */
int KAndR_test(){
	TEST_HEADER;
	printf("now type what you want, when you type 9, will show you the RTC blinking!!!\n");
	while(if9pressed!=-2);
	clear();
	return PASS; //pass and teturn back, yehh
}

/* pageFexception Test - Drush8
 * 
 * Asserts that if we see the 	NULL pointer, we need to turn to pageF exception handler and Blue Screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: exception handler, page
 * Files: idt.c/h, e391exception.c/h
 */

int pageFexception_test(){
	TEST_HEADER;
	int *p = NULL;
	printf("\n\n final eception test, if the blue screen exception is Page Fault,\n it will stands as pass.\n\n\n");
	while(if9pressed!=-1);
	*p = 999;
	return PASS; //this should not be reached.
}

// add more tests here

/* Checkpoint 2 tests */
void list_all_files_test() {
	list_all_files();
}

void print_fish_test() {
	read_file_i(10);
}

void print_exe_test() {
	read_file_i(16);
}

void clear_screen() {
	clear();
}

/*
 * test file_open and file_read
 */
int file_OpenRead_test() {

	TEST_HEADER;

	int fd = 3;
	fdInfo_t fd_info;
    fd_info.inode_index = 0;
    fd_info.file_position = 0;
    fd_info.flags = 0;
	Temp_fd_array[fd] = fd_info;

	clear();
	uint8_t fname[32] = "frame1.txt";

	int check = file_open(fname, fd);
	if (check == 0)
		printf("file_open succeed! \n");
	else
		printf("file_open FAIL! \n");
	
	int8_t buf[6000];													//allocate enough(6000) space to store the date
	int32_t i, count;
	count = file_read(fd, buf, 6000);					//6000 is the size of the buf
	printf("Successfully read %d Bytes!\n", count);
	for (i = 0; i < count; i++) {
			if (buf[i] != '\0') {
				putc(buf[i]);
			}
	}
	file_close(3);

	if (count > 0)
		return PASS;

	return FAIL;

}

/*
 * test dir_open and dir_read
 */
int dir_OpenRead_test() {

	TEST_HEADER;

	int fd = 4;
	fdInfo_t fd_info;
    fd_info.inode_index = 0;
    fd_info.file_position = 0;
    fd_info.flags = 0;
	Temp_fd_array[fd] = fd_info;

	clear();
	uint8_t fname[32] = ".";								 //allocate a buf(33) to store its name
	int8_t buf[33];													//allocate enough(33) space to store the data
	dir_open(fname);
	dir_read(fd, buf, 0);
	
	dir_close(fd);
	return PASS;
}



/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//// TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
	// TEST_OUTPUT("page_test", page_test());
	// TEST_OUTPUT("syscall_test", syscall_test());
	// TEST_OUTPUT("keyboardandRTC_test", KAndR_test());

	//warning: this final test will lead to the blue screen of the kernel. drush8
	// TEST_OUTPUT("PageFault_test", pageFexception_test());

	TEST_OUTPUT("file_open&read test", file_OpenRead_test());

}
