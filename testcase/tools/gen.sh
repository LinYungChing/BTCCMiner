#!/bin/bash

OUTPUT=$1
ROOT=$2

FILE_LIST=($(ls -- ${ROOT%/}/*.json))
set -x
./converter "${OUTPUT}" "${FILE_LIST[@]}"
set +x
