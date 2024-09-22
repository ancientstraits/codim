#include "queue.h"

// #define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
// #include "error.h"
// #define QASSERT(cond, __VA_ARGS__) ASSERT(cond, ERROR_QUEUE)


void queue_init(QueueContext *qc) {
}

int queue_add_cb(QueueContext *qc, QueueOnUpdate *cb, void* data, size_t data_size) {
    for (size_t i = 0; i < arrlen(qc->update_cbs); i++) {
        if (qc->update_cbs[i] == cb)
            return i;
    }
    arrput(qc->update_cbs, cb);

    if (data_size == 0) {
        arrput(qc->update_udatas, data);
        arrput(qc->free_udatas_on_deinit, 0);
    } else {
        void* heap_copy = malloc(data_size);
        memcpy(heap_copy, data, data_size);

        arrput(qc->update_udatas, heap_copy);
        arrput(qc->free_udatas_on_deinit, 0);
    }


    return arrlen(qc->update_cbs) - 1;
}

void queue_add_time(QueueContext* qc, double t, int cb_id) {
    QueueEntry qe = {.time = t, .cb_id = cb_id};
    if (arrlen(qc->entries) == 0 || qc->entries[arrlen(qc->entries)-1].time <= t) {
        arrput(qc->entries, qe);
    } else for (int i = arrlen(qc->entries)-2; i >= 0; i--) {
        if (qc->entries[i].time < t) {
            arrins(qc->entries, i+1, qe);
            return;
        }
    }
    arrins(qc->entries, 0, qe);
}

void queue_update(QueueContext* qc, double t) {
    QueueEntry last_entry = qc->entries[arrlen(qc->entries)-1];
    if (last_entry.time >= t) {
        arrpop(qc->entries);
        qc->update_cbs[last_entry.cb_id](t, qc->update_udatas[last_entry.cb_id]);
    }
}

void queue_deinit(QueueContext* qc) {
    for (int i = 0; i < arrlen(qc->update_udatas); i++) {
        if (qc->free_udatas_on_deinit[i])
            free(qc->update_udatas[i]);
    }

    arrfree(qc->entries);
    arrfree(qc->update_cbs);
    arrfree(qc->update_udatas);
}
