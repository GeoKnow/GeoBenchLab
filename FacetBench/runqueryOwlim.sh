tmpfile=`echo $1 | sed 's/\//\_/g'`.tmp

echo "connect http://localhost:8080/openrdf-sesame." > $tmpfile

echo "open olgeo1." >> $tmpfile

cat $1 >> $tmpfile

echo "quit." >> $tmpfile

console.sh < $tmpfile

rm $tmpfile
