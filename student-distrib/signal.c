#include "signal.h"
#include "lib.h"
#include "scheduler.h"
#include "Syscalls.h"
#include "x86_desc.h"

extern int nowterminalno;

extern void signal_handler_stack_setup(signal_handler handler, int32_t signum, hw_context_t *hw_context_addr); // defined in signal_asm_helper.S

signal_handler default_handler[MAX_SIGNAL_NUM];

/*
 * set_handler:
 *   DESCRIPTION: system call to set handler
 *   INPUTS: signum: the index of signal handler
 *           handler_address: the function pointer of the signal handler
 *   OUTPUTS: 0 if success, -1 fail
 *   RETURN VALUE: none
 */
int32_t set_handler(int32_t signum, void *handler_address)
{

    if (signum < 0 || signum >= MAX_SIGNAL_NUM)
    {
        printf("Invalid signum in sys_set_handle, signal.c\n");
        return -1;
    }

    int32_t flags;
    cli_and_save(flags);

    /* set signal handler address for a certain signal */
    if (handler_address == NULL)
    {
        get_PCB()->signal.signal_handler[signum] = default_handler[signum];
    }
    else
    {
        get_PCB()->signal.signal_handler[signum] = (signal_handler)handler_address;
    }

    restore_flags(flags);

    return 0;
}

/*
 * check_signal:
 *   DESCRIPTION: check the current process signal
 *   INPUTS: hw - hard_ware context
 *   OUTPUTS: None
 *   RETURN VALUE: None
 */
void check_signal(hw_context_t hw)
{

    if (hw.cs != USER_CS)
    {
        // if not in user space, return
        return;
    }

    int32_t flag;
    cli_and_save(flag);

    uint32_t cur_signal;
    // need to check the mask
    cur_signal = get_PCB()->signal.pending_signal & (~get_PCB()->signal.mask);
    // if (get_PCB() == sche_list[nowterminalno].pcb_ptr)
    //     printf("Same!\n");
    // cur_signal = sche_list[nowterminalno].pcb_ptr->signal.pending_signal & (~sche_list[nowterminalno].pcb_ptr->signal.mask);

    /* no signal need to dealt with */
    if (cur_signal == 0)
    {
        restore_flags(flag);
        return;
    }

    /* get one signal */
    int32_t signal_i;
    for (signal_i = 0; signal_i < MAX_SIGNAL_NUM; signal_i++)
    {
        if (cur_signal & 0x1)
        {
            break;
        }
        cur_signal = cur_signal >> 1;
    }

    // reset the signal in PCB
    get_PCB()->signal.prev_mask = get_PCB()->signal.mask;
    get_PCB()->signal.mask = MASK_ALL_SIGNALS;
    get_PCB()->signal.pending_signal = 0;
    signal_handler nowhandler = get_PCB()->signal.signal_handler[signal_i];
    if (nowhandler == default_handler[signal_i])
    {
        // printf("run default hanlder!\n");
        default_handler[signal_i]();
        restore_signal_mask();
    }

    else
    {
        // setup the stack frame
        // printf("set up user signal helper stack!\n");
        signal_handler_stack_setup(get_PCB()->signal.signal_handler[signal_i], signal_i, &hw);
    }

    restore_flags(flag);
}


/*
 * restore_signal_mask:
 *   DESCRIPTION: restore the signals masks
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 */
void restore_signal_mask()
{
    int32_t flags;
    cli_and_save(flags);

    get_PCB()->signal.mask = get_PCB()->signal.prev_mask;

    restore_flags(flags);
}

/*
 * signal_mask:
 *   DESCRIPTION: mask a certain signal
 *   INPUTS: signum: the index of signal to mask
 *   OUTPUTS: 0 if success, -1 fail
 *   RETURN VALUE: none
 */
int32_t signal_mask(int32_t signum)
{
    if (signum < 0 || signum >= MAX_SIGNAL_NUM)
    {
        printf("Invalid signum in signal_mask\n");
        return -1;
    }

    // Set masked signal corresponding to input para
    int32_t flags;
    cli_and_save(flags);

    get_PCB()->signal.mask |= (1 << signum);

    restore_flags(flags);

    return 0;
}

