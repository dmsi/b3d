#!/bin/bash

for example in $(ls -1 build/ | grep '^[0-9][0-9]_.\+') 
do
  ./build/$example
done
