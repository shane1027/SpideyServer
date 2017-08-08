/* handler.c: HTTP Request Handlers */

#include "spidey.h"

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>

#include <dirent.h>
#include <unistd.h>

#define CHUNK_SIZE 5000

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
    if ((requestStatus = parse_request(r)) != 0) { 
        debug("request status was bad");
        result = handle_error(r, HTTP_STATUS_BAD_REQUEST); }


    /* Determine request path */
    char *real = determine_request_path(r->uri);
    if (strcmp(real, "NULL") == 0) {
        result = handle_error(r, HTTP_STATUS_NOT_FOUND);
        return result;
    }
    debug("request parsed");
    debug("determined request path");
    r->path = real;
    debug("HTTP REQUEST PATH: %s", r->path);

    /* Dispatch to appropriate request handler type */
    request_type type = determine_request_type(r->path);
    debug("request type determined to be %d", type);
    if (type == REQUEST_BROWSE) { result = handle_browse_request(r); }
    else if (type == REQUEST_FILE) { result = handle_file_request(r); }
    else if (type == REQUEST_CGI) { result = handle_cgi_request(r); }
    else { result = handle_error(r, HTTP_STATUS_NOT_FOUND); }

    debug("about to log result: %d", result);
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
        debug("directory opened");

        if (n < 0) {
            fprintf(stderr, "scanning dir returned an error\n");
            //free(n);
            exit(EXIT_FAILURE);
        }

    /* Write HTTP Header with OK Status and text/html Content-Type */
        char return_string[BUFSIZ] = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html>\r\n\t<ul>\r\n\t";
        fputs(return_string, r->file);

    /* For each entry in directory, emit HTML list item */
        for (int i = 1; i < n; i++) {
        	char full[BUFSIZ];
        	char *hyperlink;
        	sprintf(full, "%s/%s", r->path, entries[i]->d_name);
        	hyperlink = full + strlen(RootPath);
            fputs("\t<li>", r->file);
            fprintf(r->file, "<a href=\"%s\">", hyperlink);
            fprintf(r->file, "%s</a>", entries[i]->d_name);
            fputs("</li>\r\n\t", r->file);
        }

        fputs("</ul>\r\n", r->file);
        fputs("</html>", r->file);

        debug("printed all entries");

    /* Flush socket, return OK */

        if (fflush(r->file)) {
            fprintf(stderr, "Error flushing socket!\n");
        }

        for (int i = 0; i < n; i++) {
            free(entries[i]);
        }

        debug("flushed socket");

    return HTTP_STATUS_OK;
}

/**
 * Send all data in a buffer to a socket
 *
 * This acts as a wrapper around the send() function to ensure all data in the
 * given buffer is actually sent to the destination, in case the kernel decides
 * to cut us off early
 **/
uint64_t sendall(int socket, void * buf, uint64_t len) {

    // fprintf(stderr, "length provided: %ld\n", len);
    
    long int bytes_sent = 0;
    uint64_t total_sent = 0;

    while (total_sent < len) {
        bytes_sent = send(socket, buf, len, 0);
        // fprintf(stderr, "bytes sent: %ld\n", bytes_sent);
        if (bytes_sent > 0) {
            total_sent += bytes_sent;
        } else {
            fprintf(stderr, "Couldn't write to destination socket!\n");
            //return -1;
        }
    }

    // fprintf(stderr, "total sent: %ld\n", total_sent);

    return total_sent;

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
    int socket_file_descriptor;
    uint8_t buffer[CHUNK_SIZE];
    char *mimetype = NULL;
    size_t byte_count = 0;
    size_t write_count = 0;


    /* Open file for reading */
    if ((fs = fopen(r->path, "rb")) == NULL) {      // read binary!
        fprintf(stderr, "Couldn't open file while handling file request\n");
        exit(EXIT_FAILURE);
    }

    /* Get the file descriptor  */
    if (!(socket_file_descriptor = fileno(r->file))) {
        fprintf(stderr, "Couldn't get file descriptor for file stream!\n");
        exit(EXIT_FAILURE);
    }

    debug("File path %s opened successfully", r->path);

    /* Determine mimetype */
    debug("checking mimetype");
    mimetype = determine_mimetype(r->path);
    debug("mimetype determined to be %s", mimetype);

    /* Write HTTP Headers with OK status and determined Content-Type */
        char return_string[BUFSIZ] = "HTTP/1.0 200 OK\r\nContent-Type: ";
        strcat(return_string, mimetype);
        strcat(return_string, "\r\n\r\n");

        fputs(return_string, r->file);

    /*  Flush now so that this gets to the socket before the data itself    */
        if (fflush(r->file)) {
            fprintf(stderr, "Error flushing socket!\n");
        }

        debug("Current HTTP Header: %s", return_string);

    /* Get the current file size    */
        fseek(fs, 0L, SEEK_END);
        uint32_t file_size = ftell(fs);
        rewind(fs);

    /* Read from file and write to socket in chunks */
        while((byte_count = fread(buffer, 1, sizeof(buffer), fs))) {
            write_count = sendall(socket_file_descriptor, buffer, byte_count);
            if (byte_count != write_count)
                fprintf(stderr, "Error writing file to socket!\n");
        }

        debug("Sent file of size %d bytes", file_size);

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

    debug("CGI request received");
    /* Export CGI environment variables from request:
    * http://en.wikipedia.org/wiki/Common_Gateway_Interface */

    /*  request_path uses strdup, so free when done */
    char * temp_path = determine_request_path(r->uri);

    debug("got the path: %s", temp_path);

    setenv("DOCUMENT_ROOT", temp_path, 1);
    debug("DOCUMENT_ROOT set to path");
    free(temp_path);
    debug("freed temp_path");

    if (strcmp(r->query,"NULL") != 0)
        setenv("QUERY_STRING", r->query, 1);
    debug("got past query");
    setenv("REMOTE_ADDR", r->host, 1);
    setenv("REMOTE_PORT", r->port, 1);
    setenv("REQUEST_METHOD", r->method, 1);
    setenv("REQUEST_URI", r->uri, 1);
    setenv("SCRIPT_FILENAME", r->path, 1);
    setenv("SERVER_PORT", Port, 1);

    debug("all ENV VARS set");


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
    debug("got to the 'handle_error' function");
    const char *status_string = http_status_string(status);

    /* Write HTTP Header */
    char return_string[BUFSIZ] = "HTTP/1.0 ";
    debug("status_string: %s", status_string);
    strcat(return_string, status_string);
    strcat(return_string, "\r\nContent-Type: text/html\r\n\r\n<html>\r\n");

    /* Write HTML Description of Error*/
    char error_string[BUFSIZ] = "<body>\r\n\r\n<h1>Error: ";
    strcat(error_string, status_string);
    strcat(error_string, "</h1></body></html>");

    fputs(return_string, r->file);
    fputs(error_string, r->file);

    if (fflush(r->file)) {
        fprintf(stderr, "error flushing socket");
    }

    /* Return specified status */
    return status;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
