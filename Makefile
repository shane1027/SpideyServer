CC=		gcc
CFLAGS=		-g -gdwarf-2 -Wall -std=gnu99
LD=		gcc
LDFLAGS=	-L.
TARGETS=	spidey

all:		$(TARGETS)

spidey:		forking.o handler.o request.o single.o socket.o spidey.o utils.o
	@echo "Linking $@..."
	@$(LD) $(LDFLAGS) -o $@ $^ spidey.h
	
forking.o:	forking.c
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $^
	
handler.o:	handler.c
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $^
	
request.o:	request.c
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $^
	
single.o:	single.c
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $^
	
socket.o:	socket.c
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $^
	
spidey.o:	spidey.c
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $^
	
utils.o:	utils.c
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $^

clean:
	@echo Cleaning...
	@rm -f $(TARGETS) *.o *.log *.input

.PHONY:		all clean
