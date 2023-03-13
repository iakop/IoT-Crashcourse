#!/bin/bash

# Call: dropshadow.sh [inputfile] [deviation] [dx] [dy] [slope] [outputfile]
inputfile=$1
deviation=$2 # 3
dx=$3 # 4
dy=$4 # 4
slope=$5 # 0.7
outputfile=$6

cat > /tmp/add.txt << EOF1
<defs>
<filter id="dropshadow">
  <feGaussianBlur in="SourceAlpha" stdDeviation="$deviation"/> 
  <feOffset dx="$dx" dy="$dy"/>
  <feComponentTransfer>
    <feFuncA type="linear" slope="$slope"/>
  </feComponentTransfer>
  <feMerge> 
    <feMergeNode/>
    <feMergeNode in="SourceGraphic"/> 
  </feMerge>
</filter>
</defs>
<g partID='all' filter="url(#dropshadow)">

EOF1
# First grep for the svg start (One occurence of <svg>):
sed '/<svg>/r /tmp/add.txt' $inputfile > /tmp/tmp.svg

# Then grep for the end of svg (simply search for the </svg>)
sed -i 's/<\/svg>/<\/g><\/svg>/' /tmp/tmp.svg

#inkscape --export-type=svg --export-area-drawing /tmp/tmp.svg -o $outputfile