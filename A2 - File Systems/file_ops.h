#ifndef FILE_OPS_H_
#define FILE_OPS_H_

/* Constants and declarations for the file system. Items indicated with TODO
 * must be implemented by you. You may not change other parts of this file.*/

#include <stdio.h>

#define MAXNAME 24  // The maximum length allowed for a file name
#define MAXFILES 16
#define READSIZE 64
#define MAX_FS_SIZE 1024
#define METADATA_ENDS (int)(sizeof(Fnode) * MAXFILES)   // 

typedef struct fnode {
    char name[MAXNAME];
    int offset;
    int length;
} Fnode;

typedef struct freeblock {
    int offset;
    int length;
    struct freeblock *next;
    /* TODO: You may add fields to this struct.   
     * (Our solution had no additional fields)*/
} Freeblock;

typedef struct fs {
    Fnode metadata[MAXFILES];   // A place to store the meta data so we don't
                                // need to keep reading it.
    Freeblock *freelist;        // A pointer to the linked list of free blocks
    FILE *fp;                   // The open file handle to the file containing
                                // the simulated file system.
} FS;


FS *init_fs(char *filename);
FS *open_fs(char *filename);
void close_fs(FS *fs);

void print_fs(FS *fs);
void create_file(FS *fs, char *filename, int size, char *buf);
void delete_file(FS *fs, char *filename);

void fs_list(FS *fs);

#endif /*FILE_OPS_H_*/
