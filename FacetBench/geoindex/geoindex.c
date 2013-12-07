/* LOD2 Geobench QuadTile based FacetTile/TileFacet indexing 
 *
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
 *
 * works on the N3 dumps from LinkedOpenGeodata
 * 
 * program accepts a stream of NT triples in SPO order and produces a tempfile with one record for every 24-bits
 * QuadTile that intersects with an facet-instance with geometry (intersection only checked on bounding box). 
 *
 * record format: [ facethash quadtile granuarity sortkey(nodeid) lat lon ] 
 *
 * this tempfile contains space-separated per line:                  
 * - facethash:   a 20-bits (perfect) hash number, identifying a facet 
 * - quadtile:    the quadtile number at the bit resolution indicated by the filename 
 * - granularity: minimum QuadTile bit granularity for which this point is relevant
 * - nodeid:      de-triplified (shortened) node identifier
 * - lat,lon:     left upper corner (of sub-box)
 * - facet:       full facet URI (redundant wrt facethash) 
 *
 * this tempfile is split in 64 files named [1..8]_[1..8].txt on both the highest 
 * quadtile and facethash bits. Thus, subsequent sorts both on quadtile and on facet can be paralellized.
 * 
 * from these tempfiles we later create with some shell scripts 3 kinds of outputs:
 * - FacetTile at maximum granularity (24-bits):             [ sortkey(facethash quadtile24) nodeid ]
 * - FacetTile 16x8 summary at ZZ in {12,14,16,18}-bits:     [ sortkey(facethash quadtileZZ cell) count nodeid ]
 *   remark: only nonzero counts
 *   remark: quadtileZZ is quadtile restricted to ZZ highest bits
 *   remark: cell = 7 deeper quadtile bits granularity ZZ 
 *   remark: nodeid comes from one random node in the rectangle  
 * - TileFacet counts at ZZ in {12,14,16,18,20,22,24}-bits:  [ sortkey(quadtileZZ facethash) count ]  
 *   remark: only nonzero counts, only counts records with lat,lon (counts objects only once)
 *   remark: quadtileZZ is quadtile restricted to ZZ highest bits
 * 
 * the shell scripts further immediately triplify the outputs.
 * - FacetTile at maximum granularity (24-bits):
 *   <http://linkedgeodata.org/triplify/node1000XXXXXX> <http://linkedgeodata.org/intersects/facettile>  facethash|quadtile24
 * - FacetTile 16x8 summary at ZZ in {12,14,16,18}-bits, with a cell nr in {0,127}:
 *   http://linkedgeodata.org/facetmap/ZZ0000YYYYYY> <http://linkedgeodata.org/facetmap/facettileZZ>    facethash|quadtileZZ
 *   <http://linkedgeodata.org/facetmap/ZZ0000YYYYYY> <http://linkedgeodata.org/facetmap/count>          count
 *   <http://linkedgeodata.org/facetmap/ZZ0000YYYYYY> <http://linkedgeodata.org/facetmap/cell>           cell
 *   <http://linkedgeodata.org/facetmap/ZZ0000YYYYYY> <http://linkedgeodata.org/facetmap/example>        <http://linkedgeodata.org/triplify/node1000XXXXXX>
 *   <http://linkedgeodata.org/facetmap/ZZ0000YYYYYY> <http://www.w3.org/2003/01/geo/wgs84_pos#lat>      latitude
 *   <http://linkedgeodata.org/facetmap/ZZ0000YYYYYY> <http://www.w3.org/2003/01/geo/wgs84_pos#long>     longitude
 *   
 * - TileFacet counts at ZZ in {12,14,16,18,20,22,24}-bits:
 *   <http://linkedgeodata.org/facetcount/ZZ000YYYYYY> <http://linkedgeodata.org/facetcount/tilefacetZZ> quadtileZZ|facethash
 *   <http://linkedgeodata.org/facetcount/ZZ000YYYYYY> <http://linkedgeodata.org/facetcount/count>       count
 */ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINE 16384

#define PRIME0 899809343 
#define MISS (1<<20)

unsigned int strhash20(char* s) {
    unsigned int h32 = 0;
    while(*s) {
        h32 = (h32*PRIME0)^(*s++);
    }
    return (h32^(h32>>16)) & (MISS-1);
}

typedef struct { void* next; unsigned int id; char val[1]; } hash_t;
#define FACETBITS 10
#define HASH ((1<<14)-1)
hash_t *hash[HASH+1] = {0};
char *facet[1<< FACETBITS];
unsigned int facetbits = 3;

