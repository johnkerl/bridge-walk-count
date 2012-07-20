================================================================
John Kerl
kerl.john.r@gmail.com
2010-02-03
================================================================

================================================================
DESCRIPTION

This is code for exhaustive enumeration of constrained walks on the 2D integer
lattice for small N (< 25 or so).  This is not the same as MCMC methods which
work for much larger N, but which only sample instead of enumerating
exhaustively.  This code uses a recursive backtracking algorithm to find all
walks of length <= N.  Constraints are specified on the command line (see the
next section): self-avoidance constraints include none (e.g. simple random
walk), previous walk only (immediate-backstep avoidance), or full
self-avoidance; location constraints include full-plane, upper half-plane,
upper right quadrant, or Shane Passon's rational-slope constraint.  Please see
walk_count_lib.h for more information.

================================================================
USAGE INSTRUCTIONS

Type "make".  Then:
	count_walks {constraints name} {N} [options]
or:
	count_walks {constraints name} {Nmin-Nmax} [options]
Options: "kerl" or "shane" for output styles; "stdout" or "stderr" to print
individual walks.

E.g.
	count_walks above-slope=3/7 1-12
	count_walks UPSAW           1-12
	count_walks UPSAW           1-5 stdout

================================================================
TEST CASES

Here are test cases I have run.  There are three self-avoidance constraints
and three location-conditioning constraints (besides Passon's slope
constraints).  Some combinations of those have well-known names:

* No self-avoidance in the full plane:  this is a simple random walk
  ("SRW").  There are 4^N such walks, since at each step one can choose any
  of the four directions.
* No self-avoidance in the upper half-plane:  this is the half-plane
  excursion (HPE).
* No self-avoidance in the upper right quadrant:  this shows up in Sloane's
  integer sequences.

* Avoiding only immediate backsteps in the full plane:  there are 3^N  such
  walks, since at each step one can choose from any of the three
  non-back-step directions.
* Avoiding only immediate backsteps in the upper half-plane or upper right
  quadrant:  I haven't found references to these anywhere.

* Self-avoiding walk in the full plane:  "SAW".  Counts are available on the
  web, including via Sloane.
* Self-avoiding walk in the upper half-plane:  "UPSAW".  Counts are
  available on the web, including via Sloane.
* Self-avoiding walk in the upper right quadrant:  I don't know of a short
  name for these.  Counts are available via Sloane.

Neil Sloane's sequences may be found at
http://www.research.att.com/~njas/sequences,
and/or Google for OEIS (On-line Encyclopedia of Integer Sequences).

The following table summarizes my results, using this program, for these
known-value test cases.  Counts are shown for N=0,1,2,3,... .  What I am
showing here is the correctness of my code:  for each case where there were
results to compare to, I indeed obtained the correct results.

Slf-avd Loc Desc. Verif. Count
------- --- ----  ------ -----
None    FP  SRW   Yes    4^N            1,4,16,64,256,1024,4096,16384,65536
None    UHP HPE   Yes    Sloane A001700 1,1,3,10,35,126,462,1716,6435,24310
None    URQ ???   Yes    Sloane A005566 1,2,6,18,60,200,700,2450,8820,31752

Prev.   FP  ???   Yes    4*3^(N-1)      1,4,12,36,108,324,972,2916,8748,26244
Prev.   UHP ???   N/A    ???            1,1,3,7,19,53,147,411,1163,3313,9475
Prev.   URQ ???   N/A    ???            1,2,4,10,26,66,174,462,1256,3410,9410

Full    FP  SAW   Yes    Sloane A001411 1,4,12,36,100,284,780,2172,5916
Full    UHP UPSAW Yes    Sloane A116903 1,1,3,7,19,49,131,339,899,2345,6199
Full    URQ ???   Yes    Sloane A038373 1,2,4,10,24060,146,366,912,2302,5800
