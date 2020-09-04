/** paging.h - methods related to paging initialization 
 *  and management
 * 
 *  @author Josh Sanchez
 */

#ifndef _PAGING_H
#define _PAGING_H

#ifndef ASM

#include "page_structs.h"

/* Used to set the Paging and Page Size Extension flags in the appropriate 
 * control registers. See the IA-32 Documentation for more details (3.6.1). */
extern void paging_init(uint32_t *pg_dir_ptr);

#endif

#endif /* _PAGING_H */