int readfacets(char *facetfile) {
    char buf[MAXLINE];
    FILE *fp = fopen(facetfile, "r");
    unsigned int id = 0;
    if (fp == NULL) {
        fprintf(stderr, "error: cannot open facet URI file %s.\n", facetfile);
        return -1;
    }
    while(fgets(buf, MAXLINE, fp) != NULL) {
        unsigned l, nr;
        hash_t* h;
        if (id >= (1<<FACETBITS)) {
            fprintf(stderr, "warning: more than %d facets in %s (ignoring the tail).\n", 1<<FACETBITS, facetfile);
            return 0;
        }
        buf[MAXLINE-1] = 0;
	l = strlen(buf);
	if (l && buf[l-1] == '\n') buf[l-1] = 0;
        nr = strhash20(buf);
        h = (hash_t*) malloc(sizeof(hash_t)+l);
        if (h == NULL) {
            fprintf(stderr, "error: failed to allocate hash table.");
            return -1;
        }
        memcpy(h->val, buf, l+1);
        h->id = id;
        facet[id++] = h->val; 
        h->next = hash[nr&HASH];
        hash[nr&HASH] = h;
    }
    fclose (fp);
    if (id > 8) {
        for(id--, facetbits=0; id > 0; facetbits++) id >>=1;
    } 
    return 0;
}

unsigned int getcode(char* p) {
    unsigned int nr = strhash20(p);
    hash_t *h;
    for(h = hash[nr&HASH]; h; h = (hash_t*) h->next) 
         if (strcmp(p, h->val)==0) 
		return h->id;
    return MISS;
}

/*                        12345678901234567890123456789012345678901234567890  */
#define URI_POLYGON      "<http://www.georss.org/georss/polygon>"
#define URI_POLYGON_LEN  38
#define URI_LINE         "<http://www.georss.org/georss/line>"
#define URI_LINE_LEN     35
#define URI_POINT        "<http://www.w3.org/2003/01/geo/wgs84_pos#geometry>"
#define URI_POINT_LEN    50
#define URI_TYPE         "<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>"
#define URI_TYPE_LEN     49 
#define HTTP             "<http://"
#define POINT            "\"POINT("
#define POINT_LEN        7

#define RESET_BOX { lat_lo = 91; lat_hi = -91; lon_lo = 181; lon_hi = -181; }
#define MAXTILES 100

static int flushfacets(char *nid);

unsigned int linenr=0, tilenr=0, tile[MAXTILES];
double lat_lo, lat_hi, lon_lo, lon_hi;
FILE *fpout[64];

int main(int argc, char* argv[]) {
    unsigned int i;
    char buf[MAXLINE], *nid, oldnid[MAXLINE], *p, *q;
    FILE *fpin = stdin;
    RESET_BOX;

    /* open input */
    if (argc != 4 && argc != 3) {
        fprintf(stderr, "usage: %s facetfile outputdir [input].\n", argv[0]);
        return -1;
    }
    if (readfacets(argv[1]) <0) return 1;

    /* open output files */
    for(i=0; i<64; i++) {
        char *path = alloca(strlen(argv[2])+20);
        sprintf(path, "%s%u_%u.txt", argv[2], 1+(i/8), 1+(i%8));
        fpout[i] = fopen(path, "a");
        if (fpout[i] == NULL) {
            fprintf(stderr, "error: cannot open file %s.\n", path);
            return -1;
        }
    }
    if (argc == 4) {
        fpin = fopen(argv[3], "r");
        if (fpin == NULL) {
            fprintf(stderr, "error: could not open inputfile %s.\n", argv[3]);
            return -1;
        }
    }

    /* parse N3 triples from stdin, assumed in SPO order (!) */
    *oldnid = 0;
    while((p=fgets(buf, MAXLINE, fpin)) != NULL) {
        buf[MAXLINE-1] = 0; 
        linenr++;
        
        /* find subject (here called nid 'nodeid') */               
        while(*p == ' ' || *p == '\t') p++;
        nid = p;
        while(*p && *p != ' ' && *p != '\t') p++;
        if (*p == 0) {
            fprintf(stderr, "warning: illegal subject at line %u.\n", linenr);
            RESET_BOX;
        }
        *p++ = 0;
        if (strcmp(nid,oldnid)) {
            if (*oldnid && flushfacets(oldnid) < 0) return -1;
            strcpy(oldnid, nid);
            RESET_BOX;
        }
        if ((p = strstr(p, HTTP)) == NULL) {
            fprintf(stderr, "warning: illegal property at line %u.\n", linenr);
            RESET_BOX;
        } else {
            if (strncmp(p, URI_POLYGON, URI_POLYGON_LEN) == 0 || strncmp(p, URI_LINE, URI_LINE_LEN) == 0) {
                if ((p = strchr(p+1+((URI_POLYGON_LEN<URI_LINE_LEN)?URI_POLYGON_LEN:URI_LINE_LEN), '"')) != NULL) {
                    int d=1;
                    RESET_BOX;
                    for(p++; *p != 0 && *p != '"'; d++) {
                        double lon, lat = strtod(p, &p);
                        if (p == NULL) { 
                            fprintf(stderr, "warning: illegal polyline at line %u, could not find longitude %d.\n", d, linenr);
                            RESET_BOX;
                            break; 
                        }
                        while(*p == ' ' || *p == '\t') p++;
                        lon = strtod(p, &p);
                        if (p == NULL) { 
                            fprintf(stderr, "warning: illegal polyline at line %u, could not find latitude %d.\n", d, linenr);
                            RESET_BOX;
                            break; 
                        }
                        while(*p == ' ' || *p == '\t') p++;
                        /* get bounding box */
                        if (lat < lat_lo) lat_lo = lat;
                        if (lat > lat_hi) lat_hi = lat;
                        if (lon < lon_lo) lon_lo = lon;
                        if (lon > lon_hi) lon_hi = lon;
                    }
                }
            } else if (strncmp(p,URI_POINT, URI_POINT_LEN) == 0) {
                if ((p = strstr(p+URI_POINT_LEN+1, POINT)) != NULL) {
                    lon_lo = lon_hi = strtod(p+POINT_LEN, &p);
                    if (p) {
                        while(*p == ' ' || *p == '\t') p++;
                        lat_hi = lat_lo = strtod(p, &p);
                    }
                    if (p == NULL) {
                        fprintf(stderr, "warning: illegal point at line %u.\n", linenr);
                        RESET_BOX;
                    }
                }
            } else if (strncmp(p,URI_TYPE, URI_TYPE_LEN) == 0) {
                p += URI_TYPE_LEN;
                while(*p == ' ' || *p == '\t') p++;
                for(q=p; *q; q++) 
                    if (*q == ' ' || *q == '\t') { *q = 0; break; }
                if (tilenr < MAXTILES) {
                    unsigned int id = getcode(p);
                    if (id != MISS) tile[tilenr++] = id; 
                }
            }
        }
    }
    return flushfacets(nid);
}

