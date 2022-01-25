#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>


#define MIN(a, b) (a < b ? a : b)

#define HASH_TABLE_SIZE 35866000


typedef struct treeNode {
	struct treeNode *left;
	struct treeNode *right;
	int size;
} treeNode;

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


int readMatrixColumn();
void printArray(int array[], FILE *filePtr);
void printPBWTColumn(int column[], int perm[], FILE *filePtr);
void updatePermDiv(int column[], int oldPerm[], int oldDiv[]);

treeNode *createTree(int div[], int columnNum);
int nodeCompare(const void *x, const void *y);
uint64_t hashTree(treeNode *root);
void printRules(FILE *filePtr);

void printGrammar(FILE *filePtr);

int height = 358660;
int cap_height = 358660;
chainNode *table[HASH_TABLE_SIZE] = {NULL};
int collisionFlag = 0;

int* newPerm;
int* newDiv;
treeNode **global_leaves;
listNode **global_leaf_list;

void freeTree(treeNode *root) {
	if(root == NULL) { return; }
	if(root->left != NULL) {
		freeTree(root->left);
	}
	if(root->right!= NULL) {
		freeTree(root->right);
	}
	free(root);
}

int main() {
	if (scanf("%i", &height) != 1) {
		return(1);
	}
	cap_height = height;
	
	global_leaves = (treeNode**) malloc(height*sizeof(treeNode*));


	int* perm = (int*) malloc(height*sizeof(int)); // suffix array like, colex 
	int* div = (int*) malloc(height*sizeof(int)); // LCS array
	newDiv = (int*) malloc(height*sizeof(int)); // LCS array
	newPerm = (int*) malloc(height*sizeof(int)); // LCS array

	global_leaf_list = (listNode**) malloc(sizeof(listNode*)*(height + 1));
	for (int i = 1; i < height; i++) {
		global_leaf_list[i] = (listNode *) malloc(sizeof(listNode));
	}
	
	for (int i = 0; i < height; i++) {
		perm[i] = i;
		div[i] = 0;
	}
	
	int* column = (int*) malloc(height*sizeof(int));

	fprintf(stderr, "Startup\n");
	
	for (int columnNum = 0; columnNum < 30 && readMatrixColumn(column) == height; columnNum++) {
		//printArray(column, stdout);

		/* printPBWTColumn(column, perm, stdout); */

		/* printf("\n %d PERM = ", columnNum); */
		/* printArray(perm, stdout); */
		/* printf("\n %d DIV = ", columnNum); */
		/* printArray(div, stdout); */
		/* printf("\n"); */

		treeNode *root = createTree(div, columnNum);
		uint64_t hash = hashTree(root);

		freeTree(root);

		if (collisionFlag == 0) {
			fprintf(stderr, "%i: %lli\n\n", columnNum, hash);
		} else {
			fprintf(stderr, "\n\nConstruction Failed!\n\n");
			return(1);
		}
		if(columnNum % 10 == 0 ) {
			fprintf(stderr, "Round %d\n", columnNum);
		}
		
		updatePermDiv(column, perm, div);
	}
	
	/* printRules(stdout); */
	printGrammar(stdout);

	for (int x = 1; x < height; x++) {
		free(global_leaf_list[x]);
		/* free(global_leaves[x]); */
	}
	free(global_leaf_list);
	free(global_leaves);

	free(column);
	free(perm);
	free(div);
	free(newDiv);
	free(newPerm);
	return(0);
}


int readMatrixColumn(int column[]) {
	int i;
	
	for (i = 0; i < height; i++) {
		const int ret = scanf("%i", &column[i]);
		if(ret != 1) { 
			break; 
			fprintf(stderr, "wanted to read int, but found unparsable character: %c\n", getchar());
		}
		if(column[i] < 0 || column[i] > 1) {
			break;
		}
	}
	
	return(i);
}


void printArray(int array[], FILE *filePtr) {
	
	for (int i = 0; i < cap_height; i++) {
		fprintf(filePtr, "%i ", array[i]);
	}
	
	fprintf(filePtr, "\n");

	return;
}


void printPBWTColumn(int column[], int perm[], FILE *filePtr) {
	
	for (int i = 0; i < cap_height; i++) {
		fprintf(filePtr, "%i ", column[perm[i]]);
	}
	
	fprintf(filePtr, "\n");

	return;
}


