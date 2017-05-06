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

		/* Handle request */
		http_status status = handle_request(request);

		/* Free request */
		free_request(request);
    }

    /* Close socket and exit */
    close(sfd);
    exit(0);
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
