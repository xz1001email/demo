TARGET1= pthread_cpu
#CC=gcc
#CC=/opt/wisermind_HI3536/arm-hisiv400-linux/bin/arm-hisiv400-linux-gnueabi-gcc
#CC = /opt/wisermind_MPC82xx/bin/ppc_82xx-gcc

CC =arm-hisiv300-linux-gcc
CFLAGS = -pthread -O2 -Wall

dstdir=/project/target/



policy: policy.o
	$(CC) $(CFLAGS) -o $@ $^

policy.o:policy.c
	$(CC) $(CFLAGS) -o $@ -c $^



all: $(TARGET1)

$(TARGET1):bondcpu.o
	$(CC) $(CFLAGS) -o $@ $^
	#cp $@ $(dstdir) -rf

bondcpu.o:bondcpu.c
	$(CC) $(CFLAGS) -o $@ -c $^

clean:
	rm policy $(TARGET1) *.o -rf

