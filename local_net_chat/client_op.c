#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "client_op.h"
#include "protocol.h"
#include "user.h"

#define BUFF_SIZE        1024           // 缓冲区最大长度

/*
    描述 : 关键数据　全局变量
*/
int sock_fd ;
int client_state = STATE_NORMAL;


void sig_alarm_handle(int signo)
{
    t_potol_type recv_buff ;
    recv_buff.msg_type = TYPE_HEART ;    
    send(sock_fd, &recv_buff, sizeof(t_potol_type), 0) ;
    alarm( HEARTBEAT_PAUSE ) ;    
}

/*
        描述：　服务器的地址配置文件读取及处理
*/
static int client_config(struct sockaddr_in * addr)
{
    int i = 0;    
    FILE * fl ;
    char buff[ BUFF_SIZE ] ;

    fl = fopen(FILE_SERVER_CONFIG, "r+");
    if(NULL == fl)
        return -1 ;
           
    fgets(buff, BUFF_SIZE, fl) ;
    *(buff+strlen(buff)-1) = '\0' ;
    if(0 == strncmp("SERVER", buff, 6))
    {
        if(1 != inet_pton(AF_INET, strchr(buff, '=')+1, &addr->sin_addr.s_addr))
        {
            fclose(fl) ;
            return -1 ;
        }
        i++ ;                                               // 第一步 : 读取服务器 IP地址
    }
    fgets(buff, BUFF_SIZE, fl) ;
    *(buff+strlen(buff)-1) = '\0' ;
    if(0 == strncmp("PORT", buff, 4))     
    {
        addr->sin_port = htons(atoi(strchr(buff, '=')+1)) ;
        i++;                                                // 第二步 ： 读取服务器口    
    }
    if(2 != i)   
        return -1 ;
    else
        return 0 ;
}

/*
    描述 : 支持重试连接的
*/
static int connect_retry(int sockfd, const struct sockaddr *addr, socklen_t alen)
{
	int numsec;
	for (numsec = 1; numsec <= MAXSLEEP; numsec <<= 1) 
    {
		if (connect(sockfd, addr, alen) == 0)
        {
			return(0);
		}

		if (numsec <= MAXSLEEP/2)
			sleep(numsec);
	}
	return(-1);
}

/*
    描述 ： 初始化客户端　返回建立的套接字
*/
int init_client_system(void)
{
    struct sigaction act, oact;
    struct sockaddr_in addr = { 0 }; 
    
    addr.sin_family = AF_INET ;   
    sock_fd = socket(AF_INET, SOCK_STREAM, 0) ;
    if(-1 == sock_fd)
        return (-1) ;
    if(-1 == client_config(&addr))
        return (-1);
    if( -1 == connect_retry(sock_fd, (struct sockaddr * )&addr, sizeof( struct sockaddr_in)))
        return (-1) ;

    act.sa_handler = sig_alarm_handle ;
    act.sa_flags = SA_RESTART ;
    sigemptyset(&act.sa_mask) ;
    alarm(HEARTBEAT_PAUSE) ;
    sigaction(SIGALRM, &act, &oact) ;               //绑定SIGALRM 信号   以触发　心跳机制

    act.sa_handler = SIG_IGN ;                      //忽略SIGPIPE 信号　以免触发中断退出的效果
    sigaction(SIGPIPE, &act, &oact) ;

    return sock_fd ;
}

/*
    描述 ： 发送命令请求,
*/
int client_send_cmd(char * cmd)
{
    t_potol_type msg ;
    msg.msg_type = TYPE_CMD ;
    msg.msg_size = strlen(cmd) ;
    strcpy(msg.data, cmd) ;

    if( sizeof(t_potol_type) != send(sock_fd, &msg, sizeof(t_potol_type), 0))
        return (-1) ;

    client_state = STATE_CMD ;      //暂时入命令输入状态，待 返回结果时 回 STATE_LOGIN 状态
    return 0 ;
}

/*
    描述 : 登陆操作
        ０　：　 成功
       err　:    失败
*/
int client_login(char * name, char * passwd)
{
    t_potol_type msg ;
    t_user_desc user = {0} ;
    strcpy(user.nickname, name) ;
    strcpy(user.passwd, passwd) ;
        
    msg.msg_type = TYPE_LOGIN ;
    msg.msg_size = sizeof(t_user_desc) ;
    memcpy(msg.data, &user, sizeof(t_user_desc)) ;

    if( sizeof(t_potol_type) != send(sock_fd, &msg, sizeof(t_potol_type), 0))
        return (-1) ;
    
    client_state = STATE_LOGIN ;    // 暂时进入 STATE_LOGIN 状态， 返回结果后再选择进入 STATE_NORMAL / STATE_LOGIN
    return 0 ;
}

/*
    描述 : 客户端注册
           0    ：注册成功
        其它值 : 注册失败
*/
int client_regster(char * name , char * passwd)
{
     t_potol_type msg ;

    t_user_desc user = {0} ;
    strcpy(user.nickname, name) ;
    strcpy(user.passwd, passwd) ;
        
    msg.msg_type = TYPE_REG ;
    msg.msg_size = sizeof(t_user_desc) ;
    memcpy(msg.data, &user, sizeof(t_user_desc)) ;

    if( sizeof(t_potol_type) != send(sock_fd, &msg, sizeof(t_potol_type), 0)) 
        return (-1) ;
    client_state = STATE_REGISTER ;          // 暂时进入注册状态，待返回结果 进入 STATE_NORMAL / STATE_LOGIN 
    return 0 ;
}

/*
    描述 ： 整合聊天包并发送出去
*/
int client_send_msg_to(char * name, char *msg)
{
    t_potol_type dat ;
    t_user_desc user ;
 
    strcpy(user.nickname, name);   
    memcpy(dat.data, &user, sizeof(t_user_desc)) ;
    memcpy(dat.data+sizeof(t_user_desc), msg, strlen(msg)+1) ;
    
    dat.msg_size = sizeof(t_user_desc)+strlen(msg)+1;
    dat.msg_type = TYPE_CHAT ;
        
    if(sizeof(t_potol_type) != send(sock_fd, &dat,  sizeof(t_potol_type), 0))
        return (-1) ;

    client_state = STATE_CHAT ;
    return 0 ;
}

/*
    用户登出操作
*/
int client_logout( void )
{
    t_potol_type dat ;

    dat.msg_type = TYPE_EXIT ;    
    dat.msg_size = 0 ;

    client_state = STATE_CHAT ;   //回归到正常模式    
    if(sizeof(t_potol_type) != send(sock_fd, &dat, sizeof(t_potol_type), 0))
        return (-1) ;

    return 0 ;
}
