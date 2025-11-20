// Priority queue for storing thread sthat arr waiting
// Function to see if a higher priority thread is waiting in the queue
// Define locks and condition variablesx

#include <pthread.h>
#include <stdio.h>

typedef struct thread_control_block {
    pthread_t thread_id;
    int priority;
    pthread_cond_t *cond;
} tcb_t;

typedef struct wait_queue {
    tcb_t *threads;
    int count;
} wait_queue_t;


int get_highest_priority(wait_queue_t *q) {
    if (q->count == 0) return -1;

    int max_idx = 0;
    for (int i = 1; i < q->count; i++) {
        if (q->threads[i].priority > q->threads[max_idx].priority) {
            max_idx = i;
        }
    }
    return max_idx;
}

void wake_highest_priority(wait_queue_t *q, pthread_mutex_t *lock) {
    int idx = get_highest_priority(q);
    if (idx < 0) return;

    tcb_t t = q->threads[idx];

    printf("Waking thread %lu (priority %d)\n",
           (unsigned long)t.thread_id, t.priority);

    pthread_cond_signal(t.cond);

    for (int i = idx; i < q->count - 1; i++) {
        q->threads[i] = q->threads[i + 1];
    }

    q->count -= 1;
}
