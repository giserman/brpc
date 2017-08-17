// bthread - A M:N threading library to make applications more concurrent.
// Copyright (c) 2017 Baidu.com, Inc. All Rights Reserved

// Author: Ge,Jun (gejun@baidu.com)
// Date: Sun, 22 Jan 2017

#ifndef BAIDU_BTHREAD_REMOTE_TASK_QUEUE_H
#define BAIDU_BTHREAD_REMOTE_TASK_QUEUE_H

#include "base/containers/bounded_queue.h"
#include "base/macros.h"

namespace bthread {

class TaskGroup;

// A queue for storing bthreads created by non-workers. Since non-workers
// randomly choose a TaskGroup to push which distributes the contentions,
// this queue is simply implemented as a queue protected with a lock.
// The function names should be self-explanatory.
class RemoteTaskQueue {
public:
    RemoteTaskQueue() {}

    int init(size_t cap) {
        const size_t memsize = sizeof(bthread_t) * cap;
        void* q_mem = malloc(memsize);
        if (q_mem == NULL) {
            return -1;
        }
        base::BoundedQueue<bthread_t> q(q_mem, memsize, base::OWNS_STORAGE);
        _tasks.swap(q);
        return 0;
    }

    bool pop(bthread_t* task) {
        if (_tasks.empty()) {
            return false;
        }
        _mutex.lock();
        const bool result = _tasks.pop(task);
        _mutex.unlock();
        return result;
    }

    bool push(bthread_t task) {
        _mutex.lock();
        const bool res = push_locked(task);
        _mutex.unlock();
        return res;
    }

    bool push_locked(bthread_t task) {
        return _tasks.push(task);
    }

    size_t capacity() const { return _tasks.capacity(); }
    
private:
friend class TaskGroup;
    DISALLOW_COPY_AND_ASSIGN(RemoteTaskQueue);
    base::BoundedQueue<bthread_t> _tasks;
    base::Mutex _mutex;
};

}  // namespace bthread

#endif  // BAIDU_BTHREAD_REMOTE_TASK_QUEUE_H