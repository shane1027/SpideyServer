#!/usr/bin/env	python2.7

import time
import requests
from prettytable import PrettyTable

#test 30 times

table = PrettyTable(['Test', 'Latency'])

for i in range(30):

	start_time = time.time()

	r = requests.get('http://google.com')

	end_time = time.time()

	latency = (end_time - start_time) 
	
	table.add_row([i, elapsed])
print tabulate(ListofData)


