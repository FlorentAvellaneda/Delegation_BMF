#!/usr/bin/env bash


echo -e "Dataset\trank\tTime"

for item in $(ls ../Data/Real/SimplIteress)
do
	if test -f ../Data/Real/SimplIteress/$item; then
		echo -e -n "$item: "
		timeout $1 ../otherTool/undercoverBMF/build/inferbmf --OptiBlock -k 100000 -v 0 fromFile ../Data/Real/SimplIteress/$item

		if [ $? -eq 124 ]; then
		    echo "timeout"
		fi

	fi
done
