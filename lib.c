/* lib.c - Some basic library functions (printf, strlen, etc.)
 * vim:ts=4 noexpandtab */

#include "lib.h"
#include "devices/terminal_structs.h"
#include "scheduler/scheduler.h"
#include "interrupts/syscall_structs.h"

static int screen_x;
static int screen_y;
static char* video_mem = (char *)VIDEO;
static int status_bar_colors[3] = {4,2,1};

/* void clear(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears video memory */
void clear(void) {
    int32_t i;
    for (i = 0; i < (NUM_ROWS - STATUS_BAR_HEIGHT) * NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;
    }
    set_screen_position(0, 0);
}

/* void clear_multi(uint8_t* ptr_vid, uint32_t terminal_idx);
 * Inputs: uint_8* c = character to print
           uint_8* ptr_vid -> pointer to start of video mem address
           uint32_t terminal_idx -> which terminal are we printing to
 * Return Value: none
 * Function: Clears video memory */
void clear_multi(uint8_t* ptr_vid, uint32_t terminal_idx){
    int32_t i;
    for (i = 0; i < (NUM_ROWS - STATUS_BAR_HEIGHT) * NUM_COLS; i++) {
        *(uint8_t *)(ptr_vid + (i << 1)) = ' ';
        *(uint8_t *)(ptr_vid + (i << 1) + 1) = ATTRIB;
    }
    three_terminals[terminal_idx].screen_x = 0; 
    three_terminals[terminal_idx].screen_y = 0; 
}

/* void set_screen_position(void);
 * Inputs: x -- x coordinate
 *         y -- y coordinate
 * Return Value: none
 * Function: Sets cursor position to 0, 0 */
void set_screen_position(int x, int y) {
    screen_x = x;
    screen_y = y;
}

void update_status_bar_time(uint8_t h, uint8_t m, uint8_t s) {
    uint8_t time_buf[TIME_BAR_WIDTH - 2];
    memset((void *) time_buf, '0', TIME_BAR_WIDTH - 2);
    int8_t dig1[3];
    memset((void *) dig1, '0', 2);
    int8_t dig2[3];
    memset((void *) dig2, '0', 2);
    int8_t dig3[3];
    memset((void *) dig3, '0', 2);
    if(h < 10)
        itoa(h, (int8_t *) (dig1 + 1), 10);
    else    
        itoa(h, (int8_t *) dig1, 10);
    if(m < 10)
        itoa(m, (int8_t *) (dig2 + 1), 10);
    else    
        itoa(m, (int8_t *) dig2, 10);
    if(s < 10)
        itoa(s, (int8_t *) (dig3 + 1), 10);
    else    
        itoa(s, (int8_t *) dig3, 10);
    time_buf[0] = dig1[0];
    time_buf[1] = dig1[1];
    time_buf[2] = ':';
    time_buf[3] = dig2[0];
    time_buf[4] = dig2[1];
    time_buf[5] = ':';
    time_buf[6] = dig3[0];
    time_buf[7] = dig3[1];
    uint8_t col;
    uint8_t idx = 0;
    uint32_t vid_mem;
    for(vid_mem = 0xB8000; vid_mem <= 0xBB000; vid_mem += 0x1000) {
        idx = 0;
        for(col = NUM_COLS - TIME_BAR_WIDTH; col < NUM_COLS; col++) {
            if(col > NUM_COLS - TIME_BAR_WIDTH && col < NUM_COLS - 1) {
                *(uint8_t *)(vid_mem + ((NUM_COLS * (NUM_ROWS - 1) + col) << 1)) = time_buf[idx];
                *(uint8_t *)(vid_mem + ((NUM_COLS * (NUM_ROWS - 1) + col) << 1) + 1) = ATTRIB;
                idx++;
            }
        }
    }
}

