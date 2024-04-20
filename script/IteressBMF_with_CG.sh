#!/usr/bin/env bash


for item in $(ls ../Data/Real/SimplIteress)
do
	if test -f ../Data/Real/SimplIteress/$item; then
		echo -e -n "$item: "
		timeout $1 python ../otherTool/CG/execut_CG2.py --t $1 ../Data/Real/SimplIteress/$item 


		if [ $? -eq 124 ]; then
		    echo "timeout"
		fi

	fi
done
