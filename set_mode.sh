#!/bin/bash

if [ "$#" -ne 2 ]; then
	echo "Argumente Insuficiente!"
	exit 1
fi

director="$1"
caracter="$2"

if [ ! -d "$director" ]; then
	echo "Directorul specificat nu exista."
	exit 1
fi

for file in "$director"/*.txt; do
	if [ -f "$file" ]; then
		chmod "+$caracter" "$file"
		echo "Am setat dreptul '$caracter' pentru $file"
	fi
done
for file in "$director"/*; do
	echo $file
	if [ -d "$file" ]; then
		bash $0 $file $2
	fi 
done
echo "Operatiune finalizata."
