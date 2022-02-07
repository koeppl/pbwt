# Compact data structures for the PBWT 

## 🧰 Prequesites:
 - `make` 
 - `gcc`
 - `rust` and `cargo` for converting the DAG-compressed Cartesian trees into the SOLCA input format used in ShapedSLP
 - [ShapedSLP](https://github.com/itomomoti/ShapedSlp) for encoding an SLP grammar such as RePair or SOLCA

## 🚀 Complete Test Run
```bash
	git clone https://github.com/koeppl/pbwt
	cd pbwt
	wget http://dolomit.cs.tu-dortmund.de/tudocomp/pbwt_matrix.xz
	unxz pbwt_matrix.xz
	make
	./pbwt.x -i pbwt_matrix -d div_array -c cartesian_dag -m matrix -w 30 -h 30
	cd transform_grammar
	cargo build
	cargo run -- -i ../cartesian_dag -o cartesian_dag.solca
```

## 🏗️ Building

The build process via `make` compiles the executable `pbwt.x`.  
The directory `transform_grammar` contains rust code that can be compiled via `cargo build`.

## 🎌 Parameters of pbwt.x

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

## 📚 References
- Richard Durbin: Efficient haplotype matching and storage using the positional Burrows-Wheeler transform (PBWT). Bioinform. 30(9): 1266-1272 (2014)
