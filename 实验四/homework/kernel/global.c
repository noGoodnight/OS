
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "global.h"


PUBLIC    PROCESS proc_table[NR_TASKS];

PUBLIC    char task_stack[STACK_SIZE_TOTAL];

PUBLIC    TASK task_table[NR_TASKS] = {{TestA, STACK_SIZE_TESTA, "TestA"},
                                       {TestB, STACK_SIZE_TESTB, "TestB"},
                                       {TestC, STACK_SIZE_TESTC, "TestC"},
                                       {TestD, STACK_SIZE_TESTD, "TestD"},
                                       {TestE, STACK_SIZE_TESTE, "TestE"},
                                       {TestF, STACK_SIZE_TESTF, "TestF"}};

PUBLIC    irq_handler irq_table[NR_IRQ];

PUBLIC    system_call sys_call_table[NR_SYS_CALL] = {sys_get_ticks, sys_print_str, sys_sleep, sys_sem_p, sys_sem_v};

PUBLIC int first_time;
PUBLIC char *s;
