/*
 * Copyright (c) 2012, CWI (www.cwi.nl), written by Peter Boncz
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 *    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* start and end of a query string -- now Virtuoso specific */
#define SPARQL_PROLOGUE "sparql "
/*#define SPARQL_PROLOGUE "sparql define sql:signal-void-variables 1 "*/
#define SPARQL_EPILOGUE ";\n\n"

/* we have seeds for the 1,2,4,8 and 16-way tput runs with parameter values that are kind of similar */
unsigned int seeds[32] = {
	 67867979,
	492876847, 160481219,
	198491317, 295075147, 104395303, 236887691,
	715225741,  49979687, 533000401, 275604541, 756065179,  49979693, 217645199, 160481183, 
	 15485863, 492876863, 613651349, 797003413, 122949829, 236887699, 715225739, 179424691, 217645177, 573259433, 104395301, 512927357, 533000389, 593441861,  67867967, 694847533,  
         86028121, /* reserve */
};

char path[1024];
int qid = 0;
FILE *fp = NULL;
void openquery() {
	char buf[1024];
	sprintf(buf, "%s/%d_q%02d%c.sparql", path, qid/24, 1+(qid%24), (qid&1)?((qid%24)<12)?'m':'i':'c'); 
	++qid;
	fp = fopen(buf, "w+");
	if (fp == NULL) {
		fprintf(stderr, "Failed to open file %s.\n", buf);
		exit(-1);
	}
	fprintf(fp, SPARQL_PROLOGUE);
}

void closequery() {
	fprintf(fp, SPARQL_EPILOGUE);
	fclose(fp);
}

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))

#define SELECT_BASIC 0  
#define SELECT_RTREE 1
#define SELECT_QUAD  2 
#define SELECT_RTREE_BOX 3

#define RESOLUTION_MIN 12
#define RESOLUTION_STEP 2

#define URI_FACETMAP "http://linkedgeodata.org/facetmap/facettile%u"
#define URI_FACETCNT "http://linkedgeodata.org/facetcount/tilefacet%u"
#define URI_FACETGET "http://linkedgeodata.org/intersects/facettile"

#define NUM_FACETS 3
#define NUM_LEVELS 4

typedef struct { char* name; unsigned long long code; } facet_t;
facet_t facets[NUM_LEVELS][NUM_FACETS] = { 
	{ { "http://linkedgeodata.org/ontology/Place"		, 293 }, 
	  { "http://linkedgeodata.org/ontology/Parking"		, 933 },
	  { "http://linkedgeodata.org/ontology/Village"		, 594 } }, 
	{ { "http://linkedgeodata.org/ontology/School"		, 370 },
	  { "http://linkedgeodata.org/ontology/PlaceOfWorship"	, 484 },  
	  { "http://linkedgeodata.org/ontology/Leisure"		, 910 } }, 
	{ { "http://linkedgeodata.org/ontology/Peak"		, 262 }, 
	  { "http://linkedgeodata.org/ontology/Restaurant"	, 643 },
	  { "http://linkedgeodata.org/ontology/Tourism"		, 145 } },
	{ { "http://linkedgeodata.org/ontology/Sport"		, 721 },
	  { "http://linkedgeodata.org/ontology/PostBox"		, 572 }, 
	  { "http://linkedgeodata.org/ontology/Supermarket"	, 653 } } 
};

