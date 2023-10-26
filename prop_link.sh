#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage: $0 <dir> <file_out>"
    exit 1
fi

director=$1
file_out=$2
aux=""
nr_leg=0
nr_leg1=0

if [ ! -d "$director" ]; then
    echo "Invalid folder"
    exit 1
fi

if [ -e "$file_out" ]; then
    if [ ! -f "$file_out" ]; then
        echo "Corrupted existing output file $file_out"
        exit 1
    fi
else
    touch $file_out
fi

for entity in "$director"/*; do
    if [ -f "$entity" ] && [[ "$entity" == *.txt ]]; then
        aux=$(cat $entity | grep -E "^[A-Z][a-zA-Z0-9 ,]+\.$" | grep -E -v "si[ ]*\," | grep -E -v "n[pb]")
        if [ ! -z "$aux" ]; then
            echo "File name: $entity" >> "$file_out"
        aux=""
        fi
    elif [ -L "$entity" ]; then
        nr_leg=$((nr_leg + 1))
    elif [ -d "$entity" ]; then
        nr_leg1=`bash $0 $entity $file_out`
        nr_leg=$((nr_leg + $nr_leg1))
    fi
done

echo $nr_leg