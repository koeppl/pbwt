#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>


size_t g_limit_individuals = 0;
size_t g_number_of_individuals = 0;

bool die_errno(const char* str) {
	fprintf(stderr, "%s: %s\n", str, strerror(errno));
	exit(1);
	return true;
}

bool die(const char* str) {
	fprintf(stderr, "%s\n", str);
	exit(1);
	return true;
}


size_t readMatrixLine(FILE* input, int** matrixColumn, size_t* number_of_individuals) {
	char* line = NULL;
	size_t line_capacity = 0;
	int line_length = 0;
	if((line_length = getline(&line, &line_capacity, input)) == -1) {
		if(line != NULL) { free(line); }
		return 0;
		/* die("could not get line from input"); */
	}

	size_t column = 0;
	size_t i = 0;
	//@ an entry has the form 'SITE:   0        8.51469239e-06     0.118007104 <actual data>'
	//so drop the first 4 columns
	for(; column < 4 && i < (size_t)line_length; ++i) {
		if(line[i] == ' ' || line[i] == '\t') {
			while(line[i] == ' ' || line[i] == '\t') {
				++i;
			}
			++column;
		}
	}
	size_t end_line = line_length;
	while(end_line > 0 && (line[end_line-1] == '\r' || line[end_line-1] == '\n')) {
		--end_line;
	}
	if(*matrixColumn == NULL) {
		*number_of_individuals = end_line-i;
		*matrixColumn = (int*) malloc(*number_of_individuals*sizeof(int));
	}
	size_t matrixColumnIndex = 0;
	for(; i < end_line; ++i) {
		if(line[i] != '1' && line[i] != '0') {
			fprintf(stderr, "could not parse '%d' in line ...%s\n", line[i], line+i);
			exit(1);
		}
		(*matrixColumn)[matrixColumnIndex++] = line[i] == '0' ? 0 : 1;
		if(matrixColumnIndex > *number_of_individuals) {
			fprintf(stderr, "line surpassed g_number_of_individuals : %s\n", line);
			exit(1);
		}
	}
	free(line);
	return matrixColumnIndex;
}




#define MIN(a, b) (a < b ? a : b)

#define HASH_TABLE_SIZE 35866000


typedef struct treeNode {
	struct treeNode *left;
	struct treeNode *right;
	int size;
	int start;
	int end;
} treeNode;

treeNode** g_gargabe_nodes = NULL;
size_t g_gargabe_nodes_length = 0;

void mark_garbage_node(treeNode* node) {
	g_gargabe_nodes = (treeNode**) realloc(g_gargabe_nodes, (++g_gargabe_nodes_length)*sizeof(treeNode *));
	g_gargabe_nodes[g_gargabe_nodes_length-1] = node;
}
void clean_garbage_nodes() {
	if(g_gargabe_nodes == NULL) { return; }
	for(size_t i = 0; i < g_gargabe_nodes_length; ++i) {
		free(g_gargabe_nodes[i]);
	}
	free(g_gargabe_nodes);
	g_gargabe_nodes = NULL;
	g_gargabe_nodes_length = 0;
}


typedef struct listNode {
	struct listNode *prec;
	treeNode *left;
	int div;
	treeNode *right;
	struct listNode *succ;
} listNode;

typedef struct chainNode {
	uint64_t rootHash;
	uint64_t leftHash;
	uint64_t rightHash;
	int size;
	struct chainNode *succ;
} chainNode;

size_t g_interval_count = 0;
int printIntervals(treeNode *v, int PBWTColumn[], FILE *filePtr) {
#define PURPLE		-1
	/* if( v == NULL) { */
	/* 	return VOID; */
	/* } */
	if (v -> left == NULL && v -> right == NULL) {
		for (size_t i = v -> start; i < (size_t) v -> end; i++) {
			assert(i+1 < g_limit_individuals);
			if (PBWTColumn[i] != PBWTColumn[i + 1]) {
				fprintf(filePtr, "[%i, %i] ", v -> start, v -> end);
				return(PURPLE);
			}
		}
		
		return(PBWTColumn[v -> start]);
	}
	assert(v -> left != NULL && v -> right != NULL);
	
	int leftColour = printIntervals(v -> left, PBWTColumn, filePtr);
	int rightColour = printIntervals(v -> right, PBWTColumn, filePtr);
	
	if (leftColour == rightColour) {
		return(leftColour);
	}

	fprintf(filePtr, "[%i, %i] ", v -> start, v -> end);
	g_interval_count++;
	return(PURPLE);
}


