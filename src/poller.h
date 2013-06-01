#ifndef __POLLER_H__
#define __POLLER_H__

#include "threadsafe_queue.h"

class Acceptor;
class Connection;
struct epoll_event;

class Poller {
  public:
    Poller(const Acceptor& a, threadsafe_queue<Connection*>& q);
    ~Poller();

    void listen_fd_handler();
    
    void add_to_read_poll(Connection*);
    void rearm_read(Connection*);
    void rearm_write(Connection*); 
    void remove(Connection*); 
    void start();

  private:
    const Acceptor& acceptor_;
    threadsafe_queue<Connection*>& work_queue_;
    int epoll_fd_;
    struct epoll_event *events_;
};

#endif
