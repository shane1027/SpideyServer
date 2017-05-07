/* forking.c: Forking HTTP Server */

#include "spidey.h"

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

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
        debug("about to accept request");
    	request = accept_request(sfd);
        debug("accepted request");
    	if (request == NULL) { continue; }

	/* Ignore children */
		pid = fork();
		if (pid < 0) {
			fprintf(stderr, "Fork failed: %s", strerror(errno));
                        //free_request(request);
			//close(sfd);
			continue;
		}
		if (pid == 0) { //I am child
                    debug("In child");
			handle_request(request);
                        debug("request handled");
                        free_request(request);
                        debug("request freed");
			//close(sfd);
                        //fclose(request->file);
                        debug("closed sfd");
			exit(0);
		} else { 
                        debug("got to else");
                        int status;
                        waitpid(pid, &status, 0);
                        //fclose(request->file);
			//close(sfd); 
		}
                        fclose(request->file);

	/* Fork off child process to handle request */
		//free_request(request);
    }

    /* Close server socket and exit*/

    exit(0);
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