/* int readMatrixColumn(); */
void printArray(int array[], FILE *filePtr);
void printPBWTColumn(int matrixColumn[], int perm[], FILE *filePtr);
void updatePermDiv(int matrixColumn[], int oldPerm[], int oldDiv[]);

treeNode *createTree(int div[], int columnNum);
int nodeCompare(const void *x, const void *y);
uint64_t hashTree(treeNode *root);
void printRules(FILE *filePtr);

void printGrammar(FILE *filePtr);

chainNode *table[HASH_TABLE_SIZE] = {NULL};
int g_collision_flag = 0;

int* newPerm;
int* newDiv;
treeNode **g_leaves;
listNode **g_leaf_list;

/* void freeTree(treeNode *root) { */
/* 	if(root->size == 1 && root->left == NULL && root->right == NULL) { return; } //@ are freed by g_leaves separetely */
/* 	if(root == NULL) { return; } */
/* 	if(root->left != NULL) { */
/* 		freeTree(root->left); */
/* 	} */
/* 	if(root->right!= NULL) { */
/* 		freeTree(root->right); */
/* 	} */
/* 	free(root); */
/* } */

void printDivArray(FILE* div_file, int* div_array) {
	for(size_t i = 0; i < g_limit_individuals; ++i) {
		uint32_t diff = abs(div_array[i] - div_array[i-1])<<1;
		if(div_array[i] < div_array[i-1]) diff |= 1;
		assert(diff < 1ULL<<30);
		fwrite(&diff, sizeof(int32_t),  1, div_file);
	}
}
size_t number_of_individuals = 0;

