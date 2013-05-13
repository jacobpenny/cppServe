#ifndef CONNECTION_POLLER_H
#define CONNECTION_POLLER_H

#include<string>

/* 
  This class blah blah

*/

class ConnectionPoller {
  public:
    ConnectionPoller(int listen_port);
    ~ConnectionPoller();


    void listen_fd_handler(struct epoll_event &ev);
    void read_fd_handler(struct epoll_event &ev);
    void write_fd_handler(struct epoll_event &ev);

    void add_to_read_poll(int fd, std::string address);

    void start();

  private:
    ConnectionPoller(const ConnectionPoller& other);
    void init_listen_fd();
    void *get_in_addr(struct sockaddr *sa); 

  private:
    int listen_port_;
    int listen_fd_;
    int epoll_fd_;
    //    struct epoll_event listen_fd_event_;
    //    struct epoll_event *events_;

};

#endif
