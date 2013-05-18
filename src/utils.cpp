#include "utils.h"

#include <fcntl.h>

int make_non_blocking(int fd)
{
  int flags = fcntl(fd, F_GETFL, 0);
  if (-1 == flags) {
    return -1;
  }
  flags |= O_NONBLOCK;
  int rv = fcntl (fd, F_SETFL, flags);
  if (-1 == rv) {
    return -1;
  }
  return 0;
}
