#ifndef __POLLER_H__
#define __POLLER_H__

class Acceptor;
/* 
  This class blah blah

*/

class Poller {
  public:
    Poller(const Acceptor& acceptor);
    ~Poller();

    void listen_fd_handler(struct epoll_event &ev);
    void read_fd_handler(struct epoll_event &ev);
    void write_fd_handler(struct epoll_event &ev);
    void add_to_read_poll(int fd);
    void start();
    void print_data(int fd);

  private:
    const Acceptor& acceptor_;
    int epoll_fd_;
};

#endif
