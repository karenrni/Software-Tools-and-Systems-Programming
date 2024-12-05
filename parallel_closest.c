#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "point.h"
#include "serial_closest.h"
#include "parallel_closest.h"
#include "utilities_closest.h"


/*
 * Multi-process (parallel) implementation of the recursive divide-and-conquer
 * algorithm to find the minimal distance between any two pair of points in p[].
 * Assumes that the array p[] is sorted according to x coordinate.
 */
double closest_parallel(struct Point *p, int n, int pdmax, int *pcount) {
    if (n < 4 || pdmax == 0) { // i.e. maximum depth has been reached
        return closest_serial(p, n);
    }

    // 2: Split Array
    int midpoint = n / 2;
    struct Point *left = p;  // Left half-- floor n/2
    struct Point *right = p + midpoint; // Right half + 1 if odd
    int leftHalf = midpoint; // size left
    int rightHalf = n - midpoint;

    // 3: Children
    int leftPipe[2];
    if (pipe(leftPipe) == -1) {
        perror("pipe");
        exit(1);
    }
    // Fork left child 
    int leftResult = fork();

    if (leftResult == -1) { 
        perror("fork");
        exit(1);

    } else if (leftResult == 0) {
        if (close(leftPipe[0]) == -1) {
            perror("close reading end from inside left child");
            exit(1);
        } 

        // Send the distance to parent
        double closest_left = closest_parallel(left, leftHalf, pdmax - 1, pcount);

        if (write(leftPipe[1], &closest_left, sizeof(double)) != sizeof(double)) {
            perror("write from left child to pipe");
            exit(1);
        }

        if (close(leftPipe[1]) == -1) {
            perror("close lefts' pipe after writing");
            exit(1);
        } 
        (*pcount)++;
        exit(*pcount); // Exit with status num worker processes

    } else {  // in parent
        if (close(leftPipe[1]) == -1) {
            perror("close writing end of left's pipe in parent");
            exit(1);
        }
    }

    int rightPipe[2];
    if (pipe(rightPipe) == -1) {
        perror("pipe");
        exit(1);
    }
    // Fork right child 
    int rightResult = fork();

    if (rightResult == -1) {  
        perror("fork");
        exit(1);

    } else if (rightResult == 0) {
        if (close(rightPipe[0]) == -1) {
            perror("close reading end from inside right child");
            exit(1);
        } 

        // Send the distance to parent
        double closest_right = closest_parallel(right, rightHalf, pdmax - 1, pcount);

        if (write(rightPipe[1], &closest_right, sizeof(double)) != sizeof(double)) {
            perror("write from right child to pipe");
            exit(1);
        }

        if (close(rightPipe[1]) == -1) {
            perror("close right's pipe after writing");
            exit(1);
        } 
        (*pcount)++;
        exit(*pcount); // Exit with status num worker processes

    } else {  // in parent
        if (close(rightPipe[1]) == -1) {
            perror("close writing end of right's pipe in parent");
            exit(1);
        }
    }

    // 4: Wait for both children 
    int status;
    for (int i = 0; i < 2; i++) {
        if (wait(&status) == -1) {
            perror("wait");
            exit(1);
        }
        if (WIFEXITED(status)) {
            // Add worker process count from child exit status
            *pcount += WEXITSTATUS(status);  
        }
    }

    // 5: Read results 
    double left_distance, right_distance;

    if (read(leftPipe[0], &left_distance, sizeof(double)) != sizeof(double)) {
        perror("read from left pipe");
        exit(1);
    }
    if (close(leftPipe[0]) == -1) {
        perror("close reading end of left pipe in parent");
        exit(1);
    }

    if (read(rightPipe[0], &right_distance, sizeof(double)) != sizeof(double)) {
        perror("read from right pipe");
        exit(1);
    }
    if (close(rightPipe[0]) == -1) {
        perror("close reading end of right pipe in parent");
        exit(1);
    }

    // 6: step 4 from the single-process recursive divide-and-conquer solution
    
    double d = min(left_distance, right_distance);

    struct Point *strip = malloc(sizeof(struct Point) * n);
    if (!strip) {
        perror("malloc");
        exit(1);
    }
    // Make strip with points near the line passing through the middle point
    int strip_count = 0;
    for (int i = 0; i < n; i++) {
        if (abs(p[i].x - p[midpoint].x) < d) {
            strip[strip_count++] = p[i];
        }
    }

    // Sort strip by y
    qsort(strip, strip_count, sizeof(struct Point), compare_y);

    // 7: Find the closest points in strip
    double strip_distance = strip_closest(strip, strip_count, d);
    free(strip);
    return min(d, strip_distance);
}

