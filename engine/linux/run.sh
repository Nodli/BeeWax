#!/bin/bash

cd $(dirname $0)/bin
P=$PWD/..
BinDir=$1
DataDir=$2

LSAN_OPTIONS=suppressions=${P}/asan_suppress.supp ./Application