typedef struct { char* name; double lat; double lon; } city_t;
#define NUM_CITIES 40
city_t cities[NUM_CITIES] = { /* the most densely populated european cities */ 
	{ "Paris"	,  48.86	,   2.351	},
	{ "Essen"	,  51.459	,   7.011	},
	{ "Madrid"	,  40.421	,  -3.702	},
	{ "Milan"	,  45.465	,   9.188	},
	{ "Barcelona"	,  41.391	,   2.153	},
	{ "Berlin"	,  52.526	,  13.40	},
	{ "Athens"	,  37.976	,  23.736	},
	{ "Birmingham"	,  52.481	,  -1.889	},
	{ "Rome"	,  41.9		,  12.49	},
	{ "Dusseldorf"	,  51.230	,   6.772	},
	{ "Cologne"	,  50.941	,   6.960	},
	{ "Katowice"	,  50.266	,  19.023	},
	{ "Hamburg"	,  53.552	,   9.990	},
	{ "Naples"	,  40.847	,  14.234	},
	{ "Warsaw"	,  52.239	,  21.011	},
	{ "Frankfurt"	,  50.114	,   8.682	},
	{ "Munich"	,  48.140	,  11.576	},
	{ "Brussels"	,  50.853	,   4.353	},
	{ "Lisbon"	,  38.753	,  -9.151	},
	{ "Vienna"	,  48.231	,  16.372	},
	{ "Manchester"	,  53.479	,  -2.248	},
	{ "Budapest"	,  47.498	,  19.041	},
	{ "Amsterdam"	,  52.361	,   4.887	},
	{ "Leeds"	,  53.800	,  -1.550	},
	{ "Stuttgart"	,  48.777	,   9.177	},
	{ "Liverpool"	,  53.418	,  -2.948	},
	{ "Stockholm"	,  59.342	,  18.056	},
	{ "Rotterdam"	,  51.927	,   4.477	},
	{ "Copenhagen"	,  55.684	,  12.568	},
	{ "Prague"	,  50.088	,  14.422	},
	{ "Lyon"	,  45.765	,   4.836	},
	{ "Zurich"	,  47.383	,   8.525	},
	{ "Turin"	,  45.074	,   7.653	},
	{ "Newcastle"	,  54.977	,  -1.610	},
	{ "Sheffield"	,  53.384	,  -1.468	},
	{ "Southampton"	,  50.915	,  -1.402	},
	{ "Nottingham"	,  52.958	,  -1.156	},
	{ "Marseille"	,  43.405	,   5.407	},
	{ "Bucharest"	,  44.439	,  26.097	},
	{ "Dublin"	,  53.345	,  -6.283	}};

#define PRIME0 899809343
unsigned long long seed = 1; 

unsigned int rnd() {
	return (unsigned int) ((seed *= PRIME0) >> 5);
}

unsigned long long getcode(unsigned int facetnr)  {
	unsigned long long scale = facetnr&1023;
	unsigned long long fset = (facetnr>>10)&3;
	unsigned long long fnr = (facetnr>>12)%3;
	return (scale<<10)|facets[fset][fnr].code;
}

char b1[128], b2[128], b3[128], b4[128];
char *geturi(unsigned int facetnr, char* buf)  {
	unsigned long long scale = facetnr&1023;
	unsigned long long fset = (facetnr>>10)&3;
	unsigned long long fnr = (facetnr>>12)%3;
	int len = strlen(facets[fset][fnr].name);
	memcpy(buf, facets[fset][fnr].name, len);
	if (scale && 0) {  /* stopped varying the attributes, we want the warmup to show all predicates to the system */
		sprintf(buf+len, "%Ld", scale); 
	} else {
		buf[len] = 0;
	}
	return buf;
}

unsigned int demux(unsigned int quad) {
	unsigned int r=0;
	int b;
	for(b=30; b>=0; b-=2)
		r = (r<<1) | ((quad >> b)&1);
	return r;
}

int test_quad(unsigned int quad, unsigned int nbits, double lat_lo, double lon_lo, double lat_hi, double lon_hi) {
	/* test whether a rectangle intersects with a quadtile */
	double delta = 1 << (16-(nbits/2));
	double quad_lat = demux(quad);
	double quad_lon = demux(quad>>1);
	double quad_lat_lo = 180*(quad_lat / 65536.0) - 90.0;
	double quad_lon_lo = 360*(quad_lon / 65536.0) - 180.0;
	double quad_lat_hi = 180*((quad_lat + delta) / 65536.0) - 90.0;
	double quad_lon_hi = 360*((quad_lon + delta) / 65536.0) - 180.0;

	if (quad_lat_lo > lat_hi || quad_lat_hi <= lat_lo || quad_lon_lo > lon_hi || quad_lon_hi <= lon_lo)
		return 0; /* disjunct (not counting quad higher bounds) */
	if (quad_lat_lo >= lat_lo && quad_lat_hi <= lat_hi && quad_lon_lo >= lon_lo && quad_lon_hi <= lon_hi)
		return 2; /* fully enclosed (optimistically wrt boundaries) */
	return 1; /* intersects */
}

