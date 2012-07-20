// ================================================================
// John Kerl
// kerl.john.r@gmail.com
// 2010-02-02
// ================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "walk_count_lib.h"

#define EPS 1e-8

// ----------------------------------------------------------------
// Definitions private to this file.

// Function pointers (somewhat of a programming trick) are used here to
// avoid redundant checking at run-time.  This code handles various types
// of constraint.  Without function pointers, each one of millions of
// constraint checks would take the form if-else-if-....  With function
// pointers, we can choose the correct routines for each constraint type
// once, and invoke only those.
typedef int (*sa_func_ptr_t )(int** walk_points, int curr_num_pts);
typedef int (*loc_func_ptr_t)(int** walk_points, int curr_num_pts, double arg);

// This is a recursive routine to be called by count_constrained_walks.
static void count_constrained_walks_aux(
	int**          walk_points,  // dimensions: full_num_pts x 2
	int            curr_num_pts,
	int            full_num_pts,
	double         slope,
	sa_func_ptr_t  psa_func,
	loc_func_ptr_t ploc_func,
	long long*     counts,       // Dimension num_pts+1.
	int            Nmin_to_print,// If walk_fp != 0, print walks >= this length.
	FILE*          walk_fp);     // For list of walks.  Set to 0 if not wanted.

// Self-avoidance constraint-checking routines:
static int passes_sa_constraints_none (int** walk_pts, int curr_num_pts);
static int passes_sa_constraints_prev (int** walk_pts, int curr_num_pts);
static int passes_sa_constraints_full (int** walk_pts, int curr_num_pts);

// Location-constraint-checking routines:
static int passes_loc_constraints_fp  (int** walk_pts, int curr_num_pts,
	double not_used);
static int passes_loc_constraints_uhp (int** walk_pts, int curr_num_pts,
	double not_used);
static int passes_loc_constraints_urq (int** walk_pts, int curr_num_pts,
	double not_used);
static int passes_loc_constraints_above_slope(int** walk_pts, int curr_num_pts,
	double slope);
static int passes_loc_constraints_below_slope(int** walk_pts, int curr_num_pts,
	double slope);

