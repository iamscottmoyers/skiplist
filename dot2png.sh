#!/bin/bash

# If there are no dot files avoid iterating over the list '*.dot'
shopt -s nullglob

for i in *.dot
do
    dot -Tpng $i > `basename $i .dot`.png
done
