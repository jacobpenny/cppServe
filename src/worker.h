#ifndef __WORKER_H__
#define __WORKER_H__

#include "threadsafe_queue.h"

class Connection;
class Poller;

class Worker {
  public:
    Worker(threadsafe_queue<Connection*> &q, Poller &p) : queue_(q), poller_(p) {} 
    void operator()();

  private:
    void read_data(Connection*) const;
    
    threadsafe_queue<Connection*> &queue_;
    Poller &poller_;
};

#endif