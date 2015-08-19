# Script that converts FCMat files into csv file

FILE=$1
FILE_BASENAME=$(basename $FILE)
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

echo "Processing "${FILE_BASENAME%%.FCMat}
echo "FileName = "${FILE_BASENAME%%.FCMat} > $FILE_BASENAME.new
while read -r property || [[ -n "$property" ]]; do
	value=$(cat $FILE | awk '{if ($1!="") print $0; else "X" }' | grep -w $property | cut -f3- -d " ")
	echo $property' = '$value >> $FILE_BASENAME.new
done < headers

echo "FileName" | tr '\n' "|" > $FILE_BASENAME.csv
cat headers | tr '\n' "|" | sed 's/|$/\n/' >> $FILE_BASENAME.csv
cat $FILE_BASENAME.new | sed 's/^[\[,;].*//' | sed '/^$/d' | cut -d"=" -f2- | cut -d " " -f2- | sed 's/^/\"/' | sed 's/$/\"/' | tr '\n' "|" | sed 's/|$/\n/'  >> $FILE_BASENAME.csv

rm $FILE_BASENAME.new