// ----------------------------------------------------------------
// This recursively counts the number of walks of length num_pts which
// satisfy the given constraints.
void count_constrained_walks(
	int Nmin,
	int Nmax,
	int self_avoidance_constraint, // SELF_AVOIDANCE_... types #define'd above.
	int location_constraint,    // LOCATION_...       types #define'd above.
	int p,                      // Only used for LOCATION_SLOPE; else set to 0.
	int q,                      // Only used for LOCATION_SLOPE; else set to 0.
	char* constraints_desc,
	FILE* walk_fp)              // For list of walks.  Set to 0 if not wanted.
{
	int**  walk_points = 0;     // Points in the walk: (x0,y0), (x1,y1), ...
	int**  z0s;                 // Walk starting points.
	int    num_z0s = 0;
	int    i, N;
	double slope;
	long long** z0_counts = 0;
	sa_func_ptr_t  psa_func  = 0; // Self-avoidance-constraint checker.
	loc_func_ptr_t ploc_func = 0; // Location-constraint checker.

	if (Nmax < 1) {
		fprintf(stderr,
			"count_constrained_walks:  Nmax must be >= 1; got %d.\n",
			Nmax);
		exit(1);
	}

	if ((location_constraint == LOCATION_ABOVE_SLOPE)
	||  (location_constraint == LOCATION_BELOW_SLOPE))
	{
		// Example:  p=3, q=7, slope=3/7.  There are 7 starting points:
		// largest integer <= 0/7, 3/7, 6/7, 9/7, 12/7, 15/7, 18/7.

		// Put p and q positive.
		if (p < 0)
			p = -p;
		if (q < 0)
			q = -q;

		// Put p/q into lowest terms.
		// JRK 2010-04-18:  let the caller reduce their fractions if they want
		// to.  It's useful for scaling purposes to make sure results for 1/2
		// are compatible with results for 2/4.
#if 0
		int g = wc_gcd(p, q);
		p = p / g;
		q = q / g;
#endif

		if (p > q) {
			int t;
			// Swap p and q to make p <= q, using the g variable as a temporary
			// swap variable.
			t = p;
			p = q;
			q = t;
		}

		num_z0s = q + 1; // q = max(p, q) since we put p <= q.

		z0s = allocate_array_of_pairs(num_z0s);
		slope = (double)p / (double)q;

		for (i = 0; i <= q; i++) {
			z0s[i][0] = i;
			// 2010-04-02: SCP/JRK to match TGK's point selection.
			// z0s[i][1] = (int) floor(i*p/q);
			if (location_constraint == LOCATION_ABOVE_SLOPE)
				z0s[i][1] = (int) ceil(i*slope - EPS);
			else
				z0s[i][1] = (int) floor(i*slope - EPS);
		}
	}
	else {
		// For all location constraints except slope constraints, there
		// is one starting point:  the origin.
		num_z0s = 1;
		z0s = allocate_array_of_pairs(num_z0s);
		slope = 0.0;
		z0s[0][0] = 0;
		z0s[0][1] = 0;
	}

	// Choose the desired constraint-checking routines.
	switch (self_avoidance_constraint) {
	case SELF_AVOIDANCE_NONE: psa_func = passes_sa_constraints_none; break;
	case SELF_AVOIDANCE_PREV: psa_func = passes_sa_constraints_prev; break;
	case SELF_AVOIDANCE_FULL: psa_func = passes_sa_constraints_full; break;
	default:
		fprintf(stderr,
		"%s, line %d:  unrecognized self-avoidance constraint %d.  Exiting.\n",
				__FILE__, __LINE__, self_avoidance_constraint);
		exit(1);
		break;
	}

	switch (location_constraint) {
	case LOCATION_FP:
		ploc_func = passes_loc_constraints_fp;
		break;
	case LOCATION_UHP:
		ploc_func = passes_loc_constraints_uhp;
		break;
	case LOCATION_URQ:
		ploc_func = passes_loc_constraints_urq;
		break;
	case LOCATION_ABOVE_SLOPE:
		ploc_func = passes_loc_constraints_above_slope;
		break;
	case LOCATION_BELOW_SLOPE:
		ploc_func = passes_loc_constraints_below_slope;
		break;
	default:
		fprintf(stderr,
			"%s, line %d:  unrecognized location constraint %d.  Exiting.\n",
				__FILE__, __LINE__, location_constraint);
		exit(1);
		break;
	}

	// Allocate walk points.
	walk_points = allocate_array_of_pairs(Nmax);
	z0_counts   = allocate_llmatrix(num_z0s, Nmax+1);

	// For each starting point z0, find all walks of length Nmax which
	// start at z0 and satisfy the desired constraints.
	for (i = 0; i < num_z0s; i++) {
		walk_points[0][0] = z0s[i][0];
		walk_points[0][1] = z0s[i][1];
		//printf("z0 = (%d,%d)\n", z0s[i][0], z0s[i][1]);

		// Initialize counts of constrained walks starting at this starting
		// point.
		for (N = 0; N <= Nmax; N++)
			z0_counts[i][N] = 0;

		// Call the recursive routine to count constrained walks starting at
		// this starting point.
		//
		// Note:  even though we want to report only on walks from length Nmin
		// to Nmax (e.g. 10 to 20), we necessarily enumerate, as a side effect
		// of the backtracking algorithm, all walks of length 1 to Nmax.
		count_constrained_walks_aux(walk_points, 1, Nmax, slope,
			psa_func, ploc_func, z0_counts[i], Nmin, walk_fp);
	}

	printf("#N");
	for (i = 0; i < num_z0s; i++)
		printf(" (%d,%d)", z0s[i][0], z0s[i][1]);
	printf("\n");

	printf("#-");
	for (i = 0; i < num_z0s; i++)
		printf(" --------");
	printf("\n");

	for (N = Nmin; N <= Nmax; N++) {
		printf("%2d", N);
		for (i = 0; i < num_z0s; i++)
			printf(" %8lld", z0_counts[i][N]);
		printf("\n");
	}

	// Free walk points and counts.
	free_array_of_pairs(walk_points);
	free_array_of_pairs(z0s);
	free_llmatrix(z0_counts, num_z0s);
}

