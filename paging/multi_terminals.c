#include "multi_terminals.h"

uint32_t virtual_terminal_addresses[3] = {TERM_1_VIRTUAL_BACKUP, TERM_2_VIRTUAL_BACKUP, TERM_3_VIRTUAL_BACKUP};
uint32_t physical_terminal_addresses[3] = {TERM_1_PHYSICAL_BACKUP, TERM_2_PHYSICAL_BACKUP, TERM_3_PHYSICAL_BACKUP};

void three_term_init() {
    // for each term, call terminal init
    int term;
    for (term = 0; term < 3; term++){
        current_term = term;
        terminal_init();
    }
    // default term to terminal 1
    current_term = 0; 
}
