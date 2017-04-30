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
	r = requests.get(URL)
    pass

# Main execution

if __name__ == '__main__':
    # Parse command line arguments
	args = sys.argv[1:]
	while len(args) and args[0].startswith('=') and len(args[0]) > 1:
		arg = args.pop(0)
		if arg == '-v':
			VERBOSE = true
		elif arg == '-p':
			PROCESSES = int(args.pop(0))
		elif arg == '-r':
			REQUESTS = int(args.pop(0))
		elif arg == '-h':
			usage(0)
		else:
			usage(1)
			
	if len(args) == 1:
		URL = args[0]
		
    # Create pool of workers and perform requests
    pool = multiprocessing.Pool(PROCESSES)
#    pool.map(do_request(), (URL for x in range(1,REQUESTS)))
	pass

# vim: set sts=4 sw=4 ts=8 expandtab ft=python:
