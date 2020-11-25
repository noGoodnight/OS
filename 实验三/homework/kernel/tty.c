
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

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

#define TTY_FIRST    (tty_table)
#define TTY_END        (tty_table + NR_CONSOLES)

PRIVATE void init_tty(TTY *p_tty);

PRIVATE void tty_do_read(TTY *p_tty);

PRIVATE void tty_do_write(TTY *p_tty);

PRIVATE void put_key(TTY *p_tty, u32 key);

/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty() {
    TTY *p_tty;

    init_keyboard();

    for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++) {
        init_tty(p_tty);
    }
    select_console(0);
    while (1) {
        for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++) {
            tty_do_read(p_tty);
            tty_do_write(p_tty);
        }
    }
}

/*======================================================================*
			   init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY *p_tty) {
    p_tty->inbuf_count = 0;
    p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;
    p_tty->p_console->index_chs = 0;
    p_tty->p_console->index_cursors = 0;
    p_tty->p_console->index_search_chs = 0;
    p_tty->p_console->search_mode = 0;
    p_tty->p_console->search_mode_lock = 0;

    init_screen(p_tty);
}

/*======================================================================*
				in_process
 *======================================================================*/
PUBLIC void in_process(TTY *p_tty, u32 key) {
    char output[2] = {'\0', '\0'};

    if (!(key & FLAG_EXT)) {
        if (!p_tty->p_console->search_mode_lock) {
            put_key(p_tty, key);
        }
    } else {
        int raw_code = key & MASK_RAW;
        switch (raw_code) {
            case ENTER:
                if (!p_tty->p_console->search_mode_lock) {
                    if (p_tty->p_console->search_mode) {
                        p_tty->p_console->search_mode_lock = 1;
                    }
                }
                put_key(p_tty, '\n');
                break;
            case BACKSPACE:
                if (!p_tty->p_console->search_mode_lock) {
                    put_key(p_tty, '\b');
                }
                break;
            case UP:
                if (!p_tty->p_console->search_mode_lock) {
                    if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
                        scroll_screen(p_tty->p_console, SCR_DN);
                    }
                }
                break;
            case DOWN:
                if (!p_tty->p_console->search_mode_lock) {
                    if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
                        scroll_screen(p_tty->p_console, SCR_UP);
                    }
                }
                break;
            case TAB:
                if (!p_tty->p_console->search_mode_lock) {
                    put_key(p_tty, '\t');
                }
                break;
            case ESC:
                if (!p_tty->p_console->search_mode) {
                    p_tty->p_console->search_mode = 1;
                    p_tty->cursor_position = p_tty->p_console->cursor;
                } else {
                    p_tty->p_console->search_mode_lock = 0;
                    p_tty->p_console->search_mode = 0;
                    p_tty->p_console->cursor = p_tty->cursor_position;
                    flush2(p_tty->p_console);
                    p_tty->p_console->index_chs -= p_tty->p_console->index_search_chs;
                    u8 *p_vmem = (u8 *) (V_MEM_BASE + p_tty->p_console->cursor * 2);
                    for (int i = 0; i < p_tty->p_console->index_search_chs; i++) {
                        *(p_vmem + 2 * i) = ' ';
                        *(p_vmem + 2 * i + 1) = DEFAULT_CHAR_COLOR;
                    }
                    p_tty->p_console->index_search_chs = 0;
                }
            case F1:
            case F2:
            case F3:
            case F4:
            case F5:
            case F6:
            case F7:
            case F8:
            case F9:
            case F10:
            case F11:
            case F12:
                /* Alt + F1~F12 */
                if (!p_tty->p_console->search_mode_lock) {
                    if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {
                        select_console(raw_code - F1);
                    }
                }
                break;
            default:
                if (!p_tty->p_console->search_mode_lock) {}
                break;
        }
    }

}

/*======================================================================*
			      put_key
*======================================================================*/
PRIVATE void put_key(TTY *p_tty, u32 key) {
    if (p_tty->inbuf_count < TTY_IN_BYTES) {
        *(p_tty->p_inbuf_head) = key;
        p_tty->p_inbuf_head++;
        if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
            p_tty->p_inbuf_head = p_tty->in_buf;
        }
        p_tty->inbuf_count++;
    }
}


/*======================================================================*
			      tty_do_read
 *======================================================================*/
PRIVATE void tty_do_read(TTY *p_tty) {
    if (is_current_console(p_tty->p_console)) {
        keyboard_read(p_tty);
    }
}


/*======================================================================*
			      tty_do_write
 *======================================================================*/
PRIVATE void tty_do_write(TTY *p_tty) {
    if (p_tty->inbuf_count) {
        char ch = *(p_tty->p_inbuf_tail);
        p_tty->p_inbuf_tail++;
        if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
            p_tty->p_inbuf_tail = p_tty->in_buf;
        }
        p_tty->inbuf_count--;

        out_char(p_tty->p_console, ch);
    }
}


