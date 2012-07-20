#!/usr/bin/python -Wall

# ================================================================
# John Kerl
# kerl at math dot arizona dot edu
# 2009-10-08
# Please see http://math.arizona.edu/~kerl/doc/python-talk.pdf for
# more information.
# ================================================================

import tabutil_m
import pylab
import sys

[theta1s, w1s] = tabutil_m.float_columns_from_file('tgk-lattice-effect.txt')
[theta2s, w2s] = tabutil_m.float_columns_from_file('w_theta_fit_a1.txt')
[theta3s, w3s] = tabutil_m.float_columns_from_file('w_theta_fit_a3.txt')

w1s = map(lambda w: w*15, w1s)

pylab.figure()
pylab.plot(theta1s, w1s, label='tgk')
pylab.plot(theta2s, w2s, label='a1')
pylab.plot(theta3s, w3s, label='a3')
pylab.legend()
pylab.show()

