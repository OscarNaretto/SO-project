CC=gcc
COMPILATION_FLAGS= -std=c89 -pedantic -Wall -Werror
DEPENDENCIES = src/*.h

OBJTAXI = build/Taxi.o build/Common.o 
OBJSOURCE = build/Source.o build/Common.o 
OBJMASTER = build/Master.o build/Common.o 

all: bin/Taxi bin/Source bin/Master

run:
	bin/Master

build/%.o: src/%.c $(DEPENDENCIES)
	$(CC) $(CFLAGS) -c $< -o $@

bin/Taxi: $(OBJTAXI) $(DEPENDENCIES)
	$(CC) $(COMPILATION_FLAGS) -o bin/Taxi $(OBJTAXI) 

bin/Source: $(OBJSOURCE) $(DEPENDENCIES)
	$(CC) $(COMPILATION_FLAGS) -o bin/Source $(OBJSOURCE) 

bin/Master: $(OBJMASTER) $(DEPENDENCIES)
	$(CC) $(COMPILATION_FLAGS) -o bin/Master $(OBJMASTER) 

clean:
	$(RM) build/*
	$(RM) bin/*