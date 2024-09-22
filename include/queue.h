#ifndef CODIM_QUEUE_H
#define CODIM_QUEUE_H

#include <stdint.h>

typedef void QueueOnUpdate(double t, void* data);
typedef struct QueueEntry {
    double time;
    int cb_id;
} QueueEntry;
typedef struct QueueContext {
    int sequential; // if true, only the first time matters
    QueueEntry* entries;
    QueueOnUpdate** update_cbs;
    void** update_udatas;
    int* free_udatas_on_deinit;
} QueueContext;
void queue_init(QueueContext* qc);
// if `data_size` is 0, then no heap copy will be allocated
// If `data_size` is the size of the `data`, then a heap copy will be allocated
int  queue_add_cb(QueueContext* qc, QueueOnUpdate cb, void* data, size_t data_size);
void queue_add_time(QueueContext* qc, double t, int cb_id);
void queue_update(QueueContext* qc, double t);
int  queue_isover(QueueContext* qc);
void queue_deinit(QueueContext* qc);

#endif // !CODIM_QUEUE_H
