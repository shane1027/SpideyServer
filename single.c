/* single.c: Single User HTTP Server */

#include "spidey.h"

#include <errno.h>
#include <string.h>

#include <unistd.h>

/**
 * Handle one HTTP request at a time
 **/
void
single_server(int sfd)
{
    struct request *request;

    /* Accept and handle HTTP request */
    while (true) {
    	/* Accept request */
    	request = accept_request(sfd);
        debug("received request");
    	if (request == NULL) { debug("NULL request");continue; }

        debug("about to handle request");
		/* Handle request */
		handle_request(request);

                debug("about to free request");
		/* Free request */
		free_request(request);
    }

    /* Close socket and exit */
    debug("about to close socket");
    close(sfd);
    exit(0);
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
