#ifndef __POLLER_H__
#define __POLLER_H__

class Acceptor;
class Connection;

class Poller {
  public:
    Poller(const Acceptor& acceptor);
    ~Poller();

    void listen_fd_handler();
    void read_fd_handler(struct epoll_event &ev);
    void write_fd_handler(struct epoll_event &ev);
    void add_to_read_poll(Connection*);
    void start();
    void print_data(int fd) const;

  private:
    const Acceptor& acceptor_;
    int epoll_fd_;
};

#endif
