// ================================================================
// John Kerl
// kerl.john.r@gmail.com
// 2010-02-02
// ================================================================
// These are routines for exhaustively enumerating walks on the plane integer
// lattice, with two types of constraints:
//
// * self-avoidance, or lack thereof, and
//
// * location conditioning e.g. full-plane, upper half-plane, above a line
//   y=mx, etc.
//
// They were written in spring 2010 for use by Shane Passon in examining the
// y=mx ("slope") case; the other cases were written so that I could
// sanity-check my code.  This work is part of Tom Kennedy's bridge group,
// which mixes numerical methods with analytical predictions for random walks
// in the plane vs. Schramm-Loewner Evolution (SLE).
// ================================================================

#ifndef WALK_COUNT_LIB_H
#define WALK_COUNT_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

// ----------------------------------------------------------------
// Self-avoidance constraints:
//
// * SELF_AVOIDANCE_NONE is for simple random walks (maybe with location
//   conditioning).
//
// * SELF_AVOIDANCE_PREV is for walks which never undo their previous step but
//   which may otherwise self-intersect.  I only implemented this case as any
//   easy test case for my code.  The number of such walks, with no location
//   conditioning, is easily seen to be 4*3^(N-1).
//
// * SELF_AVOIDANCE_FULL is for self-avoiding walks (maybe with location
//   conditioning).

#define SELF_AVOIDANCE_NONE       111 // No self-avoidance
#define SELF_AVOIDANCE_PREV       222 // Backstep avoidance
#define SELF_AVOIDANCE_FULL       333 // Full self-avoidance
#define SELF_AVOIDANCE_UNDEFINED -999

// ----------------------------------------------------------------
// Location constraints:
//
// * LOCATION_FP allows for full-plane walks.
//
// * LOCATION_UHP allows for upper-half-plane walks.
//
// * LOCATION_URQ allows for upper-right-quadrant walks (for which I found
//   some  good test cases in Sloane's on-line encyclopedia of integer
//   sequences, OEIS).
//
// * LOCATION_ABOVE_SLOPE and LOCATION_BELOW_SLOPE use Shane Passon's
//   rational-slope algorithm.  Slopes are rational in the form m=p/q.  All
//   walk points after the start point must remain above the line y = m*x-eta.
//   (eta is 10^-8).  If p > q, p/q is replaced with q/p -- so then, p <= q and
//   0 <= p/q <= 1.  One tries q start points (x_0, y_0) = (k, floor(kp/q)) for
//   k = 0 to q-1.  E.g.  for p/q = 3/7, there are seven start points:
//     floor( 0/7) -> (0, 0),
//     floor( 3/7) -> (1, 0),
//     floor( 6/7) -> (2, 0),
//     floor( 9/7) -> (3, 1),
//     floor(12/7) -> (4, 1),
//     floor(15/7) -> (5, 2),
//     floor(18/7) -> (6, 2).
//   We find the number of walks starting at each of these q points which
//   remain above the line y=mx-eta.
//
//   Note that an slope of 0 does not give the same constraint as
//   upper-half-plane:  UHP has z0 = (0,0) and subsequent steps all above the
//   real line.  Since we use the line y=mx-eta, subsequent steps may hit the
//   real line, with the above-slope location constraint.  With the below-slope
//   location constraint, one does get UHP, but upside-down.

#define LOCATION_FP          44 // Full-plane
#define LOCATION_UHP         55 // Upper half-plane
#define LOCATION_URQ         66 // Upper right quadrant
#define LOCATION_ABOVE_SLOPE 77 // Rational slopes
#define LOCATION_BELOW_SLOPE 88 // Rational slopes
#define LOCATION_UNDEFINED  -99

// ----------------------------------------------------------------
// Prototypes for functions in this file.

// This recursively counts the number of walks of length N, for N from Nmin to
// Nmax, which satisfy the given constraints.
void count_constrained_walks(
	int Nmin,
	int Nmax,
	int self_avoidance_constraint, // SELF_AVOIDANCE_... types #define'd above.
	int location_constraint,    // LOCATION_...       types #define'd above.
	int p,                      // Only used for LOCATION_SLOPE; else set to 0.
	int q,                      // Only used for LOCATION_SLOPE; else set to 0.
	char* constraints_desc,
	FILE* walk_fp);             // For list of walks.  Set to 0 if not wanted.

// Maps "UPSAW" to SELF_AVOIDANCE_FULL and LOCATION_UHP, etc.  Returns 1 if the
// name was recognized, else prints an error message and returns 0.  See the
// function body in walk_count.c to see (or to add to) the full list of names.
int constraints_name_to_types(char* name, int* psa_constraint,
	int* ploc_constraint, int* pp, int* pq);
void list_constraints_names(FILE*  ofp);

// Pass walk_fp = NULL for no printout of individual walks.
// Pass walk_fp = stdout to print the walk to the screen.
// Pass walk_fp = the result of an fopen in order to print the walk to a file.
// (In the latter case,  make sure to fclose the file pointer after all your
// calls to print_walk.)
void  print_walk(int** walk_points, int num_pts, FILE* walk_fp);

// ----------------------------------------------------------------
// Prototypes for lower-level routines:

// Allocates a num_pts x 2 array of integers.
int** allocate_array_of_pairs(int num_pts);

// Frees the data structure returned by the previous routine.
void  free_array_of_pairs(int** array_of_pairs);

// Allocates an L x M array of long longs.
long long** allocate_llmatrix(int L, int M);

// Frees the data structure returned by the previous routine.
void  free_llmatrix(long long** llmatrix, int L);

// A wrapper around malloc which prints a message and aborts the process if the
// malloc fails.  Otherwise, does just what malloc does.
void* malloc_or_die(size_t num_bytes);

// This is called wc_gcd instead of gcd in order to avoid multiply-defined
// symbols in case you ever link this stuff with something else which has a gcd
// routine.
int   wc_gcd(int a, int b);

#endif // WALK_COUNT_LIB_H
