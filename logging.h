#include <stdlib.h>
#include <string.h>
#include "sys/time.h"

char fname[32] = "hash.log";

long long current_timestamp() {  
    struct timeval te;  
    gettimeofday(&te, NULL); // get current time  
    long long microseconds = (te.tv_sec * 1000000) + te.tv_usec; // calculate milliseconds  
    return microseconds;
}