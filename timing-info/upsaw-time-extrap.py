#!/usr/bin/python -Wall
# ================================================================
# John Kerl
# kerl.john.r@gmail.com
# 2010-02-02
# ================================================================

from __future__ import division
import math
import tabutil_m
import stats_m

# #UPSAW
# #11      6199  0.004
# #12     16225  0.004
# #13     42811  0.008
# #14    112285  0.019
# #15    296051  0.044
# 16    777411  0.083
# 17   2049025  0.219
# 18   5384855  0.585
# 19  14190509  1.560
# 20  37313977  4.184
# 21  98324565 11.214
# 22 258654441 30.012
# 23 681552747 80.664

[Ns, counts, times] = tabutil_m.float_columns_from_file('upsaw-times.txt')

log_counts = map(math.log, counts)
log_times  = map(math.log, times)

[cm, cb, csm, csb] = stats_m.linear_regression(Ns, log_counts)
[tm, tb, tsm, tsb] = stats_m.linear_regression(Ns, log_times)

print '#N exact_count exact_time appx_count appx_time'
print '#- ----------- ---------- ---------- ---------'
for N in range(10, 41):
	approx_count = math.exp(cm*N+cb)
	approx_time  = math.exp(tm*N+tb)
	if N in Ns:
		exact_count = counts[Ns.index(N)]
		exact_time  = times[Ns.index(N)]
		print '%2d %11.7f %11.7f %11.7f %11.7f' % \
			(N, exact_count, exact_time, approx_count, approx_time)
	else:
		print '%2d _ _ %11.7f %11.7f' % (N, approx_count, approx_time)
