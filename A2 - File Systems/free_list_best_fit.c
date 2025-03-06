#include <string.h>
#include <stdlib.h>

#include "free_list.h"

/* Search the free list for the smallest block that has at least size bytes.
 * If a block is found of exactly the right size, remove the block from the
 * free list and return the offset of the block.
 * If a block is found that is larger than size, adjust the block in the free
 * list to reflect the remaining free space, and return the offset of the block.
 * Return -1 if no block is found that is large enough.
 */

int get_free_block(FS *fs, int size) {
    Freeblock *curr = fs->freelist;
    Freeblock *prev = NULL;
    Freeblock *curr_smallest = NULL;
    Freeblock *smallest_prev = NULL;

    //search for smallest block 
    while (curr != NULL) {
        if (curr->length >= size) {
            if (curr_smallest == NULL || curr->length < curr_smallest->length) {
                curr_smallest = curr;
                smallest_prev = prev;
            }
        }
        prev = curr;
        curr = curr->next;
    }

    // no block is found that is large enough
    if (curr_smallest == NULL) {
        return -1;
    }

    int offset = curr_smallest->offset;

    //  if block exactly the right size
    if (curr_smallest->length == size) {
        if (smallest_prev == NULL) { // if the smallest is the first block
            fs->freelist = curr_smallest->next;
        } else {
            smallest_prev->next = curr_smallest->next;
        }
        free(curr_smallest); // can free block entirely
    } else {  // adjust the block in the freelist to reflect the remaining free space
        curr_smallest->offset += size;
        curr_smallest->length -= size;
    }

    return offset;
}


