#include <stdlib.h>
#include <string.h>
#include "transactions.h"
#include "file_ops.h"
#include "free_list.h"

/* Each line of a transaction file represents a single operation on the
 * simulated file system.  A line is divided into several fields and the fields
 * are separated by a single space.
 * 
 * The first field of a transaction is a single character.
 * c = create_file, d = delete_file, 
 * s = print_freelist, p = print_fs
 * The remaining fields (if any) are the arguments of the operation in order
 */

#define MAXARGS 5

static void split(char **args, int num_args, char *line) {
    char *ptr = line;
    args[0] = line;
    int i = 1;
    while((ptr = strchr(ptr, ' ')) != NULL) {
        *ptr = '\0';
        ptr++;
        args[i] = ptr;
        i++;
    }

    while(i < num_args) {
        args[i] = NULL;
        i++;
    }
}

void process_transactions(char *transfile) {
    char line[MAXLINE];
    FS *fs = NULL;

    FILE *tf = fopen(transfile, "r");
    if (tf == NULL ) {
        perror("fopen");
        exit(1);
    }

    

    char * result;

    // Skip leading comment lines
    while(((result = fgets(line, MAXLINE, tf)) != NULL) && line[0] == '#');

    // first line that isn't a comment must be init or open
    if(result != NULL) {
        char *args[MAXARGS];

        // remove newline character
        line[strlen(line) - 1] = '\0';
        split(args, MAXARGS, line);

        if(args[1] == NULL) {
            fprintf(stderr, "First transaction must have a file name\n");
            exit(1);
        }

        if(line[0] == 'i') {
            fs = init_fs(args[1]);
        } else if(line[0] == 'o') {
            fs = open_fs(args[1]);
        } else {
            fprintf(stderr, "First transaction must be init_fs or open_fs\n");
            exit(1);
        }
    }

    while((fgets(line, MAXLINE, tf)) != NULL) {
        char *args[MAXARGS];

        line[strlen(line) - 1] = '\0';
        split(args, MAXARGS, line);

        switch(line[0]) {
        case 'd':
            if(args[1] == NULL) {
                fprintf(stderr, "delete_file must have a file name\n");
                exit(1);
            }
            delete_file(fs, args[1]);
            break;
        case 'c':
            if(args[1] == NULL || args[2] == NULL || args[3] == NULL) {
                fprintf(stderr, "create_file must have a file name, size, and data\n");
                exit(1);
            }
            create_file(fs, args[1], atoi(args[2]), args[3]);
            break;
        case 's': // show free list
            print_freelist(fs);
            break;
        case 'p': // print file system
            print_fs(fs);
            break;
        case 'x':  // close the file system file and free the metadata
            close_fs(fs);
            break;
        case '#':  // just do nothing on comment line
            break;
        default:
            fprintf(stderr, "Error: bad operation, %s\n", line);
            exit(1);
        }
    }
    fclose(tf);
}
