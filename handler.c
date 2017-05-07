/* handler.c: HTTP Request Handlers */

#include "spidey.h"

#include <errno.h>
#include <limits.h>
#include <string.h>

#include <dirent.h>
#include <unistd.h>

/* Internal Declarations */
http_status handle_browse_request(struct request *request);
http_status handle_file_request(struct request *request);
http_status handle_cgi_request(struct request *request);
http_status handle_error(struct request *request, http_status status);

/**
 * Handle HTTP Request
 *
 * This parses a request, determines the request path, determines the request
 * type, and then dispatches to the appropriate handler type.
 *
 * On error, handle_error should be used with an appropriate HTTP status code.
 **/
http_status
handle_request(struct request *r)
{
    http_status result;

    /* Parse request */
    int requestStatus;
    if ((requestStatus = parse_request(r)) != 0) { result = handle_error(r, HTTP_STATUS_BAD_REQUEST); }

    /* Determine request path */
    char *real = determine_request_path(r->uri);
    r->path = real;
    debug("HTTP REQUEST PATH: %s", r->path);

    /* Dispatch to appropriate request handler type */
    request_type type = determine_request_type(r->path);
    if (type == REQUEST_BROWSE) { result = handle_browse_request(r); }
    else if (type == REQUEST_FILE) { result = handle_file_request(r); }
    else if (type == REQUEST_CGI) { result = handle_cgi_request(r); }
    else { result = handle_error(r, 404); }

    log("HTTP REQUEST STATUS: %s", http_status_string(result));
    return result;
}

/**
 * Handle browse request
*
 * This lists the contents of a directory in HTML.
 *
 * If the path cannot be opened or scanned as a directory, then handle error
 * with HTTP_STATUS_NOT_FOUND.
 **/
http_status
handle_browse_request(struct request *r)
{
    struct dirent **entries;
    int n;

    /* Open a directory for reading or scanning */
	n = scandir(r->path, &entries, NULL, alphasort);

        if (n < 0) {
            fprintf(stderr, "scanning dir returned an error\n");
            exit(EXIT_FAILURE);
        }
	 
    /* Write HTTP Header with OK Status and text/html Content-Type */
        char return_string[BUFSIZ] = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html>\r\n\t<ul>\r\n\t";
        fputs(return_string, r->file);

    /* For each entry in directory, emit HTML list item */
        for (int i = 0; i < n; i++) {
            fputs("\t<li>", r->file);
            fputs(entries[i]->d_name, r->file);
            fputs("</li>\r\n\t", r->file);
        }

        fputs("</ul>\r\n", r->file);
        fputs("</html>", r->file);

    
    /* Flush socket, return OK */

        if (fflush(r->file)) {
            fprintf(stderr, "Error flushing socket!\n");
        }

    return HTTP_STATUS_OK;
}

/**
 * Handle file request
 *
 * This opens and streams the contents of the specified file to the socket.
 *
 * If the path cannot be opened for reading, then handle error with
 * HTTP_STATUS_NOT_FOUND.
 **/
http_status
handle_file_request(struct request *r)
{
    FILE *fs;
    char buffer[BUFSIZ];
    char *mimetype = NULL;
    size_t nread;

    /* Open file for reading */
    if ((fs = fopen(r->path, "r")) == NULL) {
        fprintf(stderr, "Couldn't open file while handling file request\n");
        exit(EXIT_FAILURE);
    }

    /* Determine mimetype */
    mimetype = determine_mimetype(r->path);

    /* Write HTTP Headers with OK status and determined Content-Type */
        char return_string[BUFSIZ] = "HTTP/1.0 200 OK\r\nContent-Type: ";
        strcat(return_string, mimetype);
        strcat(return_string, "\r\n\r\n<html>\r\n\t<ul>\r\n\t");

        debug("Current HTTP Header: %s", return_string);

    /* Read from file and write to socket in chunks */
        while(fgets(buffer, BUFSIZ, fs)) {
            fputs(buffer, r->file);
        }

    /* Close file, flush socket, deallocate mimetype, return OK */
        fclose(fs);

        if (fflush(r->file)) {
            fprintf(stderr, "Error flushing socket!\n");
        }

        free(mimetype);
        
    return HTTP_STATUS_OK;
}

/**
 * Handle file request
 *
 * This popens and streams the results of the specified executables to the
 * socket.
 *
 *
 * If the path cannot be popened, then handle error with
 * HTTP_STATUS_INTERNAL_SERVER_ERROR.
 **/
http_status
handle_cgi_request(struct request *r)
{
    FILE *pfs;
    char buffer[BUFSIZ];
    struct header *header;

    /* Export CGI environment variables from request:
    * http://en.wikipedia.org/wiki/Common_Gateway_Interface */

    /*  request_path uses strdup, so free when done */
    char * temp_path = determine_request_path(r->uri);
    setenv("DOCUMENT_ROOT", temp_path, 1);
    free(temp_path);

    setenv("QUERY_STRING", r->query, 1);
    setenv("REMOTE_ADDR", r->host, 1);
    setenv("REMOTE_PORT", r->port, 1);
    setenv("REQUEST_METHOD", r->method, 1);
    setenv("REQUEST_URI", r->uri, 1);
    setenv("SCRIPT_FILENAME", r->path, 1);
    setenv("SERVER_PORT", Port, 1);


    /* Export CGI environment variables from request headers */

    header = r->headers;

    while (header) {
        if (strcmp(header->name, "Host") == 0) {
            setenv("HTTP_HOST", header->value, 1);
        } else if (strcmp(header->name, "Accept") == 0) {
            setenv("HTTP_ACCEPT", header->value, 1);
        } else if (strcmp(header->name, "Accept-Language") == 0) {
            setenv("HTTP_ACCEPT_LANGUAGE", header->value, 1);
        } else if (strcmp(header->name, "Accept-Encoding") == 0) {
            setenv("HTTP_ACCEPT_ENCODING", header->value, 1);
        } else if (strcmp(header->name, "Connection") == 0) {
            setenv("HTTP_CONNECTION", header->value, 1);
        } else if (strcmp(header->name, "User-Agent") == 0) {
            setenv("HTTP_USER_AGENT", header->value, 1);
        }

        header = header->next;
    }

    /* POpen CGI Script */
    if ((pfs = popen(r->path, "r")) == NULL) {
        fprintf(stderr, "Couldn't execute command!\n");            
    }

    /* Copy data from popen to socket */
    while (fgets(buffer, BUFSIZ, pfs)) {
        fputs(buffer, r->file);
    }

    /* Close popen, flush socket, return OK */
    fclose(pfs);

    if (fflush(r->file)) {
        fprintf(stderr, "Error flushing socket!\n");
    }

    return HTTP_STATUS_OK;
}

/**
 * Handle displaying error page
 *
 * This writes an HTTP status error code and then generates an HTML message to
 * notify the user of the error.
 **/
http_status
handle_error(struct request *r, http_status status)
{
    const char *status_string = http_status_string(status);

    /* Write HTTP Header */
    char return_string[BUFSIZ] = "HTTP/1.0 ";
    strcat(return_string, status_string);
    strcat(return_string, "\r\nContent-Type: text/html\r\n\r\n<html>\r\n");

    /* Write HTML Description of Error*/
    char error_string[BUFSIZ] = "<body>\r\n\r\n<h1>Error: ";
    strcat(error_string, status_string);
    strcat(error_string, "</h1></body></html>");

    fputs(return_string, r->file);
    fputs(error_string, r->file);

    /* Return specified status */
    return status;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
