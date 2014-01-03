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
#include <sys/stat.h>
#include <sys/types.h>

/* start and end of a query string -- now Virtuoso specific */
#define SQL_PROLOGUE " "
#define SQL_EPILOGUE ";\n\n"

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
	sprintf(buf, "%s/%d_q%02d%c.sql", path, qid/24, 1+(qid%24), (qid&1)?((qid%24)<12)?'m':'i':'c'); 
	++qid;
	fp = fopen(buf, "w+");
	if (fp == NULL) {
		fprintf(stderr, "Failed to open file %s.\n", buf);
		exit(-1);
	}
	fprintf(fp, SQL_PROLOGUE);
}

void closequery() {
	fprintf(fp, SQL_EPILOGUE);
	fclose(fp);
}

#define SELECT_BASIC 0  
#define SELECT_RTREE 1
#define SELECT_RTREE_BOX 3

#define VIRTUOSO_DB 0
#define POSTGIS_DB 1

#define NUM_FACETS 3
#define NUM_LEVELS 4

typedef struct { char* name; unsigned long long code; } facet_t;
facet_t facets[NUM_LEVELS][NUM_FACETS] = { 
 	{ { "http://linkedgeodata.org/ontology/Place", 293 }, 
	  { "http://linkedgeodata.org/ontology/Parking", 933 },
	  { "http://linkedgeodata.org/ontology/Village", 594 } }, 
	{ { "http://linkedgeodata.org/ontology/School", 370 },
	  { "http://linkedgeodata.org/ontology/PlaceOfWorship", 484 },  
	  { "http://linkedgeodata.org/ontology/Leisure", 910 } }, 
	{ { "http://linkedgeodata.org/ontology/Peak", 262 }, 
	  { "http://linkedgeodata.org/ontology/Restaurant", 643 },
	  { "http://linkedgeodata.org/ontology/Tourism", 145 } },
	{ { "http://linkedgeodata.org/ontology/Sport", 721 },
	  { "http://linkedgeodata.org/ontology/PostBox", 572 }, 
	  { "http://linkedgeodata.org/ontology/Supermarket", 653 } } 
};

typedef struct { char* name; double lat; double lon; } city_t;
#define NUM_CITIES 40
city_t cities[NUM_CITIES] = { /* the most densely populated european cities */ 
	{ "Paris",  48.86, 2.351 },
	{ "Essen",  51.459, 7.011 },
	{ "Madrid",  40.421, -3.702 },
	{ "Milan",  45.465, 9.188 },
	{ "Barcelona",  41.391, 2.153 },
	{ "Berlin",  52.526, 13.40 },
	{ "Athens",  37.976, 23.736 },
	{ "Birmingham",  52.481, -1.889 },
	{ "Rome",  41.9, 12.49 },
	{ "Dusseldorf",  51.230, 6.772 },
	{ "Cologne",  50.941, 6.960 },
	{ "Katowice",  50.266, 19.023 },
	{ "Hamburg",  53.552, 9.990 },
	{ "Naples",  40.847, 14.234 },
	{ "Warsaw",  52.239, 21.011 },
	{ "Frankfurt",  50.114, 8.682 },
	{ "Munich",  48.140, 11.576 },
	{ "Brussels",  50.853,4.353 },
	{ "Lisbon",  38.753, -9.151 },
	{ "Vienna",  48.231, 16.372 },
	{ "Manchester",  53.479, -2.248 },
	{ "Budapest",  47.498, 19.041 },
	{ "Amsterdam",  52.361, 4.887 },
	{ "Leeds",  53.800, -1.550 },
	{ "Stuttgart",  48.777, 9.177 },
	{ "Liverpool",  53.418, -2.948 },
	{ "Stockholm",  59.342, 18.056 },
	{ "Rotterdam",  51.927, 4.477 },
	{ "Copenhagen",  55.684, 12.568 },
	{ "Prague",  50.088, 14.422 },
	{ "Lyon",  45.765, 4.836 },
	{ "Zurich",  47.383, 8.525 },
	{ "Turin",  45.074, 7.653 },
	{ "Newcastle",  54.977, -1.610 },
	{ "Sheffield",  53.384, -1.468 },
	{ "Southampton",  50.915, -1.402 },
	{ "Nottingham",  52.958, -1.156 },
	{ "Marseille",  43.405, 5.407 },
	{ "Bucharest",  44.439, 26.097 },
	{ "Dublin",  53.345, -6.283 } };

