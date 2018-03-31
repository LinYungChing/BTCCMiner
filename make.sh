#!/bin/bash
g++ -O3 -std=c++11 -o btc_pool btc_pool.cpp tcp_client.cpp json-parser/json.c json_utils.cpp stratum.cpp sha256.c -D_DEBUG_
