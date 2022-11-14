/* x86_page.h - Defines for page directory and page table
 * created by LYS on 2022/10/15
 */

#ifndef _X86_PAGE_H
#define _X86_PAGE_H

#include "types.h"

#ifndef ASM



/**********************************************************************/
/*                                                                    */
/*                           Data Structure                           */
/*                                                                    */
/**********************************************************************/
/* Page Directory Entry for 4KB */
typedef struct PDE {
    union {
        uint32_t val;
        struct {
            union {
                struct {
                    uint32_t P             : 1; 
                    uint32_t R_W           : 1;
                    uint32_t U_S           : 1;
                    uint32_t PWT           : 1;
                    uint32_t PCD           : 1;
                    uint32_t A             : 1;
                    uint32_t D             : 1;
                    uint32_t PS            : 1;
                    uint32_t G             : 1;
                    uint32_t Avail         : 3;
                    uint32_t PAT           : 1;
                    uint32_t Reserved      : 9;
                    uint32_t PBase_Addr    : 10;
                }__attribute__ ((packed));

                struct {
                    uint32_t P             : 1;  
                    uint32_t R_W           : 1;
                    uint32_t U_S           : 1;
                    uint32_t PWT           : 1;
                    uint32_t PCD           : 1;
                    uint32_t A             : 1;
                    uint32_t D             : 1;
                    uint32_t PS            : 1;
                    uint32_t G             : 1;
                    uint32_t Avail         : 3;
                    uint32_t PTBase_Addr   : 20;
                }__attribute__ ((packed));
            } __attribute__ ((packed));          
        } __attribute__ ((packed));
    };
} PDE_t;

/* Page Directory Entry for 4MB */
/*typedef struct PDE_4M {
    union {
        uint32_t val;
        struct {
            uint32_t PBase_Addr    : 10;
            uint32_t Reserved      : 9;
            uint32_t PAT           : 1;
            uint32_t Avail         : 3;
            uint32_t G             : 1;
            uint32_t PS            : 1;
            uint32_t D             : 1;
            uint32_t A             : 1;
            uint32_t PCD           : 1;
            uint32_t PWT           : 1;
            uint32_t U_S           : 1;
            uint32_t R_W           : 1;
            uint32_t P             : 1;            
        } __attribute__ ((packed));
    };
} PDE_4M_t;*/

/* Page Table Entry */
typedef struct PTE {
    union {
        uint32_t val;
        struct {
            uint32_t P             : 1;    
            uint32_t R_W           : 1;
            uint32_t U_S           : 1;
            uint32_t PWT           : 1;
            uint32_t PCD           : 1;
            uint32_t A             : 1;
            uint32_t D             : 1;
            uint32_t PAT           : 1;
            uint32_t G             : 1;
            uint32_t Avail         : 3;
            uint32_t PBase_Addr    : 20;
        } __attribute__ ((packed));
    };
} PTE_t;


/**********************************************************************/
/*                                                                    */
/*                           Global Variables                         */
/*                                                                    */
/**********************************************************************/
//start memory address for Page Directory and Page Table
extern PDE_t PD[1024] __attribute__((aligned (0x1000)));
extern PTE_t PT[1024] __attribute__((aligned (0x1000)));
extern PTE_t PT_user[1024] __attribute__((aligned (0x1000)));


/**********************************************************************/
/*                                                                    */
/*                           MACRO FUNCTIONS                          */
/*                                                                    */
/**********************************************************************/
/* set an PDE_4K entry to point to a page table whose address given by base_addr */
#define SET_PD_ENTRY_4K(pde, base_addr, u_s, r_w)            \
do {                                                             \
    (pde).PTBase_Addr = ((((uint32_t)base_addr) >> 12) & 0xFFFFF);    \
    (pde).PS = 0;                                             \
    (pde).U_S = u_s&1;                                        \
    (pde).R_W = r_w&1;                                        \
    (pde).P = 1;                                              \
} while (0)

#define UNSET_PD_ENTRY_4K(pde)            \
do {                                                             \
    (pde).P = 0;                                              \
} while (0)

/* set an PDE_4M entry to point to a 4M page whose address given by base_addr */
#define SET_PD_ENTRY_4M(pde, base_addr, u_s, r_w)            \
do {                                                             \
    (pde).PBase_Addr = ((((uint32_t)base_addr) >> 22) & 0x3FF);       \
    (pde).PS = 1;                                             \
    (pde).U_S = u_s&1;                                        \
    (pde).R_W = r_w&1;                                        \
    (pde).P = 1;                                              \
} while (0)

/* unset an PDE_4M entry to point to a 4M page whose address given by base_addr */
#define UNSET_PD_ENTRY_4M(pde)            \
do {                                                             \
    (pde).PBase_Addr = 0;       \
    (pde).PS = 0;                                             \
    (pde).U_S = 0;                                        \
    (pde).R_W = 0;                                        \
    (pde).P = 0;                                              \
} while (0)

/* set an PTE entry to point to a 4K page whose address given by base_addr */
#define SET_PT_ENTRY(pte, base_addr, u_s, r_w)               \
do {                                                             \
    (pte).PBase_Addr = ((((uint32_t)base_addr) >> 12) & 0xFFFFF);     \
    (pte).U_S = u_s&1;                                        \
    (pte).R_W = r_w&1;                                        \
    (pte).P = 1;                                              \
} while (0)

#define UNSET_PT_ENTRY(pte)               \
do {                                                             \
    (pte).P = 0;                                              \
} while (0)
/* enable_paging.  This macro takes a 32-bit passed in address of the page directory, load this to CR3 
 * and set the paging (PG) and protection (PE) bits of CR0, as well as enable PSE (4 MiB pages) through cr4 */
#define enable_paging(pd)                \
do {                                     \
    asm volatile (                       \
        "movl %0, %%eax          \n\t"   \
        "movl %%eax, %%cr3       \n\t"   \
        "movl %%cr0, %%eax       \n\t"   \
        "orl  $0x80000000, %%eax \n\t"   \
        "movl %%eax, %%cr0       \n\t"   \
        "movl %%cr4, %%eax       \n\t"   \
        "orl  $0x00000010, %%eax \n\t"   \
        "movl %%eax, %%cr4"              \
            :                            \
            : "r" (pd)                   \
            : "eax"                      \
    );                                   \
} while (0)

/* Flush_TLB.  reload cr3 to flush TLB */
#define Flush_TLB()                      \
do {                                     \
    asm volatile (                       \
        "movl %%cr3, %%eax;       \n\t"  \
        "movl %%eax, %%cr3;       \n\t"  \
            :                            \
            :                            \
            : "eax"                      \
    );                                   \
} while (0)


/**********************************************************************/
/*                                                                    */
/*                        FUNCTIONS from x86_page.c                   */
/*                                                                    */
/**********************************************************************/
extern void init_paging();
extern void map_4M(uint32_t vm, uint32_t phys);
extern void map_4M_U(uint32_t vm, uint32_t phys);
extern void unmap_4M(uint32_t vm, uint32_t phys);


#endif //ASM
#endif //_X86_PAGE_H
