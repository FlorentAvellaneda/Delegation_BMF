#!/usr/bin/env bash


echo -e "Dataset  \tSimplification"

for item in $(ls ../Data/Synthetic/dat)
do
	if test -f ../Data/Synthetic/dat/$item; then
		echo -e -n "$item\t"
		../otherTool/iteress-ins/iteressShowSimplfication -n0 ../Data/Synthetic/dat/$item > /dev/null
	fi
done
