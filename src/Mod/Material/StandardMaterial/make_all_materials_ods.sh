#!/bin/bash
rm Materials.csv
ls *.FCMat | xargs -I [] ./FCMat2csv.sh [] && ls *.csv | xargs -I [] cat [] | awk 'NR==1; NR%2 == 0' > Materials.csv
unoconv -i FilterOptions=124,34,76 -f ods Materials.csv

ls *.FCMat | xargs -I [] rm [].new
ls *.FCMat | xargs -I [] rm [].csv
