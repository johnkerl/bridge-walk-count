#!/usr/bin/python -Wall
# ================================================================
# John Kerl
# kerl.john.r@gmail.com
# 2010-02-02
# ================================================================

from __future__ import division
import sys, re
import tabutil_m, stats_m
from math import log, exp

# ----------------------------------------------------------------
def inv2x2(matrix):
	[[a, b], [c, d]] = matrix
	det = a*d - b*c
	return [[d/det, -b/det], [-c/det, a/det]]

# ----------------------------------------------------------------
# [log a]   [     n      sum log N_i  ]-1 [sum log ci - log mu sum N_i        ]
# [     ] = [                         ]   [                                   ]
# [  c  ]   [sum log N_i sum log^2 N_i]   [sum logci logNi - logmu sum NilogNi]

def find_a_and_b(Ns, cs, mu):
	n      = len(Ns)
	log_Ns = map(log,   Ns)
	log2Ns = map(lambda x: log(x)**2, Ns)
	NlogNs = map(lambda x: log(x)*x,  Ns)
	log_cs = map(log,   cs)

	sum_Ni        = sum(Ns)
	sum_log_Ni    = sum(log_Ns)
	sum_log2Ni    = sum(log2Ns)
	sum_Ni_log_Ni = sum(NlogNs)
	sum_log_ci    = sum(log_cs)
	sum_log_ci_log_Ni = 0.0
	for i in range(0, n):
		sum_log_ci_log_Ni += log_Ns[i] * log_cs[i]

	matrix = [[n, sum_log_Ni], [sum_log_Ni, sum_log2Ni]]
	matrixinv = inv2x2(matrix)

	rhsvec = [sum_log_ci - log(mu) * sum_Ni,
		sum_log_ci_log_Ni - log(mu) * sum_Ni_log_Ni]

	log_a = matrixinv[0][0] * rhsvec[0]  +  matrixinv[0][1] * rhsvec[1]
	b     = matrixinv[1][0] * rhsvec[0]  +  matrixinv[1][1] * rhsvec[1]

	return [exp(log_a), b]

# ----------------------------------------------------------------
def scale(Ns, cs, theta):
	log_Ns = map(log, Ns)
	log_cs = map(log, cs)

	# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	# First fit:
	# c(N) = a b^N
	#
	# log c = log a + N log b
	# Linear fit:
	#   x     = N
	#   y     = log c
	#   slope = log b
	#   yint  = log a
	#
	#   a = exp(yint)
	#   b = exp(slope)

	[slope, yint, csm, csb] = stats_m.linear_regression(Ns, log_cs)
	a1 = exp(yint)
	b1 = exp(slope)
	print '# theta = %.4f  a1 = %11.7f  b1 = %11.7f' % (theta, a1, b1)

	# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	# Second fit:
	#
	# c(N) = a N^b
	# log c = log a + b log N
	# Linear fit:
	#   x     = log N
	#   y     = log c
	#   slope = b
	#   yint  = log a
	#
	#   a = exp(yint)
	#   b = slope

	[slope, yint, csm, csb] = stats_m.linear_regression(log_Ns, log_cs)
	a2 = exp(yint)
	b2 = slope
	#print '# a2 = %11.7f  b2 = %11.7f' % (a2, b2)

	# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	# Third fit:
	#
	# c(N) = a mu^N N^b
	#
	# Note:  the b estimate is very sensitive to minute changes in mu.

	[a3, b3] = find_a_and_b(Ns, cs, mu)
	print '# theta = %.4f  a3 = %11.7f  b3 = %11.7f' % (theta, a3, b3)
	print '# mu = %11.7f' % (mu)

	# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	# Display the original data along with the fits
	#print '#N exact_c       appx1c        appx2c        appx3c'
	#print '#- ------------- ------------- ------------- --------------'
	print '#N exact_c                 a b^N                   a mu^N N^b'
	print '#- ----------------------- ----------------------- ------------------------'
	#for N in range(1, 25):
	for N in range(1, 45):

		# c = a b^N
		approx1_c = a1 * b1**N

		# c = a N^b
		#approx2_c = a2 * N**b2

		# c = a mu^N N^b
		approx3_c = a3 * mu**N * N**b3

		print '%2d' % (N),
		if N in Ns:
			print '%21.0f' % (cs[Ns.index(N)]),
		else:
			print '%21s' % ('_'),

		#print '%11.4e %11.4e %11.4e' % (approx1_c, approx2_c, approx3_c)
		#print '%14.0f %14.0f %14.0f' % (approx1_c, approx2_c, approx3_c)

		#print '%11.4e %11.4e' % (approx1_c, approx3_c)
		print '%21.0f %21.0f' % (approx1_c, approx3_c)

# ----------------------------------------------------------------
# Sample input data:

# #UPSAW
#  #N      c(N)
# #-- ---------
# #11      6199
# #12     16225
# #13     42811
# #14    112285
# #15    296051
#  16    777411
#  17   2049025
#  18   5384855
#  19  14190509
#  20  37313977
#  21  98324565
#  22 258654441
#  23 681552747

mu = 2.61987
input_file_name = 's'
if len(sys.argv) == 2:
	input_file_name = sys.argv[1]
if len(sys.argv) == 3:
	input_file_name = sys.argv[1]
	mu = float(sys.argv[2])

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# Acquire the data.
columns = tabutil_m.float_columns_from_file(input_file_name)
rows    = tabutil_m.float_rows_from_file   (input_file_name)
labels  = tabutil_m.labels_from_file       (input_file_name)

thetas  = columns[0]
N_strings = labels[1:]
Ns = []
for N_string in N_strings:
	N = int(re.sub('N=', '', N_string))
	Ns.append(N)

num_thetas = len(thetas)

for i in range(0, num_thetas):
	theta = thetas[i]
	counts_for_theta = rows[i][1:]

	print 'theta = %.4f' % (theta)
	scale(Ns, counts_for_theta, theta)
	print
