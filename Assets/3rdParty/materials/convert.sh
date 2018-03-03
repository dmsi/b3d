#!/bin/bash

remote=ds@192.168.100.80
source=$1
destination=$2

scp $source $remote:.  > /dev/null
ssh $remote "convert $source $destination"
scp $remote:$destination . > /dev/null
#cleanup
ssh $remote "rm $source $destination"
