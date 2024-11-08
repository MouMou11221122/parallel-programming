#!/bin/bash

max_num=""
rm -rf ESOE7045.* 
echo "---------------------------------------------------------------------"
for i in {1..8}
do
    tmp_num=$max_num
    qsub -v nproc=$i pbs.sh > /dev/null

    max_num=$(ls ESOE7045.o* 2>/dev/null | sed -E 's/.*o([0-9]+)$/\1/' | sort -n | tail -n 1)

    if [ -z "$tmp_num" ]; then
        while [ -z "$max_num" ]; do
            sleep 0.5
            max_num=$(ls ESOE7045.o* 2>/dev/null | sed -E 's/.*o([0-9]+)$/\1/' | sort -n | tail -n 1)
        done
    else
        while [ "$max_num" == "$tmp_num" ]; do
            sleep 0.5
            max_num=$(ls ESOE7045.o* 2>/dev/null | sed -E 's/.*o([0-9]+)$/\1/' | sort -n | tail -n 1)
        done
    fi

    cat "ESOE7045.o$max_num"
    echo "---------------------------------------------------------------------"
done

