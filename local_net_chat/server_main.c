#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <pthread.h>

#include "server_ui.h"
#include "server_op.h"
#include "protocol.h"
#include "user_op.h"
#include "log.h"

extern t_user_table * table ;

pthread_mutex_t  sockfd_mutex = PTHREAD_MUTEX_INITIALIZER ; //客户端请求网络描述符

struct client_info {
    int fd ;
    char ip[INET6_ADDRSTRLEN] ;
} ;

/*
    描述 :　线程取消清理函数
*/
static void clean_up(void * args) 
{
    t_user_desc * index ; 
                 
    index =table->header ;
    while(NULL != index)
    {
        if(0 != pthread_equal(index->tid, pthread_self()))
            break;
        index = index->next ;
    }
    log_quit(index->addr, index->run_time) ;            //  连接日志
    pthread_mutex_lock( &table_mutex ) ;
    del_user_op(table, index) ;
    pthread_mutex_unlock( &table_mutex ) ;
    close(((struct client_info *)args)->fd) ;           //　回收网络描述符
}

/*
    客户端处理线程
*/
void * client_thread_handle(void * args)
{
    int login_res = 1 ;
    struct client_info info = {0} ;
    t_user_desc  index = {0} ;
    t_potol_type recv_buf = {0}; 
    t_user_desc * index2 = NULL ;
    memcpy( &info, args, sizeof(struct client_info)) ;
    pthread_mutex_unlock(&sockfd_mutex) ;      //保护客户端连接描述符
    
    log_connect(info.ip);                     //记录连接日志
    
    pthread_cleanup_push(clean_up, &info) ;   //清理注册

    index.tid = pthread_self() ;              //　保存客户线程句柄
    strcpy(index.addr, info.ip) ;             //  连接进程的ip
    index.life_time = OFFTIME_SEC ;           //  设置脱机时间
    index.sock_fd = info.fd ;                 //　记录描述符
    index.status = OFF_LINE ;                 //  初始无用户登陆

    pthread_mutex_lock( &table_mutex ) ;
    add_user_op(table, &index) ;
    pthread_mutex_unlock( &table_mutex ) ;

    while(1)
    {
        memset(&recv_buf, 0, sizeof(t_potol_type)) ;
        if(0 == recv(info.fd, &recv_buf, sizeof(t_potol_type), 0))
            pthread_exit(NULL) ;  
        switch(recv_buf.msg_type)           //  数据包处理分析
        {
            case TYPE_CHAT :
                    user_chat_op(info.fd, &recv_buf);         
                break ;
            case TYPE_LOGIN :               //  用户登陆请求信息  
                    login_res = user_login_op(info.fd, &recv_buf) ;
                break ;
            case TYPE_REG :                 //  用户注册请求信息  
                    user_register_op(info.fd, &recv_buf) ;            
                break ;
            case TYPE_HEART :               //  客户端心跳信号处理信息
                    user_live_op(&recv_buf) ;
                break ;
            case TYPE_CMD :                 //  客户端登陆处理命令信息                            
                    if(0 == login_res )
                        user_cmd_op(info.fd, &recv_buf) ;
                break ;
            case TYPE_EXIT :                //  类型退出                    
                    index2 = table->header ;
                    while( NULL != index2)
                    {
                        if( 0 != pthread_equal(index2->tid, pthread_self()))
                            break ;
                        index2 = index2->next ;
                    }
                    log_logout(index2->nickname) ;
                    user_exit_op();
                    login_res = 1 ;         //  用户退出登录
                break ;
            default :
                break ;
        }
    }

    pthread_cleanup_pop(0) ;        //注册函数
    pthread_exit(NULL) ;
}

/*
        描述 : 创建服务线程 ，这里要注意的是该线程的属性  
        第一，该线程可以进行分离操作，便于回收资源
        第二, 该线程在一定情况下是可以被取消的 (如与客户端失去连接...)　系统会主动　取消该线程
*/
static void client_handle(void * args)
{
    int ostate ;
    pthread_t tid ;
    pthread_attr_t attr ; 
    pthread_attr_init( &attr ) ;
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);    //设置可以分享线程
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &ostate) ;        //设置可以取消线程
    pthread_create(&tid, &attr, client_thread_handle, args) ;       //创建　“客户端”　处理线程

    pthread_attr_destroy( &attr ) ;
}

/*      
       服务器的主函数  主线程　模块
 */
int main(int argc, char **argv)
{
    int fd ;
    struct sockaddr_in addr ;
    struct client_info info = {0} ;
    socklen_t   len = sizeof(struct sockaddr_in);
    
    if( -1 == init_window_ui())
        return (1) ;
    
    fd = init_server_op() ;                                    //　初始化服务器的操作
    if(-1 == fd)
    {
        endCDK();
        return (2) ;
    }

    while(1)
    {
        memset(&addr, 0, sizeof(struct sockaddr_in)) ;
        pthread_mutex_lock(&sockfd_mutex) ;
        info.fd = accept(fd, (struct sockaddr *)&addr, &len) ; //　监听网络端口请求并响应
        if(-1 == info.fd)
            continue ;   
        inet_ntop(AF_INET, &(addr.sin_addr.s_addr), info.ip, sizeof(struct sockaddr_in)) ;
        client_handle( &info ) ;
    }
}