/* hardcoded balancing functions that make the partitions a bit more equi-sized */
int qsplit[32] = { 0,0,0,0,0,0,0,0,0,1,1,1,2,3,4,4,5,5,5,5,5,5,5,5,5,5,6,7,7,7,7,7 };

static int flushfacets(char *nid) {
    if (lat_lo > lat_hi || lon_lo > lon_hi || lon_lo < -180 || lon_hi > 180 || lat_lo < -90 || lat_hi > 90) {
        fprintf(stderr, "warning: ignoring facet at line %u without valid geometry (%g,%g)-(%g,%g).\n", linenr, lon_lo, lat_lo, lon_hi, lat_hi);
    } else while(tilenr > 0) {
        unsigned int i, j, x, y, h = tile[--tilenr];
        unsigned int ilat_lo = (lat_lo+90)*(65536.0/180.0);
        unsigned int ilat_hi = (lat_hi+90)*(65536.0/180.0);
        unsigned int ilon_lo = (lon_lo+180)*(65536.0/360.0);
        unsigned int ilon_hi = (lon_hi+180)*(65536.0/360.0);
        double lon = lon_lo, lat = lat_lo;
        for(x=0; ilon_lo <= ilon_hi; x++, ilon_lo += (1<<5)) { /* bits per lon cell 5 = 18/2 - 4 */
            for(y=0; ilat_lo <= ilat_hi; y++, ilat_lo += (1<<6)) { /* bits per lat cell 6 = 18/2 - 3 */
                unsigned int j, q=0, z = x|y;

                /* determine the quadtile number */
                for(j=15; j<16; j--) {
                    q |= (((ilon_lo>>j)&1)<<(j+j+1)) | (((ilat_lo>>j)&1)<<(j+j+0));
                }

                /* determine the bit granularity for which this is relevant */
                for(j=0;j<3;j++) {
                    if (z & (1<<j)) break; /* count common lowest 0 bits in x,y */
                }
                z = 12 + 2*(3-j); /* 3 common=>12bits, 0 common=>18bits */ 

                /* determine the output file based on quadtile and facet highest bits */
                j = (qsplit[(q >> 27) & 31] << 3) | ((h >> (facetbits-3))&7);

                if (x | y) { 
                    lat = (ilon_lo*360.0)/(65536)-180;
                    lon = (ilat_lo*180.0)/(65536)-90;
                }
                if (fprintf(fpout[j], "%u %u %u %s %g %g %s\n", h, q, z, nid, lon, lat, facet[h]) >= 0) continue;
                fprintf(stderr, "error: writing to file %u_%u.txt.\n", j>>3, j&7);
                return -1; 
            }
        }
    }
    return 0;
}
