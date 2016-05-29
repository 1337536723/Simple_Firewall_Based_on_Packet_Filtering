all: icmp4.c tcp4.c udp4.c 
	gcc -Wall -o icmp4 icmp4.c -lpcap 
	gcc -Wall -o tcp4 tcp4.c -lpcap 
	gcc -Wall -o udp4 udp4.c -lpcap 
    
clean:  
	rm -rf *.o test 
