#!/bin/bash

if [ $# -ne 0 ]; then
    echo "Incorrect usage"
    exit 1
fi


while read linie
do
  echo $linie | grep -E "^((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\.){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])$"
done
