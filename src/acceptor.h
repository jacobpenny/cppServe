#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include <vector>

class Connection;
struct sockaddr;

class Acceptor {
  public:
    Acceptor(const char *port);
    ~Acceptor();
    std::vector<Connection*> accept_connections() const;
    int get_listen_fd() const { return listen_fd_; }
  
  private:
    static void* get_in_addr(struct sockaddr *sa);
  
  private:
    int listen_fd_;
};


#endif
