WARNINGS_OPTS=-Werror -Wall -W -Wno-unused-parameter
CFLAGS=${WARNINGS_OPTS} -g -I ./include `pkg-config --cflags libelf`
LDFLAGS=`pkg-config --libs libelf`

all: hyperu-loader

LINUX_X86_OBJS=linux/ops.o \
	       linux/x86/msr.o \
	       linux/x86/io.o \
	       linux/x86/arch.o

hyperu-loader: loader.o init.o ops.o ${LINUX_X86_OBJS}
	${CC} $^ ${LDFLAGS} -o $@

clean:
	rm -f linux/x86/*.o
	rm -f linux/*.o
	rm -f *.o
	rm -f hyperu-loader

run:
	sudo ./hyperu-loader ../../build/vmlinux karg0 karg1=v1 hello-world

index: clean
	cscope -b -q -R
	ctags -R

.PHONY:	index clean
