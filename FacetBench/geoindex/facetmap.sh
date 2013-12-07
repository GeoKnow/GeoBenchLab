#!/bin/bash
# Copyright (c) 2012, CWI (www.cwi.nl). Author: Peter Boncz
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
#
NBITS=$1
BASE=$2
awk "\
BEGIN { \
  nid  = $NBITS*1000000000000 + $BASE;\
  fmsk = compl(lshift(1,32-$NBITS)-1);\
  msk  = compl(lshift(1,25-$NBITS)-1);\
  cnt  = o = 0; \
} { \
  if (\$3 <= $NBITS){ \
    v = and(\$2,msk); \
    if (v == o) { \
      cnt++;\
    } else { \
      if (o > 0) { \
        x = and(rshift(o,25-$NBITS),1) + and(rshift(o,27-$NBITS),2) + and(rshift(o,29-$NBITS),4)+ and(rshift(o,31-$NBITS),8);\
        y = and(rshift(o,26-$NBITS),1) + and(rshift(o,28-$NBITS),2) + and(rshift(o,30-$NBITS),4);\
        print \"<http://linkedgeodata.org/facetmap/\" nid \"> <http://linkedgeodata.org/facetmap/facettile$NBITS> \42\" and(o,fmsk) \"\42^^<http://www.w3.org/2001/XMLSchema#integer> .\";\
        print \"<http://linkedgeodata.org/facetmap/\" nid \"> <http://linkedgeodata.org/facetmap/latitude> \42\" lat \"\42^^<http://www.w3.org/2001/XMLSchema#double> .\";\
        print \"<http://linkedgeodata.org/facetmap/\" nid \"> <http://linkedgeodata.org/facetmap/longitude> \42\" long \"\42^^<http://www.w3.org/2001/XMLSchema#double> .\";\
        print \"<http://linkedgeodata.org/facetmap/\" nid \"> <http://linkedgeodata.org/facetmap/count> \42\" cnt \"\42^^<http://www.w3.org/2001/XMLSchema#integer> .\";\
        nid++;\
      }\
      o = v;\
      exmpl = \$1;\
      long = \$4;\
      lat = \$5;\
      cnt = 1;\
    }\
  }\
}" $3