int main(const int argc, char *const argv[]) {
	size_t column_limit = (size_t)-1;
	bool flag_verbose = false;
	const char* infilename = NULL;
	FILE* infile = NULL;
	FILE* out_hash_file = NULL;
	FILE* out_divarray_file = NULL;
	FILE* out_interval_file = NULL;
	FILE* out_log_file = stdout;
	double flag_threshold = 0.0;
	{
		int c;
		while((c = getopt(argc, argv, "i:h:d:g:n:vc:l:m:t:")) != -1) { 
			switch (c) {
				case 't': 
					errno = 0;
					flag_threshold = strtod(optarg, NULL);
					if(errno != 0) { die_errno("could not parse flag_threshold"); }
					break;
				case 'c': 
					errno = 0;
					g_limit_individuals = strtoul(optarg, NULL, 10);
					if(errno != 0) { die_errno("could not parse g_limit_individuals"); }
					break;
				case 'm': 
					errno = 0;
					column_limit = strtoul(optarg, NULL, 10);
					if(errno != 0) { die_errno("could not parse column_limit"); }
					break;
				case 'v':
					flag_verbose = true;
					break;
				case 'i':
					infilename = optarg;
					infile = fopen(optarg, "r");
					if(infile == NULL) { die_errno("could not open infile"); }
					break;
				case 'h':
					out_hash_file = fopen(optarg, "w");
					if(out_hash_file == NULL) { die_errno("could not open outfile for hash table"); }
					break;
				case 'l':
					out_log_file = fopen(optarg, "w");
					if(out_log_file == NULL) { die_errno("could not open outfile for log file"); }
					break;
				case 'd':
					out_divarray_file = fopen(optarg, "w");
					if(out_divarray_file == NULL) { die_errno("could not open outfile for divarray"); }
					break;
				case 'n':
					out_interval_file = fopen(optarg, "w");
					if(out_interval_file == NULL) { die_errno("could not open outfile for interval"); }
					break;
				case '?':
					if (optopt == 'c')
						fprintf (stderr, "Option -%c requires an argument.\n", optopt);
					else
						fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
					return 1;
				default:
					abort ();
			}
		}
	}
	if(infile == NULL) { 
		die(
				"No infile given!\n\n"
	"\
	parameters: \n\
	-v : [flag] verbose flag \n\
	-t : [double] threshold t: if less than t% of all entries of the column are 1, it is discarded. (default = 0 = disabled) \n\
	-i : [filename] input binary matrix file \n\
	-n : [filename] outfile for the intervals \n\
	-d : [filename] outfile for div array \n\
	-l : [filename] write the log to a log file instead of stdout \n\
	-h : [filename] output the hash table storing the DAG-compressed Cartesian trees \n\
	-n : [filename] output the interval representation of the Cartesion trees \n\
	-m : [int] limit the number of columns to process \n\
	-c : [int] limit the number of individuals to process \n\
	"
	"");

	}

	int* matrixColumn = NULL; //(int*) malloc(g_number_of_individuals*sizeof(int));
	{
		char* line = NULL;
		size_t line_length = 0;
		//@ read header consisting of two rows
		for(size_t i = 0; i < 2; ++i){
			if(getline(&line, &line_length, infile) == -1) { die("could not parse header"); }
			if(line != NULL) {
				free(line);
				line = NULL;
			}
		}
		readMatrixLine(infile, &matrixColumn, &g_number_of_individuals);
		if(g_number_of_individuals == 0) { die("Number of individuals is zero!"); }
		if(g_limit_individuals == 0) { g_limit_individuals = g_number_of_individuals; }
		fclose(infile);
		infile = fopen(infilename, "r");
		for(size_t i = 0; i < 2; ++i){
			if(getline(&line, &line_length, infile) == -1) { die("could not parse header"); }
			if(line != NULL) {
				free(line);
				line = NULL;
			}
		}
	}


	/* #<{(| int maxcolumns = INT_MAX; |)}># */
	/* if (scanf("%i", &g_number_of_individuals) != 1) { */
	/* 	return(1); */
	/* } */
	/* g_limit_individuals = g_number_of_individuals; */
	/* if(argc > 1) { */
	/* 	g_limit_individuals = atoi(argv[1]); */
	/* 	if(g_limit_individuals > g_number_of_individuals) { */
	/* 		g_limit_individuals = g_number_of_individuals; */
	/* 	} */
	/* 	fprintf(stderr, "Limiting #individuals to %d\n", g_limit_individuals); */
	/* } */
	
	g_leaves = (treeNode**) malloc(g_limit_individuals*sizeof(treeNode*));
	for(size_t i = 0; i < g_limit_individuals; ++i) {
		g_leaves[i] = (treeNode *) malloc(sizeof(treeNode));
	}


	int* perm = (int*) malloc(g_limit_individuals*sizeof(int)); // suffix array like, colex 
	int* div = (int*) malloc(g_limit_individuals*sizeof(int)); // LCS array
	newDiv = (int*) malloc(g_limit_individuals*sizeof(int)); // LCS array
	newPerm = (int*) malloc(g_limit_individuals*sizeof(int)); // LCS array

	g_leaf_list = (listNode**) malloc(sizeof(listNode*)*(g_limit_individuals + 1));
	for(size_t i = 1; i < g_limit_individuals; i++) {
		g_leaf_list[i] = (listNode *) malloc(sizeof(listNode));
	}
	
	for(size_t i = 0; i < g_limit_individuals; i++) {
		perm[i] = i;
		div[i] = 0;
	}


	size_t number_of_ones = 0;

	size_t number_of_pbwt_runs = 0;
	
	int* PBWTColumn = (int*) malloc(g_limit_individuals*sizeof(int));

	if(flag_verbose) { fprintf(stderr, "Startup\n"); }
	
	size_t processedColumns = 0;
	size_t columnNum = 0;
	assert(matrixColumn != NULL);
	for(; columnNum < column_limit; columnNum++) {
		const size_t read_values = readMatrixLine(infile, &matrixColumn, &g_number_of_individuals);
		if(read_values == 0) { break; }
		assert(read_values == g_number_of_individuals);
		
		size_t count_ones = 0;
		for(size_t i = 0; i < g_limit_individuals; ++i) {
			if(matrixColumn[i] == 1) { ++count_ones; }
		}
		if(count_ones < g_limit_individuals*flag_threshold) {
			continue;
		}
		number_of_ones += count_ones;


		
		if(flag_verbose) {
			printArray(matrixColumn, out_log_file);
			printPBWTColumn(matrixColumn, perm, out_log_file);
			/* printf("\n %d PERM = ", processedColumns); */
			/* printArray(perm, stdout); */
			/* printf("\n %d DIV = ", processedColumns); */
			/* printArray(div, stdout); */
			/* printf("\n"); */
		}

		treeNode *root = createTree(div, processedColumns);
		if(out_hash_file != NULL) {
			uint64_t hash = hashTree(root);
			if(g_collision_flag == 0) {
				if(flag_verbose) { fprintf(out_log_file, "%zu: %lu\n\n", processedColumns, hash); }
			} else {
				fprintf(stderr, "\n\nConstruction Failed!\n\n");
				return(1);
			}
		}

		for(size_t i = 0; i < g_limit_individuals; i++) {
			PBWTColumn[i] = matrixColumn[perm[i]];
		}
		{
			int run_char = PBWTColumn[0];
			++number_of_pbwt_runs;
			for(size_t i = 1;  i < g_limit_individuals; i++) {
				if(run_char != PBWTColumn[i]) {
					++number_of_pbwt_runs;
					run_char = PBWTColumn[i];
				}
			}
		}

		if(out_interval_file) {
			printIntervals(root, PBWTColumn, out_interval_file);
		}

		clean_garbage_nodes();
		/* freeTree(root); */
		
		updatePermDiv(matrixColumn, perm, div);
		if(out_divarray_file) {
			printDivArray(out_divarray_file, div);
		}

		if(processedColumns % 50 == 0 ) {
			fprintf(out_log_file, "Processed/Read: %zu/%zu\r", processedColumns, columnNum);
			fflush(out_log_file);
		}
		++processedColumns;
	}
	fprintf(out_log_file, "#1s in processed columns: %zu\n", number_of_ones);
	fprintf(out_log_file, "Processed Columns: %zu\n", processedColumns);
	fprintf(out_log_file, "Total Columns: %zu\n", columnNum);
	fprintf(out_log_file, "PBWT Character Runs: %zu\n", number_of_pbwt_runs);
	
	if(out_hash_file) {
		/* printRules(out_hash_file); */
		printGrammar(out_hash_file);
	}

	if(out_interval_file) {
		fprintf(stderr, "#intervals = %zu\n", g_interval_count);
	}

	fclose(infile);
	if(out_hash_file) { fclose(out_hash_file); }

	if(out_divarray_file) { fclose(out_divarray_file); }
	if(out_interval_file) { fclose(out_interval_file); }
	if(out_log_file != stdout) { fclose(out_log_file); }

	clean_garbage_nodes();

	for(size_t x = 0; x < g_limit_individuals; ++x) {
		free(g_leaf_list[x]);
		free(g_leaves[x]);
	}
	free(g_leaf_list);
	free(g_leaves);

	free(PBWTColumn);
	free(matrixColumn);
	free(perm);
	free(div);
	free(newDiv);
	free(newPerm);
	return(0);
}


