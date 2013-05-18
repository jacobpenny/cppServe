#include "poller.h"
#include "acceptor.h"
#include "connection.h"
#include "threadsafe_queue.h"

#include <thread>

int main() 
{
  threadsafe_queue<Connection*> work_queue;
  Acceptor a("3000");
  Poller p(a, work_queue);

  p.start();

  return 0;
}
