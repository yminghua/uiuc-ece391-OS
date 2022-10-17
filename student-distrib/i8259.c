/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

//   i8529_init
//   Function: initialize master and slave
//   Inputs: none
//   Return Value: none
//   write by yst
void i8259_init(void) {

    unsigned long flags;
    int k;
    // lock
    cli_and_save(flags);    // Saves the EFLAGS register into the variable "flags", and then disables interrupts on this processor

    // mask
    outb(0xff, MASTER_8259_PORT+1);         // 0xff, mask all of 8259A-1, in port 0x21
    outb(0xff, SLAVE_8259_PORT+1);   // 0xff, mask all of 8259A-2, in port 0xA1    

    // init master PIC
    outb(ICW1, MASTER_8259_PORT);           // ICW1, init
    outb(ICW2_MASTER, MASTER_8259_PORT+1);  // ICW2, high bits of vector, in port 0x21
    outb(ICW3_MASTER, MASTER_8259_PORT+1);  // ICW3, primary PIC: bit vector of secondary PIC;, in port 0x21
    outb(ICW4, MASTER_8259_PORT+1);         // ICW4, ISA=x86, normal/auto EOI, in port 0x21
    
    // init slave PIC
    outb(ICW1, SLAVE_8259_PORT);            // ICW1, init
    outb(ICW2_SLAVE, SLAVE_8259_PORT+1);    // ICW2, high bits of vector, in port 0xA1
    outb(ICW3_SLAVE,SLAVE_8259_PORT+1);     // ICW3, secondary PIC: input pin on primary PIC;, in port 0xA1
    outb(ICW4, SLAVE_8259_PORT+1);          // ICW4, ISA=x86, normal/auto EOI, in port 0xA1
    
    for(k=0;k<16;k++)disable_irq(k); //disable all the mask...
    // enable slave interrupts to be processed on master.
    //enable_irq(2); // 2, the irq_num of slave line on master

    // unlock
    sti();                  // set interrupt flag - enable interrupts on this processor
    restore_flags(flags);   // puts the value in "flags" into the EFLAGS register.  Most often used after a cli_and_save_flags(flags)
}

//   enable_irq
//   function: unmask specified irq
//   inputs: irq_num, the number of IRQ busline, only 0-15
//   return Value: none
//   write by yst
void enable_irq(uint32_t irq_num) {

    // legal iqr_num ?
    if(irq_num >= 2 * irq_port_total || irq_num < 0){ // 2 for both PIC
        printf("illegal irq_num in function enable_irq");
        return; 
    }


    if(irq_num < irq_port_total) {
        master_mask = master_mask & ~(1 << irq_num);// get inverse and "and", only unmask the specified bit
        outb(master_mask, MASTER_8259_PORT+1);// Write mask into master, into 0x21 prot

    } else {
        slave_mask = slave_mask & ~(1 << irq_num); // get inverse and "and", only unmask the specified bit
        outb(slave_mask, SLAVE_8259_PORT+1);// Write mask into master, into 0xA1 prot
    }
}

//   disable_irq
//   Function: mask the specified irq
//   Inputs: irq_num, the number of IRQ busline, only 0-15
//   Return Value: none
//   write by yst
void disable_irq(uint32_t irq_num) {

    // legal iqr_num ?
    if(irq_num >= 2 * irq_port_total || irq_num < 0 ){ // 2 for both PIC
        printf("illegal irq_num in function disable_irq");
        return; 
    }


    if(irq_num < irq_port_total) {
        master_mask = master_mask | (1 << irq_num);// "or", only mask specified irq
        outb(master_mask, MASTER_8259_PORT+1);// Write mask into master, into 0x21 prot

    } else {
        slave_mask = slave_mask | (1 << irq_num); // "or", only mask specified irq
        outb(slave_mask, SLAVE_8259_PORT+1);// Write mask into master, into 0xA1 prot
    }
}

/*   send_eoi
 *   function: Send "end of interrupt" signal
 *   inputs: irq_num, the number of IRQ busline, only 0-15
 *   return: none
 */
//   write by yst
void send_eoi(uint32_t irq_num) {

    // legal iqr_num ?
    if(irq_num >= 2 * irq_port_total || irq_num <0){ // 2 for both PIC
        printf("illegal irq_num in function send_eoi");
        return; 
    }

    uint8_t eoi_send = EOI;

    if(irq_num < irq_port_total) {
        eoi_send |= irq_num;                    // OR the data
        outb(eoi_send, MASTER_8259_PORT);       // send the EOI to master

    } else {
        eoi_send |= (irq_num - irq_port_total); // OR the data
        outb(eoi_send, SLAVE_8259_PORT);        // send the EOI to slave
        outb(EOI | 2, MASTER_8259_PORT);        // send EOI to master for the second line(2) for slave
        
    }
}
