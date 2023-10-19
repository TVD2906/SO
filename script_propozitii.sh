#!/bin/bash

while read linie
do
    echo $linie | grep -E "^[A-Z][a-zA-Z0-9 ,\-]+\.$" | grep -E -v "(si[ ]*\,)|(Si[ ]*\,)" | grep -E -v "n[pb]"
done

