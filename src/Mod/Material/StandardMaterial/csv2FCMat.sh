#!/bin/bash
FCMat_output_dir="FCMat_from_csv"
mat_file=$1

function get_xy() {
	value_xy=$(cat $mat_file | awk -v x=$x -v y=$y -F\| 'NR==y {print $x}' | sed 's/\"//g')
}

number_of_columns=$(cat $mat_file | awk --field-separator="|" "NR==1 { print NF }")
#echo "number of columns" $number_of_columns

number_of_rows=$(cat $mat_file | wc -l)
#echo "number of rows" $number_of_rows

mkdir $FCMat_output_dir
rm $FCMat_output_dir/*

for mat_no in $(seq 2 $number_of_rows)
do
	x=1
	y=$mat_no
	get_xy
	file_name=$value_xy
	echo "Generating material file: " $FCMat_output_dir/$file_name.FCMat
	touch $FCMat_output_dir/$file_name.FCMat

	for x in $(seq 2 $number_of_columns)
	do
		y=1
		get_xy
		property=$value_xy
		y=$mat_no
		get_xy
		value=$value_xy
		if [ -n "$value_xy" ]
		then
			echo $property = $value >> $FCMat_output_dir/$file_name.FCMat;
		fi
	done
done
