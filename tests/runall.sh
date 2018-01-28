#!/bin/bash

for test in $(ls -1 build/ | grep '^test_.\+') 
do
  ./build/$test
done
