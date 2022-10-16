#include "intrexcenum.h"
#include "e391device.h"
//create by drush8

/* drush8:
 * Rather than create a case for each number of arguments, we simplify
 * use a macro. Well just learn form the sys_call.
 */
#define INTR_LINK(name,func)   \
.GLOBL name                   ;\
name:                     ;\
	pushal  ;\
	pushfl  ;\
	call func ;\
	popfl ;\
	popal ;\
	iret          ;\



/* the system call library wrappers */
INTR_LINK(rtc_handler_linkage, rtc_handler)
INTR_LINK(keyboard_handler_linkage,keyboard_handler)


