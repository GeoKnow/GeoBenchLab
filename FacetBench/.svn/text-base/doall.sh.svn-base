#!/bin/bash
# LOD2 Geo Benchmark - execute all experiments
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

DBMS=$1
MODE=$2

if [ "$DBMS" == "owlim" ]; then
	SCRIPT="./georunowlim.sh"
	SETOFMODE="3 2 0"
else	#for virtuoso
	SCRIPT="./georun.sh"
	SETOFMODE="2 0 1"
fi

if [ "$MODE" == "" ]; then
	echo "Running all queries with $SCRIPT"
	
	for mode in $SETOFMODE
	do 
		echo "Mode $mode"
		for run in 0 1 2 4 8
		do 
			$SCRIPT $mode $run; 
		done 
	done
else
	echo "Only run with mode $MODE"
	for run in 0 1 2 4 8
	do 
		$SCRIPT $MODE $run; 
	done 
fi	
