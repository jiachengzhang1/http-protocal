#!/bin/bash

EXE=./http_get

if [ ! -z "$1" ] && [ ! -z "$2" ] && [ ! -z "$3" ]; then
    req_num=$1
    site1=$2
    site2=$3
else
    echo "Perform default testing."
    req_num=100
    site1=http://assignment.jiachengzhang.workers.dev
    site2=http://static-links-page.signalnerve.workers.dev
fi

if [[ ! -f "$EXE" ]]; then
    echo "ERROR: executable \"http_get\" is not found."
else
    printf "Sites: \"$site1\" and \"$site2\"\nNumber of requests: $req_num\n"
    ./http_get --profile $req_num $site1 > profile.txt
    printf "\n" >> profile.txt
    ./http_get --profile $req_num $site2 >> profile.txt
    echo "Done, results can be found in \"profile.txt\"."
fi