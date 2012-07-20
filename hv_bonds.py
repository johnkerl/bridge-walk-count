#!/usr/bin/python -Wall

# ================================================================
# See the paper ... this is hard to describe without a picture! :)
# ----------------------------------------------------------------
# John Kerl
# kerl.john.r@gmail.com
# 2010-04-15
# ================================================================

from __future__ import division # 7/2 = 3.5, not 3
from math import * # ceil and floor
import sys, re, copy
import tabutil_m, stats_m

eps = 1e-8

# ----------------------------------------------------------------
def get_H_bonds(p, q):
	H_bonds = []
	for y in range(0, p): # 0, 1, ..., p-1
		x1 = int(floor(q/p * (y + eps)))
		x2 = int(ceil (q/p * (y + eps)))
		left  = [x1, y] # above
		right = [x2, y] # below
		H_bonds.append([left, right])
	return H_bonds

def get_V_bonds(p, q):
	V_bonds = []
	for x in range(1, q+1): # 1, 2, ..., q
		y1 = int(floor(p*x/q - eps))
		y2 = int(ceil (p*x/q - eps))
		up    = [x, y2] # above
		down  = [x, y1] # below
		V_bonds.append([up, down])
	return V_bonds

def get_H_and_V_bonds(p, q):
	return get_H_bonds(p, q) + get_V_bonds(p, q)

def get_points_above(p, q):
	points_above = []
	for x in range(0, q+1): # i.e. 0 to q inclusive
		y = int(ceil (p*x/q - eps))
		points_above.append([x, y])
	return points_above

def get_points_below(p, q):
	points_below = []
	for x in range(0, q+1): # i.e. 0 to q inclusive
		y = int(floor (p*x/q - eps))
		points_below.append([x, y])
	return points_below

# ----------------------------------------------------------------
def look_up_in_hash(hash, above_or_below, p, q, x0, y0, N):
	try:
		c = hash[((p, q), (x0, y0), N)]
		return c
	except:
		print >> sys.stderr, \
			'z0=(%d,%d) not found %s p=%d, q=%d, N=%d.' % \
			(x0, y0, above_or_below, p, q, N)
		sys.exit(1)

# ----------------------------------------------------------------
def get_wbar_of_Npq(p, q, N, bonds, above_counts_hash, below_counts_hash, \
include_above=True, include_below=True):

	sum = 0.0
	num_bonds = len(bonds)
	for [start, end] in bonds:
		[start_x0, start_y0] = start # above
		[end_x0,   end_y0]   = end   # below

		c1 = 1
		c2 = 2

		if include_above:
			c1 = look_up_in_hash(above_counts_hash, 'above', p, q, \
				start_x0, start_y0, N)
		if include_below:
			c2 = look_up_in_hash(below_counts_hash, 'below', p, q, \
				end_x0,   end_y0, N)

		sum += c1 * c2

	#return sum / sqrt(p**2+q**2)
	return sum / num_bonds

# ----------------------------------------------------------------
def get_above_scaled_count_of_Npq(p, q, N, points_above, above_counts_hash):

	sum = 0.0
	num_points = len(points_above)
	for [x, y] in points_above:
		c1 = look_up_in_hash(above_counts_hash, 'above', p, q, x, y, N)
		sum += c1

	#return sum
	#return sum / sqrt(p**2+q**2)
	return sum / num_points

# ----------------------------------------------------------------
def get_below_scaled_count_of_Npq(p, q, N, points_below, below_counts_hash):

	sum = 0.0
	num_points = len(points_below)
	for [x, y] in points_below:
		c2 = look_up_in_hash(below_counts_hash, 'below', p, q, x, y, N)
		sum += c2

	#return sum
	#return sum / sqrt(p**2+q**2)
	return sum / num_points

# ----------------------------------------------------------------
# Example input:  data/raw_counts_1_3.txt
#
# #N    (0,0)    (1,1)    (2,1)
# #- -------- -------- --------
# 10     4268     6300     5570
# 11    11379    16742    14334
# 12    29472    43472    38316
# 13    78434   115421    99117
# 14   203739   300433   264235
# 15   541422   797137   685851
# 16  1409539  2078243  1824875
# 17  3741997  5510907  4748493
# 18  9758256 14387069 12617423
# 19 25885698 38130894 32891904
# 20 67592411 99655040 87310954

