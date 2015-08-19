#!/bin/bash
# That script creates Materials.ods file from all FCMat files in current directory

FCMAT_DIR="../"
MATERIALS_FILE_NAME=Materials

# Remove exisitng Material.csv
rm $MATERIALS_FILE_NAME.csv

# Create new Materials.csv from all FCMat files
ls $FCMAT_DIR*.FCMat | xargs -I [] ./FCMat2csv.sh [] && ls *.csv | xargs -I [] cat [] | awk "NR==1; NR%2 == 0" > $MATERIALS_FILE_NAME.csv

# Convert Materials.csv to Materials.ods
echo "Creating "$MATERIALS_FILE_NAME.ods
unoconv -i FilterOptions=124,34,76 -f ods $MATERIALS_FILE_NAME.csv

# Remove all temporary files
rm *.csv
