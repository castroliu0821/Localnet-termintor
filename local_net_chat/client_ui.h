#ifndef _SERVER_H_
#define _SERVER_H_

#include <cdk.h>
#include <curses.h>

#include "user.h"
#include "protocol.h"

extern CDKENTRY     * name_widget ;         //　发送名称控件
extern CDKSCROLL    * cmd_widget ;          //　命令控件
extern CDKMENTRY    * chat_widget ;         //  字符内容控件
extern CDKSWINDOW   * msg_widget ;          //  消息输入控件
extern CDKCALENDAR  * calendar_widget ;     //　日历窗口控件

/*
    初始化客户端界面
*/
extern int init_window_ui( void ) ;

/*
    设计请文件连接状态
*/
extern void set_user_status( int status) ;

/*
    通过窗口获取姓名及密码
*/
extern int login_input(int pot,char *name, char * passwd);

/*
    退出程式清理操作
*/
extern void exit_window_ui( void ) ;    //　清理窗口

/*
    描述 ： 处理来访消息
    0  成功
   -1  失败
*/
extern int client_recv_msg( t_potol_type * dat) ;

/*
    描述 ： 错误编号翻译工作
*/
extern void err_log( int err ) ;


/*
    描述 : 对聊天输入操作的隐藏操作等
*/
extern int show_init_cmd( void ) ;

extern int show_login_cmd( void ) ;

extern void add_msg_widget(char * name) ;

extern void sleep_us(int usec) ;

#endif