# Hash all these data on the key
#
#   ((p, q), (x0, y0), N)

def load_counts_hash(pqs, above_or_below, datadir):

	counts_hash = {}

	for [p, q] in pqs:
		counts_file = '%s/raw_counts_%s_%d_%d.txt' \
			% (datadir, above_or_below, p, q)

		# Read the data file
		Ncc_columns = tabutil_m.float_columns_from_file(counts_file)
		Ns          = Ncc_columns[0]
		Ncc_rows    = tabutil_m.float_rows_from_file   (counts_file)
		z0_labels   = tabutil_m.labels_from_file       (counts_file)

		# Omit the 'N' label.
		z0_labels   = z0_labels[1:]

		num_rows    = len(Ncc_rows)
		num_columns = len(z0_labels)

		# Parse the column labels as (x0,y0) pairs
		x0y0_pairs = []
		for z0_label in z0_labels:
			[x0, y0] = re.split(',', re.sub('[()]', '', z0_label))
			[x0, y0] = [int(x0), int(y0)]
			x0y0_pairs.append([x0, y0])

		for i in range(0, num_rows):
			N = int(Ns[i])

			for j in range(0, num_columns):
				[x0, y0] = x0y0_pairs[j]

				key  = ((p, q), (x0, y0),  N)
				data = Ncc_rows[i][j+1]
				counts_hash[key] = data

	return counts_hash

def get_Ns(datadir, p, q):
	counts_file = '%s/raw_counts_%s_%d_%d.txt' % (datadir, 'above', p, q)
	Ncc_columns = tabutil_m.float_columns_from_file(counts_file)
	return Ncc_columns[0]

# ----------------------------------------------------------------
# Need counts data loaded into a hash by:
#  ((p, q), (x0, y0), N)

# Example input:  data/raw_counts_above_1_3.txt
#
# #N    (0,0)    (1,1)    (2,1)
# #- -------- -------- --------
# 10     4268     6300     5570
# 11    11379    16742    14334
# 12    29472    43472    38316
# 13    78434   115421    99117
# 14   203739   300433   264235
# 15   541422   797137   685851
# 16  1409539  2078243  1824875
# 17  3741997  5510907  4748493
# 18  9758256 14387069 12617423
# 19 25885698 38130894 32891904
# 20 67592411 99655040 87310954

# ----------------------------------------------------------------
pqs = [
	[0,  1],
	[1, 10],
	[1,  9],
	[1,  8],
	[1,  7],
	[1,  6],
	[1,  5],
	[2,  9],
	[1,  4],
	[2,  7],
	[3, 10],
	[1,  3],
	[3,  8],
	[2,  5],
	[3,  7],
	[4,  9],
	[1,  2],
	[3,  6],
	[5,  9],
	[4,  7],
	[3,  5],
	[5,  8],
	[2,  3],
	[7, 10],
	[5,  7],
	[3,  4],
	[7,  9],
	[4,  5],
	[5,  6],
	[6,  7],
	[7,  8],
	[8,  9],
	[9, 10],
	[1,  1],
	]

print_counts_hashes = 0
print_raw_bonds     = 1

datadir = './data'

if len(sys.argv) == 2:
	datadir = sys.argv[1]

above_counts_hash = load_counts_hash(pqs, 'above', datadir)
below_counts_hash = load_counts_hash(pqs, 'below', datadir)

if print_counts_hashes:
	print 'above counts hash:'
	for key in above_counts_hash.keys():
		print key, above_counts_hash[key]
	print
	print 'below counts hash:'
	for key in below_counts_hash.keys():
		print key, below_counts_hash[key]
	print

