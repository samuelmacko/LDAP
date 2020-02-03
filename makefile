CC = g++
CFLAGS = -std=c++11 -Wall -Wextra -pedantic

myldap: main.o LDAP_tools.o Network.o Parameters.o 
	$(CC) $(CFLAGS) -o myldap main.o LDAP_tools.o Network.o Parameters.o
	
main.o: main.cpp main.h
	$(CC) $(CFLAGS) -c main.cpp
	
LDAP_tools.o: LDAP_tools.cpp LDAP_tools.h
	$(CC) $(CFLAGS) -c LDAP_tools.cpp
	
Network.o: Network.cpp Network.h
	$(CC) $(CFLAGS) -c Network.cpp
	
Parameters.o: Parameters.cpp Parameters.h
	$(CC) $(CFLAGS) -c Parameters.cpp