void updatePermDiv(int column[], int oldPerm[], int oldDiv[]) {
	/* int newPerm[cap_height]; */
	/* int newDiv[cap_height]; */
	
	int count0 = 0;
	int lcs = 0;
	
	for (int i = 0; i < cap_height; i++) {
		lcs = MIN(lcs, oldDiv[i] + 1);
		if (column[oldPerm[i]] == 0) {
			newPerm[count0] = oldPerm[i];
			newDiv[count0] = lcs;
			count0++;
			lcs = INT_MAX;
		}
	}
	
	int count1 = 0;
	lcs = 0;

	for (int i = 0; i < cap_height; i++) {
		lcs = MIN(lcs, oldDiv[i] + 1);
		if (column[oldPerm[i]] == 1) {
			newPerm[count0 + count1] = oldPerm[i];
			newDiv[count0 + count1] = lcs;
			count1++;
			lcs = INT_MAX;
		}
	}
	
	memcpy(oldPerm, newPerm, cap_height * sizeof(int));
	memcpy(oldDiv, newDiv, cap_height * sizeof(int));
	
	return;
}


treeNode *createTree(int div[], int columnNum) {
	
	
	for (int i = 0; i < height; i++) {
		global_leaves[i] = (treeNode *) malloc(sizeof(treeNode));
		global_leaves[i] -> left = NULL;
		global_leaves[i] -> right = NULL;
		global_leaves[i] -> size = 1;
	}
	

	global_leaf_list[0] = NULL;
	global_leaf_list[height] = NULL;
	
	for (int i = 1; i < height; i++) {
		global_leaf_list[i] -> left = global_leaves[i - 1];
		global_leaf_list[i] -> div = div[i];
		global_leaf_list[i] -> right = global_leaves[i];
	}
	
	for (int i = 1; i < height; i++) {
		global_leaf_list[i] -> prec = global_leaf_list[i - 1];
		global_leaf_list[i] -> succ = global_leaf_list[i + 1];
	}
	
	qsort(&global_leaf_list[1], height - 1, sizeof(listNode *), nodeCompare);
	
	for (int i = 1;; i++) {
		treeNode *parent = (treeNode *) malloc(sizeof(treeNode));
		parent -> size = (global_leaf_list[i] -> left) -> size + (global_leaf_list[i] -> right) -> size;
		
		if (global_leaf_list[i] -> div < columnNum) {
			parent -> left = global_leaf_list[i] -> left;
			parent -> right = global_leaf_list[i] -> right;
		} else { // non-binary case
			parent -> left = NULL;
			parent -> right = NULL;
		}
		
		if (global_leaf_list[i] -> prec != NULL) {
			(global_leaf_list[i] -> prec) -> right = parent;
			(global_leaf_list[i] -> prec) -> succ = global_leaf_list[i] -> succ;
		}
		
		if (global_leaf_list[i] -> succ != NULL) {
			(global_leaf_list[i] -> succ) -> left = parent;
			(global_leaf_list[i] -> succ) -> prec = global_leaf_list[i] -> prec;
		}
		
		if (global_leaf_list[i] -> prec == NULL && global_leaf_list[i] -> succ == NULL) {
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
					collisionFlag = 1;
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
	
	for (int i = 0; i < HASH_TABLE_SIZE; i++) {
		chainNode *chainPtr = table[i];
		
		while (chainPtr != NULL) {
			fprintf(filePtr, "%llx [label=\"%llx\\l%i\"]\n",
				chainPtr -> rootHash, chainPtr -> rootHash, chainPtr -> size);
			if (chainPtr -> leftHash != -1) {
				fprintf(filePtr, "%llx -> {%lli %lli}\n",
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

	for (int i = 0; i < HASH_TABLE_SIZE; i++) {
		chainNode *chainPtr = table[i];

		while(chainPtr != NULL) {
			fprintf(filePtr, "%lu\t%d\t%lu\t%lu\n", chainPtr->rootHash, chainPtr->size,  chainPtr->leftHash, chainPtr->rightHash);
			chainPtr = chainPtr -> succ;
		}
	}
}

