
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"


PUBLIC    PROCESS proc_table[NR_TASKS];

PUBLIC    char task_stack[STACK_SIZE_TOTAL];

PUBLIC    TASK task_table[NR_TASKS] = {{TestA, STACK_SIZE_TESTA, "TestA"},
                                       {TestB, STACK_SIZE_TESTB, "TestB"},
                                       {TestC, STACK_SIZE_TESTC, "TestC"},
                                       {TestC, STACK_SIZE_TESTD, "TestD"},
                                       {TestC, STACK_SIZE_TESTE, "TestE"},
                                       {TestC, STACK_SIZE_TESTF, "TestF"}};

PUBLIC    irq_handler irq_table[NR_IRQ];

PUBLIC    system_call sys_call_table[NR_SYS_CALL] = {sys_get_ticks, sys_print_str, sys_sem_p, sys_sem_v, sys_sleep};

PUBLIC char *s;
PUBLIC int standard;
PUBLIC int nr_readers;
PUBLIC int nr_writers;
PUBLIC SEMAPHORE read_block;
PUBLIC SEMAPHORE write_block;
PUBLIC SEMAPHORE reader_control;
PUBLIC SEMAPHORE mutex;
PUBLIC SEMAPHORE mutex_w;