thetas = []
wbar_output_rows = []
aw_output_rows = []
bw_output_rows = []
ac_output_rows = []
bc_output_rows = []
for [p, q] in pqs:
	theta = atan(p/q) * 180/pi
	thetas.append(theta)

	wbar_output_row = []
	aw_output_row = []
	bw_output_row = []
	ac_output_row = []
	bc_output_row = []

	bonds = get_H_and_V_bonds(p, q)
	points_above = get_points_above(p, q)
	points_below = get_points_below(p, q)
	if print_raw_bonds:
		print 'Bonds for (p,q) = (%d,%d):' % (p, q)
		for bond in bonds:
			print bond
		print
		print 'Points above:'
		for [x, y] in points_above:
			print '  (%d,%d)' % (x, y)
		print
		print 'Points below:'
		for [x, y] in points_below:
			print '  (%d,%d)' % (x, y)
		print

	Ns = get_Ns(datadir, p, q)
	wNs = map(lambda N: 2*N+1, Ns)
	#wNs = map(lambda N: 2*N, Ns)

	for N in Ns:
		wbar = get_wbar_of_Npq(p, q, N, bonds, \
			above_counts_hash, below_counts_hash, \
			include_above=True, include_below=True)
		wbar_output_row.append(wbar)

	for N in Ns:
		above_wbar = get_wbar_of_Npq(p, q, N, bonds, \
			above_counts_hash, below_counts_hash, \
			include_above=True, include_below=False)
		below_wbar = get_wbar_of_Npq(p, q, N, bonds, \
			above_counts_hash, below_counts_hash, \
			include_above=False, include_below=True)
		above_scaled_count = get_above_scaled_count_of_Npq(p, q, N, \
			points_above, above_counts_hash)
		below_scaled_count = get_below_scaled_count_of_Npq(p, q, N, \
			points_below, below_counts_hash)

		aw_output_row.append(above_wbar)
		bw_output_row.append(below_wbar)
		ac_output_row.append(above_scaled_count)
		bc_output_row.append(below_scaled_count)

	wbar_output_rows.append(wbar_output_row)
	aw_output_rows.append(aw_output_row)
	bw_output_rows.append(bw_output_row)
	ac_output_rows.append(ac_output_row)
	bc_output_rows.append(bc_output_row)

print 'Using data directory "%s"' % (datadir)
iospecs = [
	[wNs, wbar_output_rows,'w_theta_with_N_series.txt',  '%d',   False],
	[Ns,  aw_output_rows,  'aw_theta_with_N_series.txt', '%d',   False],
	[Ns,  bw_output_rows,  'bw_theta_with_N_series.txt', '%d',   False],
	[Ns,  ac_output_rows,  'ac_theta_with_N_series.txt', '%d',   False],
	[Ns,  bc_output_rows,  'bc_theta_with_N_series.txt', '%d',   False],
	[wNs, wbar_output_rows,'w_N_with_theta_series.txt',  '%.4f', True],
	[Ns,  aw_output_rows,  'aw_N_with_theta_series.txt', '%.4f', True],
	[Ns,  bw_output_rows,  'bw_N_with_theta_series.txt', '%.4f', True],
	[Ns,  ac_output_rows,  'ac_N_with_theta_series.txt', '%.4f', True],
	[Ns,  bc_output_rows,  'bc_N_with_theta_series.txt', '%.4f', True]]

for [xNs, output_rows, output_file_name, label_format, transpose] \
in iospecs:

	tabutil_m.matrix_and_labels_to_file(
		output_rows,      # matrix
		'theta',          # row_index_name
		thetas,           # row_index_values
		'N',              # col_index_name
		xNs,              # col_index_values
		output_file_name, # file_name
		transpose,
		matrix_format='%11.7f',
		label_format=label_format)
	print 'Wrote %s' % (output_file_name)

# ----------------------------------------------------------------
# Now do w_theta_over_w_zero
w_theta_over_w_zero = copy.deepcopy(wbar_output_rows)
output_file_name = 'w_theta_w_zero.txt'

nr = len(w_theta_over_w_zero)
nc = len(w_theta_over_w_zero[0])
for i in range(0, nr):
	for j in range(0, nc):
		w_theta_over_w_zero[i][j] = wbar_output_rows[i][j] / wbar_output_rows[0][j]

iospecs = [
	['w_ratio_theta_with_N_series.txt', False],
	['w_ratio_N_with_theta_series.txt', True]]

for [output_file_name, transpose] in iospecs:
	tabutil_m.matrix_and_labels_to_file(
		w_theta_over_w_zero, # matrix
		'theta',             # row_index_name
		thetas,              # row_index_values
		'N',                 # col_index_name
		wNs,                 # col_index_values
		output_file_name,    # file_name
		transpose=transpose,
		matrix_format='%11.7f',
		label_format=label_format)
	print 'Wrote %s' % (output_file_name)


