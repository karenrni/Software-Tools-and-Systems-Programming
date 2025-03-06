#ifndef FREE_LIST_H_
#define FREE_LIST_H_

#include "file_ops.h"

/* You may implement these functions in any way you see fit.
 * Remember that your Makefile must build two targets:
 * - a file system with best-fit search
 * - and a file system with first-fit search
 */

/* Implemented separately in free_list_best_fit.c and free_list_first_fit.c */
int get_free_block(FS *fs, int size);

/* Functions implemented in free_list_common.c*/

void add_free_block(FS *fs, int location, int size);
void rebuild_freelist(FS *fs);
void print_freelist(FS *fs); 


#endif /* FREE_LIST_H_ */
