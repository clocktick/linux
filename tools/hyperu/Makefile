CFLAGS=-Wall -g -I ./include `pkg-config --cflags libelf`
LDFLAGS=`pkg-config --libs libelf`

all: hyperu-loader

LINUX_X86_OBJS=linux/ops.o linux/x86/msr.o linux/x86/io.o

hyperu-loader: loader.o init.o ops.o ${LINUX_X86_OBJS}
	${CC} $^ ${LDFLAGS} -o $@

clean:
	rm -f linux/x86/*.o
	rm -f linux/*.o
	rm -f *.o
	rm -f hyperu-loader

run:
	sudo ./hyperu-loader ../../build/vmlinux karg0 karg1=v1 hello-world

.PHONY:	clean
