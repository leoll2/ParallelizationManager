#!/bin/bash

for i in {1..3}
do
  echo ==================
  bin/test$i
done

echo ==================

