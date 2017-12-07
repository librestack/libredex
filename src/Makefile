CC=gcc
CCOPTS=-Wall -g
PROGRAM=libredex
LIBS=-llibrecast
COMMON_OBJECTS=${PROGRAM}.o cJSON.o

${PROGRAM}: ${COMMON_OBJECTS}
	${CC} ${CCOPTS} -o ${PROGRAM} ${COMMON_OBJECTS} ${LIBS}

$(PROGRAM).o: $(PROGRAM).c
	${CC} ${CCOPTS} -c ${PROGRAM}.c ${LIBS}

cJSON.o: cJSON.c cJSON.h
	${CC} ${CCOPTS} -c cJSON.c

.PHONY: clean

clean:
	rm -f *.o ${PROGRAM}
