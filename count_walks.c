// ================================================================
// John Kerl
// kerl.john.r@gmail.com
// 2010-02-02
// ================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "walk_count_lib.h"

// ----------------------------------------------------------------
static void usage(char* argv0)
{
	fprintf(stderr, "Usage: %s {constraints name} {N} [options]\n", argv0);
	fprintf(stderr, "Or:    %s {constraints name} {Nmin-Nmax} [options]\n",
		argv0);
	list_constraints_names(stderr);
	fprintf(stderr, "Options: \"stdout\" or \"stderr\" to print individual walks.\n");
	exit(1);
}

// ----------------------------------------------------------------
int main(int argc, char** argv)
{
	char* conarg = 0;
	char* Narg   = 0;
	int self_avoidance      = SELF_AVOIDANCE_UNDEFINED;
	int location_constraint = LOCATION_UNDEFINED;
	int p = 0;
	int q = 0;
	int argi;

	int Nmin = 1;
	int Nmax = 15;

	FILE* walk_fp = NULL; // Don't print out individual walks.

	// Parse the command line.
	if (argc < 3)
		usage(argv[0]);

	conarg = argv[1];
	Narg   = argv[2];

	if (sscanf(argv[2], "%d-%d", &Nmin, &Nmax) == 2)
		;
	else if (sscanf(argv[2], "%d", &Nmax) == 1)
		Nmin = Nmax;
	else
		usage(argv[0]);

	for (argi = 3; argi < argc; argi++) {
		if      (strcmp(argv[argi], "stdout") == 0)
			walk_fp = stdout; // Print individual walks to stdout.
		else if (strcmp(argv[argi], "stderr") == 0)
			walk_fp = stderr; // Print individual walks to stderr.
		else
			usage(argv[0]);
	}

	if (Nmin > Nmax) {
		fprintf(stderr, "Must have Nmin <= Nmax; got %d and %d.\n", Nmin, Nmax);
		exit(1);
	}

	if (!constraints_name_to_types(conarg, &self_avoidance,
		&location_constraint, &p, &q))
	{
		// Error message was already printed out.
		exit(1);
	}

	// Count walks.  The routine we're calling will print out
	// any desired output.
	count_constrained_walks(Nmin, Nmax, self_avoidance, location_constraint,
		p, q, conarg, walk_fp);

	return 0;
}
