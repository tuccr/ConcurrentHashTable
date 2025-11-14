#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sys/time.h"
#include <pthread.h>

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER; // mutex for safe logging (no logging events in the middle of each other)

const char fname[] = "hash.log";

enum cmd_param {
    WAIT, AWAKENED, READ_LOCK_ACQUIRE, READ_LOCK_RELEASE, WRITE_LOCK_ACQUIRE, WRITE_LOCK_RELEASE
};

long long current_timestamp() {  
    struct timeval te;  
    gettimeofday(&te, NULL); // get current time  
    long long microseconds = (te.tv_sec * 1000000) + te.tv_usec; // calculate milliseconds  
    return microseconds;
}

void log_event(const char* command, int priority) {

    long long timestamp = current_timestamp();
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "%lld: THREAD %d %s\n", timestamp, priority, command);

    pthread_mutex_lock(&log_mutex); // lock mutex for safe logging

    FILE *logfile = fopen(fname, "a");
    if (logfile == NULL) {
        printf("Error opening log file\n");
        return; // Could not open log file
    }
    else {
        fprintf(logfile, "%s", buffer);
    }
    fclose(logfile);

    pthread_mutex_unlock(&log_mutex); // unlock mutex after logging
}

void log_event(int param, int priority) {
    long long timestamp = current_timestamp();
    char buffer[128];

    switch(param) {
        case WAIT:
            snprintf(buffer, "%lld: THREAD %d WAITING FOR MY TURN\n", timestamp, priority);
            break;
        case AWAKENED:
            snprintf(buffer, "%lld: THREAD %d AWAKENED FOR WORK\n", timestamp, priority);
            break;
        case READ_LOCK_ACQUIRE:
            snprintf(buffer, "%lld: THREAD %d READ LOCK ACQUIRED\n", timestamp, priority);
            break;
        case READ_LOCK_RELEASE:
            snprintf(buffer, "%lld: THREAD %d READ LOCK RELEASED\n", timestamp, priority);
            break;
        case WRITE_LOCK_ACQUIRE:
            snprintf(buffer, "%lld: THREAD %d WRITE LOCK ACQUIRED\n", timestamp, priority);
            break;
        case WRITE_LOCK_RELEASE:
            snprintf(buffer, "%lld: THREAD %d WRITE LOCK RELEASED\n", timestamp, priority);
            break;
        default:
            break;
    }

    pthread_mutex_lock(&log_mutex); // lock mutex for safe logging

    FILE *logfile = fopen(fname, "a");
    if (logfile == NULL) {
        printf("Error opening log file\n");
        return; // Could not open log file
    }
    fprintf(logfile, "THREAD %d %s", priority, buffer);
    fclose(logfile);

    pthread_mutex_unlock(&log_mutex); // unlock mutex after logging
}