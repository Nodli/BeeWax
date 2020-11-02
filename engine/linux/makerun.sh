#!/bin/bash

clear
cd $(dirname $0)

echo "-------- Compilation"
if ./make.sh; then
echo "-------- Execution"
    ./run.sh
fi
