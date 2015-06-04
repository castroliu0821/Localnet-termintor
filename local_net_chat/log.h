#ifndef _LOG_H_
#define _LOG_H_

#include <pthread.h>

void log_heartbeat_err( char * ip_addr ) ;
void log_register(char * name , int res) ;
void log_login(char *name , int res) ;
void log_logout(char *name) ;
void log_cmd(char *name, char * cmd) ;
void log_quit(char * ip_addr, long time) ;
void log_send(char *src, char *dest) ;
void log_connect(char * ip_addr) ;

extern pthread_mutex_t log_mutex ;

#endif
