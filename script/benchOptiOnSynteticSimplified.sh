#!/usr/bin/env bash


echo -e "k\td\tFile_matrix     \toptk\tErr\tTime (sec)"

for k in 20 30 40; do
	for d in 0.1 0.2 0.3 0.4; do
	    echo -e -n "$k\t$d\t"
	    ../build/RdmMatrix -d $d -k $k -U 1000 -V 500 > /tmp/tmp.csv
	    ../build/Simpli --uni -S /tmp/tmpSimple.csv /tmp/tmp.csv > /dev/null 2> /dev/null
	    ../otherTool/undercoverBMF/build/inferbmf -v 0 -k $k --optimal fromFile /tmp/tmpSimple.csv
	done
done
