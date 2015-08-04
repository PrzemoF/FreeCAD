#!/bin/bash

number_of_columns=$(cat $1 | awk --field-separator="|" "NR==1 { print NF }")
echo "number of columns" $number_of_columns
