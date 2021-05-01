#!/bin/bash

DestinationDirectory=$1
SourceDirectory=$2

echo -------- Mirroring data

echo "Source:       " $SourceDirectory
echo "Destination:  " $DestinationDirectory

mkdir -p $DestinationDirectory
if [[ -d $SourceDirectory ]];
then
    cp -r -u $SourceDirectory $DestinationDirectory
else
    echo "WARNING: Source folder does not exist"
fi

echo -------- Finished mirroring data
