#!/usr/bin/env bash


echo -e "Dataset  \tSimplification"

for item in $(ls ../Data/Real/dat)
do
	if test -f ../Data/Real/dat/$item; then
		echo -e -n "$item\t"
		../otherTool/iteress-ins/iteressShowSimplfication -n0 ../Data/Real/dat/$item > /dev/null
	fi
done