void draw_status_bar(void) {
    uint8_t num_processes = MAX_MULTIPROCESS_NUM;
    uint8_t dist_per_column = (NUM_COLS - TIME_BAR_WIDTH) / num_processes;
    uint8_t i;
    uint8_t row;
    uint8_t col;
    uint8_t total_idx;
    uint8_t p_name[128];
    uint8_t pid;
    uint8_t cmd_len;
    uint8_t arg_len;
    uint8_t starting_text_pt;
    for(i = 0; i < num_processes; i++) {
        memset((void *) p_name, 0, 128);
        total_idx = 0;
        pid = multi_process_idx[i];
        cmd_len = strlen((int8_t *) PCB[pid]->cmd_name);
        arg_len = strlen((int8_t *) PCB[pid]->args);
        for(col = 0; col < cmd_len; col++) {
            p_name[total_idx] = PCB[pid]->cmd_name[col];
            total_idx++;
        }
        if(arg_len > 0) {
            p_name[total_idx] = ' ';
            total_idx++;
            for(col = 0; col < arg_len; col++) {
                p_name[total_idx] = PCB[pid]->args[col];
                total_idx++;
            }
        }
        p_name[total_idx] = '\0';
        starting_text_pt = ((i * dist_per_column) + (dist_per_column / 2) - (cmd_len + arg_len + 2) / 2) + 1;
        for(col = i * dist_per_column; col < (i + 1) * dist_per_column; col++) {
            for(row = STATUS_BAR_HEIGHT; row > 0; row--) {
                uint8_t color = (current_term == i) ? status_bar_colors[i] + 8 : status_bar_colors[i];
                if(col >= starting_text_pt && col < starting_text_pt + (cmd_len + arg_len + 2)) {
                    *(uint8_t *)(video_mem + ((NUM_COLS * (NUM_ROWS - row) + col) << 1)) = p_name[col - starting_text_pt];
                    *(uint8_t *)(video_mem + ((NUM_COLS * (NUM_ROWS - row) + col) << 1) + 1) = color << 4 | 0x0F;
                } else {
                    *(uint8_t *)(video_mem + ((NUM_COLS * (NUM_ROWS - row) + col) << 1)) = ' ';
                    *(uint8_t *)(video_mem + ((NUM_COLS * (NUM_ROWS - row) + col) << 1) + 1) = color << 4;
                }
            }
        }
    }
}

/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output. */
int32_t printf(int8_t *format, ...) {

    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            putc('%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    puts(conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    puts(&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            putc((uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            puts(*((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                putc(*buf);
                break;
        }
        buf++;
    }
    return (buf - format);
}

/* int32_t puts(int8_t* s);
 *   Inputs: int_8* s = pointer to a string of characters
 *   Return Value: Number of bytes written
 *    Function: Output a string to the console */
int32_t puts(int8_t* s) {
    register int32_t index = 0;
    while (s[index] != '\0') {
        putc(s[index]);
        index++;
    }
    return index;
}

/* void putc(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 * Function: Output a character to the console */
void putc(uint8_t c) {
    if(c == '\n' || c == '\r') {
        screen_y++;
        screen_x = 0;
    } else {
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1)) = c;
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = ATTRIB;
        screen_x++;
    }
    screen_y = (screen_y + (screen_x / NUM_COLS)) % (NUM_ROWS - STATUS_BAR_HEIGHT);
    screen_x %= NUM_COLS;
}

/* void putc_multi(uint8_t c, uint8_t* ptr_vid, uint32_t terminal_idx);
 * Inputs: uint_8* c = character to print
           uint_8* ptr_vid -> pointer to start of video mem address
           uint32_t terminal_idx -> which terminal are we printing to
 * Return Value: void
 *  Function: Output a character to the console */
void putc_multi(uint8_t c, uint8_t* ptr_vid, uint32_t terminal_idx){
    if(c == '\n' || c == '\r') {
        three_terminals[terminal_idx].screen_y++;
        three_terminals[terminal_idx].screen_x = 0; 
    } else {
        *(uint8_t *)(ptr_vid + ((NUM_COLS * three_terminals[terminal_idx].screen_y + three_terminals[terminal_idx].screen_x) << 1)) = c; 
        *(uint8_t *)(ptr_vid + ((NUM_COLS * three_terminals[terminal_idx].screen_y + three_terminals[terminal_idx].screen_x) << 1) + 1) = ATTRIB;
        three_terminals[terminal_idx].screen_x++;
    }
    three_terminals[terminal_idx].screen_y = (three_terminals[terminal_idx].screen_y + (three_terminals[terminal_idx].screen_x / NUM_COLS)) % (NUM_ROWS - STATUS_BAR_HEIGHT); 
    three_terminals[terminal_idx].screen_x %= NUM_COLS; 
}

/* void setc_multi(int x, int y, uint8_t c, uint8_t* ptr_vid);
 * Inputs: location and char and pointer to vid mem
 * Return Value: void
 *  Function: Output a character to the console */
void setc_multi(int x, int y, uint8_t c, uint8_t* ptr_vid){
            *(uint8_t *)(ptr_vid + ((NUM_COLS * y + x) << 1)) = c;
        *(uint8_t *)(ptr_vid + ((NUM_COLS * y + x) << 1) + 1) = ATTRIB;
}


