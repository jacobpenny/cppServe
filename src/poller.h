#ifndef __POLLER_H__
#define __POLLER_H__

#include "threadsafe_queue.h"

class Acceptor;
class Connection;

class Poller {
  public:
    Poller(const Acceptor& a, threadsafe_queue<Connection*>& q);
    ~Poller();

    void listen_fd_handler();
    void read_fd_handler(struct epoll_event &ev);
    void write_fd_handler(struct epoll_event &ev);
    
    void add_to_read_poll(Connection*);
    void remove_from_read_poll(Connection*);
    void rearm_read(Connection*);
    
    void add_to_write_poll(Connection*);
    void remove_from_write_poll(Connection*);
    void rearm_write(Connection*); 
    void switch_to_write_poll(Connection*); 
    
    void remove(Connection*); 
    void start();

  private:
    const Acceptor& acceptor_;
    threadsafe_queue<Connection*>& work_queue_;
    int epoll_fd_;
};

#endif
