
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/


#include <stdio.h>
#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);

PRIVATE void set_video_start_addr(u32 addr);

PRIVATE void flush(CONSOLE *p_con);

PRIVATE void change_red(CONSOLE *p_con);

/*======================================================================*
			   init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY *p_tty) {
    int nr_tty = p_tty - tty_table;
    p_tty->p_console = console_table + nr_tty;

    int v_mem_size = V_MEM_SIZE >> 1;    /* 显存总大小 (in WORD) */

    int con_v_mem_size = v_mem_size / NR_CONSOLES;
    p_tty->p_console->original_addr = nr_tty * con_v_mem_size;
    p_tty->p_console->v_mem_limit = con_v_mem_size;
    p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

    /* 默认光标位置在最开始处 */
    p_tty->p_console->cursor = p_tty->p_console->original_addr;

    if (nr_tty == 0) {
//        disp_pos = 0;
        /* 第一个控制台沿用原来的光标位置 */
        p_tty->p_console->cursor = disp_pos / 2;
//        u8 *p_vmem = (u8 *) (V_MEM_BASE + p_tty->p_console->cursor * 2);
//        for (unsigned int i = 0; i < 8000; i++) {
//            *p_vmem = ' ';
//            *(p_vmem + 1) = DEFAULT_CHAR_COLOR;
//            p_vmem += 2;
//        }
//        clear_screen(p_tty->p_console);
    } else {
        out_char(p_tty->p_console, nr_tty + '0');
        out_char(p_tty->p_console, '#');
    }

    set_cursor(p_tty->p_console->cursor);
}


/*======================================================================*
			   is_current_console
*======================================================================*/
PUBLIC int is_current_console(CONSOLE *p_con) {
    return (p_con == &console_table[nr_current_console]);
}


/*======================================================================*
			   out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE *p_con, char ch) {
    u8 *p_vmem = (u8 *) (V_MEM_BASE + p_con->cursor * 2);

    switch (ch) {
        case '\n':
            if (p_con->search_mode == 0) {
//                if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - SCREEN_WIDTH) {
                    p_con->chs[p_con->index_chs] = '\n';
                    p_con->index_chs++;
                    p_con->line_cursors[p_con->index_line_cursors] = p_con->cursor;
                    p_con->index_line_cursors++;
                    p_con->cursor = p_con->original_addr +
                                    SCREEN_WIDTH * ((p_con->cursor - p_con->original_addr) / SCREEN_WIDTH + 1);
//                }
            } else {
                change_red(p_con);
            }
            break;
        case '\b':
            if ((p_con->search_mode == 0 && (p_con->cursor > p_con->original_addr)) ||
                (p_con->search_mode == 1 && (p_con->cursor > p_con->search_original_addr))) {
                switch (p_con->chs[p_con->index_chs - 1]) {
                    case '\n':
                        p_con->cursor = p_con->line_cursors[p_con->index_line_cursors - 1];
                        p_con->index_line_cursors--;
                        p_con->index_chs--;
                        break;
                    case '\t':
                        for (int i = 0; i < 4; i++) {
                            p_con->index_chs--;
                            p_con->cursor--;
                            if (p_con->search_mode == 1) {
                                p_con->index_search_chs--;
                            }
                            p_vmem -= 2;
                            *p_vmem = ' ';
                            *(p_vmem + 1) = p_con->search_mode ? SEARCH_CHAR_COLOR : DEFAULT_CHAR_COLOR;
                        }
                        break;
                    default:
                        p_con->cursor--;
                        *(p_vmem - 2) = ' ';
                        *(p_vmem - 1) = DEFAULT_CHAR_COLOR;
                        p_con->index_chs--;
                        if (p_con->search_mode == 1) {
                            p_con->index_search_chs--;
                        }
                        break;
                }
            }
            break;
        case '\t':
            for (int i = 0; i < 4; i++) {
                p_con->chs[p_con->index_chs] = '\t';
                p_con->index_chs++;
                if (p_con->search_mode == 1) {
                    p_con->search_chs[p_con->index_search_chs] = '\t';
                    p_con->index_search_chs++;
                }
                *p_vmem = ' ';
                *(p_vmem + 1) = p_con->search_mode ? SEARCH_CHAR_COLOR : DEFAULT_CHAR_COLOR;
                p_vmem += 2;
                p_con->cursor++;
            }
            break;
        default:
            if (p_con->search_mode == 1) {
                p_con->search_chs[p_con->index_search_chs] = ch;
                p_con->index_search_chs++;
            }
            p_con->chs[p_con->index_chs] = ch;
            p_con->index_chs++;
//            if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - 1) {
                *p_vmem++ = ch;
                *p_vmem++ = p_con->search_mode ? SEARCH_CHAR_COLOR : DEFAULT_CHAR_COLOR;
                p_con->cursor++;
//            }
            break;
    }

    while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
        scroll_screen(p_con, SCR_DN);
    }

    flush(p_con);
}

/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE *p_con) {
    set_cursor(p_con->cursor);
    set_video_start_addr(p_con->current_start_addr);
}

/*======================================================================*
			    set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position) {
    disable_int();
    out_byte(CRTC_ADDR_REG, CURSOR_H);
    out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, CURSOR_L);
    out_byte(CRTC_DATA_REG, position & 0xFF);
    enable_int();
}

/*======================================================================*
			  set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr) {
    disable_int();
    out_byte(CRTC_ADDR_REG, START_ADDR_H);
    out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, START_ADDR_L);
    out_byte(CRTC_DATA_REG, addr & 0xFF);
    enable_int();
}



/*======================================================================*
			   select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console)    /* 0 ~ (NR_CONSOLES - 1) */
{
    if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
        return;
    }

    nr_current_console = nr_console;

    set_cursor(console_table[nr_console].cursor);
    set_video_start_addr(console_table[nr_console].current_start_addr);
}

