#include <string.h>
#include <stdlib.h>
#include "free_list.h"

/* These functions operate on a linked list of free blocks.  Each node of the
 * list contains the starting location and the length of a free block.
 */

/* Give free space back to the free list.  Since the list is ordered by 
 * location, this function is the same for both algorithms.
 * If the new block is adjacent to an existing block, merge the blocks.
 */
void add_free_block(FS *fs, int location, int size) {
    Freeblock *curr = fs -> freelist;
    Freeblock *next_block = curr -> next;
    Freeblock *order_block = NULL;

    // create the first free block when fs->freelist is NULL
    if (fs->freelist == NULL) {
        fs->freelist = malloc(sizeof(Freeblock));
        fs->freelist->offset = location;
        fs->freelist->length = size;
        fs->freelist->next = NULL;
        return;
    }

    // looking for adjacent blocks to merge with
    while (curr != NULL) {
        // Merge with adjacent block after curr block
        if (location + size == curr->offset) { //case 4, 0, 2
            curr->offset = location;
            curr->length += size;
            return;
        // merge with adjacent block before 
        } else if (curr->offset + curr->length == location) { //case 3 
            curr->length += size;
            // merge if there is another adjacent, since we merged with 
            // previous non-consecutive block
            if(next_block -> offset == location + size){ // case 5
                curr->length += next_block->length;
                curr->next = next_block->next;
            }
            return;
        }
        // block to maintain order for adding new freeblock
        if (curr->offset < location && next_block != NULL && next_block->offset > location) {
            order_block = curr;
        }
        curr = curr->next;
    }

    // create new block in case of no merge 
    Freeblock *added_block = malloc(sizeof(Freeblock));
    added_block->offset = location;
    added_block->length = size;

    // add block to freelist at correct position
    if (order_block == NULL) {
        added_block->next = fs->freelist;
        fs->freelist = added_block;
    } else { // Insert after order_block
        added_block->next = order_block->next;
        order_block->next = added_block;
    }
}

/* Print the contents of the free list to standard output
*/
void print_freelist(FS *fs) {
    // Use this format string to print each free list block:
    // "(offset: %d, length: %d)\n"
    printf("Free List\n");
    Freeblock *curr = fs->freelist;

    while (curr != NULL) {
        printf("(offset: %d, length: %d)\n", curr->offset, curr->length);
        curr = curr->next;
    }
}


// Helper for qsort comparisons
int rebuild_helper(const void *a, const void *b) {
    Fnode *fa = (Fnode *)a;
    Fnode *fb = (Fnode *)b;
    return fa->offset - fb->offset;
}

/* Build the freelist using the metadata read from a file.
 */
void rebuild_freelist(FS *fs) {
    Freeblock *prev = NULL;
    int data_start = METADATA_ENDS;
    int data_end = MAX_FS_SIZE;

    // sort metadata for easier freelist building in order of location/offset
    qsort(fs->metadata, MAXFILES, sizeof(Fnode), rebuild_helper);

    // search through sorted metadata
    int curr_offset = data_start;
    for (int i = 0; i < MAXFILES; i++) {
        int file_offset = fs->metadata[i].offset;
        int file_length = fs->metadata[i].length;

        // add empty space to freelist
        if (curr_offset < file_offset) {
            Freeblock *new_block = malloc(sizeof(Freeblock));
            new_block->offset = curr_offset;
            new_block->length = file_offset - curr_offset;
            new_block->next = NULL;

            if (fs->freelist == NULL) {
                fs->freelist = new_block;
            } else {
                prev->next = new_block;
            }
            prev = new_block;  
        }

        // Move current_offset to the end of the current file
        curr_offset = file_offset + file_length;
    }

    // add last chunk of space after all metadata files
    if (curr_offset < data_end) {
        Freeblock *new_block = malloc(sizeof(Freeblock));
        new_block->offset = curr_offset;
        new_block->length = data_end - curr_offset;
        new_block->next = NULL;

        if (fs->freelist == NULL) {
            fs->freelist = new_block;
        } else {
            prev->next = new_block;
        }
    }

}