#define F(x) (base+(((unsigned long long) x)<<lshift))
#define MAXTILE 16384 /* max #tiles on a screen */
double filter_quad(char* indent, double lat_lo, double lon_lo, double lat_hi, double lon_hi, unsigned long long base, unsigned int lshift, char *uri, unsigned int resolution_max) {
	unsigned int buf1[MAXTILE], *hits = buf1;
	unsigned int buf2[MAXTILE], *next = buf2;
	unsigned int i, j, r=0, n, nhits = 1, t = 0;
	char buf[128];

	/* breadth-first binary search to find all overlapping tiles at each level */
	hits[0] = 0;
	while(r+RESOLUTION_STEP <= resolution_max) {
		r += RESOLUTION_STEP;
		for(j=0; j<nhits; j++) {
			for(i=0; i < (1<<RESOLUTION_STEP); i++) {
				unsigned int quad = hits[j] + (i << (32 - r));
				int intersects = test_quad(quad, r, lat_lo, lon_lo, lat_hi, lon_hi);
				if (intersects) {     
					next[t] = quad; 
					if (t++ >= MAXTILE) {
						fprintf(stderr, "overflow failure in test_quad\n");
						exit(-1);
					}
					if (intersects > 1 && r >= RESOLUTION_MIN) {
						resolution_max = 0; /* at least one tile is fully enclosed. stop at this level */
					}
				}
			}
		}
		nhits = t; t = 0;
		next = hits; hits = (next == buf1)?buf2:buf1; 
	}
 	if (uri == NULL) /* return relevant fraction of the surface of the selected tiles */
		return ((lon_hi-lon_lo)*(lat_hi-lat_lo)) / (nhits*64800.0/(1<<r));

	/* generate the results */
	n = 1 << (32 - r);
	fprintf(fp, "%sfilter(", indent); 
	sprintf(buf, "\n%s    || ", indent);

	for(i=j=0; i < nhits; i++) {
		unsigned int lo = hits[i];
		while(i+1 < nhits && hits[i+1] == (hits[i]+n)) i++;
		fprintf(fp, 
			"%s(?g >= %llu && ?g <= %llu)", j++?buf:"", 
			F(lo), F(hits[i]+n-1));
	}
	sprintf(buf, uri, r);
	if (j == 1) {
		fprintf(fp, ") .\n%s?s <%s> ?g .\n", indent, buf);
	} else {
		fprintf(fp, 
			") .\n" 
			"%s{ #subquery-start\n"
			"%s  select ?s ?g\n"
			"%s  where\n"
			"%s  { #where-start\n"
			"%s    ?s <%s> ?g .\n"
			"%s    filter (?g >= %llu && ?g <= %llu)\n"
			"%s  } #where-end\n"
			"%s} #subquery-end\n",
				indent, 
				indent, 
				indent, 
				indent, 	
				indent, buf,
				indent, F(hits[0]), F(hits[nhits-1]+n-1), 
				indent, 
				indent);
	}
	return 0;
}

