#!/bin/bash
# LOD2 Geo Benchmark - compute power (single-stream) scores
#
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

# price of the system
PRICE=$1

# perf.out file with milliseconds per query in a stream in position four
PERFOUT=$2

awk "{ idx = int(1 + (((q)/2) % 12)); val = \$2; q++; k[idx] += val } END {\
  for(i=p=1;i<=6; i++) { \
    k[i]=10000/k[i]; p*=k[i]; printf k[i] \" \" \
  }\
  printf \"%s (%s) \", p^0.1666, (p^0.1666)*1000/$PRICE; \
  for(i=q=1;i<=6; i++) { \
    k[6+i]=10000/k[6+i]; q*=k[6+i]; printf k[6+i] \" \" \
  } \
  printf \"%s (%s) %s (%s)\n\", q^0.1666, (q^0.1666)*1000/$PRICE, (p*q)^0.0833, ((p*q)^0.0833)*1000/$PRICE; \
}" $PERFOUT