/*
 * signal_unmask:
 *   DESCRIPTION: unmask a certain signal
 *   INPUTS: signum: the index of signal to unmask
 *   OUTPUTS: 0 if success, -1 fail
 *   RETURN VALUE: none
 */
int32_t signal_unmask(int32_t signum)
{
    if (signum < 0 || signum >= MAX_SIGNAL_NUM)
    {
        printf("Invalid signum in signal_unmask\n");
        return -1;
    }

    // unmasked corresponding signal
    int32_t flags;
    cli_and_save(flags);

    get_PCB()->signal.mask &= ~(1 << signum);

    restore_flags(flags);

    return 0;
}

/*
 * signal_send:
 *   DESCRIPTION: send signal to the current running process by changing the pending
 *   INPUTS: signum: the index of signal
 *   OUTPUTS: 0 if success, -1 fail
 *   RETURN VALUE: none
 */
int32_t signal_send(int32_t signum)
{
    if (signum < 0 || signum >= MAX_SIGNAL_NUM)
    {

        printf("Invalid signum in signal_send\n");
        return -1;
    }

    // Set pending signal corresponding to input para
    int32_t flags;
    cli_and_save(flags);

    if (signum == 2 || signum == 3) // interrupt or alarm
    {
        if (sche_list[nowterminalno].pcb_ptr != NULL)
        {
            sche_list[nowterminalno].pcb_ptr->signal.pending_signal |= (1 << signum);
        }
    }
    else
    {
        get_PCB()->signal.pending_signal |= (1 << signum);
    }

    restore_flags(flags);

    return 0;
}

/********************* signal init ***************************/

/*
 * init_signal_default_handler:
 *   DESCRIPTION: init default signal handler
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
void init_signal_default_handler()
{
    default_handler[SIGNAL_DIV_ZERO] = sig_div_zero_default_handler;
    default_handler[SIGNAL_SEGFAULT] = sig_seg_default_handler;
    default_handler[SIGNAL_INTERRUPT] = sig_interrupt_default_handler;
    default_handler[SIGNAL_ALARM] = sig_alarm_default_handler;
    default_handler[SIGNAL_USER1] = sig_user1_default_handler;
}

/*
 * signal_init:
 *   DESCRIPTION: init the signal struct
 *   INPUTS: signal_array: the signal struct
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if success, -1 fail
 */
int32_t signal_init(signal_t *signal)
{

    if (signal == NULL)
    {
        printf("fail to init signal struct, empty input");
        return -1;
    }

    signal->mask = 0;
    signal->pending_signal = 0;
    signal->alarm_time = 0;

    int i;
    for (i = 0; i < MAX_SIGNAL_NUM; i++)
    {
        signal->signal_handler[i] = default_handler[i];
    }

    return 0;
}

/********************* default handler *************************/

/*
 * sig_div_zero_default_handler:
 *   DESCRIPTION: handler for divide by zero
 *   INPUTS: None
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
int32_t sig_div_zero_default_handler()
{
    int32_t flags;
    cli_and_save(flags);

    halt(256);

    restore_flags(flags);

    return -1;
}
/*
 * sig_seg_default_handler:
 *   DESCRIPTION: handler for segmentation fault
 *   INPUTS: None
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
int32_t sig_seg_default_handler()
{
    int32_t flags;
    cli_and_save(flags);

    halt(256);

    restore_flags(flags);

    return -1;
}

/*
 * sig_interrupt_default_handler:
 *   DESCRIPTION: handler for interrupt
 *   INPUTS: None
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
int32_t sig_interrupt_default_handler()
{
    int32_t flags;
    cli_and_save(flags);

    halt(0);

    restore_flags(flags);

    return -1;
}

/*
 * sig_alarm_default_handler:
 *   DESCRIPTION: handler for alarm
 *   INPUTS: None
 *   OUTPUTS: 0
 *   RETURN VALUE: none
 */
int32_t sig_alarm_default_handler()
{
    return 0;
}

/*
 * sig_user1_default_handle:
 *   DESCRIPTION: handler for user1
 *   INPUTS: None
 *   OUTPUTS: 0
 *   RETURN VALUE: none
 */
int32_t sig_user1_default_handler()
{
    return 0;
}
