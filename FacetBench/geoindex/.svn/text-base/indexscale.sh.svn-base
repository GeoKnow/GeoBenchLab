#!/bin/bash
SCALE=`expr $1 - 1`

for f in 12 14 16 18 20 22 24
do
   F=FacetCount$f.$1.nt
   (zcat FacetCount$f.nt.gz 
   for i in $(seq 1 $SCALE)
   do
    zcat FacetCount$f.nt.gz | 
       awk "{ o=\$3; if (\$2 == \"<http://linkedgeodata.org/facetcount/tilefacet$f>\") { o = substr(\$3,2,length(\$3)-46); o += lshift($i,10); o =\"\42\" o \"\42\"\"^^<http://www.w3.org/2001/XMLSchema#integer>\" } else if (\$2 == \"<http://linkedgeodata.org/facetcount/facet>\") { o = substr(\$3,1,length(\$3)-1) \"$i>\" } print substr(\$1,1,length(\$1)-1) \"$i> \" \$2 \" \" o \" .\"}" 
   done) | gzip > $F.gz&
done

for f in 12 14 16 18 
do
   F=FacetMap$f.$1.nt
   (zcat FacetMap$f.nt.gz 
   for i in $(seq 1 $SCALE)
   do
    zcat FacetMap$f.nt.gz | 
       awk "{ o=\$3; if (\$2 == \"<http://linkedgeodata.org/facetmap/facettile$f>\") { o = substr(\$3,2,length(\$3)-46); o += lshift($i,42); o =\"\42\" o \"\42\"\"^^<http://www.w3.org/2001/XMLSchema#integer>\" } print substr(\$1,1,length(\$1)-1) \"$i> \" \$2 \" \" o \" .\"}" 
   done) | gzip > $F.gz&
done

F=FacetTile.$1.nt
(zcat FacetTile.nt.gz 
 for i in $(seq 1 $SCALE)
 do
   zcat FacetTile.nt.gz | awk "{ o=\$3; o = substr(\$3,2,length(\$3)-46); o += lshift($i,42); o =\"\42\" o \"\42\"\"^^<http://www.w3.org/2001/XMLSchema#integer>\"; print \$1 \" \" \$2 \" \" o \" .\"}" 
 done) | gzip > $F.gz
wait