void filter_box(int mode, char* indent, double lat_lo, double lon_lo, double lat_hi, double lon_hi) {
	if (mode == SELECT_RTREE) {
		unsigned int radius_in_km = sqrt((lat_hi-lat_lo)*(lat_hi-lat_lo)+(lon_hi-lon_lo)*(lon_hi-lon_lo))*56.0;
		fprintf(fp, 
			"%s?s <http://www.w3.org/2003/01/geo/wgs84_pos#geometry> ?p .\n"
			"%sfilter(bif:st_intersects(bif:st_geomfromtext('POINT(%g %g)',2000), ?p, %u) && \n" 
			"%s       ?a >= %g && ?a <= %g && ?o >= %g && ?o <= %g)\n",
			indent, indent, (lon_lo+lon_hi)/2, (lat_lo+lat_hi)/2, radius_in_km, indent, lat_lo, lat_hi, lon_lo, lon_hi);
	} else if (mode == SELECT_QUAD) {
		/* because the main filter is on quadtiles, block the optimizer from using a OPS/POS index on lat/lon by hiding them in a xsd:double() cast */
		fprintf(fp, 
			"%sfilter(xsd:double(?a) >= %g && xsd:double(?a) <= %g && xsd:double(?o) >= %g && xsd:double(?o) <= %g)\n", 
				indent, lat_lo, lat_hi, lon_lo, lon_hi);
	} else if (mode == SELECT_RTREE_BOX) {
		fprintf(fp, 
			"%s?s <http://www.w3.org/2003/01/geo/wgs84_pos#geometry> ?p .\n"
			"%sfilter(bif:st_intersects(bif:st_geomfromtext(\"BOX(%g %g, %g %g)\"), ?p))\n",
			indent, indent, lon_lo, lat_lo, lon_hi, lat_hi);
	} else {
		fprintf(fp, 
			"%sfilter(?a >= %g && ?a <= %g && ?o >= %g && ?o <= %g)\n", 
				indent, lat_lo, lat_hi, lon_lo, lon_hi);
	}
	
}
void facetcnt(int mode, double lat_lo, double lon_lo, double lat_hi, double lon_hi) {
	openquery();
	if (mode == SELECT_BASIC || mode == SELECT_RTREE) {
		fprintf(fp,
			"select ?f as ?facet count(?s) as ?cnt\n"
			"where\n"
			"{ #where-start\n"
			"  ?s <http://www.w3.org/2003/01/geo/wgs84_pos#lat> ?a ;\n"
			"     <http://www.w3.org/2003/01/geo/wgs84_pos#long> ?o ;\n"
			"     <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?f .\n");
		filter_box(mode, "  ", lat_lo, lon_lo, lat_hi, lon_hi);
		fprintf(fp,
			"} #where-end\n"
			"group by ?f\n"
			"order by desc(?cnt)\n"
			"limit 50\n");
	} else if (mode == SELECT_RTREE_BOX) {
		fprintf(fp,
			"select ?f as ?facet count(?s) as ?cnt\n"
			"where\n"
			"{ #where-start\n"
			"  ?s <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?f .\n");
		filter_box(mode, "  ", lat_lo, lon_lo, lat_hi, lon_hi);
		fprintf(fp,
			"} #where-end\n"
			"group by ?f\n"
			"order by desc(?cnt)\n"
			"limit 50\n");
	} else {
		double frac = filter_quad("  ", lat_lo, lon_lo, lat_hi, lon_hi, 0, 20, NULL, 24);
		fprintf(fp,
			"select ?f as ?facet xsd:integer(sum(?c * %g)) as ?cnt\n"
			"where\n"
			"{ #where-start\n"
			"  ?s <http://linkedgeodata.org/facetcount/count> ?c ;\n"
			"     <http://linkedgeodata.org/facetcount/facet> ?f .\n", frac);
		filter_quad("  ", lat_lo, lon_lo, lat_hi, lon_hi, 0, 20, URI_FACETCNT, 24);
		fprintf(fp, 
			"} #where-end\n"
			"group by ?f\n"
			"order by desc(?cnt)\n"
			"limit 50\n");
	}
	closequery();
}

