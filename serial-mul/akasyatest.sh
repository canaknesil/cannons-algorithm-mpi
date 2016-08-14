for i in 100 200 300 400 500 600 700 800 900 1000 1100 1200 1300 1400
do
	mpirun -np 4 ./serial-mul.out -s $i -o >> akasyaoutput.txt
done

