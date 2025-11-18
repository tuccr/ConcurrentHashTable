#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sys/time.h"
#include <pthread.h>

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER; // mutex for safe logging (no logging events in the middle of each other)

const char fname[] = "hash.log";

enum state {
    WAIT, AWAKENED, READ_LOCK_ACQUIRE, READ_LOCK_RELEASE, WRITE_LOCK_ACQUIRE, WRITE_LOCK_RELEASE
};

enum cmd {
    INSERT, DELETE, UPDATE, SEARCH, PRINT
};

/*
Gets current timestamp in microseconds.
*/
long long current_timestamp() {  
    struct timeval te;  
    gettimeofday(&te, NULL); // get current time  
    long long microseconds = (te.tv_sec * 1000000) + te.tv_usec; // calculate milliseconds  
    return microseconds;
}

/*
Write line to log file atomically.
*/
void write_log(const char* line) {
    pthread_mutex_lock(&log_mutex); // lock mutex for safe logging

    FILE *logfile = fopen(fname, "a");
    if (logfile == NULL) {
        printf("Error opening log file\n");
        return; // Could not open log file
    }
    else {
        fprintf(logfile, "%s", line);
    }
    fclose(logfile);

    pthread_mutex_unlock(&log_mutex); // unlock mutex after logging
}

/*
Generate string for logging command with priority. Takes parameters as a string built by calling function.
*/
void log_event(int command, const char* params, int priority) {
    long long timestamp = current_timestamp();
    char buffer[128];
    char command_str[16];
    switch(command) {
        case INSERT:
            strcpy(command_str, "INSERT");
            break;
        case DELETE:
            strcpy(command_str, "DELETE");
            break;
        case UPDATE:
            strcpy(command_str, "UPDATE");
            break;
        case SEARCH:
            strcpy(command_str, "SEARCH");
            break;
        case PRINT:
            strcpy(command_str, "PRINT");
            break;
        default:
            strcpy(command_str, "UNKNOWN");
            break;
    }
    snprintf(buffer, sizeof(buffer), "%lld: THREAD %d %s,%s\n", timestamp, priority, command_str, params);

    write_log(buffer);
}

/*
Generate string for logging thread task/state change with priority.
*/
void log_event(int param, int priority) {
    long long timestamp = current_timestamp();
    char buffer[128];

    switch(param) {
        case WAIT:
            snprintf(buffer, sizeof(buffer), "%lld: THREAD %d WAITING FOR MY TURN\n", timestamp, priority);
            break;
        case AWAKENED:
            snprintf(buffer, sizeof(buffer), "%lld: THREAD %d AWAKENED FOR WORK\n", timestamp, priority);
            break;
        case READ_LOCK_ACQUIRE:
            snprintf(buffer, sizeof(buffer), "%lld: THREAD %d READ LOCK ACQUIRED\n", timestamp, priority);
            break;
        case READ_LOCK_RELEASE:
            snprintf(buffer, sizeof(buffer), "%lld: THREAD %d READ LOCK RELEASED\n", timestamp, priority);
            break;
        case WRITE_LOCK_ACQUIRE:
            snprintf(buffer, sizeof(buffer), "%lld: THREAD %d WRITE LOCK ACQUIRED\n", timestamp, priority);
            break;
        case WRITE_LOCK_RELEASE:
            snprintf(buffer, sizeof(buffer), "%lld: THREAD %d WRITE LOCK RELEASED\n", timestamp, priority);
            break;
        default:
            break;
    }

    write_log(buffer);
}