/* int readMatrixColumn(int matrixColumn[]) { */
/* 	size_t i; */
/* 	 */
/* 	for (i = 0; i < g_number_of_individuals; i++) { */
/* 		const int ret = scanf("%i", &matrixColumn[i]); */
/* 		if(ret != 1) {  */
/* 			break;  */
/* 			fprintf(stderr, "wanted to read int, but found unparsable character: %c\n", getchar()); */
/* 		} */
/* 		if(matrixColumn[i] < 0 || matrixColumn[i] > 1) { */
/* 			break; */
/* 		} */
/* 	} */
/* 	 */
/* 	return(i); */
/* } */


void printArray(int array[], FILE *filePtr) {
	for (size_t i = 0; i < g_limit_individuals; i++) {
		fprintf(filePtr, "%i ", array[i]);
	}
	
	fprintf(filePtr, "\n");

	return;
}


void printPBWTColumn(int matrixColumn[], int perm[], FILE *filePtr) {
	for (size_t i = 0; i < g_limit_individuals; i++) {
		fprintf(filePtr, "%i ", matrixColumn[perm[i]]);
	}
	
	fprintf(filePtr, "\n");

	return;
}


void updatePermDiv(int matrixColumn[], int oldPerm[], int oldDiv[]) {
	/* int newPerm[g_limit_individuals]; */
	/* int newDiv[g_limit_individuals]; */
	
	int count0 = 0;
	int lcs = 0;
	
	for (size_t i = 0; i < g_limit_individuals; i++) {
		lcs = MIN(lcs, oldDiv[i] + 1);
		if (matrixColumn[oldPerm[i]] == 0) {
			newPerm[count0] = oldPerm[i];
			newDiv[count0] = lcs;
			count0++;
			lcs = INT_MAX;
		}
	}
	
	int count1 = 0;
	lcs = 0;

	for (size_t i = 0; i < g_limit_individuals; i++) {
		lcs = MIN(lcs, oldDiv[i] + 1);
		if (matrixColumn[oldPerm[i]] == 1) {
			newPerm[count0 + count1] = oldPerm[i];
			newDiv[count0 + count1] = lcs;
			count1++;
			lcs = INT_MAX;
		}
	}
	
	memcpy(oldPerm, newPerm, g_limit_individuals * sizeof(int));
	memcpy(oldDiv, newDiv, g_limit_individuals * sizeof(int));
	
	return;
}



