#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sys/time.h"
#include <pthread.h>
#include "command.h"
#include "jenkins.h"

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER; // mutex for safe logging (no logging events in the middle of each other)
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER; // mutex for safe printing

const char fname[] = "hash.log";

enum state {
    WAIT, AWAKENED, READ_LOCK_ACQUIRE, READ_LOCK_RELEASE, WRITE_LOCK_ACQUIRE, WRITE_LOCK_RELEASE
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
Write line to log file atomically (avoids interleaved log entries).
*/
void *write_log(void* arg) {
    char* line = (char*)arg;

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

    return NULL;
}

/*
Generate string for logging command with priority. Takes parameters as a string built by calling function.
*/
void log_cmd(command_t* cmd) {
    long long timestamp = current_timestamp();
    char buffer[128];
    char command_str[16];

    uint32_t hash = jenkins_one_at_a_time_hash((uint8_t *)(cmd->name), strlen(cmd->name));

    switch(cmd->cmd) {
        case INSERT:
            snprintf(buffer, sizeof(buffer), "%lld: THREAD %d INSERT,%d,%s,%d\n", timestamp, cmd->priority, hash, cmd->name, cmd->salary);
            break;
        case DELETE:
            snprintf(buffer, sizeof(buffer), "%lld: THREAD %d DELETE,%d,%s\n", timestamp, cmd->priority, hash, cmd->name);
            break;
        case UPDATE:
            snprintf(buffer, sizeof(buffer), "%lld: THREAD %d UPDATE,%d,%s,%d\n", timestamp, cmd->priority, hash, cmd->name, cmd->salary);
            break;
        case SEARCH:
            snprintf(buffer, sizeof(buffer), "%lld: THREAD %d SEARCH,%d,%s\n", timestamp, cmd->priority, hash, cmd->name);
            break;
        case PRINT:
            snprintf(buffer, sizeof(buffer), "%lld: THREAD %d PRINT\n", timestamp, cmd->priority);
            break;
        default:
            snprintf(buffer, sizeof(buffer), "%lld: THREAD %d INVALID COMMAND\n", timestamp, cmd->priority);
            break;
    }

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

void protected_print(const char* str) {
    pthread_mutex_lock(&print_mutex);
    printf("%s", str);
    pthread_mutex_unlock(&print_mutex);
    return;
}