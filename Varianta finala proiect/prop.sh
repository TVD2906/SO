#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage error"
    exit 1
fi

character=$1
counter=0

while read linie
do
  echo $linie | grep -qE "^[A-Z][a-z0-9 ,]+[\.\?\!]$" && 
  echo $linie | grep -qE -v "\,si[ ]*" && 
  echo $linie | grep -qE -v "n[pb]" &&
  echo $linie | grep -qE $character &&
    ((counter++))
done

exit $counter