// ----------------------------------------------------------------
// This is a recursive routine, to be called by count_constrained_walks.
//
// The walk_pts array should be dimensioned num_full_pts x 2.
//
// On the first call, curr_num_pts should be set to 1, and walk_pts[0][0] and
// walk_pts[0][1] should contain the x and y coordinates, respectively, of z0.
//
// For slope constraints, slope should be set to the floating-point quotient
// of p/q; otherwise, it can be safely set to zero.
//
// Constraint-checking function pointers were selected in
// count_constrained_walks.
//
// If walk_fp is non-null, walk coordinates for constraint-satisfying walks
// will be written to it.  The caller is responsible for fopen'ing the fp
// before the call, and fclose'ing it afterward.  If walk_fp is null, no walks
// will be written.  If walk_fp is stdout, walks will be written to the screen
// (and the caller should not fclose stdout after this routine returns).
//
// *pcount should be initially set to 0; on return, it will be incremented
// by the number of walks found satisfying the desired constraints.

static void count_constrained_walks_aux(
	int**          walk_pts,     // dimensions: full_num_pts x 2
	int            curr_num_pts,
	int            full_num_pts,
	double         slope,
	sa_func_ptr_t  psa_func,
	loc_func_ptr_t ploc_func,
	long long*     counts,       // Dimension num_pts+1.
	int            Nmin_to_print,// If walk_fp != 0, print walks >= this length.
	FILE*          walk_fp)      // For list of walks.  Set to 0 if not wanted.
{
	static int turns[4][2] = {{1, 0}, {0, 1}, {-1 , 0}, {0, -1}};
	int num_turns = 4;
	int i;
	int passes_sa_constraint, passes_loc_constraint;

	counts[curr_num_pts]++;
	if (curr_num_pts >= Nmin_to_print)
		print_walk(walk_pts, curr_num_pts, walk_fp);

	if (curr_num_pts == full_num_pts) {
		// No more extensions are possible.
		return;
	}

	// Incomplete walk of length curr_num_pts.  Recurse to find all possible
	// completions.
	for (i = 0; i < num_turns; i++) {

		// For each of four choices -- right, up, left, down -- extend the
		// walk by one step in that direction and see if the extended walk
		// satisfies the desired constraints.  If so, recursively attempt
		// to complete the walk.
		//
		// Note that the constraint-checker routines are only ever called with
		// 2 <= curr_num_pts <= full_num_pts.  So, they may safely omit checks
		// which apply to the first point.  (For example, upper-half-plane
		// walks must have y_0 = 0 but y_k > 0 for all k >= 1.)

		walk_pts[curr_num_pts][0] = walk_pts[curr_num_pts-1][0] + turns[i][0];
		walk_pts[curr_num_pts][1] = walk_pts[curr_num_pts-1][1] + turns[i][1];

		passes_sa_constraint  = psa_func (walk_pts, curr_num_pts+1);
		passes_loc_constraint = ploc_func(walk_pts, curr_num_pts+1, slope);

		if (passes_sa_constraint && passes_loc_constraint) {
			count_constrained_walks_aux(walk_pts, curr_num_pts+1, full_num_pts,
				slope, psa_func, ploc_func, counts, Nmin_to_print, walk_fp);
		}
	}
}

// ----------------------------------------------------------------
// Tests whether the last added step passes the self-avoidance constraint.  See
// also the comments to count_constrained_walks_aux.  Note that this is always
// called with curr_num_pts >= 2.

