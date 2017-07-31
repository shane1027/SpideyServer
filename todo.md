##Bug Fixes
----------

- Mime-type handling issues
    - for some god-forsaken reason, reading the possible mime-types doesn't
      include the first character of file extensions

- change fgets() to fread() in file streaming

- use fstat() to lookup file size before sending, include in header

- Handling bad request fails if URI is null (check URI isn't null)

- figure out what those weird requests are which i'm not making??

- 'handle_error' function sometimes breaks

- Need to close sockets (bound to whatever port) when crashing
    - temporary workaround is binding to a different port on startup
    - in reality, need to catch stop and kill signals and clean up
      appropriately, including closing all open file descriptors

