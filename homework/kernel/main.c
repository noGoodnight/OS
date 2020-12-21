
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "string.h"
#include "global.h"


/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main() {
    disp_str("-----\"kernel_main\" begins-----\n");

    TASK *p_task = task_table;
    PROCESS *p_proc = proc_table;
    char *p_task_stack = task_stack + STACK_SIZE_TOTAL;
    u16 selector_ldt = SELECTOR_LDT_FIRST;

    for (int i = 0; i < NR_TASKS; i++) {
        strcpy(p_proc->p_name, p_task->name);    // name of the process
        p_proc->pid = i;            // pid
        p_proc->ldt_sel = selector_ldt;
        memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
        p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
        memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
        p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
        p_proc->regs.cs = ((8 * 0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
        p_proc->regs.ds = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
        p_proc->regs.es = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
        p_proc->regs.fs = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
        p_proc->regs.ss = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
        p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | RPL_TASK;
        p_proc->regs.eip = (u32) p_task->initial_eip;
        p_proc->regs.esp = (u32) p_task_stack;
        p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */
        p_proc->ready_time = 0;
        p_proc->wait_for_sem = FALSE;
        p_proc->turns = 0;

        p_task_stack -= p_task->stacksize;
        p_proc++;
        p_task++;
        selector_ldt += 1 << 3;
    }

    standard = read_first;
    nr_readers = 0;
    nr_writers = 0;

    init_sem(&mutex, 1);
    init_sem(&write_block, 1);
    init_sem(&reader_control, max_nr_readers);
    init_sem(&mutex_w, 1);
    init_sem(&read_block, 1);

    // todo: clean the screen
    disp_pos = 0;

    k_reenter = 0;
    ticks = 0;

    p_proc_ready = proc_table;

    /* 初始化 8253 PIT */
    out_byte(TIMER_MODE, RATE_GENERATOR);
    out_byte(TIMER0, (u8) (TIMER_FREQ / HZ));
    out_byte(TIMER0, (u8) ((TIMER_FREQ / HZ) >> 8));

    put_irq_handler(CLOCK_IRQ, clock_handler); /* 设定时钟中断处理程序 */
    enable_irq(CLOCK_IRQ);                     /* 让8259A可以接收时钟中断 */

    restart();

    while (1) {}
}

/*======================================================================*
                               TestA
 *======================================================================*/
void TestA() {
    p_read("A", a_color, 2 * round);
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB() {
    p_read("B", b_color, 3 * round);
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestC() {
    p_read("C", c_color, 3 * round);
}

void TestD() {
    p_write("D", d_color, 3 * round);
}

void TestE() {
    p_write("E", e_color, 4 * round);
}

void TestF() {
    while (1) {
        sem_p(&mutex_w);
        if (nr_writers > 0) {
//            print("F");
            disp_color_str("F", f_color);
            disp_color_str(": write.       ", f_color);
        } else {
//            print("F");
            disp_color_str("F", f_color);
            disp_color_str(": read. ", f_color);
            disp_int(nr_readers);
            disp_color_str("    ", f_color);
        }
        sem_v(&mutex_w);
        sleep(round);
    }
}
