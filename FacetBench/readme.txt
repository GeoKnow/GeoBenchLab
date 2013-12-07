# LOD2 Geo Benchmark
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

datasets
========

Virtuoso does not read .bz files, but it does read .gz files; therefore it gets converted

wget -O - http://downloads.linkedgeodata.org/releases/2011-04-06/LGD-Dump-110406-Ontology.nt.bz2 | bzcat | gzip > LGD-Dump-Ontology.nt.gz
wget -O - http://downloads.linkedgeodata.org/releases/2011-04-06/LGD-Dump-110406-RelevantNodes.sorted.nt.bz2 | bzcat | gzip > LGD-Dump-RelevantNodes.sorted.nt.gz
wget -O - http://downloads.linkedgeodata.org/releases/2011-04-06/LGD-Dump-110406-RelevantWays.sorted.nt.bz2 | bzcat | gzip > LGD-Dump-RelevantWays.sorted.nt.gz

programs
========
- geoqgen.c (make: gcc geoqgen.c -o geoqgen -lm)
  query generator for the LOD2 Geo Benchmark

  usage:   ./geoqgen run scale path method
           run:     warmup=0, power=1, tput={2,4,8,16}
           method: default=0, rtree=1, quad=2 rtree with intersection with bounding boxes=3

  example: ./geoqgen 0 1 /tmp/ 0

  example that creates all query runs:

  mkdir 0 1 2
  for method in 0 1 2; do for run in 0 1 2 4 8 16; do ./geoqgen $run 10 $method/$run $method; done; done
 
  the workload consists of running the 240 generated queries in order, sequentially.
  it mimicks a browsing session in the LGD browser
   * run 0 is is the 'warmup run'. It does not touch the data touched by the other runs, so warmup is relative.
   * run 1 is the power run, that should be run in isolation (1 query at-a-time)
     both run 0 and run 1 have a single subdirectory 01/ with a single query stream. It should be run sequentially.
   * runs 2,4,8,16 are the throughput runs.
     Here, runxx has subdirectories 01/, .., xx/ that all contain one stream of 240 queries. These xx are run in parallel.

- georun.sh
  perform a benchmark run, leaves output files in method/run/*/ directories (with the perf.out file with msec per query)

  usage:   ./georun.sh method run
  example: ./georun.sh 2 8

- geopower.sh
  compute the single-stream (power) scores for a benchmark run (used dor runs 0 and 1)

  usage:   ./geopower.sh price perf.out
  example: ./geopower.sh 12765 0/0/01/perf.out

- geotput.sh
  compute the multi-stream (throughput) scores for a benchmark run (used for runs > 1)

  usage:   ./geotput.sh price perf.out*
  example: ./geotput.sh 12765 0/8/*/perf.out

geoindex
========

There is an optimized/approximate implmentation of the benchmark based on precomputed triples organized in multiple resolutions along QuadTiles.

- geoindex/geoindex.c  (make: gcc geoindex.c -o geoindex)
  program that reads the RelevantNodes/Ways files and creates extra triples. Is called from the geoindex.sh script.

- geoindex/geoindex.sh
  shell script that generate all FacetTile/TileFacet extra triples. No parameters needed.

  example usage: ./geoindex.sh

benchmark scaling
=================

The standard benchmark takes consists of onlu 10M geometric objects. Still, for current RDF technology this is not a trivial dataset. There benchmark can be scaled by essentially copying. The instances get copied, but the facets they have are not the same as the original facet URIs; as these are also copied. So the 10x scaled dataset has 10x the amount of triples, and also 10x the amount of facets.

- geoscale.sh
  scale the indexing datasets (should be in wordking dir) by a certain factor.

  usage:   ./geoscale.sh scale
  example: ./geoscale.sh 10

- geoindex/indexscale.sh
  scale the indexing datasets (should be in wordking dir) by a certain factor.

  usage:   ./geoindex.sh scale
  example: ./geoindex.sh 10
