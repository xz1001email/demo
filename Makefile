

target=generate_bmp

$(target):generate_bmp.o
	$(CC) -o $@ $^

%.o:%.c
	$(CC) -c $^

clean:
	rm $(target) *.o -f

