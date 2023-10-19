#!/bin/bash

if [ "$#" -ne 2 ]; then
	echo "Argumente Insuficiente! "
	exit 1
fi

director="$1"
output_file="$2"

if [ ! -d "$director" ]; then
	echo "Directorul nu exista."
	exit 1
fi

> "$output_file"

total_caractere=0
find "$director" -type f -name "*.txt" | while read -r file; do
	numar_caractere=$(wc -c < "$file")
	echo "$file $numar_caractere" >> "$output_file"
	total_caractere=$((total_caractere + numar_caractere))
done

echo "TOTAL $total_caractere" >> "$output_file"
echo "Operatiune finalizata. Rezultatele au fost scrise in $output_file."
