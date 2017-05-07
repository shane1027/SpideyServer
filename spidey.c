/* spidey: Simple HTTP Server */

#include "spidey.h"

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>

/* Global Variables */
char *Port	      = "9898";
char *MimeTypesPath   = "/etc/mime.types";
char *DefaultMimeType = "text/plain";
char *RootPath	      = "www";
mode  ConcurrencyMode = SINGLE;

/**
 * Display usage message.
 */
void
usage(const char *progname, int status)
{
    fprintf(stderr, "Usage: %s [hcmMpr]\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    -h            Display help message\n");
    fprintf(stderr, "    -c mode       Single or Forking mode\n");
    fprintf(stderr, "    -m path       Path to mimetypes file\n");
    fprintf(stderr, "    -M mimetype   Default mimetype\n");
    fprintf(stderr, "    -p port       Port to listen on\n");
    fprintf(stderr, "    -r path       Root directory\n");
    exit(status);
}

/**
 * Parses command line options and starts appropriate server
 **/
int
main(int argc, char *argv[])
{
    int sfd;

    /* Parse command line options */
	int argind = 1;
	char *progname = argv[0];
	char *path;
        debug("aboue to check input params");

    if (argc > 0) {
        debug("found input params");
	while (argind < argc && strlen(argv[argind]) > 1 && (argv[argind])[0] == '-') {
		char *arg = argv[argind++];
                debug("popped arg");
		if (arg[1] == 'c') {
                        debug("handled arg");
                    if (argind < argc) {
			if ((strcmp(argv[argind++], "forking")) == 0) {
                            ConcurrencyMode = FORKING;
                            debug("concurrency mode: FORKING");
                        } else {
                            ConcurrencyMode = SINGLE;
                            debug("concurrency mode: SINGLE");
                        }
                    } else {
                            debug("concurrency mode: SINGLE");
                    }
		}
		else if (arg[1] == 'm') {
                        debug("handled arg");
			MimeTypesPath = argv[argind++];
		}
		else if (arg[1] == 'M') {
                        debug("handled arg");
			DefaultMimeType = argv[argind++];
		}
		else if (arg[1] == 'p') {
                        debug("handled arg");
			Port = argv[argind++];
		}
		else if (arg[1] == 'r') {
                        debug("handled arg");
			path = argv[argind++];
		}
		else if (arg[1] == 'h') {
                        debug("handled arg");
			usage(progname, 0);
		}
		else { 
                    debug("handled arg");
                    usage(progname, 1); 
                }
	}
    }
        debug("about to listen to server socket");
    /* Listen to server socket */
	sfd = socket_listen(Port);

    /* Determine real RootPath */
	RootPath = realpath(path, RootPath);

    log("Listening on port %s", Port);
    debug("RootPath        = %s", RootPath);
    debug("MimeTypesPath   = %s", MimeTypesPath);
    debug("DefaultMimeType = %s", DefaultMimeType);
    debug("ConcurrencyMode = %s", ConcurrencyMode == SINGLE ? "Single" : "Forking");

    /* Start either forking or single HTTP server */
    if (ConcurrencyMode == SINGLE) { single_server(sfd); }
    else { forking_server(sfd); }
    
    return EXIT_SUCCESS;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
