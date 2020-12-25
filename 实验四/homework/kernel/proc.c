
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

PRIVATE void p_read(PROCESS *, char *, Color, int);

PRIVATE void p_write(PROCESS *, char *, Color, int);

PRIVATE void print_read_start(char* , int);

PRIVATE void print_write_start(char*,int);

PRIVATE void print_end(char *, int);

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

PUBLIC void print(char *str) {
    s = str;
    print_str();
}

PUBLIC void sys_print_str() {
    disp_str(s);
}

PUBLIC void sys_sleep(int milli_seconds) {
    p_proc_ready->ready_time = get_ticks() + milli_seconds / (1000 / HZ);
    schedule();
}

PUBLIC void sys_sem_p(SEMAPHORE *p) {
    p->value--;
    if (p->value < 0) {
        p_proc_ready->wait_for_sem = TRUE;
        p->wait_list[p->end] = p_proc_ready;
        p->end = (p->end + 1) % SEMAPHORE_LIST_SIZE;
        schedule();
    }
}

PUBLIC void sys_sem_v(SEMAPHORE *v) {
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

PUBLIC void process(PROCESS *p, char *name, Color color, int time) {
    if (p->p_type == reader) {
        p_read(p, name, color, time);
    } else if (p->p_type == writer) {
        p_write(p, name, color, time);
    }
}

PRIVATE void p_read(PROCESS *p, char *name, Color color, int time) {
    while (1) {
        if (standard == read_first) {
            sem_p(&reader_control);

            sem_p(&mutex);
            nr_readers++;
            if (nr_readers == 1) {
                sem_p(&write_block);
            }
            sem_v(&mutex);

            print_read_start(name, color);
            p->p_run = TRUE;
            milli_delay(time);

            sem_p(&mutex);
            nr_readers--;
            if (nr_readers == 0) {
                sem_v(&write_block);
            }
            sem_v(&mutex);

            print_end(name, color);
            p->p_run = FALSE;
            sem_v(&reader_control);
        } else if (standard == write_first || standard == write_first_2) {
            sem_p(&read_block);

            sem_p(&reader_control);

            sem_p(&mutex);
            nr_readers++;
            if (nr_readers == 1) {
                sem_p(&write_block);
            }
            sem_v(&mutex);

            sem_v(&read_block);

            print_read_start(name, color);
            p->p_run = TRUE;
            milli_delay(time);

            sem_p(&mutex);
            nr_readers--;
            if (nr_readers == 0) {
                sem_v(&write_block);
            }
            sem_v(&mutex);

            print_end(name, color);
            p->p_run = FALSE;
            sem_v(&reader_control);
        }
        p->turns++;
        sum_read_turns++;
        if (read_sleep_turns == 0) {
            if (sum_read_turns - sum_write_turns > 5 * max_nr_readers) {
                read_sleep_turns++;
                sleep(10 * round);
            }
        } else {
            if (sum_read_turns - sum_write_turns > 5 * max_nr_readers * read_sleep_turns) {
                read_sleep_turns++;
                sleep(10 * round);
            }
        }
    }
}

PRIVATE void p_write(PROCESS *p, char *name, Color color, int time) {
    while (1) {
        if (standard == read_first) {
            sem_p(&write_block);

            sem_p(&mutex_w);
            nr_writers += 1;
            sem_v(&mutex_w);

            print_read_start(name, color);
            p->p_run = TRUE;
            milli_delay(time);

            sem_p(&mutex_w);
            nr_writers -= 1;
            sem_v(&mutex_w);

            print_end(name, color);
            p->p_run = FALSE;
            sem_v(&write_block);
        } else if (standard == write_first || standard == write_first_2) {
            sem_p(&read_block);

            sem_p(&write_block);

            sem_p(&mutex_w);
            nr_writers += 1;
            sem_v(&mutex_w);

            print_read_start(name, color);
            p->p_run = TRUE;
            milli_delay(time);

            sem_p(&mutex_w);
            nr_writers -= 1;
            sem_v(&mutex_w);

            print_end(name, color);
            p->p_run = FALSE;
            sem_v(&write_block);

            sem_v(&read_block);
        }
        p->turns++;
        sum_write_turns++;
        if (write_sleep_turns == 0) {
            if (sum_write_turns - sum_read_turns > 5 * max_nr_readers) {
                write_sleep_turns++;
                sleep(10 * round);
            }
        } else {
            if (sum_write_turns - sum_read_turns > 5 * max_nr_readers * write_sleep_turns) {
                write_sleep_turns++;
                sleep(10 * round);
            }
        }
    }
}

PRIVATE void print_read_start(char *name, Color color) {
    disp_color_str(name, color);
    disp_color_str(": start.       ", color);
    disp_color_str(name, color);
    disp_color_str(": reading.     ", color);
}

PRIVATE void print_write_start(char*name, Color color){
    disp_color_str(name, color);
    disp_color_str(": start.       ", color);
    disp_color_str(name, color);
    disp_color_str(": writing.     ", color);
}

PRIVATE void print_end(char *name, Color color) {
    disp_color_str(name, color);
    disp_color_str(": end.         ", color);
}

