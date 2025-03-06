FLAGS = -Wall -g 

all: closest generate_points

closest: closest.o utilities_closest.o serial_closest.o parallel_closest.o
	gcc ${FLAGS} -o $@ $^ -lm

generate_points: generate_points.o 
	gcc ${FLAGS} -o $@ $^

closest.o: closest.c utilities_closest.h serial_closest.h parallel_closest.h point.h
generate_points.o: generate_points.c point.h

serial_closest.o: serial_closest.h utilities_closest.h point.h
parallel_closest.o: parallel_closest.h serial_closest.h utilities_closest.h point.h
utilities_closest.o: utilities_closest.h point.h

# Separately compile each C file
%.o : %.c 
	gcc ${FLAGS} -c $<

clean:
	rm -f *.o closest generate_points
