echo "Split the scale factor $1"

gunzip LGD-Dump-RelevantNodes.sorted.$1.nt.gz

split -l 100000000 -d "LGD-Dump-RelevantNodes.sorted.$1.nt" "LGD-Dump-RelevantNodes.sorted.$1.nt"


gunzip LGD-Dump-RelevantWays.sorted.$1.nt.gz

split -l 100000000 -d "LGD-Dump-RelevantWays.sorted.$1.nt" "LGD-Dump-RelevantWays.sorted.$1.nt"


gzip LGD-Dump-Relevant*.nt?*
