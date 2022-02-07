# Compact data structures for the PBWT 

Prequesites:
 - `make` 
 - `gcc`


The build process via `make` compiles the executable `pbwt.x`.
Parameters:

```
	-v : [flag] verbose flag 
	-s : [double] sampling threshold s: if less than s% of all entries of the column are 1, it is discarded. (default = 0 = disabled) 
	-i : [filename] input binary matrix file 
	-d : [filename] outfile for div array 
	-l : [filename] write the log to a log file instead of stdout 
	-c : [filename] output the hash g_hashtable storing the DAG-compressed Cartesian trees 
	-n : [filename] output the interval representation of the Cartesian trees 
	-m : [filename] file to write the input matrix without the columns removed by the threshold (the matrix is stored rowwise by individuals, i.e., the transpose of the orignal input)
	-w : [int] limit the number of columns to process 
	-h : [int] limit the number of individuals to process (height of the PBWT matrix)
```

A sample dataset as input can be found at http://dolomit.cs.tu-dortmund.de/tudocomp/pbwt_matrix.xz
