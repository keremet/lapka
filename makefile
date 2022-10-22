all: lapka_server lapka_client

protocol.o: protocol.c protocol.h
	gcc -c $< -o $@

lapka_server: lapka_server.c protocol.o protocol.h
	gcc $< protocol.o -pthread -o $@

lapka_client: lapka_client.c protocol.o protocol.h
	gcc $< protocol.o -o $@

clean:
	rm -rf lapka_server lapka_client protocol.o

install-server: lapka_server
	cp lapka_server /usr/sbin/
	cp lapka-server.service /lib/systemd/system/

install-client: lapka_client
	cp lapka_client /usr/sbin/
	cp lapka-client.service /lib/systemd/system/
