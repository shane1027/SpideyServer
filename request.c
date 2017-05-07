/* request.c: HTTP Request Functions */

#include "spidey.h"

#include <errno.h>
#include <string.h>

#include <unistd.h>

/* Constants */
#define WHITESPACE              " \t\n"


int parse_request_method(struct request *r);
int parse_request_headers(struct request *r);

/**
 * Accept request from server socket.
 *
 * This function does the following:
 *
 *  1. Allocates a request struct initialized to 0.
 *  2. Initializes the headers list in the request struct.
 *  3. Accepts a client connection from the server socket.
 *  4. Looks up the client information and stores it in the request struct.
 *  5. Opens the client socket stream for the request struct.
 *  6. Returns the request struct.
 *
 * The returned request struct must be deallocated using free_request.
 **/
struct request * accept_request(int sfd)
{
    struct request *r;
    struct sockaddr raddr;
    socklen_t rlen = sizeof(struct sockaddr);

    /* Allocate request struct (zeroed) */
    r = calloc(1, sizeof(struct request));
    
    // unsure about intializing headers
    r->headers = NULL;

    /* Accept a client */
    int client_fd;
    if ((client_fd = accept(sfd, &raddr, &rlen)) < 0) {
    	fprintf(stderr, "Unable to accept: %s\n", strerror(errno));
    	goto fail;
    }

    r->fd = client_fd;

    /* Lookup client information */
    int status;
    if ((status = getnameinfo(&raddr, rlen, r->host, sizeof(r->host), r->port, sizeof(r->port), 0) != 0)) {
        fprintf(stderr, "Getnameinfo failed: %s\n", gai_strerror(status));
        goto fail;
    }

    /* Open socket stream */
    r->file = fdopen(client_fd, "w+");

    log("Accepted request from %s:%s", r->host, r->port);
    return r;

fail:
    free_request(r);
    return NULL;
}

/**
 * Deallocate request struct.
 *
 * This function does the following:
 *
 *  1. Closes the request socket stream or file descriptor.
 *  2. Frees all allocated strings in request struct.
 *  3. Frees all of the headers (including any allocated fields).
 *  4. Frees request struct.
 **/
void
free_request(struct request *r)
{
    struct header *header;  // not sure what to do with this

    if (r == NULL) {
    	return;
    }

    /* Close socket or fd */
    close(r->fd);
    debug("closed socket");

    /* Free allocated strings */
    free(r->method);
    free(r->uri);
    free(r->path);
    if (strcmp(r->query, "NULL"))
        free(r->query);

    debug("freed some stuff");

    /* Free headers */
    header = r->headers;
    struct header *tmp;

    while (header) {
        tmp = header;
        header = header->next;
        free(tmp->value);
        free(tmp->name);
        free(tmp);
    }

    debug("freed all headers");

    free(r->headers);
    debug("freed header structure");

    /* Free request */
    free(r);
    debug("freed request!");
}

/**
 * Parse HTTP Request.
 *
 * This function first parses the request method, any query, and then the
 * headers, returning 0 on success, and -1 on error.
 **/
int
parse_request(struct request *r)
{
    /* Parse HTTP Request Method */
    if ( parse_request_method(r) < 0 ) {
        return -1;
    }
    /* Parse HTTP Request Headers*/
    if ( parse_request_headers(r) < 0) {
        return -1;
    }

    return 0;
}

/**
 * Parse HTTP Request Method and URI
 *
 * HTTP Requests come in the form
 *
 *  <METHOD> <URI>[QUERY] HTTP/<VERSION>
 *
 * Examples:
 *
 *  GET / HTTP/1.1
 *  GET /cgi.script?q=foo HTTP/1.0
 *
 * This function extracts the method, uri, and query (if it exists).
 **/
int
parse_request_method(struct request *r)
{   
    char buffer[BUFSIZ];

    debug("parsing method");

    /* Read line from socket */

    if (fgets(buffer, BUFSIZ, r->file) == NULL) {
        goto fail;
    }

    /* Parse method and uri */

    char * method = strtok(buffer, WHITESPACE);
    char * uri = strtok(NULL, WHITESPACE);
    char * temp_uri = strdup(uri);

    debug("got method and uri");

    //NOTE TO SELF: Remember that this means it won't work if there is a second '?'' in the query string
    char * query = strchr(uri, '?');

    char * real_uri = strtok(temp_uri, "?");

    debug("got query");

    /* ≈ */

    /* Record method, uri, and query in request struct */
    debug("METHOD: %s", method);
    debug("URI: %s", real_uri);

    r->method = strdup(method);
    r->uri = strdup(real_uri);
    if (query) {
        r->query = strdup(++query);
        debug("QUERY: %s", query);
    } else {
        r->query = "NULL";
    }

    debug("HTTP METHOD: %s", r->method);
    debug("HTTP URI:    %s", r->uri);
    debug("HTTP QUERY:  %s", r->query);

    free(temp_uri);
    return 0;

fail:
    return -1;
}

/**
 * Parse HTTP Request Headers
 *
 * HTTP Headers come in the form:
 *
 *  <NAME>: <VALUE>
 *
 * Example:
 *
 *  Host: localhost:8888
 *  User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:29.0) Gecko/20100101 Firefox/29.0
 *  Accept: text/html,application/xhtml+xml
 *  Accept-Language: en-US,en;q=0.5
 *  Accept-Encoding: gzip, deflate
 *  Connection: keep-alive
 *
 * This function parses the stream from the request socket using the following
 * pseudo-code:
 *
 *  while (buffer = read_from_socket() and buffer is not empty):
 *      name, value = buffer.split(':')
 *      header      = new Header(name, value)
 *      headers.append(header)
 **/
int
parse_request_headers(struct request *r)
{
    struct header *curr = NULL;
    char buffer[BUFSIZ];
    char *name;
    char *value;

    debug("parsing header");

    r->headers = curr;

    
    /* Parse headers from socket */

    while( fgets(buffer, BUFSIZ, r->file) && strlen(buffer) > 2)  {
        curr = calloc(1, sizeof(struct header));

        name = strtok(buffer, ": ");
        value = strtok(NULL, "\r\n");

        curr->name = strdup(name);
        curr->value = strdup(value);
        
        curr = curr->next;

    }

    /*
    //Check if no headers?
    if (r->header == NULL) {
        goto fail;
    }
    */

#ifndef NDEBUG
    for (struct header *header = r->headers; header != NULL; header = header->next) {
    	debug("HTTP HEADER %s = %s", header->name, header->value);
    }
#endif
    return 0;

fail:
    return -1;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
