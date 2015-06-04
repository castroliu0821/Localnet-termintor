server :
	gcc -I ./ user.c user_op.c server.c server_ui.c server_op.c server_main.c log.c -o server -lpthread -lcdk -lncurses -Wall -g
	gcc -I ./ user.c client_ui.c client_op.c client_main.c -o client -lpthread -lcdk -lncurses -Wall -g
clean :
	rm -f server client

