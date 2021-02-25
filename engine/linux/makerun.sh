#!/bin/bash

clear
cd $(dirname $0)

echo "-------- Compilation"

./make.sh
ReturnCode=$?

# GCC exists with the code of 1 if any phase of the compiler returns a non-success return code.
if [ $ReturnCode == 0 ] ; then
    echo "-------- Execution"
    ./run.sh
fi

echo "-------- Finished"
