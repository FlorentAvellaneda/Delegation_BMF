#!/usr/bin/env bash


for item in $(ls ../Data/Real)
do
	if test -f ../Data/Real/$item; then
		echo -e -n "$item: "
		timeout $1 python ../otherTool/CG/execut_CG2.py --t $1 ../Data/Real/$item 


		if [ $? -eq 124 ]; then
		    echo "timeout"
		fi

	fi
done
