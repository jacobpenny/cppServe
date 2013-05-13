#include "connectionpoller.h"
#include "./boost/threadpool.hpp"
#include <thread>
#include <functional>
#include <iostream>

using namespace std;
using namespace boost::threadpool;

const int PORT = 5656;



int main() {
  ConnectionPoller poller(PORT);
  poller.start();

  pool tp(2);
  //thread acceptThread(std::ref(acceptor));
 
  //acceptThread.join();
}
