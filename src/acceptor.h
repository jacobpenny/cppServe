#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include <vector>

struct sockaddr;

class Acceptor {
  public:
    Acceptor(const char *port);
    ~Acceptor();

    static void* get_in_addr(struct sockaddr *sa);
    
    // Accepts a connection and returns associated non-blocking file-descriptor
    std::vector<int> accept_connections() const;
    int get_listen_fd() const { return listen_fd_; }

  private:
    int listen_fd_;
};


#endif