treeNode *createTree(int div[], int columnNum) {
	for (size_t i = 0; i < g_limit_individuals; i++) {
		g_leaves[i] -> left = NULL;
		g_leaves[i] -> right = NULL;
		g_leaves[i] -> size = 1;
		g_leaves[i] -> start = i;
		g_leaves[i] -> end = i;
	}
	
	g_leaf_list[0] = NULL;
	g_leaf_list[g_limit_individuals] = NULL;
	
	for (size_t i = 1; i < g_limit_individuals; i++) {
		g_leaf_list[i] -> left = g_leaves[i - 1];
		g_leaf_list[i] -> div = div[i];
		g_leaf_list[i] -> right = g_leaves[i];
	}
	
	for (size_t i = 1; i < g_limit_individuals; i++) {
		g_leaf_list[i] -> prec = g_leaf_list[i - 1];
		g_leaf_list[i] -> succ = g_leaf_list[i + 1];
	}
	
	qsort(&g_leaf_list[1], g_limit_individuals - 1, sizeof(listNode *), nodeCompare);
	
	for (size_t i = 1;; i++) {
		treeNode *parent = (treeNode *) malloc(sizeof(treeNode));
		mark_garbage_node(parent);
		parent -> size = (g_leaf_list[i] -> left) -> size + (g_leaf_list[i] -> right) -> size;
		
		if (g_leaf_list[i] -> div < columnNum) {
			parent -> left = g_leaf_list[i] -> left;
			parent -> right = g_leaf_list[i] -> right;
		} else { // non-binary case
			parent -> left = NULL;
			parent -> right = NULL;
		}

		parent -> start = (g_leaf_list[i] -> left) -> start;
		parent -> end   = (g_leaf_list[i] -> right) -> end;
		
		if (g_leaf_list[i] -> prec != NULL) {
			(g_leaf_list[i] -> prec) -> right = parent;
			(g_leaf_list[i] -> prec) -> succ = g_leaf_list[i] -> succ;
		}
		
		if (g_leaf_list[i] -> succ != NULL) {
			(g_leaf_list[i] -> succ) -> left = parent;
			(g_leaf_list[i] -> succ) -> prec = g_leaf_list[i] -> prec;
		}
		
		if (g_leaf_list[i] -> prec == NULL && g_leaf_list[i] -> succ == NULL) {
			return(parent);
		}
	}
}


int nodeCompare(const void *x, const void *y) {
	listNode *p = *((listNode **) x);
	listNode *q = *((listNode **) y);
	
	return(q -> div - p -> div);
}


