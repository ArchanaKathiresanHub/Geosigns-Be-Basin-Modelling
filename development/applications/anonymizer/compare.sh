#!/bin/sh
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ $# -ne 2 ]; then
  echo "ERROR: 2 inputs required, original and anonymized folders"
  exit 1
fi

DIR_ORIG=$1
DIR_ANON=$2

# Check original output folder existence
if [ ! -d "$DIR_ORIG" ]; then
  echo "ERROR: $DIR_ORIG does not exist"
  exit 1
fi

# Check anonymized output folder existence
if [ ! -d "$DIR_ANON" ]; then
  echo "ERROR: $DIR_ANON does not exist"
  exit 1
fi

# Check that the second input is the anonymzed folder and swapping them if necessary
if [ -z $(find $DIR_ANON -name "*namesMapping.txt" -print -quit) ]; then
  tmp=$DIR_ORIG
  DIR_ORIG=$DIR_ANON
  DIR_ANON=$tmp
fi

# Loop over .HDF files and removes anonymized attributes
for f in $(find $(find $DIR_ANON -type d -name "*_CauldronOutputDir") -type f -name "*.HDF");
do
  file2=$f
  $DIR/anonymizer -clear2DAttributes $file2 > /dev/null 2>&1
  file1=$(find $(find $DIR_ORIG -type d -name "*_CauldronOutputDir") -type f -name $(basename $f))
  if [ ! -f $file1 ]; then echo "WARNING: Cannot find $file1"; continue; fi
  $DIR/anonymizer -clear2DAttributes $file1 > /dev/null 2>&1
  NUM_SURF_PROP=$(h5dump -n $file2 | grep -E "dataset.*Layer" | wc -l)
  # Loop over datasets, which means all properties of all formations
  for p in $(h5dump -n $file2 | grep -E "dataset.*Layer" | grep -o "Layer.*");
  do
    NUM_SURF_PROP=$((NUM_SURF_PROP-1))
    echo -en "\r$(basename $file1) - Surface properties to be processed: $NUM_SURF_PROP    "
    # current anonymized formation name
    formation_anon=$(echo $p | cut -d_ -f4)
    # current original formation name
    # we look for the original name from the anonymized key in the namesMapping.txt file, whitespaces are replaced with underscores
    if [ ! -z "$formation_anon" ] && [ ! "$formation_anon" == "Mantle" ]; then
      formation_orig=$(cat $(find $DIR_ANON -type f -name "*namesMapping.txt") | grep "\"$formation_anon\"" | cut -f2 -d"\"" | sed -r 's/[ ]+/_/g');
    else
      formation_orig=$formation_anon
    fi
    # current anonymized surface name
    layer_anon=$(echo $p | cut -d_ -f3)
    # current original surface name
    # we look for the original name from the anonymized key in the namesMapping.txt file, whitespaces are replaced with underscores
    if [ ! -z "$layer_anon" ]; then layer_orig=$(cat $(find $DIR_ANON -type f -name "*namesMapping.txt") | grep "\"$layer_anon\"" | cut -f2 -d"\"" | sed -r 's/[ ]+/_/g'); fi
    data_orig=$p
    # we need the second condition for the special case of /Layer=SeepageBasinTop_Gas_0_ in HighResMigration_Results.HDF
    if [ ! -z "$layer_anon" ] && [ ! "$layer_anon" == "0" ]; then data_orig="$(echo $p | sed -e "s/$layer_anon/$layer_orig/g")"; fi
    if [ ! -z "$formation_anon" ] && [ ! "$formation_anon" == "0" ]; then data_orig="$(echo $data_orig | sed -e "s/$formation_anon/$formation_orig/g")"; fi
    data_orig="/$data_orig"
    if [[ ! -z $(echo $p | grep -E 'Bottom_of_Crust|Bottom_of_Lithospheric_Mantle') ]]; then data_orig="/$p"; fi
    data_anon="/$p"
    # Files comparison
    h5diff -p 0.01 $file1 $file2 $data_orig $data_anon
  done
  echo
done

# Loop over .h5 files
FILES=$(find $(find $DIR_ANON -type d -name "*_CauldronOutputDir") -type f -name "*.h5" -printf "%f\n")
NUM_FILES=$(find $(find $DIR_ANON -type d -name "*_CauldronOutputDir") -type f -name "*.h5" -printf "%f\n" | wc -l)
echo -en "Volume files to be processed: "
for f in $FILES;
do
  NUM_FILES=$((NUM_FILES-1))
  echo -en "\rVolume files to be processed: $NUM_FILES    "
  file1=$(find $DIR_ORIG -type d -name "*_CauldronOutputDir")/$f
  if [ ! -f $file1 ]; then echo "WARNING: Cannot find $file1"; continue; fi
  file2=$(find $DIR_ANON -type d -name "*_CauldronOutputDir")/$f
  if [ ! -f $file2 ]; then echo "WARNING: Cannot find $file2"; continue; fi
  # Loop over datasets, which means all properties of all formations
  for p in $(h5dump -n $file2 | grep dataset | grep -o "/.*");
  do
    # current property name
    property=$(echo $p | cut -d/ -f2)
    # current anonymized formation name
    formation_anon=$(echo $p | cut -d/ -f3)
    # current original formation name
    # we look for the original name from the anonymized key in the namesMapping.txt file, whitespaces are replaced with underscores
    if [ ! -z "$formation_anon" ] && [ ! "$formation_anon" == "Mantle" ]; then
      formation_orig=$(cat $(find $DIR_ANON -type f -name "*namesMapping.txt") | grep "\"$formation_anon\"" | cut -f2 -d"\"" | sed -r 's/[ ]+/_/g');
    else
      formation_orig=$formation_anon
    fi
    # Files comparison
    out=$(h5diff -p 0.01 $file1 $file2 /$property/$formation_orig /$property/$formation_anon)
    if [[ ! -z $(echo $out | grep "differences found") ]]; then
      # Heatflow differences might need special care
      if [[ ! -z $(echo $out | grep HeatFlow) ]]; then
        out=$(h5diff -p 0.05 $file1 $file2 /$property/$formation_orig /$property/$formation_anon)
      fi
      if [ ! -z "$out" ]; then
        echo "File: $f, property: $property, original name: $formation_orig, anonymized name: $formation_anon"
        echo $out;
      fi
    else
      if [ ! -z "$out" ]; then
        echo "File: $f, property: $property, original name: $formation_orig, anonymized name: $formation_anon"
        echo $out;
      fi
    fi
  done
done

echo
echo "No differences found!"