/* void getc(int x, int y);
 * Inputs: int x and y location
 * Return Value: uint8_t char at location
 *  Function: Gets char from location */
uint8_t getc(int x, int y) {
    return *(uint8_t *)(video_mem + ((NUM_COLS * y + x) << 1));
}

/* void getc_multi(int x, int y);
 * Inputs: int x and y location along with pointer to video memory
 * Return Value: uint8_t char at location
 *  Function: Gets char from location */
uint8_t getc_multi(int x, int y, uint8_t* ptr_vid){
    return *(uint8_t *)(ptr_vid + ((NUM_COLS * y + x) << 1)); 
}

/* int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix);
 * Inputs: uint32_t value = number to convert
 *            int8_t* buf = allocated buffer to place string in
 *          int32_t radix = base system. hex, oct, dec, etc.
 * Return Value: number of bytes written
 * Function: Convert a number to its ASCII representation, with base "radix" */
int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix) {
    static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int8_t *newbuf = buf;
    int32_t i;
    uint32_t newval = value;

    /* Special case for zero */
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    /* Go through the number one place value at a time, and add the
     * correct digit to "newbuf".  We actually add characters to the
     * ASCII string from lowest place value to highest, which is the
     * opposite of how the number should be printed.  We'll reverse the
     * characters later. */
    while (newval > 0) {
        i = newval % radix;
        *newbuf = lookup[i];
        newbuf++;
        newval /= radix;
    }

    /* Add a terminating NULL */
    *newbuf = '\0';

    /* Reverse the string and return */
    return strrev(buf);
}

/* int8_t* strrev(int8_t* s);
 * Inputs: int8_t* s = string to reverse
 * Return Value: reversed string
 * Function: reverses a string s */
int8_t* strrev(int8_t* s) {
    register int8_t tmp;
    register int32_t beg = 0;
    register int32_t end = strlen(s) - 1;

    while (beg < end) {
        tmp = s[end];
        s[end] = s[beg];
        s[beg] = tmp;
        beg++;
        end--;
    }
    return s;
}

/* uint32_t strlen(const int8_t* s);
 * Inputs: const int8_t* s = string to take length of
 * Return Value: length of string s
 * Function: return length of string s */
uint32_t strlen(const int8_t* s) {
    register uint32_t len = 0;
    while (s[len] != '\0')
        len++;
    return len;
}

/* int8_t* strtrimlead(const int8_t* s);
 * Inputs: const int8_t* s = string to trim
 * Return Value: ptr to first alphanumeric character in s
 * Function: return ptr to first alphanumeric character in s */
int8_t* strtrimlead(const int8_t* s) {
    register uint32_t len = 0;
    while(s[len] == ' ') {
        len++;
    }
    if(s[len] == '\0') {
        return NULL;
    }
    return (int8_t *) (s + len);
}

/* int8_t* strlen(const int8_t* s);
 * Inputs: const int8_t* s = string to trim
 * Return Value: ptr to last alphanumeric character in s
 * Function: return ptr to last alphanumeric character in s */
int8_t* strtrimtail(const int8_t* s) {
    register uint32_t length = strlen(s);
    uint32_t len = length - 1;
    while(s[len] == ' ') {
        len--;
    }
    if(s[len] == '\0') {
        return NULL;
    }
    return (int8_t *) (s + len);
}

/* int8_t* strchr(const int8_t* s, const int8_t c);
 * Inputs: const int8_t* s = string to search for character
 * Return Value: ptr to first occurrence of char in s
 * Function: return ptr to first occurrence of char in s */
int8_t* strchr(const int8_t* s, const int8_t c) {
    register uint32_t len = 0;
    while (s[len] != '\0' && s[len] != c) {
        len++;
    }

    // Not found
    if(s[len] == '\0' && c != '\0') {
        return NULL;
    }

    return (int8_t *) (s + len);
}