static int passes_sa_constraints_full(int** walk_points, int curr_num_pts)
{
	int* p1 = walk_points[curr_num_pts-1];
	int i;

	for (i = curr_num_pts-2; i >= 0; i--) {
		int* p2 = walk_points[i];

		if ((p1[0] == p2[0]) && (p1[1] == p2[1]))
			return 0;
	}
	return 1;
}

// ----------------------------------------------------------------
// Tests whether the last added step passes the no-backstep constraint.  See
// also the comments to count_constrained_walks_aux.  Note that this is always
// called with curr_num_pts >= 2.

static int passes_sa_constraints_prev(int** walk_points, int curr_num_pts)
{
	if (curr_num_pts < 3)
		return 1;

	if ((walk_points[curr_num_pts-3][0] == walk_points[curr_num_pts-1][0])
	&&  (walk_points[curr_num_pts-3][1] == walk_points[curr_num_pts-1][1]))
	{
		return 0;
	}

	return 1;
}

// ----------------------------------------------------------------
// Tests whether the last added step passes the no-self-avoidance constraint.
// This is no constraint at all.  See also the comments to
// count_constrained_walks_aux.  Note that this is always called with
// curr_num_pts >= 2.

static int passes_sa_constraints_none(int** walk_points, int curr_num_pts)
{
	return 1;
}

// ----------------------------------------------------------------
// Tests whether the last added step passes the full-plane location constraint.
// This is no constraint at all.  See also the comments to
// count_constrained_walks_aux.  Note that this is always called with
// curr_num_pts >= 2.

static int passes_loc_constraints_fp(int** walk_points, int curr_num_pts,
	double not_used)
{
	return 1;
}

// ----------------------------------------------------------------
// Tests whether the last added step passes the upper half-plane location
// constraint.  See also the comments to count_constrained_walks_aux.  Note
// that this is always called with curr_num_pts >= 2.

static int passes_loc_constraints_uhp(int** walk_points, int curr_num_pts,
	double not_used)
{
	if (walk_points[curr_num_pts-1][1] <= 0)
		return 0;

	return 1;
}

// ----------------------------------------------------------------
// Tests whether the last added step passes the upper-right-quadrant location
// constraint.  See also the comments to count_constrained_walks_aux.  Note
// that this is always called with curr_num_pts >= 2.

static int passes_loc_constraints_urq(int** walk_points, int curr_num_pts,
	double not_used)
{
	if (walk_points[curr_num_pts-1][0] < 0)
		return 0;
	if (walk_points[curr_num_pts-1][1] < 0)
		return 0;

	return 1;
}

// ----------------------------------------------------------------
// Tests whether the last added step passes the constraint of staying above the
// line y = m x, where m is the slope.  See also the comments to
// count_constrained_walks_aux.  Note that this is always called with
// curr_num_pts >= 2.

static int passes_loc_constraints_above_slope(int** walk_points,
	int curr_num_pts, double slope)
{
	int x = walk_points[curr_num_pts-1][0];
	int y = walk_points[curr_num_pts-1][1];

#if 0
	// This helps to choose the right epsilon.  I find that differences from
	// the y=mx line are of order 1, or a few thousands, or double-precision
	// machine epsilon (10^-17 or so).  So, epsilon = 10^-8 will work fine.
	double diff = y - slope*x;
	if (fabs(diff) < 1e-4)
		printf(">>> %14.8le\n", diff);
#endif

	if (y >= slope * x - EPS)
		return 1;
	else
		return 0;
}

static int passes_loc_constraints_below_slope(int** walk_points,
	int curr_num_pts, double slope)
{
	int x = walk_points[curr_num_pts-1][0];
	int y = walk_points[curr_num_pts-1][1];

#if 0
	// This helps to choose the right epsilon.  I find that differences from
	// the y=mx line are of order 1, or a few thousands, or double-precision
	// machine epsilon (10^-17 or so).  So, epsilon = 10^-8 will work fine.
	double diff = y - slope*x;
	if (fabs(diff) < 1e-4)
		printf(">>> %14.8le\n", diff);
#endif

	if (y <= slope * x - EPS)
		return 1;
	else
		return 0;
}

