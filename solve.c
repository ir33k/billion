/* Solve 1BRC without Hash Map.

I will attempt to use DAFSA (deterministic acyclic finite state automaton)
also called DAWG (directed acyclic word graph) [1] data structure instead
of Hash Map.

[1] https://en.wikipedia.org/wiki/Deterministic_acyclic_finite_state_automaton
*/

#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#undef assert
#define assert(x)	/* Disable asserts */

#define STATION_SZ 10000
#define TREE_SZ (4096*32)

typedef struct {
	int count;
	int min;
	int max;
	int sum;
	/* MEAN is calculated at the end using COUNT and SUM. */
} Station;

typedef struct {
	char c;
	int sibling;	/* Index to TREE, 0 is consider empty */
	int child;	/* Index to TREE, 0 is consider empty */
	int station;	/* Index to STATION, 0 is consider empty */
} Node;

static Station *station;
static Node *tree;
static int station_max = 0;
static int tree_max = 0;

#if 0
static int
tree_print_result(void)
{
	int i=0;
	while (true) {
	}
}
#endif

static int
tree_create_path(int beg, char *buf, int sz)
{
	int i,j=beg;
	i=0;
	while (1) {
		tree[j].c = buf[i];
		i++;
		if (i>=sz) {
			break;
		}
		tree[j].child = ++tree_max;
		j = tree[j].child;
		assert(j < TREE_SZ);
		i++;
	}
	return j;
}

static int
tree_get_station(char *buf, int sz)
{
	int i,j=0;
	for (i=0; i<sz; i++) {
		while (1) {
			if (tree[j].c == buf[i]) {
				break;
			}
			if (!tree[j].sibling) {
				tree[j].sibling = ++tree_max;
				j = tree[j].sibling;
				assert(j < TREE_SZ);
				j = tree_create_path(j, buf+i, sz-i);
				goto skip;
			}
			j = tree[j].sibling;
		}
		if (!tree[j].child) {
			tree[j].child = ++tree_max;
			j = tree[j].child;
			assert(j < TREE_SZ);
			j = tree_create_path(j, buf+i, sz-i);
			goto skip;
		}
		j = tree[j].child;
	}
skip:
	if (!tree[j].station) {
		tree[j].station = ++station_max;
		assert(tree[j].station < STATION_SZ);
	}
	return tree[j].station;
}

static int
parse_num(char *str)
{
	int res;
	char sign = '+';
	if (*str == '-') {
		sign = '-';
		str++;
	}
	for (res=0; *str != '\n'; str++) {
		if (*str == '.') {
			continue;
		}
		res = 10*res + (0x0F & *str);
	}
	if (sign == '-') {
		res = 0 - res;
	}
	return res;
}

static void
online(char *str)
{
	int i=0, num;
	while (str[i] != ';') i++;
	num = parse_num(str + i + 1);
	i = tree_get_station(str, i);
	if (num < station[i].min) station[i].min = num;
	if (num > station[i].max) station[i].max = num;
	station[i].sum += num;
	station[i].count++;
#if 0
	printf("%d:	%d	%d	%d	%d	%s",
	       i,
	       station[i].count,
	       station[i].min,
	       station[i].max,
	       station[i].sum,
	       str);
#endif
}

int
main(void)
{
	clock_t clocks;
	char buf[BUFSIZ];
	if (!(tree = calloc(TREE_SZ, sizeof(*tree)))) {
		err(1, "calloc tree");
	}
	if (!(station = calloc(STATION_SZ, sizeof(*station)))) {
		err(1, "calloc station");
	}
	while (fgets(buf, sizeof(buf), stdin)) {
		online(buf);
	}
	clocks = clock();
	printf("Runtime: %lds %ldms\n",
	       clocks / CLOCKS_PER_SEC,
	       clocks / (CLOCKS_PER_SEC / 1000));
	free(tree);
	free(station);
	return 0;
}
