
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* EXTERN is defined as extern except in global.c */
#ifdef    GLOBAL_VARIABLES_HERE
#undef    EXTERN
#define    EXTERN
#endif

EXTERN    int ticks;

EXTERN    int disp_pos;
EXTERN    u8 gdt_ptr[6];    // 0~15:Limit  16~47:Base
EXTERN    DESCRIPTOR gdt[GDT_SIZE];
EXTERN    u8 idt_ptr[6];    // 0~15:Limit  16~47:Base
EXTERN    GATE idt[IDT_SIZE];

EXTERN    u32 k_reenter;

EXTERN    TSS tss;
EXTERN    PROCESS *p_proc_ready;

extern PROCESS proc_table[];
extern char task_stack[];
extern TASK task_table[];
extern irq_handler irq_table[];

// my addition
extern char *s;
extern int sum_read_turns;
extern int sum_write_turns;
extern int read_sleep_turns;
extern int write_sleep_turns;
