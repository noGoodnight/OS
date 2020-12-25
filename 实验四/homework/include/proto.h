
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            proto.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* klib.asm */
PUBLIC void out_byte(u16 port, u8 value);

PUBLIC u8 in_byte(u16 port);

PUBLIC void disp_str(char *info);

PUBLIC void disp_color_str(char *info, int color);

/* protect.c */
PUBLIC void init_prot();

PUBLIC u32 seg2phys(u16 seg);

/* klib.c */
PUBLIC void delay(int time);

PUBLIC void disp_int(int input);

/* kernel.asm */
void restart();

/* main.c */
void TestA();

void TestB();

void TestC();

void TestD();

void TestE();

void TestF();

/* i8259.c */
PUBLIC void put_irq_handler(int irq, irq_handler handler);

PUBLIC void spurious_irq(int irq);

/* clock.c */
PUBLIC void clock_handler(int irq);


/* 以下是系统调用相关 */

/* proc.c */
PUBLIC int sys_get_ticks();        /* sys_call */

PUBLIC void init_sem(SEMAPHORE *semaphore, int i);

/* syscall.asm */
PUBLIC void sys_call();             /* int_handler */
PUBLIC int get_ticks();

// unknown sources
PUBLIC void milli_delay(int);

PUBLIC void enable_irq(int);

PUBLIC void schedule();

// my addition
PUBLIC void print(char *);

PUBLIC void print_str();

PUBLIC void sys_call_print();

PUBLIC void sys_print_str();

PUBLIC void sleep(int);

PUBLIC void sys_call_sleep();

PUBLIC void sys_sleep(int);

PUBLIC void sem_p(SEMAPHORE *);

PUBLIC void sys_call_sem_p();

PUBLIC void sys_sem_p(SEMAPHORE *);

PUBLIC void sem_v(SEMAPHORE *s);

PUBLIC void sys_call_sem_v();

PUBLIC void sys_sem_v(SEMAPHORE *s);

PUBLIC void process(PROCESS *, char *, Color, int);
