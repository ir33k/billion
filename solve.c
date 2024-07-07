/* Solve */

#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int
main(void)
{
	clock_t clocks;
	char buf[BUFSIZ];
	int i, sz, lines=0;
	while ((sz = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
		for (i=0; i<sz; i++) {
			if (buf[i] == '\n') {
				lines++;
			}
		}
	}
	clocks = clock();
	printf("Runtime: %lds %ldms\n",
	       clocks / CLOCKS_PER_SEC,
	       clocks / (CLOCKS_PER_SEC / 1000));
	printf("Number of lines: %d\n", lines);
	return 0;
}