// ================================================================
int constraints_name_to_types(char* name, int* psa_constraint,
	int* ploc_constraint, int* pp, int* pq)
{
	int self_avoidance      = SELF_AVOIDANCE_UNDEFINED;
	int location_constraint = LOCATION_UNDEFINED;
	int p = 0;
	int q = 0;

	if      (strcmp(name, "SRW")      == 0) {
		self_avoidance      = SELF_AVOIDANCE_NONE;
		location_constraint = LOCATION_FP;
	}
	else if (strcmp(name, "SAW")      == 0) {
		self_avoidance      = SELF_AVOIDANCE_FULL;
		location_constraint = LOCATION_FP;
	}
	else if (strcmp(name, "HPE")      == 0) {
		self_avoidance      = SELF_AVOIDANCE_NONE;
		location_constraint = LOCATION_UHP;
	}
	else if (strcmp(name, "UPSAW")    == 0) {
		self_avoidance      = SELF_AVOIDANCE_FULL;
		location_constraint = LOCATION_UHP;
	}

	else if (strcmp(name, "none-fp")  == 0) {
		self_avoidance      = SELF_AVOIDANCE_NONE;
		location_constraint = LOCATION_FP;
	}
	else if (strcmp(name, "prev-fp")  == 0) {
		self_avoidance      = SELF_AVOIDANCE_PREV;
		location_constraint = LOCATION_FP;
	}
	else if (strcmp(name, "tri")      == 0) {
		self_avoidance      = SELF_AVOIDANCE_PREV;
		location_constraint = LOCATION_FP;
	}
	else if (strcmp(name, "full-fp")  == 0) {
		self_avoidance      = SELF_AVOIDANCE_FULL;
		location_constraint = LOCATION_FP;
	}

	else if (strcmp(name, "none-uhp") == 0) {
		self_avoidance      = SELF_AVOIDANCE_NONE;
		location_constraint = LOCATION_UHP;
	}
	else if (strcmp(name, "prev-uhp") == 0) {
		self_avoidance      = SELF_AVOIDANCE_PREV;
		location_constraint = LOCATION_UHP;
	}
	else if (strcmp(name, "full-uhp") == 0) {
		self_avoidance      = SELF_AVOIDANCE_FULL;
		location_constraint = LOCATION_UHP;
	}

	else if (strcmp(name, "none-urq") == 0) {
		self_avoidance      = SELF_AVOIDANCE_NONE;
		location_constraint = LOCATION_URQ;
	}
	else if (strcmp(name, "prev-urq") == 0) {
		self_avoidance      = SELF_AVOIDANCE_PREV;
		location_constraint = LOCATION_URQ;
	}
	else if (strcmp(name, "full-urq") == 0) {
		self_avoidance      = SELF_AVOIDANCE_FULL;
		location_constraint = LOCATION_URQ;
	}

	else if (sscanf(name, "none-above-slope=%d/%d", &p, &q) == 2) {
		self_avoidance      = SELF_AVOIDANCE_NONE;
		location_constraint = LOCATION_ABOVE_SLOPE;
	}
	else if (sscanf(name, "prev-above-slope=%d/%d", &p, &q) == 2) {
		self_avoidance      = SELF_AVOIDANCE_PREV;
		location_constraint = LOCATION_ABOVE_SLOPE;
	}
	else if (sscanf(name, "full-above-slope=%d/%d", &p, &q) == 2) {
		self_avoidance      = SELF_AVOIDANCE_FULL;
		location_constraint = LOCATION_ABOVE_SLOPE;
	}
	else if (sscanf(name, "above-slope=%d/%d", &p, &q) == 2) {
		self_avoidance      = SELF_AVOIDANCE_FULL;
		location_constraint = LOCATION_ABOVE_SLOPE;
	}

	else if (sscanf(name, "none-above-slope=%d", &p) == 1) {
		self_avoidance      = SELF_AVOIDANCE_NONE;
		location_constraint = LOCATION_ABOVE_SLOPE;
		q = 1;
	}
	else if (sscanf(name, "prev-above-slope=%d", &p) == 1) {
		self_avoidance      = SELF_AVOIDANCE_PREV;
		location_constraint = LOCATION_ABOVE_SLOPE;
		q = 1;
	}
	else if (sscanf(name, "full-above-slope=%d", &p) == 1) {
		self_avoidance      = SELF_AVOIDANCE_FULL;
		location_constraint = LOCATION_ABOVE_SLOPE;
		q = 1;
	}
	else if (sscanf(name, "above-slope=%d", &p) == 1) {
		self_avoidance      = SELF_AVOIDANCE_FULL;
		location_constraint = LOCATION_ABOVE_SLOPE;
		q = 1;
	}

	else if (sscanf(name, "none-below-slope=%d/%d", &p, &q) == 2) {
		self_avoidance      = SELF_AVOIDANCE_NONE;
		location_constraint = LOCATION_BELOW_SLOPE;
	}
	else if (sscanf(name, "prev-below-slope=%d/%d", &p, &q) == 2) {
		self_avoidance      = SELF_AVOIDANCE_PREV;
		location_constraint = LOCATION_BELOW_SLOPE;
	}
	else if (sscanf(name, "full-below-slope=%d/%d", &p, &q) == 2) {
		self_avoidance      = SELF_AVOIDANCE_FULL;
		location_constraint = LOCATION_BELOW_SLOPE;
	}
	else if (sscanf(name, "below-slope=%d/%d", &p, &q) == 2) {
		self_avoidance      = SELF_AVOIDANCE_FULL;
		location_constraint = LOCATION_BELOW_SLOPE;
	}

	else if (sscanf(name, "none-below-slope=%d", &p) == 1) {
		self_avoidance      = SELF_AVOIDANCE_NONE;
		location_constraint = LOCATION_BELOW_SLOPE;
		q = 1;
	}
	else if (sscanf(name, "prev-below-slope=%d", &p) == 1) {
		self_avoidance      = SELF_AVOIDANCE_PREV;
		location_constraint = LOCATION_BELOW_SLOPE;
		q = 1;
	}
	else if (sscanf(name, "full-below-slope=%d", &p) == 1) {
		self_avoidance      = SELF_AVOIDANCE_FULL;
		location_constraint = LOCATION_BELOW_SLOPE;
		q = 1;
	}
	else if (sscanf(name, "below-slope=%d", &p) == 1) {
		self_avoidance      = SELF_AVOIDANCE_FULL;
		location_constraint = LOCATION_BELOW_SLOPE;
		q = 1;
	}

	else {
		fprintf(stderr, "Unrecognized constraints name \"%s\".\n", name);
		list_constraints_names(stderr);
		return 0;
	}

	*psa_constraint  = self_avoidance;
	*ploc_constraint = location_constraint;
	*pp = p;
	*pq = q;

	return 1;
}

