#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <arpa/inet.h>

#include "server.h"
#include "server_op.h"
#include "log.h"

pthread_mutex_t table_mutex = PTHREAD_MUTEX_INITIALIZER ;           //  互斥表锁
static pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER ;     //  文件锁

/*
 * 服务器操作表项
 */
t_user_table * table = NULL;

/*
        描述 ： 定时扫描在线客户端会话链表
                在线时间加1 ，心跳时间减 1
*/
void sigal_alarm_handle(int signal)
{
    t_user_desc * index = NULL ;
    t_user_desc * index2 = NULL ;

    pthread_mutex_lock( &table_mutex ) ;
    index = table->header ;

    while(NULL != index)
    {
        ++(index->run_time) ;                       // 用户时间 加 1

        index2 = index->next ;
        if(0 == --(index->life_time))               // 心跳时间 减 1
        {
            pthread_cancel(index->tid) ;                                  
            log_heartbeat_err(index->addr) ;           
        }
        index = index2 ;
    }
    pthread_mutex_unlock( &table_mutex ) ;
    alarm(SCAN_FEQ_SEC) ;
}

/*
        描述：　线程处理客户请求
*/
static int server_config(struct sockaddr_in * addr)
{
    char buff[ BUFF_SIZE ] ;
    int i = 0;    
    FILE * fl ;
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
*   描述 : 初始化服务器的操作
*/
int init_server_op(void)
{
    struct sockaddr_in addr = {0} ;
    struct sigaction act, oact;
    
    if( 0 != init_user_system(&table))          // 初始化服务　用户列表项 
        return -1 ;

    act.sa_handler = sigal_alarm_handle ;       // 闹钟信号句柄设置
    act.sa_flags = SA_RESTART ;
    sigemptyset(&act.sa_mask) ;
    if( 0 != sigaction(SIGALRM, &act, &oact))
        return -1 ;                            // 初始化　时间扫描失败
    
    act.sa_handler = SIG_IGN;
    if( 0 != sigaction(SIGPIPE, &act, &oact))   //　不要求响应管理结束
        return -1 ;

    alarm(SCAN_FEQ_SEC) ;

    addr.sin_family = AF_INET ;                  // 建立服务器连接
    if( 0 != server_config(&addr))
        return -1 ;
    return initserver(SOCK_STREAM, (struct sockaddr *) &addr, sizeof(struct sockaddr), CONN_MAX) ;
}


/******************************************
 *          用户操作函数，如请求链表操作
 ******************************************/
/*
        描述：注册用户到服务器管理列表文件中其中格式如下
        username1:passwd2
        username2:passwd2    
*/
#define BUFF_MAX 128
int user_register_op(int fd, t_potol_type * dat) 
{
    FILE * fl ;
    int flag = 0 ;
    t_user_desc index = {0};
    t_potol_type msg = {0} ;
    char buff[BUFF_MAX] = {0} ;
    
    if(sizeof(t_user_desc) == dat->msg_size)
        memcpy(&index, dat->data, sizeof(t_user_desc)) ;
    
    if(NICKNAME_MAX < strlen(index.nickname))                   // 检查用户名及密码规则
        flag = ERR_NAME_LNG ;        
    if(NICKNAME_MIN > strlen(index.nickname))
        flag = ERR_NAME_SHR ;
    if(PASSWD_MAX < strlen(index.passwd))
        flag = ERR_PWD_LNG ;
    if(PASSWD_MIN > strlen(index.passwd))
        flag = ERR_PWD_SHR ;

    pthread_mutex_lock( &file_mutex ) ;                         //　获取保存文件互斥锁
    fl = fopen(FILE_USER_SAVE, "r+") ;
    if(NULL == fl)
        return -1 ;    

    while(NULL != fgets(buff, BUFF_MAX, fl))
    {
        *(strchr(buff, ':')) = '\0' ;
        if(0 == strcmp(index.nickname, buff))
        {
            flag = ERR_REPEAT ;
            break;
        }
    }

    if(0 == flag)       
    {
        msg.msg_type = TYPE_OK ;
        sprintf(buff,"%s:%s\n", index.nickname, index.passwd) ;
        fseek( fl, ftell(fl), SEEK_SET ) ;                      //定位到文件结尾
        fputs(buff, fl) ;
    }
    else            //有重复命名
    {
        msg.msg_size = sizeof(int) ;    
        msg.msg_type = TYPE_ERR ;
        memcpy(msg.data, &flag, msg.msg_size) ;
    }
    fclose(fl) ;
    pthread_mutex_unlock( &file_mutex ) ;                       //释放保护文件互斥锁
    log_register(index.nickname, msg.msg_type) ;
    send(fd, &msg, sizeof(t_potol_type), 0) ;                   //返回响应管理用户表的结果
    return 0 ;
}

/*
        描述：　用户登陆函数
*/
int user_login_op(int fd, t_potol_type * dat)
{
    FILE * fl ;
    int  res = 0 ;
    char buff[ BUFF_MAX ] = {0};
    char pwd_buff[ PASSWD_MAX ] = {0} ;
    char name_buff[ NICKNAME_MAX ] = {0} ;

    t_user_desc index ;
    t_user_desc * index2 ;
    t_potol_type msg = {0} ;
    
    if(sizeof(t_user_desc) == dat->msg_size)
        memcpy(&index, dat->data, sizeof(t_user_desc)) ;
    pthread_mutex_lock( &file_mutex ) ;                  // 文件锁 锁住
    fl = fopen(FILE_USER_SAVE ,"r") ;
    while(NULL != fgets(buff, BUFF_MAX, fl))
    {
        sscanf(buff, "%[^:]:%s", name_buff, pwd_buff) ;
        if(0 == strcmp(name_buff, index.nickname))
        {
             if(0 == strcmp(pwd_buff, index.passwd))
                 res = 0 ;                               // 成功匹配
             else
                 res = ERR_PWD ;                         
             break ;                                     
        }
        res = ERR_NAME_NOEXIT ;
    }
    fclose( fl ) ;
    pthread_mutex_unlock( &file_mutex ) ;               // 文件锁 解锁

    index2 = table->header ;
    while(NULL != index2)
    {
        if((0 == strcmp(index2->nickname, name_buff))&& (ON_LINE==index2->status))
            break ;
        index2 = index2->next ;
    }
    if(NULL != index2)
        res = ERR_REENTER ;

    if(0 != res)                                        //  登陆错误
    {
        msg.msg_type = TYPE_ERR ;
        msg.msg_size = sizeof(int) ;
        memcpy(msg.data, &res, sizeof(int)) ;    
    }
    else
    {
        msg.msg_type = TYPE_OK ;                        //登陆成功，返回确认消息
        index2 = table->header ;
        while(NULL != index2)
        {
            if(0 != pthread_equal(pthread_self(), index2->tid))
                break ;
            index2 = index2->next ;
        }
        index2->status = ON_LINE ;        
        strcpy(index2->nickname, name_buff) ;           //拷贝登陆用户的名称到客户端 
    }

    send(fd, &msg, sizeof(t_potol_type), 0) ;
    log_login(index.nickname, msg.msg_type) ;
    
    return res ;
}

/*
    描述 ： 处理命令包
*/
int user_cmd_op(int fd, t_potol_type * dat)
{
    FILE * fl ;
    t_user_desc * index ;
    t_potol_type msg ;
    fl = popen(dat->data, "r") ;                    //  建立管道执行命令
    
    index =table->header ;
    while(NULL != index)
    {
        if(0 != pthread_equal(index->tid, pthread_self()))
            break;
        index = index->next ;
    }
    
    log_cmd(index->nickname, dat->data) ;           //记录结果
    
    if(NULL == fl)
    {
        msg.msg_type = TYPE_ERR ;
        send(fd, &msg, sizeof(t_potol_type), 0) ;
        return 0 ;
    }
    
    while(!feof(fl))
    {
        msg.msg_type = TYPE_MSG ;
        fgets(msg.data, BUFF_MAX, fl) ;
        msg.msg_size = strlen(msg.data) ;        
        *(msg.data+strlen(msg.data)) = '\0' ;  
        send(fd, &msg, sizeof(t_potol_type), 0) ;  // 发送完数据
    }

    msg.msg_type = TYPE_OK ;
    send(fd, &msg, sizeof(t_potol_type), 0) ;      // 发送完毕，返回确认
    
    pclose(fl) ;
    return 0 ;
}

/*
    描述 ： 客户端处理　心跳信息
*/
int user_live_op(t_potol_type * dat)
{
    t_user_desc * index = NULL ;    

    index = table->header ;
    while(NULL != index)
    {
        if(0 != pthread_equal(index->tid, pthread_self()))
            break ;
        index = index->next ;
    }
    if(NULL == index)
        return -1 ;

    index->life_time = OFFTIME_SEC ;   
    return 0 ;
}

/*
    描述 ： 处理聊天接收包再次转发 目的用户
    处理报文格式 :  报文格式 ＝ 目标用户描述 ＋ 字符串消息
*/
int user_chat_op(int fd, t_potol_type * dat)
{
    t_user_desc index ;
    t_user_desc * index2 ;
    t_user_desc * index3 ;    
    t_potol_type msg ={0};

    
    memcpy(&index, dat->data, sizeof(t_user_desc)) ;
    index3 = table->header ;
    while(NULL != index3)
    {
        if(0 != pthread_equal(index3->tid, pthread_self()))
            break ;
        index3 = index3->next ;
    }
    
    index2 = table->header ;
    while(NULL != index2)
    {
        if((0 == strcmp(index.nickname, index2->nickname)) && (ON_LINE == index2->status))
        {
            break ;
        }
        index2 = index2->next ;
    }
    if(NULL == index2)                              // 目的用户离线
    {
        msg.msg_type = TYPE_ERR ;
        msg.msg_size = sizeof(int) ;
        *(int *) msg.data = ERR_OFFLINE ;
        send(fd, &msg, sizeof(t_potol_type), 0) ;
        return (-1) ;
    }
    
    msg.msg_type = TYPE_CHAT ;                      // 目的用户在线
    msg.msg_size = dat->msg_size - sizeof(t_user_desc) ;
    memcpy(msg.data, index3, sizeof(t_user_desc)) ;
    memcpy(msg.data+sizeof(t_user_desc), dat->data+sizeof(t_user_desc), msg.msg_size) ;
    send(index2->sock_fd, &msg, sizeof(t_potol_type), 0) ;

    index2 = table->header ;
    while(NULL != index2)
    {
        if((0 != pthread_equal(index2->tid, pthread_self())))
            break ;
        index2 = index2->next ;
    }
    msg.msg_type = TYPE_OK ;
    send(fd, &msg, sizeof(t_potol_type), 0) ;
    log_send(index2->nickname, index.nickname) ;

    return 0 ;
}

/*
    描述 : 客户退出操作
*/
int user_exit_op( void )
{
    t_user_desc * index = NULL ;

    index = table->header ;
    while(NULL != index)
    {
        if((0 != pthread_equal(index->tid, pthread_self())))
            break ;
        index = index->next ;
    }
    if(NULL == index)
        return (-1) ;

    memset(index->nickname, 0, NICKNAME_MAX) ;
    index->status = OFF_LINE ;
    return 0 ;
}
