for i in {1..60}
do
	sq=expr $i \* $i
	echo "${i} ${sq}"
done