size_t hash_counter = 0;
uint64_t hashTree(treeNode *root) {
	
	if (root != NULL) {
		uint64_t leftHash = hashTree(root -> left);
		uint64_t rightHash = hashTree(root -> right);
		
		uint64_t rootHash;
		/* uint64_t rootHash = */
		/* 	((459379399LL * leftHash ) ) + */
		/* 	((546141984LL * rightHash) ) + */
		/* 	((228699788LL * ((long long) (root -> size)))); */
		/* #<{(| % 963774947LL; |)}># */

		/* rootHash = */
		/* (uint64_t)((459379399LL * leftHash + 506552055LL) % 609904837LL + 609904837LL) + */
		/* (uint64_t)((546141984LL * rightHash + 151909458LL) % 677870341LL + 677870341LL) + */
		/* (uint64_t)(((228699788LL * ((long long) (root -> size)) + 771204862LL)) % 963774947LL); */

		/* rootHash = */
		/* (uint64_t)((53617LL * leftHash + 7537LL)                       % 262139LL) + */
		/* (uint64_t)((31545LL * rightHash + 17815LL)                 % 524287LL) + */
		/* (uint64_t)(((1511LL * ((long long) (root -> size)) + 5336LL)) % 1048573LL); */

		rootHash =
		(uint64_t)((9465872627808054177ULL * leftHash + 5191322160178139765ULL) % 18446744073709551557ULL) ^
		(uint64_t)((5813985129306799692ULL * rightHash + 4065133683220091270ULL) % 9223372036854775421ULL) ^
		(uint64_t)(((2460960945928696877ULL * ((uint64_t) (root -> size)) + 227720850924467681ULL)) % 4611686018427387709ULL);
		
		chainNode *chainPtr = table[(rootHash % (uint64_t) HASH_TABLE_SIZE)];
		
		while (chainPtr != NULL) {
			if (chainPtr -> rootHash == rootHash) {
				if (chainPtr -> leftHash != leftHash ||
					chainPtr -> rightHash != rightHash ||
					chainPtr -> size != root -> size) {
					g_collision_flag = 1;
					fprintf(stderr, "hash counter: %lu\n", hash_counter);
				}
				break;
			}
			chainPtr = chainPtr -> succ;
		}
		
		if (chainPtr == NULL) {
			chainNode *node = (chainNode *) malloc(sizeof(chainNode));
			node -> rootHash = rootHash;
			node -> leftHash = leftHash;
			node -> rightHash = rightHash;
			node -> size = root -> size;
			node -> succ = table[(rootHash % (uint64_t) HASH_TABLE_SIZE)];
			table[(rootHash % (uint64_t) HASH_TABLE_SIZE)] = node;
			hash_counter++;
		}
		
		return(rootHash);
	} else {
		return(-1);
	}
}


void printRules(FILE *filePtr) {

	fprintf(filePtr, "digraph G {\n");
	
	for (size_t i = 0; i < HASH_TABLE_SIZE; i++) {
		chainNode *chainPtr = table[i];
		
		while (chainPtr != NULL) {
			fprintf(filePtr, "%lx [label=\"%lx\\l%i\"]\n",
				chainPtr -> rootHash, chainPtr -> rootHash, chainPtr -> size);
			if (chainPtr -> leftHash != (uint64_t)-1) {
				fprintf(filePtr, "%lx -> {%li %li}\n",
					chainPtr -> rootHash, chainPtr -> leftHash, chainPtr -> rightHash);
			}
			chainPtr = chainPtr -> succ;
		}
	}
	
	fprintf(filePtr, "}\n");
	
	return;
}

void printGrammar(FILE *filePtr) {
	/* fprintf(filePtr, "digraph G {\n"); */
	fprintf(filePtr, "#root\tsize\tleft\tright\n#invalid=%lu\n", (uint64_t)-1);

	for (size_t i = 0; i < HASH_TABLE_SIZE; i++) {
		chainNode *chainPtr = table[i];

		while(chainPtr != NULL) {
			fprintf(filePtr, "%lu\t%d\t%lu\t%lu\n", chainPtr->rootHash, chainPtr->size,  chainPtr->leftHash, chainPtr->rightHash);
			chainPtr = chainPtr -> succ;
		}
	}
}

