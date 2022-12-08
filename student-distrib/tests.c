#include "tests.h"
#include "x86_desc.h"
#include "x86_page.h" //LYS
#include "types.h" //LYS
#include "fileSystem.h"
#include "Syscalls.h"
#include "PCB.h" //LYS: for KB, MB, GB
#include "lib.h"
//#include "e391device.h"//drush8: can be cancelled when we doesn't use cp1: pageF test
#include "e391terminal.h"
#include "rtc.h" //yst
#include "malloc.h"

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
	for (vm = 0; vm < 1*GB; vm++){  //for vm from 0 to 1GB
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

void malloc_print(){

	#define vmm 0xFF800000

	if (PD[vmm>>22].PS == 1)
		printf("A big page used for dynamic memory allocation at Virtual memory: %x, Map to Physical memory: %x\n", vmm, (PD[vmm>>22].PBase_Addr)<<22);

	long long i;
	for (i = 0; i < BLOCK_NUM; i++){
		if (memory_block_tracker[i] != 0)
		{
			printf("The following %d memory blocks are allocated!\n", memory_block_tracker[i]);
			printf("Starting form Virtual memory: %x\n", vmm+i*KB);
			i += memory_block_tracker[i]-1;
		}
	}
}

int malloc_test() {
	TEST_HEADER;
	int result = PASS;
	rtc_reset_freq(2);
	int count = 0;
	int r = 1;

	printf("---Virtual memory 4088~4092 MB are for Dynamic Memory Allocation---\n");
	printf("---we divide the above 4MB memory into 4096 blocks, each block is 1KB---\n");
	printf("------------------------------------------------------------------------\n");

	while (count<8) {
	rtc_read(0, &r, 4);
		count++;
	}
	count = 0;

	printf("malloc int a[1200], b[1200], c[1200]\n");
	int* a, *b, *c;
	int size = 1200;
	a = (int*)malloc(size*4);
	b = (int*)malloc(size*4);
	c = (int*)malloc(size*4);

	malloc_print();
	printf("------------------------------------------------------------------------\n");

	while (count<16) {
	rtc_read(0, &r, 4);
		count++;
	}
	count = 0;

	printf("free(b)\n");
	free(b);

	malloc_print();
	printf("------------------------------------------------------------------------\n");

	while (count<16) {
	rtc_read(0, &r, 4);
		count++;
	}
	count = 0;

	printf("malloc int d[2400]\n");
	int* d;
	size = 2400;
	d = (int*)malloc(size*4);

	malloc_print();
	printf("------------------------------------------------------------------------\n");

	while (count<16) {
	rtc_read(0, &r, 4);
		count++;
	}
	count = 0;

	printf("malloc int e[600]\n");
	int* e;
	size = 600;
	e = (int*)malloc(size*4);

	malloc_print();

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
/* div0 Test - Drush8
 * 
 * Asserts that if we see xx/0, will the Blue Screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: exception handler, page
 * Files: idt.c/h, e391exception.c/h
 */
int div0_test(){
	TEST_HEADER;
	int p = 9;
	p+=1;
	printf("\n\n final eception test, if the blue screen exception is DIV0 Fail,\n it will stands as pass.\n\n\n");
	while(if9pressed!=-1);
	// p/=0;                                              //make sure there is no warning in compiling, if want test div0, please set is open
	return PASS; //this should not be reached.
}

// add more tests here

/* Checkpoint 2 tests */


/* kbAndterminal Test - Drush8
 * 
 * first kb buffer, then terminal read&write test
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: terminal & kb driver
 * Files: e391keyboard.c/h, e391terminal.c/h
 */
int kbAndterminal_test(){

	TEST_HEADER;
	printf("\n\n kb testing, type what you want(maximun128 char)...\n");
	printf("set 8 to make sure that test goes to terminal tests.\n");
	while(if9pressed!=-2);
	printf("terminal testing, type lines to see if it is correct.\n");
	printf("lines containing 9 or 8 or 7 will terminate this tests .\n");
	int num=40,flag=FAIL,readnum;
	char testbuf[40];
	terminal_open(NULL);
	while(1){
		readnum = terminal_read(0,(void *)testbuf,num);
		if(terminal_write(0,(void *)testbuf,readnum)>readnum) break;
		if(if9pressed==0){
			flag =PASS;
			break;
		}
	}
	return flag;
}

/* rtc Test - Drush8
 * 
 * frequency testing
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: terminal & kb driver
 * Files: rtc.c/h
 */
int rtc_test(){
	TEST_HEADER;
	// clean the screen
	int c;
	for(c=0;c<=2;c++){ //clean 26 line
		printf("\n");
	}
	// the test part
	int32_t i=0;
	int test_f[7]={2,4,8,16,32,64,5}; // test 6 legal and one illegal, they are the frequence
	printf("\n       RTC test       \n");
	printf("\n test 6 legal and one illegal frequency                    \n");
	rtc_open(0); // set to 2 Hz
	int r=0;
	int count;

	// 7 frequences in test_f
	while(i<7){
		count=0;
		printf("            \n now, the f is %d Hz                          \n", test_f[i]);
		if(rtc_write(0, &test_f[i], 4)==0){ // set the new f
			// print 20 characters
			while(count<20){
					rtc_read(0, &r, 4); // 4 is to make function work
			putc('@');
				count++;
			}
		}
		else printf("illegal frequency %d not power of 2\n", test_f[i]);
		i++;

	}
	return PASS;
}



/* filesys3func Tests - Drush8
 * 
 * frequency testing
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: filesys's 3 function (no sys_like call testing)
 * Files: rtc.c/h
 */
//

int list_all_files_test() {
	list_all_files_by_name();
	return PASS;
}//not used in demo cp3.2

int print_ls_test() {
	TEST_HEADER;
	printf("\n\n file test additional ...\n");
	printf("\n\n this test covers ls exec...(small exec)\n");
	int check = if9pressed;
	while(if9pressed==check);
	clear();
	read_file_i(12);
	printf("\n op key to continue....");
	check = if9pressed;
	while(if9pressed==check);
	return PASS;
}
int print_fish_exe_test() {
	TEST_HEADER;
	printf("\n\n file test additional...\n");
	printf("\n\n this test covers fish exec...(large exec)\n");
	int check = if9pressed;
	while(if9pressed==check);
	clear();
	read_file_i(6);
	printf("\n");
	return PASS;
}

/* filesystem open and read test - Drush8
 * 
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: 
 * Files: fileSystem.s/h
 */
int F_syslike_openread_test(){
	TEST_HEADER;
	int check = if9pressed;
	printf("\n\n filesystem open and read testing, type 9,8,7 to begin...\n");
	printf("\n\n this test covers frame0.txt...\n");
	while(if9pressed==check);
	clear();
	file_OpenRead_test();
	return PASS;
}

/* filesystem open and read test - Drush8
 * 
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: 
 * Files: fileSystem.s/h
 */
int F_Print_dir_test(){
	int check = if9pressed;
	printf("\n\n print diractory testing, type 9,8,7 to begin...\n");
	while(if9pressed==check);
	Print_dir_test();
	return PASS;
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
int vidmap_test(){
	page_test();
	printf("===================================");
	uint8_t *vm;
	vm = (uint8_t *)(140*MB); //virtual addr at 140MB
	vidmap(&vm);
	page_test();
	return 0;
}
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){

	//C P 1 : T E S T I N G 
	//TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
	//TEST_OUTPUT("page_test", page_test());
	//TEST_OUTPUT("syscall_test", syscall_test());
	//TEST_OUTPUT("keyboardandRTC_test", KAndR_test());

	//warning: this final test will lead to the blue screen of the kernel. drush8
	//TEST_OUTPUT("PageFault_test", pageFexception_test());


	//C P 2 : T E S T I N G 
	// TEST_OUTPUT("keyboardbAndterminal_test", kbAndterminal_test());
	// TEST_OUTPUT("rtcFrequency_test", rtc_test());
	// TEST_OUTPUT("F_Print_dir_test", F_Print_dir_test());
	// TEST_OUTPUT("F_syslike_openread_test", F_syslike_openread_test());
	// printf("\n additional files read tests:: small exec and large exec...\n");
	// TEST_OUTPUT("print_ls_test", print_ls_test());
	// TEST_OUTPUT("print_fish_exe_test", print_fish_exe_test());
	

	//C P 3 : T E S T I N G
	// Syscalls_test_file();
	// Syscalls_test_dir();
	// Syscalls_test_terminal();

	//C P 4 : T E S T I N G
	// vidmap_test();

	TEST_OUTPUT("malloc_test", malloc_test());
}

