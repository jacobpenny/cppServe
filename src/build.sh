#!/bin/bash

g++ -std=c++0x worker.cpp utils.cpp connection.cpp acceptor.cpp poller.cpp request.cpp main.cpp -pthread -o cppServe -g -Wall -Wextra -pedantic