/*======================================================================*
			   scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE *p_con, int direction) {
    if (direction == SCR_UP) {
        if (p_con->current_start_addr > p_con->original_addr) {
            p_con->current_start_addr -= SCREEN_WIDTH;
        }
    } else if (direction == SCR_DN) {
        if (p_con->current_start_addr + SCREEN_SIZE <
            p_con->original_addr + p_con->v_mem_limit) {
            p_con->current_start_addr += SCREEN_WIDTH;
        }
    } else {
    }

    set_video_start_addr(p_con->current_start_addr);
    set_cursor(p_con->cursor);
}

PRIVATE void change_red(CONSOLE *p_con) {
    for (int i = 0; i < p_con->index_line_cursors; i++) {
        unsigned int start = i * SCREEN_WIDTH;
        while (start < p_con->line_cursors[i]) {
            int match = 0;
            for (int j = 0; j < p_con->index_search_chs; j++) {
                u8 *p_vmem = (u8 *) (V_MEM_BASE + start * 2);
                if (p_con->search_chs[j] != '\t') {
                    if (*(p_vmem + 2 * j) != p_con->search_chs[j]) {
                        break;
                    }
                } else {
                    if (*(p_vmem + 2 * j) != ' ') {
                        break;
                    }
                }
                if (j == p_con->index_search_chs - 1) {
                    match = 1;
                }
            }
            if (match == 1) {
                for (int j = 0; j < p_con->index_search_chs; j++) {
                    u8 *p_vmem = (u8 *) (V_MEM_BASE + start * 2);
                    *(p_vmem + 1) = SEARCH_CHAR_COLOR;
                    start++;
                }
            } else {
                start++;
            }
        }
    }
    unsigned int start = p_con->index_line_cursors * SCREEN_WIDTH;
    while (start < p_con->cursor) {
        int match = 0;
        for (int j = 0; j < p_con->index_search_chs; j++) {
            u8 *p_vmem = (u8 *) (V_MEM_BASE + start * 2);
            if (p_con->search_chs[j] != '\t') {
                if (*(p_vmem + 2 * j) != p_con->search_chs[j]) {
                    break;
                }
            } else {
                if (*(p_vmem + 2 * j) != ' ') {
                    break;
                }
            }
            if (j == p_con->index_search_chs - 1) {
                match = 1;
            }
        }
        if (match == 1) {
            for (int j = 0; j < p_con->index_search_chs; j++) {
                u8 *p_vmem = (u8 *) (V_MEM_BASE + start * 2);
                *(p_vmem + 1) = SEARCH_CHAR_COLOR;
                start++;
            }
        } else {
            start++;
        }
    }
    flush(p_con);
}

PUBLIC void change_white(CONSOLE *p_con) {
    for (int i = 0; i < p_con->index_line_cursors; i++) {
        unsigned int start = i * SCREEN_WIDTH;
        while (start < p_con->line_cursors[i]) {
            u8 *p_vmem = (u8 *) (V_MEM_BASE + start * 2);
            if (*(p_vmem + 1) == SEARCH_CHAR_COLOR) {
                *(p_vmem + 1) = DEFAULT_CHAR_COLOR;
            }
            start++;
        }
    }
    unsigned int start = p_con->index_line_cursors * SCREEN_WIDTH;
    while (start < p_con->cursor) {
        u8 *p_vmem = (u8 *) (V_MEM_BASE + start * 2);
        if (*(p_vmem + 1) == SEARCH_CHAR_COLOR) {
            *(p_vmem + 1) = DEFAULT_CHAR_COLOR;
        }
        start++;
    }
}

PUBLIC void flush2(CONSOLE *p_on) {
    flush(p_on);
}

PUBLIC void clear_screen(CONSOLE *p_con) {
    // console的左上角
    u8 *start_vmem = (u8 *) (V_MEM_BASE + p_con->original_addr * 2);
    // console的当前cursor所对应的显存的位置
    u8 *p_vmem = (u8 *) (V_MEM_BASE + p_con->cursor * 2);
    for (; start_vmem < p_vmem; start_vmem += 2) {
        *start_vmem = '~';
        *(start_vmem + 1) = DEFAULT_CHAR_COLOR;
    }
    p_con->index_chs = 0;
    p_con->index_line_cursors = 0;
    p_con->index_search_chs = 0;
    p_con->cursor = p_con->original_addr;
    p_con->current_start_addr = p_con->original_addr;
    flush(p_con);
}
