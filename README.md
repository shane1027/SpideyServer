Project 02 - README
===================

Members
-------

- Radomir Fugiel
- Shane Ryan
- Raelene Alvarez McDermott

Summary
-------
In the project we first created a HTTP client in Python and then
we created a HTTP server that can list directories, files and it
can also handle CGI scripts. After extensive debugging, our project
works compeltely, both with single calls and multiple forked calls.


Latency
-------
For latency it was fastest to access directory listings, then CGI scripts and then static files.
When comparing forking mode to single mode data, forking improved latency by approximately 20%


Throughput
----------

Analysis
--------

Conclusion
----------
What did we learn?

1. Comp sci majors don't get reading days off
2. By building all the individual functions we got a really good
understanding of how HTTP client + servers interact and we all have
a much deeper and applicable understanding of the concepts we covered
in class.


Contributions
-------------

All of us worked together to build the functions and then we worked 
together (for many hours) to figure out all the errors with debugging