void facetmap(int mode, double lat_lo, double lon_lo, double lat_hi, double lon_hi, unsigned int f1, unsigned int f2, unsigned int f3, unsigned int f4) {
	openquery();
	if (mode == SELECT_BASIC || mode == SELECT_RTREE || mode == SELECT_RTREE_BOX) {
		fprintf(fp,
			"select ?f as ?facet ?latlon ?cnt\n"
			"where\n"
			"{ #where-start\n"
			"  { #subquery-start\n"
			"    select ?f ?x ?y  max(concat(xsd:string(?a),\" \",xsd:string(?o))) as ?latlon count(*) as ?cnt\n"
			"    where\n"
			"    { #where-start\n"
			"      { #subquery-start\n"
			"        select ?f ?a ?o xsd:integer(20*(?a - %g)/%g) as ?y xsd:integer(40*(?o - %g)/%g) as ?x\n"
			"        where\n"
			"        { #where-start\n"
			"          { #union-start\n"
			"            ?s <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?f . filter (?f = <%s>)\n"
			"          } #union-end\n"
			"          union\n"
			"          { #union-start\n"
			"            ?s <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?f . filter (?f = <%s>)\n"
			"          } #union-end\n"
			"          union\n"
			"          { #union-start\n"
			"            ?s <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?f . filter (?f = <%s>)\n"
			"          } #union-end\n"
			"          union\n"
			"          { #union-start\n"
			"            ?s <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?f . filter (?f = <%s>)\n"
			"          } #union-end\n"
			"          ?s  <http://www.w3.org/2003/01/geo/wgs84_pos#lat> ?a ;\n"
			"              <http://www.w3.org/2003/01/geo/wgs84_pos#long> ?o .\n",
				lat_lo, lat_hi-lat_lo, lon_lo, lon_hi-lon_lo, 
				geturi(f1,b1), geturi(f2,b2), geturi(f3,b3), geturi(f4,b4));
		filter_box(mode, "          ", lat_lo, lon_lo, lat_hi, lon_hi);
		fprintf(fp,
			"        } #where-end\n"
			"      } #subquery-end\n"
			"    } #where-end\n"
			"    group by ?f ?x ?y\n"
			"    order by ?f ?x ?y\n"
			"  } #subquery-end\n"
			"} #where-end\n");
	} else {
		fprintf(fp,
			"select ?f as ?facet max(concat(xsd:string(?a),\" \",xsd:string(?o))) as ?latlon sum(?c) as ?cnt\n"
			"where\n"
			"{ #where-start\n"
			"  { #subquery-start\n"
			"    select ?f ?c xsd:integer(20*(?a - %g)/%g) as ?y xsd:integer(40*(?o - %g)/%g) as ?x\n"
			"    where\n"
			"    { #where-start\n"
			"      { #union-start\n"
			"        { #subquery-start\n"
			"          select ?s <%s> as ?f\n"
			"          where\n"
			"          { #where-start\n", 
				lat_lo, lat_hi-lat_lo, lon_lo, lon_hi-lon_lo, geturi(f1,b1));
		filter_quad("            ", lat_lo, lon_lo, lat_hi, lon_hi, getcode(f1)<<32, 0, URI_FACETMAP, 18);
		fprintf(fp, 
			"          } #where-end\n"
			"        } #subquery-end\n"
			"      } #union-end\n"
			"      union\n"
			"      { #union-start\n"
			"        { #subquery-start\n"
			"          select ?s <%s> as ?f\n"
 			"          where\n"
			"          { #where-start\n", geturi(f2,b2));
		filter_quad("            ", lat_lo, lon_lo, lat_hi, lon_hi, getcode(f2)<<32, 0, URI_FACETMAP, 18);
		fprintf(fp, 
			"         } #where-end\n"
			"        } #subquery-end\n"
			"      } #union-end\n"
			"      union\n"
			"      { #union-start\n"
			"        { #subquery-start\n"
			"          select ?s <%s> as ?f\n"
 			"          where\n"
			"          { #where-start\n", geturi(f3,b3));
		filter_quad("            ", lat_lo, lon_lo, lat_hi, lon_hi, getcode(f3)<<32, 0, URI_FACETMAP, 18);
		fprintf(fp, 
			"          } #where-end\n"
			"        } #subquery-end\n"
			"      } #union-end\n"
			"      union\n"
			"      { #union-start\n"
			"        { #subquery-start\n"
			"          select ?s <%s> as ?f\n"
 			"          where\n"
			"          { #where-start\n", geturi(f4,b4));
		filter_quad("            ", lat_lo, lon_lo, lat_hi, lon_hi, getcode(f4)<<32, 0, URI_FACETMAP, 18);
		fprintf(fp, 
			"          } #where-end\n"
			"        } #subquery-end\n"
			"      } #union-end\n"
			"      .\n"
			"      ?s <http://linkedgeodata.org/facetmap/latitude> ?a ;\n"
			"         <http://linkedgeodata.org/facetmap/longitude> ?o ;\n"
			"         <http://linkedgeodata.org/facetmap/count> ?c .\n");
		filter_box(mode, "      ", lat_lo, lon_lo, lat_hi, lon_hi);
		fprintf(fp, 
			"    } #where-end\n"
			"  } #subquery-end\n"
			"} #where-end\n"
			"group by ?f ?x ?y\n",
			"order by ?f ?x ?y\n",
				lat_lo, lat_hi, lon_lo, lon_hi);
	}
	closequery();
}

