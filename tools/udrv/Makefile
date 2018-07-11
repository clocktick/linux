CFLAGS=-Wall -g -I ./include `pkg-config --cflags libelf`
LDFLAGS=`pkg-config --libs libelf`

all: udrv-loader run

udrv-loader: udrv-loader.o
	${CC} $^ ${LDFLAGS} -o $@

clean:
	rm -f *.o
	rm -f udrv-loader

run:
	./udrv-loader ../../build/vmlinux karg0 karg1=v1

.PHONY:	clean
