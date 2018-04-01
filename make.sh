#!/bin/bash
g++ -O3 -std=c++11 -I./lib/ -o example001 example001.cpp lib/btc_pool.cpp lib/tcp_client.cpp lib/json-parser/json.c lib/json_utils.cpp lib/stratum.cpp sha256.c

g++ -O3 -std=c++11 -I./lib/ -o example002 example002.cpp lib/btc_pool.cpp lib/tcp_client.cpp lib/json-parser/json.c lib/json_utils.cpp lib/stratum.cpp sha256.c
