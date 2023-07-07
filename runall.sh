#!/bin/bash

for example in $(ls -1 build/examples/ | grep '^[0-9][0-9]_.\+')
do
  echo "RUNNING $example"
  ./build/examples/$example
done
