#!/bin/bash

cd $(dirname $0)
P=$PWD/..
BinDir=$1
DataDir=$2

echo -------- Mirroring data

mkdir -p ${BinDir}
cp -r -u ${P}/data/ ${P}/linux/${BinDir}/${DataDir}

echo -------- Finished mirroring data