// ----------------------------------------------------------------
void list_constraints_names(FILE* ofp)
{
	fprintf(ofp, "Recognized names:\n");
	fprintf(ofp, "  SRW, SAW, HPE, UPSAW, tri, none-fp, prev-fp, "
		"full-fp, none-uhp, prev-uhp,\n");
	fprintf(ofp, "  full-uhp, none-urq, prev-urq, full-urq,\n");
	fprintf(ofp, "  none-above-slope={p}/{q}, none-below-slope={p}/{q},\n");
	fprintf(ofp, "  prev-above-slope={p}/{q}, prev-above-slope={p}/{q},\n");
	fprintf(ofp, "  full-above-slope={p}/{q}, full-below-slope={p}/{q},\n");
	fprintf(ofp, "  above-slope={p}/{q},      below-slope={p}/{q}.\n");
	fprintf(ofp, "Example:\n");
	fprintf(ofp, "  count_walks above-slope=3/5 6-14\n");
}

// ================================================================
// Print a walk using one of two formats:
// 1. (x_0, y_0) (x_1, y_1) ... (x_{N-1}, y_{N-1})
// 2. (x_0, y_0) LRUUUDDRRL ... where the LRUD are one-digit codes for
//    a left, right, up, or down step.  This latter file format is
//    more compact.
void print_walk(int** walk_points, int num_pts, FILE* walk_fp)
{
	int i;

	if (num_pts < 1) {
		fprintf(stderr,
			"print_walk:  num_pts must be >= 1; got %d.\n",
			num_pts);
		exit(1);
	}

	if (walk_fp == NULL)
		return;

	// To do:  right now this is xyxy style only.
	// LRUD style needs to be coded up.
	fprintf(walk_fp, " %2d", num_pts);
	for (i = 0; i < num_pts; i++)
		fprintf(walk_fp, " (%3d,%3d)", walk_points[i][0], walk_points[i][1]);
	fprintf(walk_fp, "\n");
}

