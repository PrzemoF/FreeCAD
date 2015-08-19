#!/bin/bash
# That script converts Materials.csv file into FCMat files. The result files are saved in FCMat_output_dir
# The csv file has to be command line parameter

FCMat_output_dir="FCMat_from_csv"
mat_file=$1

# Helper function to retrive string from x,y position in csv file
function get_xy() {
	value_xy=$(cat $mat_file | awk -v x=$x -v y=$y -F\| 'NR==y {print $x}' | sed 's/\"//g')
}

#Determine number of columns and rows in the Materials.csv file
number_of_columns=$(cat $mat_file | awk --field-separator="|" "NR==1 { print NF }")
number_of_rows=$(cat $mat_file | wc -l)

if [ -d "$FCMat_output_dir" ]
then
	rm "$FCMat_output_dir"/*
else
	mkdir $FCMat_output_dir
fi

for mat_no in $(seq 2 $number_of_rows)
do
	x=1
	y=$mat_no
	get_xy
	file_name=$value_xy
	echo "Generating material file: " $FCMat_output_dir/$file_name.FCMat
	touch $FCMat_output_dir/$file_name.FCMat

	if [ -f "../$file_name.FCMat" ]
	then
		head -n 5 ../$file_name.FCMat > $FCMat_output_dir/$file_name.FCMat
	fi

	echo "[FCMat]" >> $FCMat_output_dir/$file_name.FCMat

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
