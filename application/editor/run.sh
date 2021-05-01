#!/bin/bash

ProjectDirectory=../../project

pushd $(dirname $0) > /dev/null

LSAN_OPTIONS=suppressions=$ProjectDirectory/ubuntu/asan_suppress.supp ./bin/Application

ReturnCode=$?

popd > /dev/null

exit $ReturnCode
