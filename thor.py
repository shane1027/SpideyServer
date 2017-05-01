#!/usr/bin/env python2.7

import multiprocessing
import os
import requests
import sys
import time

# Globals

PROCESSES = 1
REQUESTS  = 1
VERBOSE   = False
URL       = None

# Functions

def usage(status=0):
    print '''Usage: {} [-p PROCESSES -r REQUESTS -v] URL
    -h              Display help message
    -v              Display verbose output

    -p  PROCESSES   Number of processes to utilize (1)
    -r  REQUESTS    Number of requests per process (1)
    '''.format(os.path.basename(sys.argv[0]))
    sys.exit(status)

def do_request(pid):
	sum = 0
	for x in range(0, REQUESTS):
		begin = time.time()
		r = requests.get(URL)
		end = time.time()
		runTime = end - begin
		print 'Process: {}, Request: {}, Elapsed time: {}'.format(pid, x, "%.2f" % runTime)
		sum += runTime
		
	averageTime = 1.0*sum/REQUESTS
	print 'Process: {}, AVERAGE   , Elapsed time: {}'.format(pid, "%.2f" % averageTime)
	return averageTime

# Main execution

if __name__ == '__main__':
    # Parse command line arguments
	args = sys.argv[1:]
	while len(args) > 1 and args[0].startswith('-') and len(args[0]) > 1:
		arg = args.pop(0)
		if arg == '-v':
			VERBOSE = True
		elif arg == '-p':
			PROCESSES = int(args.pop(0))
		elif arg == '-r':
			REQUESTS = int(args.pop(0))
		elif arg == '-h':
			usage(0)
		else:
			usage(1)
			
	URL = args[0]
	
	if (VERBOSE):
		r = requests.get(URL)
		print r.text
			
    # Create pool of workers and perform requests
	pool = multiprocessing.Pool(PROCESSES)
	totalT = pool.map(do_request, range(PROCESSES))
	averageT = 1.0*sum(totalT) / PROCESSES
	print 'TOTAL AVERAGE ELAPSED TIME: {}'.format("%.2f" % averageT)
	pass

# vim: set sts=4 sw=4 ts=8 expandtab ft=python:
