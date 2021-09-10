#!/bin/bash

pushd $(dirname $0)/bin > /dev/null

LSAN_OPTIONS=suppressions=../../../project/ubuntu/asan_suppress.supp ./Application

ReturnCode=$?

popd > /dev/null

exit $ReturnCode
