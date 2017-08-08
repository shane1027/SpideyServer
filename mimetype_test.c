/*****************************
 * Test bed for the mime_types
 * interpretation function for
 * my SpideyServer
 * **************************/


#include <stdio.h>
#include "spidey.h" 
#include <string.h>
#include <ctype.h>

char *MimeTypesPath   = "/etc/mime.types";

char * determine_mimetype(const char *path);

int main(void) {
    
    /*  set aside some memory for storing input */
    char buffer[BUFSIZ];
    char * input;
    char * mime;

    /*  clear out our buffer for use    */
    memset(buffer, 0, BUFSIZ);
    printf("please enter a file name w/ extension: "); 

    /*  get the user input and store in memory  */
    input = fgets(buffer, BUFSIZ, stdin);

    /*  strip away the final \n in the buffer   */
    char *p = input;
    while (*p) {
        p++;            // iterate to end of the string
    }
    *(--p) = 0;         // then replace '\n' with NULL


    printf("Entered: %s\n", input);
    printf("Determining mimetype...\n");

    mime = determine_mimetype(input);

    printf("Mimetype determined to be: %s\n", mime);

    return 0;

}


/**
 * Advance string pointer pass all nonwhitespace characters
 **/
char *
skip_nonwhitespace(char *s)
{
	chomp(s);
	while ((!isspace(*s))&& (*s)) { s++; }
    return s;
}

/**
 * Advance string pointer pass all whitespace characters
 **/
char *
skip_whitespace(char *s)
{
	chomp(s);
	while ((isspace(*s)) && (*s)) { s++; }
    return s;
}



char * determine_mimetype(const char *path)
{
    char *ext;
    char *mimetype;
    char *token;
    char *temp;
    char *tmp2;
    char buffer[BUFSIZ];
    FILE *fs = NULL;

    /* Find file extension */
	ext = strrchr(path, '.');
        ext++;
        debug("extension is %s", ext);

    /* Open MimeTypesPath file */
	fs = fopen(MimeTypesPath, "r");
	if (fs == NULL) { debug("couldn't open mime.types"); goto fail; }

    /* Scan file for matching file extensions */
	while (fgets(buffer, BUFSIZ, fs)) {

        /*  skip lines that are commented out   */
   		if (strchr(buffer, '#')) { continue; }

        /*  duplicate buffer because strtok consumes it */
        temp = strdup(buffer);
        if (temp == NULL) {
            fprintf(stderr, "Error: not enough memory to duplicate mimetype buffer :( \n");
            exit(EXIT_FAILURE);
        }
        // debug("buffer:%s", buffer);
        
        /*  the first block of text is the mimetype */
   		mimetype = strtok(buffer, WHITESPACE);
        // debug("mimetype:%s", mimetype);
        
        /*  the next block is the extension list    */
   		token = skip_whitespace(skip_nonwhitespace(temp));

        /*  if there are no extensions listed, continue */
        if (isblank(token[0]) || iscntrl(token[0]))  { continue; }

        /*  otherwise, let's start ripping through the extensions, looking for
         *  a match!    */
        // debug("tokens:%s", token);
        tmp2 = strtok(token, WHITESPACE);                 
        // debug("first token:%s", tmp2);

   		while (tmp2) {
            /*  check if the extension given matches the desired one    */
            if (strcmp(ext, tmp2) == 0 && (strlen(tmp2) == strlen(ext))) {
                free(temp); 
                goto done;
            }

            /*  if not, move on to the next extension   */
            tmp2 = strtok(NULL, WHITESPACE);
            // debug("next token:%s", tmp2);
   		}

        free(temp);

   	}

    debug("Couldn't find matching extension");
    goto fail;

fail:
    //mimetype = DefaultMimeType;
    mimetype = "text/plain";
    debug("failed, using default mimetype %s", mimetype);

    return strdup(mimetype);

done:
    debug("Found extension: %s", ext);
    if (fs) {
        fclose(fs);
    }
    debug("Returning mimetype %s", mimetype);
    return strdup(mimetype);
}


/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
