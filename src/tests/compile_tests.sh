#!/bin/bash

g++ -std=c++0x -I gtest-1.6.0/include/ ../acceptor.cpp ../utils.cpp ../connection.cpp acceptor_test.cpp gtest-1.6.0/src/gtest_main.cc gtest-1.6.0/libgtest.a -lpthread -o acceptor_test
g++ -std=c++0x -I gtest-1.6.0/include/ ../acceptor.cpp ../utils.cpp ../poller.cpp ../connection.cpp poller_test.cpp gtest-1.6.0/src/gtest_main.cc gtest-1.6.0/libgtest.a -lpthread -o poller_test
g++ -std=c++0x -I gtest-1.6.0/include/ ../request.cpp request_test.cpp gtest-1.6.0/src/gtest_main.cc gtest-1.6.0/libgtest.a -lpthread -o request_test