void facetget(int mode, double lat_lo, double lon_lo, double lat_hi, double lon_hi, unsigned int f1, unsigned int f2, unsigned int f3, unsigned int f4) {
	openquery();
	fprintf(fp, 
		"select ?s as ?instance ?f as ?facet ?a as ?lat ?o as ?lon\n"
		"where\n"
		"{ #where-start\n");
	if (mode == SELECT_BASIC || mode == SELECT_RTREE || mode == SELECT_RTREE_BOX) {
		fprintf(fp, 
      			"  { #union-start\n"
			"    ?s  <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?f filter (?f = <%s>)\n"
			"  } #union-end\n"
			"  union\n"
      			"  { #union-start\n"
			"    ?s  <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?f filter (?f = <%s>)\n"
			"  } #union-end\n"
			"  union\n"
      			"  { #union-start\n"
			"    ?s  <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?f filter (?f = <%s>)\n"
			"  } #union-end\n"
			"  union\n"
      			"  { #union-start\n"
			"    ?s  <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?f filter (?f = <%s>)\n"
			"  } #union-end\n"
			"  .\n",
				geturi(f1,b1), geturi(f2,b2), geturi(f3,b3), geturi(f4,b4));
	} else {
		fprintf(fp, 
			"  { #union-start\n"
			"    { #subquery-start\n"
			"      select ?s <%s> as ?f\n"
			"      where\n"
			"      { #where-start\n", geturi(f1,b1));
		filter_quad("        ", lat_lo, lon_lo, lat_hi, lon_hi, getcode(f1)<<32, 0, URI_FACETGET, 24);
		fprintf(fp, 
			"      } #where-end\n"
			"    } #subquery-end\n"
			"  } #union-end\n"
			"  union\n"
			"  { #union-start\n"
			"    { #subquery-start\n"
			"      select ?s <%s> as ?f\n"
			"      where\n"
			"      { #where-start\n", geturi(f2,b2));
		filter_quad("        ", lat_lo, lon_lo, lat_hi, lon_hi, getcode(f2)<<32, 0, URI_FACETGET, 24);
		fprintf(fp, 
			"      } #where-end\n"
			"    } #subquery-end\n"
			"  } #union-end\n"
			"  union\n"
			"  { #union-start\n"
			"    { #subquery-start\n"
			"      select ?s <%s> as ?f\n"
			"      where\n"
			"      { #where-start\n", geturi(f3,b3));
		filter_quad("        ", lat_lo, lon_lo, lat_hi, lon_hi, getcode(f3)<<32, 0, URI_FACETGET, 24);
		fprintf(fp, 
			"      } #where-end\n"
			"    } #subquery-end\n"
			"  } #union-end\n"
			"  union\n"
			"  { #union-start\n"
			"    { #subquery-start\n"
			"      select ?s <%s> as ?f\n"
			"      where\n"
			"      { #where-start\n", geturi(f4,b4));
		filter_quad("        ", lat_lo, lon_lo, lat_hi, lon_hi, getcode(f4)<<32, 0, URI_FACETGET, 24);
		fprintf(fp, 
			"      } #where-end\n"
			"    } #subquery-end\n"
			"  } #union-end\n"
			"  .\n");
		mode = SELECT_BASIC;
	}
	fprintf(fp, 
		"  ?s  <http://www.w3.org/2003/01/geo/wgs84_pos#lat> ?a ;\n"
		"      <http://www.w3.org/2003/01/geo/wgs84_pos#long> ?o .\n");
	filter_box(mode, "  ", lat_lo, lon_lo, lat_hi, lon_hi);
	fprintf(fp, 
		"} #where-end\n");
	closequery();
}


