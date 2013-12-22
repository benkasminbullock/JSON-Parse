utf8:	main.c utf8.c
	$(CC) $(CFLAGS) -o $@ main.c

utf8.c:	utf8.pl
	perl utf8.pl

clean:
	-rm -f utf8 utf8.c
