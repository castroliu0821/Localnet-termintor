#ifndef _SERVER_H_
#define _SERVER_H_

#include <cdk.h>
#include <curses.h>

extern CDKSCROLL    * cmd_widget ;
extern CDKSWINDOW  * log_widget ;
extern CDKCALENDAR * calendar_widget ;

void clear_window(void) ;
int  init_window_ui( void ) ;
int  check_user(void) ; 
void live_user(void) ;

#endif
