DEER Benchmark
==============
DEER - RDF Data Extraction and Enrichment Framework

This benchmark consists of a set of DEER RDF specifications. 
Throughout the GeoKnow project, the execution time of executing those link specifications will be measured to track the effect of performance improvements.

We used three publicly available datasets for our experiments:

1. From the biomedical domain, we chose DrugBank5 as our first dataset. DrugBank is the Linked Data version of the DrugBank database, which is a repository of almost 5000 FDA-approved small molecule and biotech drugs. It contains detailed information about drugs including chemical, pharmacological and pharmaceutical data. We chose this dataset because it is linked with many other datasets6 , from which we can extract enrichment data using our atomic enrichment functions. For our experiments we deployed a manual enrichment pipeline Mmanual , in which we enrich the drug data found in DrugBank using abstracts dereferenced from DBpedia, then we conform both DrugBank and DBpedia source authority URIs to one unified URI. For DrugBank we manually deployed two experimental pipelines:

2. From the music domain, we chose the Jamendo8 Dataset. Jamendo contains a large collection of music related information about artists and recordings. We select this dataset as it contains a substantial amount of embedded information hidden in literal properties such as mo:biography. The goal of our enrichment process is to add a geospatial dimension to Jamendo, e.g. location of a recording or place of birth of a musician. To this end, we deployed a manual enrichment pipeline, in which we enrich Jamendo’s music data by adding additional geo-spacial data found by applying the Named Entity Recognition (NER) of the NLP enrichment function against mo:biography. For Jamendo we deploy manually one experimental pipeline

3. From the multi-domain knowledge base DBpedia we select the class AdministrativeRegion to be our third dataset. As DBpedia is a knowledge base with a complex ontology, we build a set of 5 pipelines of increasing complexity based on this dataset

