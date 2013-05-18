#include "poller.h"
#include "acceptor.h"

int main() 
{
  Acceptor a("3000");
  Poller p(a);
  p.start();

  return 0;
}