int main(int argc, char** argv) {
	unsigned int i, j=1, run, scale, mode = (argc==5)?atoi(argv[4]):SELECT_BASIC;
	if (argc < 4 || argc > 5 || ((run = atoi(argv[1])) != 0 && run != 1 && run != 2 && run != 4 && run != 8 && run != 16)) {
		fprintf(stderr, "usage: %s run scale path [ queryvariant0-2 ]\n", argv[0]);
		fprintf(stderr, "       run = {warmup=0; power=1; tput=2,4,8,16}\n");
		fprintf(stderr, "       scale >= 1 \n");
		fprintf(stderr, "       queryvariant = {basic=0, rtree=1, quadtile=2}\n");
		return -1;
	}
	scale = atoi(argv[2]);
	do {
		seed = 1^ (run?seeds[run+j-2]:0); /* run=0 ==> warmup with really disjunct queries */
		sprintf(path, "%s/%02d", argv[3], j);
		mkdir(argv[3], 0777);
		mkdir(path, 0777);

		/* a query workload consists of 240 queries: 10 sequences x 12 steps x 2 queries */
		for(i=0; i<10; i++) {
			int city = run ?(rnd() % (NUM_CITIES-10)):(NUM_CITIES+i-10);
			double height=4.5, lat = cities[city].lat + (((double) (rnd()%100))-50.0)*0.0001;
			double width=9.0,  lon = cities[city].lon + (((double) (rnd()%100))-50.0)*0.0002;
			unsigned int f1 = (rnd() % scale) + (rnd()%3)*4096;
			unsigned int f2 = (rnd() % scale) + (rnd()%3)*4096 + 1024;
			unsigned int f3 = (rnd() % scale) + (rnd()%3)*4096 + 2048;
			unsigned int f4 = (rnd() % scale) + (rnd()%3)*4096 + 3072;
			
			/* 1. display map at zoom level 0 at a center point */
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2);
			facetmap(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4);
	
			/* 2. zoom to level 1 at the same center point */
			height /= 2; width /= 2;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2);
			facetmap(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4);
	
			/* 3. zoom to level 2 at the same center point */
			height /= 2; width /= 2;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2);
			facetmap(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4);
	
			/* 4. zoom to level 3 at the same center point */
			height /= 2; width /= 2;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2);
			facetmap(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4);
	
			/* 5. zoom to level 4 at the same center point */
			height /= 2; width /= 2;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2);
			facetmap(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4);
	
			/* 6. pan one eight width east at zoom level 4 */
			lon += width/8;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2);
			facetmap(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4);
	
			/* 7. zoom to level 5 at the same center  */
			height /= 2; width /= 2;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2);
			facetget(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4);
	
			/* 8. pan a quarterheight north at zoom level 5 */
			lat -= height/4;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2);
			facetget(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4);
	
			/* 9. zoom to level 6 */
			height /= 2; width /= 2;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2);
			facetget(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4);
	
			/* 10.pan one half width west at zoom level 6 */
			lon -= width/2;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2);
			facetget(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4);
	
			/* 11.zoom to level 7 */
			height /= 2; width /= 2;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2);
			facetget(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4);
	
			/* 12.pan one height south at zoom level 7 */
			lat += height;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2);
			facetget(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4);
		}
		qid = 0;
	} while(++j <= run);
	return 0;
}
