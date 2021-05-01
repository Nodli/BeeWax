#!/bin/bash

ProjectDirectory=../../../project

pushd $(dirname $0)/bin > /dev/null

LSAN_OPTIONS=suppressions=$ProjectDirectory/ubuntu/asan_suppress.supp ./Application

ReturnCode=$?

popd > /dev/null

exit $ReturnCode
