#!/usr/bin/env bash


for item in $(ls ../Data/Real/SimplUni)
do
	if test -f ../Data/Real/SimplUni/$item; then
		echo -e -n "$item: "
		timeout $1 python ../otherTool/CG/execut_CG2.py --t $1 ../Data/Real/SimplUni/$item 


		if [ $? -eq 124 ]; then
		    echo "timeout"
		fi

	fi
done
