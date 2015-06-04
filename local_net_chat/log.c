#include <pthread.h>
#include <string.h>
#include <curses.h>
#include <cdk.h>
#include <time.h>

#include "log.h"
#include "server_ui.h"
#include "protocol.h"

#define BUFF_MAX 1024 

#define COLOR_HEARTBEAT "</B/16>"
#define COLOR_REG       "</B/24>"
#define COLOR_LOGIN     "</B/24>"
#define COLOR_CMD       "</B/32>"
#define COLOR_QUIT      "</B/32>"
#define COLOR_CONNECT   "</B/40>"
#define COLOR_ERR_CODE  "</B/40>"

pthread_mutex_t log_mutex =  PTHREAD_MUTEX_INITIALIZER ;

extern CDKSWINDOW * log_widget ;

static void send_log(char *buff);

/*
    心跳包　异常　记录项
*/
void log_heartbeat_err( char * ip_addr )
{
    char buff[BUFF_MAX] ={0};     
    memcpy(buff, COLOR_HEARTBEAT, strlen(COLOR_HEARTBEAT));
    strcat(buff, "heartbeat error :") ;
    strcat(buff,COLOR_ERR_CODE);
    strcat(buff, ip_addr) ;
    
    send_log(buff);
}

/*
    用户　注册　记录项
*/
void log_register(char * name , int res)
{
    char buff[BUFF_MAX] ={0};
    
    memcpy(buff, COLOR_REG, strlen(COLOR_REG)) ;
    strcat(buff, name);
    strcat(buff, " register");
    strcat(buff,COLOR_ERR_CODE);
    switch(res)
    {
        case TYPE_OK:
            strcat(buff," success") ;
            break;
        case TYPE_ERR:
            strcat(buff," failed") ;
            break ;
        default :
        break;        
    }    
    send_log(buff);
    
}

/*
    用户登录记录项
*/
void log_login(char *name , int res)
{
    char buff[BUFF_MAX] ={0};
    memcpy(buff, COLOR_REG, strlen(COLOR_LOGIN)) ;    
    strcat(buff, "user login: ") ;
    strcat(buff, name) ;
    strcat(buff, " result:") ;
    strcat(buff,COLOR_ERR_CODE);    
    switch(res)
    {
        case TYPE_OK:
            strcat(buff, " login sucess") ;
        break ;
        case TYPE_ERR:
            strcat(buff, " login failed") ;
        break ;
        default:
        break;
    }    
    
   send_log(buff);
}

/*
    描述 : 用户登出记录
*/
void log_logout(char *name ) 
{
    char buff[BUFF_MAX] ={0};
    memcpy(buff, COLOR_REG, strlen(COLOR_LOGIN)) ;    
    strcat(buff, "user logout: ") ;
    strcat(buff, name) ;
    send_log(buff);
}

/*
    用户执行　命令　记录项
*/
void log_cmd(char *name, char * cmd)
{
    char buff[BUFF_MAX] ={0};
    memcpy(buff, COLOR_CMD, strlen(COLOR_CMD));
    strcat(buff, name) ;
    strcat(buff, " execute command ");
    strcat(buff, "\"") ;
    strcat(buff, cmd) ;
    strcat(buff, "\"") ;
    send_log(buff);
}

/*
    用户退出  记录  项
*/
void log_quit(char * ip_addr, long time)
{
    char buff[BUFF_MAX] ={0};
    char data[16];
    memcpy(buff, COLOR_QUIT, strlen(COLOR_QUIT));
    strcat(buff, "client quit : ");
    strcat(buff,COLOR_ERR_CODE);
    strcat(buff, ip_addr);
    strcat(buff, " connect") ;
    sprintf(data, " %ld", time) ;
    strcat(buff, data) ;
    strcat(buff, " sec") ;
    send_log(buff);
}

/*
    用户客户 连接 记录项
*/
void log_connect(char * ip_addr)
{
    char buff[BUFF_MAX] ={0};
    memcpy(buff, COLOR_CONNECT, strlen(COLOR_CONNECT));
    
    strcat(buff, ip_addr);
    strcat(buff, " connected ") ;
     
    send_log(buff);
}

void log_send(char *src, char *dest)
{
    char buff[ BUFF_MAX ] ;
    memcpy(buff, COLOR_CONNECT, strlen(COLOR_CONNECT));
    strcat(buff, src) ;
    strcat(buff, " send msg to ") ;
    strcat(buff, dest) ;
    send_log(buff) ;
}

/*
    将记录的结果发送
*/
static void send_log(char *buff)
{
    char log[ BUFF_MAX ] ;
    time_t t;
    struct tm * tmp ;
    
    time(&t) ;
    tmp = localtime(&t) ;
    strftime(log, BUFF_MAX, "[%F %T]:", tmp) ;
    strcat(log, buff) ;

    pthread_mutex_lock(&log_mutex) ;
    addCDKSwindow(log_widget, log, BOTTOM);
    pthread_mutex_unlock(&log_mutex) ;
}
