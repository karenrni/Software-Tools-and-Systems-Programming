#include <stdlib.h>
#include <string.h>
#include "file_ops.h"
#include "free_list.h"

/* You must not modify code already existing in this file. Your
 * code must interact with the reference implementation provided
 * here without requiring any changes to it */

/* Return a pointer to the array of Fnodes that is read from the simulated
 * file system.
 * Notes:
 *  - The caller of this function must remember to free the memory that
 *    is returned.
 *  - The function is static because it is used only by other functions
 *    in this file.
 */
static Fnode *read_metadata(FS *fs) {
    size_t numread;
    if (fseek(fs->fp, 0, SEEK_SET) == -1) {
        perror("read_metadata:");
        exit(1);
    }
    numread = fread(fs->metadata, sizeof(Fnode), MAXFILES, fs->fp);
    if (numread < MAXFILES) {
        perror("read_metadata:");
        exit(1);
    }
    return fs->metadata;
}

/* Write the array of Fnodes to the simulated file system.
 */
static void write_metadata(FS *fs) {
    size_t numwritten;
    if (fseek(fs->fp, 0, SEEK_SET) == -1) {
        perror("write_metadata:");
        exit(1);
    }
    numwritten = fwrite(fs->metadata, sizeof(Fnode), MAXFILES, fs->fp);
    if (numwritten < MAXFILES) {
        perror("write_metadata:");
        exit(1);
    }
}

/* Return the index into the metadata array for the Fnode that contains
 * the file named filename.  Return -1 if the file is not found.
 */
static int find_fnode(Fnode *metadata, char *filename) {
    int i;
    for (i = 0; i < MAXFILES; i++) {
        if ((strcmp(metadata[i].name, filename)) == 0) {
            return i;
        }
    }
    return -1;
}

/* Initialize the simulated file system by writing the metadata to the file 
 * indicating that the file system is empty.
 */
FS *init_fs(char *filename) {
    int i;
    char buf[MAX_FS_SIZE]; // Used to store the file system data in memory.

    FS *fs = malloc(sizeof(FS));
    if (fs == NULL) {
        perror("init_fs:");
        exit(1);
    } 

    // open the file that will hold the file system data
    fs->fp = fopen(filename, "w+");
    if (fs->fp == NULL) {
        perror("init_fs:");
        exit(1);
    }

    // Initialize the metadata array and write it to the file
    for (i = 0; i < MAXFILES; i++) {
        fs->metadata[i].name[0] = '\0'; //Give the name an empty string
        fs->metadata[i].offset = -1; // Initialize to an invalid offset
        fs->metadata[i].length = -1; // Initialize to an invalid length
    }
    write_metadata(fs);

    // Fill up the data area with . so that the real file has the correct size
    memset(buf, '.', MAX_FS_SIZE);  
    if (fwrite(buf, MAX_FS_SIZE, 1, fs->fp) < 1) {
        perror("init_fs:");
        exit(1);
    }

    // Initialize the free list
    fs->freelist = NULL;
    add_free_block(fs, METADATA_ENDS, MAX_FS_SIZE);

    return fs;
}

/* Opens an existing real file containing a file_system
 * and returns a properly initialized metadata struct
 */
FS *open_fs(char *filename) {
    FS *fs = malloc(sizeof(FS));
    if (fs == NULL) {
        perror("open_fs:");
        exit(1);
    } 

    fs->fp = fopen(filename, "r+");
    if (fs->fp == NULL) {
        perror("open_fs:");
        exit(1);
    }
    
    read_metadata(fs);
    
    /* Implement rebuild_freelist, and uncomment the next 
     * line when you are ready to test it.
     */
    rebuild_freelist(fs);
    return fs;
}

/* Writes out the current state of the file system
 * and discards the in-memory state.
 */
void close_fs(FS *fs) {
    write_metadata(fs);
    fclose(fs->fp);
    
    /* Memory needs to be freed before exiting. Write the code
     * to do this
     */
    Freeblock *curr = fs->freelist;
    Freeblock *next;
    // free each block in freelist
    while (curr != NULL) {
        next = curr->next;
        free(curr);         
        curr = next;        
    }
    free(fs);

}

/* Print the contents of the simulated file system to stdout.
 */
void print_fs(FS *fs) {
    int i;
    char buf[READSIZE];
    int numread;
    int linecount = 0;

    printf("Metadata:\n");

    for (i = 0; i < MAXFILES; i++) {
        printf("%d %-24s %d %d\n", i, fs->metadata[i].name, 
                fs->metadata[i].offset, fs->metadata[i].length);
    }
    printf("\n");

    if (fseek(fs->fp, METADATA_ENDS, SEEK_SET) == -1) {
        perror("print_fs:");
        exit(1);
    }

    while((numread = fread(buf, 1, READSIZE, fs->fp)) > 0) {
        printf("[%d] ", linecount);
        fwrite(buf, numread, 1, stdout);
        fprintf(stdout, "\n");
        linecount++;
    }
}


/* Initialize metadata for a file. No space allocated to it yet
 */
void create_file(FS *fs, char *filename, int size, char *buf) {
    int i;
    for (i = 0; i < MAXFILES; i++) {
        if (fs->metadata[i].offset < 0) {
            fs->metadata[i].offset = 0;
            fs->metadata[i].length = 0;
            strncpy(fs->metadata[i].name, filename, MAXNAME);
            fs->metadata[i].name[MAXNAME-1] = '\0';
            break;
        }
    }
    if(i == MAXFILES) {
        fprintf(stderr, "Error: too many files.  Could not create %s\n", 
                filename);
        return;
    }

    /* Complete this function to
        - find an approrpriate free block which sets the offset
        - writes the simulated data to the real file at the offset
        (tip: use fwrite)
        - updates the offset in the metadata
     */
    // find suitable block
    int offset = get_free_block(fs, size);

    // if there is no space, clear metadata
    if (offset == -1) {
        fprintf(stderr, "Error: no space. Could not create %s\n", filename);
        fs->metadata[i].name[0] = '\0';
        fs->metadata[i].offset = -1;
        fs->metadata[i].length = -1;
        return;
    }

    // writes the simulated data to the real file at the offset
    fseek(fs->fp, offset, SEEK_SET);
    fwrite(buf, sizeof(char), size, fs->fp);
    // updates offset in metadata
    fs->metadata[i].offset = offset;
    fs->metadata[i].length = size;
    write_metadata(fs);
}

/* Remove metadata for this file, and return allocated space to free list.
 * This function does not delete the file data, it only modifies the 
 * Fnode and the free list.
 */
void delete_file(FS *fs, char *filename) {
    int index = find_fnode(fs->metadata, filename);

    if (index == -1) {
        fprintf(stderr, "Error: file %s does not exist\n", filename);
        return;
    }

    /* Give back the free space to the freelist */
    int file_offset = fs->metadata[index].offset;
    int file_length = fs->metadata[index].length;

    // account for deletion in freelist 
    add_free_block(fs, file_offset, file_length);

    // Update the metadata
    fs->metadata[index].name[0] = '\0';
    fs->metadata[index].offset = -1;
    fs->metadata[index].length = -1;
    write_metadata(fs);
}
