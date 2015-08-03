# Script that converts FCMat files into csv file

FILE=$1
if [ "$1" = "TEMPLATE.FCMat" ]
then
	echo "Skipping TEMPLATE.FCMat";
	exit 0
fi

if [ -f "$1.new" ]
then
	rm $1.new
fi

if [ -f "$1.ods" ]
then
	rm $1.ods
fi

if [ -f "$1.csv" ]
then
	rm $1.csv
fi

while read -r property || [[ -n "$property" ]]; do
	value=$(cat $1 | awk '{if ($1!="") print $0; else "X" }' | grep -w $property | cut -f3- -d " ")
	echo $property' = '$value >> $1.new
done < header.txt


cat header.txt | tr '\n' "|" | sed 's/|$/\n/' > $1.csv
cat $1.new | sed 's/^[\[,;].*//' | sed '/^$/d' | cut -d"=" -f2- | cut -d " " -f2- | sed 's/^/\"/' | sed 's/$/\"/' | tr '\n' "|" | sed 's/|$/\n/'  >> $1.csv

