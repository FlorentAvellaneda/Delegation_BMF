#!/usr/bin/env bash

mkdir Uni

for item in $(ls ../Data/Real)
do
	if test -f ../Data/Real/$item; then
		echo -e -n "$item: "
		../build/Simpli --uni  -v 0 -S Uni/$item ../Data/Real/$item > /dev/null
		echo "OK"
	fi
done
