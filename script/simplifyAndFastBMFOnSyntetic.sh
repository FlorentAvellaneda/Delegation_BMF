#!/usr/bin/env bash


echo -e "Dataset\t#line\t#col\t#ones\t#afterUni\t#afterExi\ttimeUni\ttimeExi\trank after fastBMF\t#Error\tTotalTime"

for item in $(ls ../Data/Synthetic)
do
	if test -f ../Data/Synthetic/$item; then
		../build/Simpli --uni --exi -v 0 ../Data/Synthetic/$item
	fi
done
