#!/bin/bash

#for run in {1..1000}
#do
#  echo $run
#  ./main
#done

for i in {1..3}
do
  echo ==================
  bin/test$i
done

echo ==================

