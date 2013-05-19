#include "poller.h"
#include "acceptor.h"
#include "connection.h"
#include "worker.h"
#include "threadsafe_queue.h"

#include <thread>

int main() 
{


  threadsafe_queue<Connection*> work_queue;
  std::vector<std::thread> threads;
  Acceptor acceptor("3000"); // port 3000
  Poller poller(acceptor, work_queue); 
  
  for(int i = 0; i < 4; ++i) {
    threads.push_back(std::thread(Worker(work_queue, poller)));
  }
  


  poller.start();

  for(auto& thread : threads) {
    thread.join();
  }
  return 0;
}
