/*****************************
 * Test bed for the mime_types
 * interpretation function for
 * my SpideyServer
 * **************************/


#include <stdio.h>
#include "spidey.h" 
#include <string.h>


char * determine_mimetype(const char *path);

void main(void) {
    
   printf("please enter a file name w/ extension: "); 

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
   		if (strchr(buffer, '#')) { continue; }
                temp = strdup(buffer);
   		mimetype = strtok(buffer, WHITESPACE);
                //debug("buffer:%s", buffer);
                //debug("mimetype:%s", mimetype);
   		token = skip_whitespace(skip_nonwhitespace(temp));
                //debug("tokens:%s", token);


                tmp2 = strtok(token, WHITESPACE);                 
                //debug("first token:%s", tmp2);

   		while ((tmp2 = strtok(NULL, WHITESPACE))) {
                //debug("next token:%s", tmp2);
   		if (strcmp(ext, tmp2) == 0 && (strlen(tmp2) == strlen(ext))) {
                    free(temp); 
                    goto done; }
   		}
            free(temp);
   	}
            debug("Couldn't find matching extension");
            goto fail;
fail:
    //mimetype = DefaultMimeType;
    mimetype = "text/javascript";
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
