
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
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

PUBLIC SEMAPHORE read_block;
PUBLIC SEMAPHORE write_block;
PUBLIC SEMAPHORE reader_control;
PUBLIC SEMAPHORE mutex;
PUBLIC SEMAPHORE mutex_w;
PUBLIC int nr_readers;
PUBLIC int nr_writers;
PUBLIC int standard;

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule() {
    // PROCESS* p;
    // int	 greatest_ticks = 0;

    // while (!greatest_ticks) {
    // 	for (p = proc_table; p < proc_table+NR_TASKS; p++) {
    // 		if (p->ticks > greatest_ticks) {
    // 			greatest_ticks = p->ticks;
    // 			p_proc_ready = p;
    // 		}
    // 	}

    // 	if (!greatest_ticks) {
    // 		for (p = proc_table; p < proc_table+NR_TASKS; p++) {
    // 			p->ticks = p->priority;
    // 		}
    // 	}
    // }
    PROCESS *p = p_proc_ready;
    while (1) {
        int t = get_ticks();
        p++;
        if (p >= proc_table + NR_TASKS) {
            p = proc_table;
        }
        if ((!p->wait_for_sem) && p->ready_time <= t) {
            p_proc_ready = p;
            break;
        }
    }
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks() {
    return ticks;
}

PUBLIC void print(char *str){
    s = str;
    print_str();
}

PUBLIC void sys_print_str(){
    disp_str(s);
}

PUBLIC void pro_sleep(int milli_seconds) {
    p_proc_ready->ready_time = get_ticks() + milli_seconds / (1000 / HZ);
    schedule();
}

/*======================================================================*
                           semaphore_p
 *======================================================================*/
PUBLIC void semaphore_p(SEMAPHORE *p) {
    p->value--;
    if (p->value < 0) {
        //阻塞该进程，置为等待状态，转向进程调度
        p_proc_ready->wait_for_sem = TRUE;
        p->wait_list[p->end] = p_proc_ready;
        p->end = (p->end + 1) % SEMAPHORE_LIST_SIZE;
        schedule();
    }
}

/*======================================================================*
                           semaphore_v
 *======================================================================*/
PUBLIC void semaphore_v(SEMAPHORE *v) {
    v->value++;
    if (v->value <= 0) {
        v->wait_list[v->start]->wait_for_sem = FALSE;
        v->start = (v->start + 1) % SEMAPHORE_LIST_SIZE;
    }
}

PUBLIC void init_sem(SEMAPHORE *semaphore, int i) {
    semaphore->value = i;
    semaphore->start = semaphore->end = 0;
}

PUBLIC void p_read(char *name, int color, int time) {
    while (1) {
        if (standard == read_first) {
            sem_p(&reader_control);

            sem_p(&mutex);
            nr_readers++;
            if (nr_readers == 1) {
                sem_p(&write_block);
            }
            sem_v(&mutex);

            disp_color_str(name, color);
            disp_color_str(": start.       ", color);
            disp_color_str(name, color);
            disp_color_str(": reading.     ", color);
            milli_delay(time);

            sem_p(&mutex);
            nr_readers--;
            if (nr_readers == 0) {
                sem_v(&write_block);
            }
            sem_v(&mutex);

            disp_color_str(name, color);
            disp_color_str(": end          ", color);
            sem_v(&reader_control);
        } else {
            sem_p(&read_block);

            sem_p(&reader_control);

            sem_p(&mutex);
            nr_readers++;
            if (nr_readers == 1) {
                sem_p(&write_block);
            }
            sem_v(&mutex);

            sem_v(&read_block);

            disp_color_str(name, color);
            disp_color_str(": start.       ", color);
            disp_color_str(name, color);
            disp_color_str(": reading.     ", color);
            milli_delay(time);

            sem_p(&mutex);
            nr_readers--;
            if (nr_readers == 0) {
                sem_v(&write_block);
            }
            sem_v(&mutex);

            disp_color_str(name, color);
            disp_color_str(": end.         ", color);
            sem_v(&reader_control);
        }
    }
}

PUBLIC void p_write(char *name, int color, int time) {
    while (1) {
        if (standard == read_first) {
            sem_p(&write_block);

            sem_p(&mutex_w);
            nr_writers += 1;
            sem_v(&mutex_w);

            disp_color_str(name, color);
            disp_color_str(": start.       ", color);
            disp_color_str(name, color);
            disp_color_str(": writing.     ", color);
            milli_delay(time);

            sem_p(&mutex_w);
            nr_writers -= 1;
            sem_v(&mutex_w);

            disp_color_str(name, color);
            disp_color_str(": end.         ", color);
            sem_v(&write_block);
        } else {
            sem_p(&read_block);

            sem_p(&write_block);

            sem_p(&mutex_w);
            nr_writers += 1;
            sem_v(&mutex_w);

            disp_color_str(name, color);
            disp_color_str(": start.       ", color);
            disp_color_str(name, color);
            disp_color_str(": writing.     ", color);
            milli_delay(time);

            sem_p(&mutex_w);
            nr_writers -= 1;
            sem_v(&mutex_w);

            disp_color_str(name, color);
            disp_color_str(": end.         ", color);
            sem_v(&write_block);

            sem_v(&read_block);
        }
    }
}