#define PRIME0 899809343
unsigned long long seed = 1; 

unsigned int rnd() {
	return (unsigned int) ((seed *= PRIME0) >> 5);
}

char b1[128], b2[128], b3[128], b4[128];
char *geturi(unsigned int facetnr, char* buf)  {
	unsigned long long scale = facetnr&1023;
	unsigned long long fset = (facetnr>>10)&3;
	unsigned long long fnr = (facetnr>>12)%3;
	int len = strlen(facets[fset][fnr].name);
	memcpy(buf, facets[fset][fnr].name, len);
	if (scale && 0) {  /* stopped varying the attributes, we want the warmup to show all predicates to the system */
		sprintf(buf+len, "%lld", scale); 
	} else {
		buf[len] = 0;
	}
	return buf;
}

char *sparql2postgissql(char* buf)  {
  if (strcmp(buf, "http://linkedgeodata.org/ontology/Place") == 0)
    strcpy(buf, "tags ? 'place'");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/Parking") == 0)
    strcpy(buf, "tags->'amenity'='parking'");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/Village") == 0)
    strcpy(buf, "tags->'place'='village'");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/School") == 0)
    strcpy(buf, "tags->'amenity'='school'");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/PlaceOfWorship") == 0)
    strcpy(buf, "tags->'amenity'='place_of_worship'");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/Leisure") == 0)
    strcpy(buf, "tags ? 'leisure'");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/Peak") == 0)
    strcpy(buf, "tags->'natural'='peak'");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/Restaurant") == 0)
    strcpy(buf, "tags->'amenity'='restaurant'");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/Tourism") == 0)
    strcpy(buf, "tags ? 'tourism'");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/Sport") == 0)
    strcpy(buf, "tags->'amenity'='sport'");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/PostBox") == 0)
    strcpy(buf, "tags->'amenity'='post_box'");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/Supermarket") == 0)
    strcpy(buf, "tags->'amenity'='supermarket'");
  return buf;
}

char *sparql2virtsql(char* buf)  {
  if (strcmp(buf, "http://linkedgeodata.org/ontology/Place") == 0)
    strcpy(buf, "(id_to_iri(tag_key_id) = 'place')");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/Parking") == 0)
    strcpy(buf, "(id_to_iri(tag_key_id) = 'amenity' and tag_value = 'parking')");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/Village") == 0)
    strcpy(buf, "(id_to_iri(tag_key_id) = 'place' and tag_value = 'village')");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/School") == 0)
    strcpy(buf, "(id_to_iri(tag_key_id) = 'amenity' and tag_value = 'school')");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/PlaceOfWorship") == 0)
    strcpy(buf, "(id_to_iri(tag_key_id) = 'amenity' and tag_value = 'place_of_worship')");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/Leisure") == 0)
    strcpy(buf, "(id_to_iri(tag_key_id) = 'leisure'");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/Peak") == 0)
    strcpy(buf, "(id_to_iri(tag_key_id) = 'natural' and tag_value = 'peak')");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/Restaurant") == 0)
    strcpy(buf, "(id_to_iri(tag_key_id) = 'amenity' and tag_value = 'restaurant')");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/Tourism") == 0)
    strcpy(buf, "(id_to_iri(tag_key_id) = 'tourism'");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/Sport") == 0)
    strcpy(buf, "(id_to_iri(tag_key_id) = 'amenity' and tag_value = 'sport')");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/PostBox") == 0)
    strcpy(buf, "(id_to_iri(tag_key_id) = 'amenity' and tag_value = 'post_box')");
  else if (strcmp(buf, "http://linkedgeodata.org/ontology/Supermarket") == 0)
    strcpy(buf, "(id_to_iri(tag_key_id) = 'amenity' and tag_value = 'supermarket')");
  return buf;
}


