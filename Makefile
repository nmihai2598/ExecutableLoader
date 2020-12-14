CC = gcc
CFLAGS = -fPIC -Wall
LDFLAGS = -Wall
LINK = -lpthread

.PHONY: build
build: libscheduler.so

libscheduler.so: so_scheduler.o utils.o
	$(CC) $(LDFLAGS) -shared -o $@ $^

so_scheduler.o: so_scheduler.c
	$(CC) $(CFLAGS) -o $@ -c $< $(LINK)

utils.o: utils.c
	$(CC) $(CFLAGS) -o $@ -c $< $(LINK)

.PHONY: clean
clean:
	-rm -f  so_scheduler.o utils.o libscheduler.so
