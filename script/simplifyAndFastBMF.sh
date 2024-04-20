#!/usr/bin/env bash


echo -e "Dataset\t#line\t#col\t#ones\t#afterUni\t#afterExi\ttimeUni\ttimeExi\trank after fastBMF\t#Error\tTotalTime"

for item in $(ls ../Data/Real)
do
	if test -f ../Data/Real/$item; then
		../build/Simpli --uni --exi -v 0 ../Data/Real/$item
	fi
done
