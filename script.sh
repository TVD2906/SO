#!/bin/bash

if [ "$#" -lt 3 ]; then
    echo "Insuficient arguments!"
    exit 1
fi
filename=$1
dirname=$2
shift 2
k=0
sum=0

for arg in "$@"
do
    if [ $arg -gt 10 ]; then
	k=$(($k+1))
    fi
    sum=$(($sum+$arg))
done

if [ ! -e $filename ]; then
    echo "k = $k; sum = $sum" > $filename
else
    if [ -f $filename ]; then
	echo "k = $k; sum = $sum" > $filename
    else
	echo "Invalid file"
	exit 1
    fi
fi

digits_sum=0 
digits_sum=$((${#sum}))
echo "digits_sum = $digits_sum"

for fisier in "$dirname"/*.txt; do
    if [ ! -f $fisier ]; then
	echo "File not good: $fisier"
	exit 1
    fi

    cat $fisier
done

