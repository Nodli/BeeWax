#!/bin/bash

ProjectDirectory=../../project

pushd $(dirname $0) > /dev/null

$ProjectDirectory/ubuntu/make.sh source/unity.cpp source/unity_settings.cpp

ReturnCode=$?

popd > /dev/null

exit $ReturnCode
