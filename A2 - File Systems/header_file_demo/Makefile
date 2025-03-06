
main : ll.o main.o stack.o
	gcc -Wall -g -o main main.o ll.o stack.o

ll.o : ll.c ll.h
	gcc -Wall -c ll.c
	
stack.o : stack.c ll.h
	gcc -Wall -c stack.c

main.o : main.c ll.h stack.h
	gcc -Wall -c main.c

clean :
	rm *.o main