// ================================================================
// Lower-level routines

// ----------------------------------------------------------------
int** allocate_array_of_pairs(int num_pts)
{
	int *xys;
	int** array_of_pairs;
	int i;

	if (num_pts < 1) {
		fprintf(stderr,
			"allocate_array_of_pairs:  num_pts must be >= 1; got %d.\n",
			num_pts);
		exit(1);
	}

	xys = (int *)malloc_or_die(2 * num_pts * sizeof(int));
	array_of_pairs = (int **)malloc_or_die(num_pts * sizeof(int*));

	for (i = 0; i < num_pts; i++)
		array_of_pairs[i] = &xys[2*i];
	return array_of_pairs;
}

// ----------------------------------------------------------------
void free_array_of_pairs(int** array_of_pairs)
{
	free(array_of_pairs[0]);
	free(array_of_pairs);
}

// ----------------------------------------------------------------
long long** allocate_llmatrix(int L, int M)
{
	long long** llmatrix;
	int i;

	if ((L < 1) || (M < 1)) {
		fprintf(stderr,
			"allocate_array_of_pairs:  L and M must be >= 1; got %d and %d.\n",
			L, M);
		exit(1);
	}

	llmatrix = (long long **)malloc_or_die(L * sizeof(long long*));
	for (i = 0; i < L; i++)
		llmatrix[i] = (long long *)malloc_or_die(M * sizeof(long long));

	return llmatrix;
}

// ----------------------------------------------------------------
void  free_llmatrix(long long** llmatrix, int L)
{
	int i;
	for (i = 0; i < L; i++)
		free(llmatrix[i]);
	free(llmatrix);
}

// ----------------------------------------------------------------
// This is nothing more than a wrapper around malloc, which prints an error
// message and aborts the process if the malloc fails.
void* malloc_or_die(size_t num_bytes)
{
	void* rv;

	if (num_bytes < 0) {
		fprintf(stderr,
			"malloc_or_die:  num_bytes must be >= 0; got %d.\n",
			(int)num_bytes);
		exit(1);
	}

	rv = malloc(num_bytes);
	if (rv == 0) {
		fprintf(stderr, "malloc(%d) failed.\n", (int)num_bytes);
		exit(1);
	}

	return rv;
}

// ----------------------------------------------------------------
// Euclid's algorithm for greatest common divisor.
int wc_gcd(int a, int b)
{
	int r;

	if (a == 0)
		return b;
	if (b == 0)
		return a;

	while (1) {
		r = a % b;
		if (r == 0)
			break;
		a = b;
		b = r;
	}
	return b;
}