/* void* memset(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive bytes of pointer s to value c */
void* memset(void* s, int32_t c, uint32_t n) {
    c &= 0xFF;
    asm volatile ("                 \n\
            .memset_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memset_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memset_aligned \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memset_top     \n\
            .memset_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     stosl           \n\
            .memset_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memset_done    \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%edx       \n\
            jmp     .memset_bottom  \n\
            .memset_done:           \n\
            "
            :
            : "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_word(void* s, int32_t c, uint32_t n);
 * Description: Optimized memset_word
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set lower 16 bits of n consecutive memory locations of pointer s to value c */
void* memset_word(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosw           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_dword(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive memory locations of pointer s to value c */
void* memset_dword(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosl           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memcpy(void* dest, const void* src, uint32_t n);
 * Inputs:      void* dest = destination of copy
 *         const void* src = source of copy
 *              uint32_t n = number of byets to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of src to dest */
void* memcpy(void* dest, const void* src, uint32_t n) {
    asm volatile ("                 \n\
            .memcpy_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memcpy_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memcpy_aligned \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memcpy_top     \n\
            .memcpy_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     movsl           \n\
            .memcpy_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memcpy_done    \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%edx       \n\
            jmp     .memcpy_bottom  \n\
            .memcpy_done:           \n\
            "
            :
            : "S"(src), "D"(dest), "c"(n)
            : "eax", "edx", "memory", "cc"
    );
    return dest;
}

/* void* memmove(void* dest, const void* src, uint32_t n);
 * Description: Optimized memmove (used for overlapping memory areas)
 * Inputs:      void* dest = destination of move
 *         const void* src = source of move
 *              uint32_t n = number of byets to move
 * Return Value: pointer to dest
 * Function: move n bytes of src to dest */
void* memmove(void* dest, const void* src, uint32_t n) {
    asm volatile ("                             \n\
            movw    %%ds, %%dx                  \n\
            movw    %%dx, %%es                  \n\
            cld                                 \n\
            cmp     %%edi, %%esi                \n\
            jae     .memmove_go                 \n\
            leal    -1(%%esi, %%ecx), %%esi     \n\
            leal    -1(%%edi, %%ecx), %%edi     \n\
            std                                 \n\
            .memmove_go:                        \n\
            rep     movsb                       \n\
            "
            :
            : "D"(dest), "S"(src), "c"(n)
            : "edx", "memory", "cc"
    );
    return dest;
}

/* int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
 * Inputs: const int8_t* s1 = first string to compare
 *         const int8_t* s2 = second string to compare
 *               uint32_t n = number of bytes to compare
 * Return Value: A zero value indicates that the characters compared
 *               in both strings form the same string.
 *               A value greater than zero indicates that the first
 *               character that does not match has a greater value
 *               in str1 than in str2; And a value less than zero
 *               indicates the opposite.
 * Function: compares string 1 and string 2 for equality */
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n) {
    int32_t i;
    for (i = 0; i < n; i++) {
        if ((s1[i] != s2[i]) || (s1[i] == '\0') /* || s2[i] == '\0' */) {

            /* The s2[i] == '\0' is unnecessary because of the short-circuit
             * semantics of 'if' expressions in C.  If the first expression
             * (s1[i] != s2[i]) evaluates to false, that is, if s1[i] ==
             * s2[i], then we only need to test either s1[i] or s2[i] for
             * '\0', since we know they are equal. */
            return s1[i] - s2[i];
        }
    }
    return 0;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 * Return Value: pointer to dest
 * Function: copy the source string into the destination string */
int8_t* strcpy(int8_t* dest, const int8_t* src) {
    int32_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src, uint32_t n)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 *                uint32_t n = number of bytes to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of the source string into the destination string */
int8_t* strncpy(int8_t* dest, const int8_t* src, uint32_t n) {
    int32_t i = 0;
    while (src[i] != '\0' && i < n) {
        dest[i] = src[i];
        i++;
    }
    while (i < n) {
        dest[i] = '\0';
        i++;
    }
    return dest;
}

/* void test_interrupts(void)
 * Inputs: void
 * Return Value: void
 * Function: increments video memory. To be used to test rtc */
void test_interrupts(void) {
    int32_t i;
    for (i = 0; i < (NUM_ROWS - STATUS_BAR_HEIGHT) * NUM_COLS; i++) {
        video_mem[i << 1]++;
    }
}

/* void log2(void)
 * Inputs: uint32_t x
 * Return Value: result of log_2(x)
 * Function: Calculates log base 2 */
uint32_t log2(const uint32_t x) {
    uint32_t y;
    asm ( "\tbsr %1, %0\n"
        : "=r"(y)
        : "r" (x)
    );
    return y;
}

int32_t max(const int32_t a, const int32_t b) {
    return (a >= b) ? a : b;
}

int32_t min(const int32_t a, const int32_t b) {
    return (a >= b) ? b : a;
}


