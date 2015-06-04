#ifndef _CLIENT_OP_H_
#define _CLIENT_OP_H_ 

#include <arpa/inet.h>
#include <sys/socket.h>

#include "protocol.h"

#define MAXSLEEP            128
#define HEARTBEAT_PAUSE     3               // 心跳间隔　单位: 秒
#define FILE_SERVER_CONFIG  "server.cfg"    // 服务器配置文件


/*
        描述 : 状态机
        客户端的运行时某些状态，分析这些状态
        有得于获取不同响应请求。
*/
#define STATE_UNCONNECT    1 //连接断开
#define STATE_NORMAL       2 //普通状态
#define STATE_LOGIN        3 //登陆状态
#define STATE_REGISTER     4 //注册状态
#define STATE_CMD          5 //发送命令状态
#define STATE_CHAT         6 //聊天状态
#define STATE_USER         7 //登陆成功状态


extern int sock_fd ;
extern int client_state;

/*
    描述: 初始化客户商网络系统
        成功　 网络套接字　描述符
        失败    -1
*/
int init_client_system(void) ;

/*
    描述 : 用户发送需要执行的命令
    0 ： 发送成功
   -1  : 发送失败
*/
int client_send_cmd(char * cmd) ;

/*
    描述 : 发送消息给某个用户
    0 ： 发送成功
   -1  : 发送失败
*/
int client_send_msg_to(char * name, char *msg) ;

/*
    描述 : 用户发送执行登陆命令
    0 : 发送成功
   -1 : 发送失败
*/
int client_login(char * name, char * passwd) ;

/*
    描述 ： 用户注册执行命令
*/
int client_regster(char * name , char * passwd) ;

/*
    描述 : 用户发送执行登陆命令
    0 : 发送成功
   -1 : 发送失败
*/
int client_logout( void ) ;

#endif
