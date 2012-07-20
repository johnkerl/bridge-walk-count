#!/usr/bin/python -Wall

from __future__ import division
import sys, os, re

Nmin = 10
Nmax = 20
dir='data'

frac_strings = [
	'0/1',
	'1/10',
	'1/9',
	'1/8',
	'1/7',
	'1/6',
	'1/5',
	'2/9',
	'1/4',
	'2/7',
	'3/10',
	'1/3',
	'3/8',
	'2/5',
	'3/7',
	'4/9',
	'1/2',
	'3/6',
	'5/9',
	'4/7',
	'3/5',
	'5/8',
	'2/3',
	'7/10',
	'5/7',
	'3/4',
	'7/9',
	'4/5',
	'5/6',
	'6/7',
	'7/8',
	'8/9',
	'9/10',
	'1/1',
]

if len(sys.argv) == 2:
	Nmin = int(sys.argv[1])
	Nmax = int(sys.argv[1])
elif len(sys.argv) == 3:
	Nmin = int(sys.argv[1])
	Nmax = int(sys.argv[2])
elif len(sys.argv) == 4:
	dir  = sys.argv[1]
	Nmin = int(sys.argv[2])
	Nmax = int(sys.argv[3])

os.system('mkdir -p %s' % (dir))

for position in ['above', 'below']:
	for frac_string in frac_strings:
		#print frac_string, '...'
		frac_name_for_file = re.sub('/', '_', frac_string)

		raw_counts_file = '%s/raw_counts_%s_%s.txt' \
			% (dir, position, frac_name_for_file)
		cmd = './count_walks %s-slope=%s %d-%d | right > %s' % \
			(position, frac_string, Nmin, Nmax, raw_counts_file)
		print cmd
		if os.system(cmd) != 0:
			print >> sys.stderr, 'Aborting.'
			sys.exit(1)
