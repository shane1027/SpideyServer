##Bug Fixes
----------

- use fstat() to lookup file size before sending, include in header
        - actually, this may not be necessary, unless needing ftp

- Handling bad request fails if URI is null (check URI isn't null)

- figure out what those weird requests are which i'm not making??

- 'handle_error' function sometimes breaks

- Need to close sockets (bound to whatever port) when crashing
    - temporary workaround is binding to a different port on startup
    - in reality, need to catch stop and kill signals and clean up
      appropriately, including closing all open file descriptors

- why do some '.txt' files open in the browser, while others immediately
  initiate a download?  Could it be because the downloading ones have chars
  that are out of ASCII bounds?

