#!/usr/bin/env python2.7

import requests
import time

HOST = "http://student00.cse.nd.edu"
PORT = 9454
SIZE = 51

URLS = {
        'dir_list': '{}:{}/'.format(HOST,PORT),
        'static_file':
        '{}:{}/text/hackers.txt'.format(HOST,PORT),
        'CGI':
        '{}:{}/scripts/cowsay.sh?message=hi+there+world%21&template=bunny'.format(HOST,PORT),
        'small_file':
        '{}:{}/test/small.txt'.format(HOST,PORT),
        'medium_file':
        '{}:{}/test/medium.txt'.format(HOST,PORT),
        'large_file':
        '{}:{}/test/large.txt'.format(HOST,PORT)
    }

DIR_LIST_TIMES = []
STATIC_FILE_TIMES = []
CGI_TIMES = []

#   check to make sure our URLs are as expected
# for key in URLS:
    # print URLS[key]

#   call each URL 50 times and measure latency
for num in range(0,SIZE):
    start_time = time.time()
    r = requests.get(URLS['dir_list'])
    stop_time = time.time()
    DIR_LIST_TIMES.append(stop_time - start_time)

for num in range(0,SIZE):
    start_time = time.time()
    r = requests.get(URLS['static_file'])
    stop_time = time.time()
    STATIC_FILE_TIMES.append(stop_time - start_time)

for num in range(0,SIZE):
    start_time = time.time()
    r = requests.get(URLS['CGI'])
    stop_time = time.time()
    CGI_TIMES.append(stop_time - start_time)

# time for some printing
headers = ["Directory Listing Times", "Static File Times", "CGI Script Times"]
print "| {} | {} | {} |".format(headers[0], headers[1], headers[2])
print "|:--------:|:--------:|:--------:|"

for num in range(0,SIZE):
    print "| {} | {} | {} |".format(DIR_LIST_TIMES[num],
            STATIC_FILE_TIMES[num], CGI_TIMES[num])

# calculate average time for each
DIR_TOTAL = 0
STATIC_TOTAL = 0
CGI_TOTAL = 0

for item in DIR_LIST_TIMES:
    DIR_TOTAL += item
DIR_AVG = DIR_TOTAL / SIZE

for item in STATIC_FILE_TIMES:
    STATIC_TOTAL += item
STATIC_AVG = STATIC_TOTAL / SIZE

for item in CGI_TIMES:
    CGI_TOTAL += item
CGI_AVG = CGI_TOTAL / SIZE

print "### Average Times:"
print "#### Directory Listing: {} seconds".format(DIR_AVG)
print "#### Static File Retrieval: {} seconds".format(STATIC_AVG)
print "#### CGI Script Execution: {} seconds".format(CGI_AVG)

# now check the throughput of the server
with open("/dev/null", 'wb') as f:
    start_time = time.time()
    r = requests.get(URLS['small_file'], stream=True)
    for chunk in r.iter_content(chunk_size=1024):
	if chunk:
	    f.write(chunk)
            f.flush()
    end_time = time.time()
    SMALL_THROUGH = end_time - start_time

with open("/dev/null", 'wb') as f:
    start_time = time.time()
    r = requests.get(URLS['medium_file'], stream=True))
    for chunk in r.iter_content(chunk_size=1024):
	if chunk:
	    f.write(chunk)
            f.flush()
    end_time = time.time()
    MEDIUM_THROUGH = end_time - start_time

with open("/dev/null", 'wb') as f:
    start_time = time.time()
    r = requests.get(URLS['large_file'], stream=True))
    for chunk in r.iter_content(chunk_size=1024):
	if chunk:
	    f.write(chunk)
            f.flush()
    end_time = time.time()
    LARGE_THROUGH = end_time - start_time

print
print "Small Throughput: 1kb in {} seconds".format(SMALL_THROUGH)
print "Medium Throughput: 1Mb in {} seconds".format(MEDIUM_THROUGH)
print "Large Throughput: 1Gb in {} seconds".format(LARGE_THROUGH)