void filter_box(int mode, char* indent, double lat_lo, double lon_lo, double lat_hi, double lon_hi, unsigned int database) {
	if (mode == SELECT_RTREE) {
                unsigned int radius_in_km = sqrt((lat_hi-lat_lo)*(lat_hi-lat_lo)+(lon_hi-lon_lo)*(lon_hi-lon_lo))*56.0;
		if (database == POSTGIS_DB)
		  fprintf(fp, 
			"%sst_distance(st_transform(st_geomfromtext('POINT(%g %g)',4326), 26986), st_transform(geom, 26986)) < %u and \n" 
			"%s(st_y(geom) >= %g and st_y(geom) <= %g and st_x(geom) >= %g and st_x(geom) <= %g)\n",
			indent, (lon_lo+lon_hi)/2, (lat_lo+lat_hi)/2, radius_in_km  * 1000, indent, lat_lo, lat_hi, lon_lo, lon_hi);
		else if (database == VIRTUOSO_DB)
		  fprintf(fp, 
			"%sst_intersects(st_geomfromtext('POINT(%g %g)',2000), geom,  %u) and \n" 
			"%s(st_y(geom) >= %g and st_y(geom) <= %g and st_x(geom) >= %g and st_x(geom) <= %g)\n",
			indent, (lon_lo+lon_hi)/2, (lat_lo+lat_hi)/2, radius_in_km, indent, lat_lo, lat_hi, lon_lo, lon_hi);
	} else if (mode == SELECT_RTREE_BOX) {
		if (database == POSTGIS_DB)
		  fprintf(fp, 
			"%sST_Intersects(geom, ST_MakeEnvelope(%g, %g, %g, %g, 4326))\n",
			indent, lon_lo, lat_lo, lon_hi, lat_hi);
		else if (database == VIRTUOSO_DB)
		  fprintf(fp, 
			"%sst_intersects(geom, st_geomfromtext('BOX(%g %g, %g %g)'))\n",
			indent, lon_lo, lat_lo, lon_hi, lat_hi);		  

	}
	else {
		fprintf(fp, 
			"%s(st_y(geom) >= %g and st_y(geom) <= %g and st_x(geom) >= %g and st_x(geom) <= %g)\n", 
				indent, lat_lo, lat_hi, lon_lo, lon_hi);
	}
	
}
void facetcnt(int mode, double lat_lo, double lon_lo, double lat_hi, double lon_hi, unsigned int database) {
	openquery();
	if (database == VIRTUOSO_DB) {
	  fprintf(fp,
		  /* "select top 50 id_to_iri(tag_key_id),  tag_value, count(*) as cnt\n" */
		  "select top 50 id_to_iri(type), count(*) as cnt\n"
		  /* "from nodes, nodes_tags\n" */
		  "from nodes, node_types\n"
		  "where\n"
		  /* "  nodes.id = nodes_tags.node_id and\n"); */
		  "  nodes.id = node_types.node_id and\n");
	  filter_box(mode, "  ", lat_lo, lon_lo, lat_hi, lon_hi, database);
	  fprintf(fp,
		  /* "group by tag_key_id, tag_value\n" */
		  "group by type\n"
		  "order by cnt desc\n");
	}
	else if (database == POSTGIS_DB) {
	  fprintf(fp,
		  /*"select (each(tags)) as type, count(*) as cnt\n"*/
		  "select t.type, count(*) as cnt\n"
		  /*"from nodes\n"*/
		  "from nodes as n, node_types as t\n"
		  /*"where\n");*/
		  "where n.id=t.node_id and\n");
	  filter_box(mode, "  ", lat_lo, lon_lo, lat_hi, lon_hi, database);
	  fprintf(fp,
		  /*"group by type\n"*/
		  "group by t.type\n"
		  "order by cnt desc\n"
		  "limit 50\n");
	}
	closequery();
}

void facetmap(int mode, double lat_lo, double lon_lo, double lat_hi, double lon_hi, unsigned int f1, unsigned int f2, unsigned int f3, unsigned int f4, unsigned int database) {
	openquery();
	if (database == VIRTUOSO_DB) {
	  fprintf(fp,
		  "select id_to_iri(t.type) as facet, CAST(20*(st_y(geom) - %g)/%g as int) as x,\n"
		  "                        CAST(40*(st_x(geom) - %g)/%g as int) as y,\n"
		  "                        max(st_y(geom) || ' ' || st_x(geom)) as latlon, count(*) as cnt\n"
		  "from nodes as n, node_types as t\n"
		  "where n.id = t.node_id and \n"
		  "  (\n"
		  "   t.type=iri_to_id('%s') or \n"
		  "   t.type=iri_to_id('%s') or \n"
		  "   t.type=iri_to_id('%s') or \n"
		  "   t.type=iri_to_id('%s')\n"
		  "  ) and \n",
		  lat_lo, lat_hi-lat_lo, lon_lo, lon_hi-lon_lo, 
		  geturi(f1,b1), geturi(f2,b2), geturi(f3,b3), geturi(f4,b4));
	} else if (database == POSTGIS_DB) {
	  fprintf(fp,
		  "select t.type as facet, CAST(20*(st_y(geom) - %g)/%g as int) as x,\n"
		  "                        CAST(40*(st_x(geom) - %g)/%g as int) as y,\n"
		  "                        max(st_y(geom) || ' ' || st_x(geom)) as latlon, count(*) as cnt\n"
		  "from nodes as n, node_types as t\n"
		  "where n.id = t.node_id and \n"
		  "  (\n"
		  "   t.type='%s' or \n"
		  "   t.type='%s' or \n"
		  "   t.type='%s' or \n"
		  "   t.type='%s'\n"
		  "  ) and \n",
		  lat_lo, lat_hi-lat_lo, lon_lo, lon_hi-lon_lo, 
		  geturi(f1,b1), geturi(f2,b2), geturi(f3,b3), geturi(f4,b4));	  
	}
	filter_box(mode, "          ", lat_lo, lon_lo, lat_hi, lon_hi, database);
	fprintf(fp,
		"group by facet, x, y\n"
		"order by facet, x, y\n");
	closequery();
}

void facetget(int mode, double lat_lo, double lon_lo, double lat_hi, double lon_hi, unsigned int f1, unsigned int f2, unsigned int f3, unsigned int f4, unsigned int database) {
	openquery();
	if (database == POSTGIS_DB) {
	  /*fprintf(fp,
		  "select id as instance, tags as tags, st_y(geom) as lat, st_x(geom) as lon\n"
		  "from nodes\n"
		  "where\n");
	  */
	  fprintf(fp,
		  "select id as instance, t.type as facet, st_y(geom) as lat, st_x(geom) as lon\n"
		  "from nodes as n, node_types as t\n"
		  "where n.id = t.node_id and \n");
	    /*fprintf(fp, 
		    "  (\n"
		    "    %s\n"
		    "  or\n"
		    "    %s\n"
		    "  or\n"
		    "    %s\n"
		    "  or\n"
		    "    %s\n"
		    "  )\n",
		    sparql2postgissql(geturi(f1,b1)), sparql2postgissql(geturi(f2,b2)), sparql2postgissql(geturi(f3,b3)), sparql2postgissql(geturi(f4,b4)));
	    */
	    fprintf(fp, 
		    "  (\n"
		    "    t.type = '%s'\n"
		    "  or\n"
		    "    t.type = '%s'\n"
		    "  or\n"
		    "    t.type = '%s'\n"
		    "  or\n"
		    "    t.type = '%s'\n"
		    "  )\n",
		    geturi(f1,b1), geturi(f2,b2), geturi(f3,b3), geturi(f4,b4));
	}
	else if (database == VIRTUOSO_DB) {
	  /* 
	  fprintf(fp,
		  "select id as instance, id_to_iri(tag_key_id) as tag_key, tag_value as value, st_y(geom) as lat, st_x(geom) as lon\n"
		  "from nodes, nodes_tags\n"
		  "where\n"
		  "id=node_id\n"
		  "and\n");
	  */
	  fprintf(fp,
		  "select id as instance, id_to_iri(t.type) as facet, st_y(geom) as lat, st_x(geom) as lon\n"
		  "from nodes as n, node_types as t\n"
		  "where\n"
		  "id=node_id\n"
		  "and\n");

	    /*
	    fprintf(fp, 
		    "  (\n"
		    "    %s\n"
		    "  or\n"
		    "    %s\n"
		    "  or\n"
		    "    %s\n"
		    "  or\n"
		    "    %s\n"
		    "  )\n",
		    sparql2virtsql(geturi(f1,b1)), sparql2virtsql(geturi(f2,b2)), sparql2virtsql(geturi(f3,b3)), sparql2virtsql(geturi(f4,b4)));
	    */
	    fprintf(fp, 
		    "  (\n"
		    "    t.type = iri_to_id('%s')\n"
		    "  or\n"
		    "    t.type = iri_to_id('%s')\n"
		    "  or\n"
		    "    t.type = iri_to_id('%s')\n"
		    "  or\n"
		    "    t.type = iri_to_id('%s')\n"
		    "  )\n",
		    geturi(f1,b1), geturi(f2,b2), geturi(f3,b3), geturi(f4,b4));
	}
	fprintf(fp, 
		"  and\n");
	filter_box(mode, "  ", lat_lo, lon_lo, lat_hi, lon_hi, database);
	closequery();
}


int main(int argc, char** argv) {
	unsigned int i, j=1, run, scale, mode = atoi(argv[4]);
	if (argc != 6 || ((run = atoi(argv[1])) != 0 && run != 1 && run != 2 && run != 4 && run != 8 && run != 16)) {
		fprintf(stderr, "usage: %s run scale path queryvariant database\n", argv[0]);
		fprintf(stderr, "       run = {warmup=0; power=1; tput=2,4,8,16}\n");
		fprintf(stderr, "       scale >= 1 \n");
		fprintf(stderr, "       queryvariant = {basic=0, rtree=1, rtree_box=3}\n");
		fprintf(stderr, "       database = {virtuoso=0, postgis=1}\n");
		return -1;
	}
	unsigned int database = atoi(argv[5]);
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
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, database);
			facetmap(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4, database);
	
			/* 2. zoom to level 1 at the same center point */
			height /= 2; width /= 2;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, database);
			facetmap(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4, database);
	
			/* 3. zoom to level 2 at the same center point */
			height /= 2; width /= 2;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, database);
			facetmap(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4, database);
	
			/* 4. zoom to level 3 at the same center point */
			height /= 2; width /= 2;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, database);
			facetmap(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4, database);
	
			/* 5. zoom to level 4 at the same center point */
			height /= 2; width /= 2;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, database);
			facetmap(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4, database);
	
			/* 6. pan one eight width east at zoom level 4 */
			lon += width/8;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, database);
			facetmap(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4, database);
	
			/* 7. zoom to level 5 at the same center  */
			height /= 2; width /= 2;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, database);
			facetget(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4, database);
	
			/* 8. pan a quarterheight north at zoom level 5 */
			lat -= height/4;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, database);
			facetget(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4, database);
	
			/* 9. zoom to level 6 */
			height /= 2; width /= 2;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, database);
			facetget(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4, database);
	
			/* 10.pan one half width west at zoom level 6 */
			lon -= width/2;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, database);
			facetget(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4, database);
	
			/* 11.zoom to level 7 */
			height /= 2; width /= 2;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, database);
			facetget(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4, database);
	
			/* 12.pan one height south at zoom level 7 */
			lat += height;
			facetcnt(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, database);
			facetget(mode, lat-height/2, lon-width/2, lat+height/2, lon+width/2, f1, f2, f3, f4, database);
		}
		qid = 0;
	} while(++j <= run);
	return 0;
}
