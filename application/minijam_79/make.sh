#!/bin/bash

pushd $(dirname $0) > /dev/null

../../project/ubuntu/make.sh source/unity.cpp source/unity_settings.cpp

ReturnCode=$?

popd > /dev/null

exit $ReturnCode
