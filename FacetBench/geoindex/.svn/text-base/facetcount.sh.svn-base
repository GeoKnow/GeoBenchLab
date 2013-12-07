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
NBITS=$1
BASE=$2
awk "\
BEGIN { \
  msk = compl(lshift(lshift(1,32-$NBITS)-1,20));\
  fct = lshift(1,20)-1;\
  cnt = p = o = 0; \
  
} { \
  v = and(\$1,msk); \
  if (v == o) { \
    cnt++;\
  } else { \
    if (o > 0) { \
      print -1*rshift(o,20) \" \" cnt \" \" and(o,fct) \" \" p;\
    }\
    o = v;\
    p = \$2;\
    cnt = 1;\
  }\
} END {\
  if (o > 0) { \
    print -1*rshift(o,20) \" \" cnt \" \" and(o,fct) \" \" p;\
  }\
} " $3 | sort -T tmp/ -rnk 1,2 | awk "\
BEGIN { \
  nid = $NBITS*1000000000000 + $BASE;\
  cnt = o = 0;\
} {\
  if (\$1 == o) {\
    cnt++;\
  } else {\
    cnt = 1;\
  }\
  if (cnt <= 100) {\
    nid++;\
    print \"<http://linkedgeodata.org/facetcount/\" nid \"> <http://linkedgeodata.org/facetcount/tilefacet$NBITS> \42\" lshift(-1*\$1,20)+\$3 \"\42^^<http://www.w3.org/2001/XMLSchema#integer> .\";\
    print \"<http://linkedgeodata.org/facetcount/\" nid \"> <http://linkedgeodata.org/facetcount/facet> \" \$4 \" .\";\
    print \"<http://linkedgeodata.org/facetcount/\" nid \"> <http://linkedgeodata.org/facetcount/count> \42\" \$2 \"\42^^<http://www.w3.org/2001/XMLSchema#integer> .\";\
 }\
}"
