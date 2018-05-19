demon := demon
client := client
source_files = $(wildcard *.c) 
obj_files = $(patsubst %.c,%.o,$(source_files)) 
all: $(demon) $(client)

$(demon): demon.o
	gcc -o $@ $^ 

$(client): client.o
	gcc -o $@ $^ 

$(obj_files): %.o : %.c
	gcc -c $<


clean:
	rm -rf $(demon) $(client) *.o *.a *.so *.txt

