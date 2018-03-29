#!/bin/bash
g++ -O3 -std=c++11 -c tcp_client.cpp -o tcp_client.o
g++ -O3 -std=c++11 -c json-parser/json.c -o json.o
g++ -O3 -std=c++11 -c stratum.cpp -o stratum.o
g++ -O3 -std=c++11 -o stratum tcp_client.o json.o stratum.o
