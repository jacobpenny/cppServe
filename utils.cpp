#include "utils.h"

#include <fcntl.h>

int make_non_blocking(int fd)
{
  int flags, s;

  flags = fcntl (fd, F_GETFL, 0);
  if (flags == -1) {
    return -1;
  }

  flags |= O_NONBLOCK;
  s = fcntl (fd, F_SETFL, flags);
  if (s == -1) {
    return -1;
  }

  return 0;
}
