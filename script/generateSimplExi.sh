#!/usr/bin/env bash

mkdir Exi

for item in $(ls ../Data/Real)
do
	if test -f ../Data/Real/$item; then
		echo -e -n "$item: "
		../build/Simpli --uni --exi -v 0 -S Exi/$item ../Data/Real/$item > /dev/null
		echo "OK"
	fi
done
