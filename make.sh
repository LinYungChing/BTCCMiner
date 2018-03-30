#!/bin/bash
g++ -O3 -std=c++11 -o stratum tcp_client.cpp json-parser/json.c json_utils.cpp stratum.cpp sha256.c
