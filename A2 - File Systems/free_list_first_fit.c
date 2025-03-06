#include <string.h>
#include <stdlib.h>

#include "free_list.h"

/* Search the free list for the first block that has at least size bytes.
 * If the block that was found has exactly the right size, remove the block 
 * from the free list and return the offset of the block.
 * If the block that was found is larger than size, adjust the block in the free
 * list to reflect the remaining free space, and return the offset of the block.
 * Return -1 if no block is found that is large enough.
 */

int get_free_block(FS *fs, int size) {
    Freeblock *curr = fs->freelist;
    Freeblock *prev = NULL;

    while (curr != NULL) {
        if (curr->length >= size) { // found a block with at least size bytes
            int offset = curr->offset;

            if (curr->length == size) { // has exactly the right size
                if (prev == NULL) { // first block in freelist
                    fs->freelist = curr->next;
                } else {
                    prev->next = curr->next;
                }
                free(curr);
            } else { // block is bigger than size
                curr->offset += size;
                curr->length -= size;
            }

            return offset;
        }
        
        prev = curr;
        curr = curr->next;
    }

    return -1; // No block is large enough
}

