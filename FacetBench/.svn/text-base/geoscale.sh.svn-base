#!/bin/bash
# Copyright (c) 2012, CWI (www.cwi.nl). Author: Peter Boncz.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
#    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
#    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
SCALE=`expr $1 - 1`

(zcat LGD-Dump-Ontology.nt.gz 
 for i in $(seq 1 $SCALE)
 do
  zcat LGD-Dump-Ontology.nt.gz | 
       awk "{ if (\$2 == \"<http://www.w3.org/2000/01/rdf-schema#subClassOf>\") { print substr(\$1,1,length(\$1)-1) \"$i> \" \$2 \" \" substr(\$3,1,length(\$3)-1) \"$i> .\" } else { print substr(\$1,1,length(\$1)-1) \"$i> \" substr(\$0,length(\$1)+1) }}" 
 done) | gzip > LGD-Dump-Ontology.$1.nt.gz&

(zcat LGD-Dump-RelevantNodes.sorted.nt.gz 
 for i in $(seq 1 $SCALE)
 do
  zcat LGD-Dump-RelevantNodes.sorted.nt.gz | 
       awk "BEGIN { srand($i); rlon = (rand() - 0.5) / 10000; rlat = (rand() - 0.5) / 20000; } \
           { \
              if (\$2 == \"<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>\") { \
                 print substr(\$1,1,length(\$1)-1) \"$i> \" \$2 \" \" substr(\$3,1,length(\$3)-1) \"$i> .\" \
              } \
              else if (\$2 == \"<http://www.w3.org/2003/01/geo/wgs84_pos#lat>\") { \
                 lat = (((substr(\$3,2) + 90) + rlat) % 180) - 90; \
                 s = sprintf(\"%.7f\", lat); \
                 print substr(\$1,1,length(\$1)-1) \"$i> \" \$2 \" \42\" s \"\42^^<http://www.w3.org/2001/XMLSchema#double> .\" \
              } \
              else if (\$2 == \"<http://www.w3.org/2003/01/geo/wgs84_pos#long>\") { \
                 lon = (((substr(\$3,2) + 180) + rlon) % 360) - 180; \
                 s = sprintf(\"%.7f\", lon); \
                 print substr(\$1,1,length(\$1)-1) \"$i> \" \$2 \" \42\" s \"\42^^<http://www.w3.org/2001/XMLSchema#double> .\" \
              } \
              else if (\$2 == \"<http://www.w3.org/2003/01/geo/wgs84_pos#geometry>\") { \
                 lon = (((substr(\$3,8) + 180) + rlon) % 360) - 180; \
                 lat = (((\$4 + 90) + rlat) % 180) - 90; \
                 s = sprintf(\"%.7f %.7f\", lon, lat); \
                 print substr(\$1,1,length(\$1)-1) \"$i> \" \$2 \" \42POINT(\" s\
                       \")\42^^<http://www.openlinksw.com/schemas/virtrdf#Geometry> .\" \
              } \
              else { \
                 print substr(\$1,1,length(\$1)-1) \"$i> \" substr(\$0,length(\$1)+1)\
              }\
           }" 
 done) | gzip > LGD-Dump-RelevantNodes.sorted.$1.nt.gz&

(zcat LGD-Dump-RelevantWays.sorted.nt.gz 
 for i in $(seq 1 $SCALE)
 do
  zcat LGD-Dump-RelevantWays.sorted.nt.gz | 
       awk "BEGIN { srand($i); rlon = (rand() - 0.5) / 10000; rlat = (rand() - 0.5) / 20000; } \
           { \
              if (\$2 == \"<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>\") { \
                 print substr(\$1,1,length(\$1)-1) \"$i> \" \$2 \" \" substr(\$3,1,length(\$3)-1) \"$i> .\" \
              } \
              else if (\$2 == \"<http://www.georss.org/georss/polygon>\" || \$2 == \"<http://www.georss.org/georss/line>\") { \
                 lat = (((substr(\$3,2) + 90) + rlat) % 180) - 90; \
                 lon = (((\$4 + 180) + rlon) % 360) - 180; \
                 s = sprintf(\"%.7f %.7f\", lat, lon); \
                 \
                 for(j=5; (j <= NF && \$j ~ /^[0-9]/); j+=2) { \
                    lat = (((\$j + 90) + rlat) % 180) - 90; \
                    lon = (((\$j + 180) + rlon) % 360) - 180; \
                    s = sprintf(\"%s %.7f %.7f\", s, lat, lon); \
                 } \
                 print substr(\$1,1,length(\$1)-1) \"$i> \" \$2 \" \42\" s \"\42 .\" \
              } \
              else { \
                 print substr(\$1,1,length(\$1)-1) \"$i> \" substr(\$0,length(\$1)+1)\
              }\
           }" 
 done) | gzip > LGD-Dump-RelevantWays.sorted.$1.nt.gz&

wait;
