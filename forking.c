/* forking.c: Forking HTTP Server */

#include "spidey.h"

#include <errno.h>
#include <signal.h>
#include <string.h>

#include <unistd.h>

/**
 * Fork incoming HTTP requests to handle the concurrently.
 *
 * The parent should accept a request and then fork off and let the child
 * handle the request.
 **/
void
forking_server(int sfd)
{
    struct request *request;
    pid_t pid;

    /* Accept and handle HTTP request */
    while (true) {
    	/* Accept request */
    	request = accept_request(sfd);
    	if (request == NULL) { continue; }

	/* Ignore children */
		pid = fork();
		if (pid < 0) {
			fprintf(stderr, "Fork failed: %s", strerror(errno));
			goto finish;
		}
		else if (pid == 0) { //I am child
			handle_request(request);
			close(sfd);
			exit(0);
		}
		else { close(sfd); }

	/* Fork off child process to handle request */
		free_request(request);
    }

finish:
    /* Close server socket and exit*/
    close(sfd);
    exit(0);
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
