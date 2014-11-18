#FAGI-gis Datasets

- <b>Polygon/Point Datasets</b>

The two datasets contain data parsed from wikimapia and tranformed into RDF (n-triples format) 
and cover the areas of Athens, London, Leipzig, Berlin and New York. 
The first dataset contain polygon geometries, while the second dataset has point geometries resulted from
the polygons of the first dataset. These points are also shifted to a degree in order 
to demonstrate better the geometry fusion process. Regarding the non-spatial data, 
both datasets contain a basic common information of the entities, whereas the polygon dataset 
contains additional edit information and the point dataset contains additional metadata for location 
and translate information. 

- <b>Links</b>

The datasets refer to the same wikimapia entities. Considering this, 
every entity of each dataset can be interlinked with its corresponding entity from the other. 
The links file contains these interlinked entities. 
We have three link files which contain 1000, 10.000 and 100.000 interlinked entities respectively.

The sample directory contains a small sample of the aforementioned datasets and links.
