# LOD2 Geo Benchmark - lossy indexing approach based on FacetTile/TileFacets
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
# 
# FacetTile.nt  -- precomputes facet-specific geo-indexing (in spirit similar to facet-partitioned R-tree).
# FacetMap.nt   -- precomputes facet-specific aggregated maps where we combine nearby markers into one
# FacetCount.nt -- withing one tile, provides a count of all Facets in it
#
# the latter two sets are available at multiple resolutions between 12-and 24 bits QuadTile indexing
#
# the indexing is lossy as with this triples, one can give approximate query answers - fast.
#
# this script orchestrates all index triple creation - takes +- 10 minutes on decent hardware

rm -rf *.nt.gz tmp/ 2>/dev/null
mkdir tmp

# extract the geometries and turn them into (facet, quadtile, subject) annotations
zcat ../LGD-Dump-RelevantNodes.sorted.nt.gz | ./geoindex Facets.txt tmp/node_&
zcat ../LGD-Dump-RelevantWays.sorted.nt.gz  | ./geoindex Facets.txt tmp/way_ 2>&1 | fgrep -v warning&
wait

# sort on facets, in parallel
(awk '{ print $4 " " lshift($1,32)+$2 " " $3 " " $5 " " $6 }' tmp/*_1.txt | sort -T tmp/ -nk 2 > tmp/FacetTile_1.tmp; 
 awk '{ print $4 " " lshift($1,32)+$2 " " $3 " " $5 " " $6 }' tmp/*_3.txt | sort -T tmp/ -nk 2 > tmp/FacetTile_3.tmp)&
(awk '{ print $4 " " lshift($1,32)+$2 " " $3 " " $5 " " $6 }' tmp/*_2.txt | sort -T tmp/ -nk 2 > tmp/FacetTile_2.tmp)&
(awk '{ print $4 " " lshift($1,32)+$2 " " $3 " " $5 " " $6 }' tmp/*_4.txt | sort -T tmp/ -nk 2 > tmp/FacetTile_4.tmp;
 awk '{ print $4 " " lshift($1,32)+$2 " " $3 " " $5 " " $6 }' tmp/*_5.txt | sort -T tmp/ -nk 2 > tmp/FacetTile_5.tmp; 
 awk '{ print $4 " " lshift($1,32)+$2 " " $3 " " $5 " " $6 }' tmp/*_6.txt | sort -T tmp/ -nk 2 > tmp/FacetTile_6.tmp)&
(awk '{ print $4 " " lshift($1,32)+$2 " " $3 " " $5 " " $6 }' tmp/*_7.txt | sort -T tmp/ -nk 2 > tmp/FacetTile_7.tmp; 
 awk '{ print $4 " " lshift($1,32)+$2 " " $3 " " $5 " " $6 }' tmp/*_8.txt | sort -T tmp/ -nk 2 > tmp/FacetTile_8.tmp)&
wait
awk '{ print $1 " <http://linkedgeodata.org/intersects/facettile> \42" $2 "\42^^<http://www.w3.org/2001/XMLSchema#integer> ." }' tmp/FacetTile_{1,2,3,4,5,6,7,8}.tmp > FacetTile.nt 

# exploit facet-sorted intermediates for map aggregation (different bits in parallel)
for b in 12 14 16 18
do
 (rm FacetMap${b}.nt 2>/dev/null; 
  for i in 1 2 3 4 5 6 7 8
  do
    ./facetmap.sh ${b} ${i}00000000000 tmp/FacetTile_${i}.tmp >> FacetMap${b}.nt
  done)&
done
wait

# sort on tile at various granularities, and aggregate by tile&facet (different bits in parallel)
for b in 12 14 16 18 20 22 24
do
 (rm FacetCount${b}.nt 2>/dev/null;
  for i in 1 2 3 4 5 6 7 8
  do
    awk "{ if (\$3 <= 12) { print lshift(and(\$2,compl(lshift(1,32-$b)-1)),20)+\$1 \" \" \$7 }}" tmp/*_${i}_*.txt | \
    sort -T tmp/ -nk 1 |\
    ./facetcount.sh ${b} ${i}00000000000 >> FacetCount${b}.nt
  done)&
done
wait

for i in FacetTile.nt FacetCount*.nt FacetMap*.nt; 
do 
  gzip $i& 
done
wait

rm -rf tmp/ 2>/dev/null
