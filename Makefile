
CFLAGS=-g -Wall -Werror

TARGETS=proj3

all: $(TARGETS)

clean:
	rm -f $(TARGETS) 
	rm -rf *.dSYM
	rm -f *.html
	rm -f *.png
	rm -f *.jpg
	rm -f *.dat
	rm -f *.out

distclean: clean
