#!/bin/bash

ProjectDirectory=../../project

pushd $(dirname $0) > /dev/null

$ProjectDirectory/ubuntu/make.sh source/unit_unity.cpp

ReturnCode=$?

popd > /dev/null

exit $ReturnCode