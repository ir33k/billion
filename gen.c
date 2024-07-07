/* Generate input file

Because generation of 1B rows can take few minutes (8m on my old ThinkPad)
program prints indicaiton of progress to STDERR after processing 1000000
rows.  Also remember that generated file with 1B rows will take more than
10 GB of disc space.

	$ ./build
	$ ./gen 1000000000 > data.tmp	# Generate 1B rows
	0M
	1M
	2M
	...
*/

#include <assert.h>
#include <err.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* File with list of stations and average temperature in form of tab
 * separated .csv file.  Watch out for the first row as it is just
 * an table header, not data. */
#define CSV "stations.csv"

typedef struct {	/* String view */
	char *p;	/* Pointer to memory */
	int n;		/* Number of bytes */
} Str;

typedef struct {	/* CSV row */
	Str name;	/* Station name */
	double avg;	/* Average temperature */
} Row;

/* Malloc content of PATH file.  Return pointer to allocated null
 * terminated string or exit with 1 on error. */
static char *
falloc(char *path)
{
	long siz = 0;
	char *str = 0;
	FILE *fp = 0;
	if ((fp = fopen(path, "rb")) == 0) {
		err(1, "malloc_file fopen(%s)", path);
	}
	if (fseek(fp, 0, SEEK_END) == -1) {
		err(1, "malloc_file fseek end");
	}
	if ((siz = ftell(fp)) == -1) {
		err(1, "malloc_file ftell");
	}
	/* +1 for null terminator */
	if ((str = malloc(siz +1)) == 0) {
		err(1, "malloc_file malloc");
	}
	if (fseek(fp, 0, SEEK_SET) == -1) {
		err(1, "malloc_file fseek beg");
	}
	if (fread(str, siz, 1, fp) == 0) {
		err(1, "malloc_file fread");
	}
	str[siz] = 0;		/* Add null terminator */
	if (fclose(fp) == EOF) {
		err(1, "malloc_file fclose");
	}
	return str;
}

static int
count_lines(char *str)
{
	int i,n=0;
	assert(str);
	for (i=0; str[i]; i++) {
		if (str[i] == '\n') {
			n++;
		}
	}
	/* NOTE(irek): Probably an overkill.  This supposed to handle
	 * the case when file does not end with \n. */
	if (str[i-1] != '\n') {
		n++;
	}
	return n;
}

static char *
skip_lines(char *str, int n)
{
	int i;
	for (i=0; n && str[i]; i++) {
		if (str[i] == '\n') {
			n--;
		}
	}
	if (str[i]) {
		i++;
	}
	return str + i;
}

static char *
parse_row(char *str, Row *row)
{
	int i=0;
	row->name.p = str;
	while (str[i] && str[i] != '\t') {
		i++;
	}
	row->name.n = i;
	i++;
	row->avg = atof(str + i);
	while (str[i] && str[i] != '\n') {
		i++;
	}
	if (str[i]) {
		i++;
	}
	if (!str[i]) {
		return 0;
	}
	return str + i;
}

static double
rand_in_range(double min, double max)
{
	double scale = (double)rand() / RAND_MAX;	/* [0, 1.0] */
	return min + scale * (max - min);		/* [min, max] */
}

/* Returns a double value pseudorandomly chosen from a Gaussian (normal)
 * distribution with a mean and standard deviation specified by the
 * arguments.  Stollen from Java 22 [1][2][3].  How low I have fallen,
 * stealing from Java standard library, shame (._.).
 *
 * [1] https://docs.oracle.com/en/java/javase/22/docs/api/java.base/java/util/random/RandomGenerator.html#nextGaussian(double,double)
 * [2] https://github.com/openjdk/jdk/blob/6f7f0f1de05fdc0f6a88ccd90b806e8a5c5074ef/src/java.base/share/classes/java/util/random/RandomGenerator.java#L948
 * [3] https://github.com/openjdk/jdk/blob/6f7f0f1de05fdc0f6a88ccd90b806e8a5c5074ef/src/java.base/share/classes/java/util/Random.java#L745
 */
static double
next_gaussian(double mean, double stddev) {
	static int have_next = 0;
	static double next_gaussian;
	double v1, v2, s, multiplier;
	assert(stddev >= 0.0);
	if (have_next) {
		have_next = 0;
		return mean + stddev * next_gaussian;
	}
	do {
		v1 = rand_in_range(-1.0, 1.0);
		v2 = rand_in_range(-1.0, 1.0);
		s = v1 * v1 + v2 * v2;
	} while (s >= 1 || s == 0);
	multiplier = sqrt(-2 * log(s)/s);
	next_gaussian = v2 * multiplier;
	have_next = 1;
	return mean + stddev * v1 * multiplier;
 }

static double
measurement(double avg)
{
	double m = next_gaussian(avg, 1.0);
	return round(m * 10.0) / 10.0;
}

static void
run(int n)
{
	int i,j;
	int lines=0;			/* Number of lines in CSV file */
	char *file, *pt;
	Row *rows;
	file = falloc(CSV);
	lines = count_lines(file);
	lines--;			/* Substract CSV header row */
	pt = skip_lines(file, 1);	/* Skip header line */
	if (!(rows = calloc(lines, sizeof(*rows)))) {
		err(1, 0);
	}
	for (i=0; pt; i++) {
		pt = parse_row(pt, rows+i);
	}
	srand(time(0));
	for (i=0; i<n; i++) {
		if (i%1000000 == 0) {
			/* Print indication of progress each 1M entries */
			fprintf(stderr, "%dM\n", i/1000000);
		}
		j = rand() % lines;	/* Index to random CSV row */
		printf("%.*s;%.1f\n",
		       rows[j].name.n,
		       rows[j].name.p,
		       measurement(rows[j].avg));
	}
	free(rows);
	free(file);
}

int
main(int argc, char **argv)
{
	int n=100;	/* Default number of rows */
	if (argc > 1) {
		n = atoi(argv[1]);
	}
	if (n < 0) {
		errx(1, "Invalid number of rows");
	}
	run(n);
	return 0;
}
