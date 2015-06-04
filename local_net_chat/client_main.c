#include <pthread.h>

#include <cdk.h>
#include <curses.h>

#include "user.h"
#include "protocol.h"
#include "client_op.h"
#include "client_ui.h"

#define BUFF_MAX    1024 
/*
    描述 : 客户处理接收数据线程
    详细 : 客户端根据状态机将不同 的接收数据分类并分开处理，
    也就是响应服务器的响应，如登陆模式，其它用户突然发现一
    消息，那么本客户端是要处理的
*/
static void * client_thread(void *args)
{
    t_potol_type    recv_buff = {0};
    
    while(1)
    {
        if(0 == recv(sock_fd, &recv_buff, sizeof(t_potol_type), 0))
        {
            client_state = STATE_UNCONNECT ;
            add_msg_widget("<L></B/24>Server's Connection Error :Unkwon");
            add_msg_widget("<C></B/2> RESTART PROGRAM IT, TRY AGAIN......");
            pthread_exit(NULL) ;
        }
        if(TYPE_CHAT == recv_buff.msg_type)
        {
                client_recv_msg(&recv_buff) ;
                continue ;
        }
        switch(client_state)
        {
            case STATE_REGISTER:                        //　注册模式 根据返回状态判断其状态
                client_state = STATE_NORMAL ;
                if(TYPE_OK != recv_buff.msg_type)
                {
                    err_log(*(int *) recv_buff.data) ;    
                    continue ;
                }
                add_msg_widget("<L></B/24>Register Success");
            break ;          
            case STATE_LOGIN:                           //　登陆模式 
                client_state = STATE_NORMAL ;
                if(TYPE_OK == recv_buff.msg_type)
                {
                    cleanCDKSwindow(msg_widget) ;
                    client_state = STATE_USER ;
                    continue ;
                }
                err_log(*(int *)recv_buff.data) ;
            break ;            
            case STATE_CHAT :                           // 聊天模式　获取聊天对方的登陆状态
                client_state = STATE_USER;
                if(TYPE_ERR == recv_buff.msg_type)
                {
                    err_log(*(int *)recv_buff.data) ;
                    set_user_status(OFF_LINE) ;
                    continue ;
                }
                set_user_status(ON_LINE) ;
            break ;
            case STATE_CMD:                             //　命令模式　命令结果接收值
                if(TYPE_MSG == recv_buff.msg_type)
                    add_msg_widget( recv_buff.data ) ;
                else if(TYPE_OK == recv_buff.msg_type)
                    cleanCDKMentry(chat_widget) ;
                else
                    client_state = STATE_USER ;
            break ;
            default :
            break;
        }
    }
    return NULL ;
}

static void send_msg(char * name)
{
    char * info ;
    char buff [BUFF_MAX] ; 
    while(1)
    {
            cleanCDKMentry(chat_widget) ;
            info = activateCDKMentry(chat_widget, NULL) ;
            if(vESCAPE_HIT == chat_widget->exitType)
            {
                client_state = STATE_USER ;
                return ;
            }
            if(BUFF_MAX-20 <= strlen(chat_widget->info))
                 continue ;
            memcpy(buff, "<R></B/32>", sizeof("<R></B/32>")) ;
            strcat(buff, info) ;
            strcat(buff, " < Me  ") ;
            while( 0 != client_send_msg_to(name, info)) ;
            sleep_us(5000);
            add_msg_widget( buff ) ;            
            sleep_us(5000);
   }
}

/*登陆后成功菜单项*/
static void second_menu(void)
{
    int opt ;
    char * info = NULL ;

    show_login_cmd() ;
    while(1)
    {
        opt = activateCDKScroll(cmd_widget, NULL);
        switch(opt)
        {
            case 0 :                                                //  日历
               activateCDKCalendar(calendar_widget, NULL) ;
            break ;
            case 1 :                                                //  查看日志
               activateCDKSwindow(msg_widget, NULL) ;
            break ;
            case 2 :                                                //  命令
                cleanCDKMentry(chat_widget);
                info = activateCDKMentry(chat_widget, NULL) ;
                client_send_cmd(info) ;  
            break ;
            case 3 :                                                // 聊天
                info = activateCDKEntry(name_widget, NULL) ;
                cleanCDKMentry(chat_widget) ;
                send_msg(info) ;
            break ;
            case 4 :                                               // 退出
                client_logout() ;
                client_state = STATE_NORMAL ;
                return ;
            break ;
            default :
            break ;
        }
    }
}

/*
    描述 ： 启始菜单
*/
static void first_menu(void)
{
    int opt ;
    char name_buff[ NICKNAME_MAX ] = {0};
    char pawd_buff[ PASSWD_MAX ] = {0} ;

    show_init_cmd( );
    opt = activateCDKScroll(cmd_widget, NULL);
    switch(opt)
    {
        case 0 :                                            // 日历
            activateCDKCalendar(calendar_widget, NULL) ;
        break;
        case 1 :                                            // 注册
            if(-1 != login_input(0, name_buff, pawd_buff))
                client_regster(name_buff, pawd_buff) ;             
        break;
        case 2 :                                            // 登陆
            if(-1 != login_input(1, name_buff, pawd_buff))
                if( -1 == client_login(name_buff, pawd_buff))
                    break ;
                opt = 0 ;
                for(opt=0; opt <=500; opt++)
                {
                    sleep_us(2000) ;                        //阻塞判断1秒种
                    if(STATE_LOGIN != client_state)
                        break ;
                }
                if(500 == opt || STATE_NORMAL == client_state)
                    break ;
                second_menu();
        break;
        case 3 :                                            // 退出
            close(sock_fd) ;
            exit_window_ui() ;
            exit(0) ;
        break;
        default :
        break;
    }
}

/*
        描述 : 创建客户端接收处理线程
*/
static void create_client_thread(void)
{
    pthread_t tid ;
    pthread_attr_t attr ;
    pthread_attr_init(&attr) ;
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) ;   //设置分离属性

    pthread_create(&tid ,&attr, client_thread, NULL);               //创建接收处理子线程
    
    pthread_attr_destroy(&attr) ;
}

/*
    描述： 主函数
*/
int main(int argc, char **argv)
{
    if(-1 == init_window_ui())          // 初始化界面
        return (-1) ;

    add_msg_widget( "<L></B/24>Try connect server......." ) ;
    if(-1 == init_client_system())      // 初始化网络服务
    {
        exit_window_ui() ;
        exit(0) ;
        return (-1) ;
    }
    
    add_msg_widget( "<L></B/32>Connect success " ) ;
    create_client_thread();            // 创建客户端子线程
    client_state = STATE_NORMAL ;
    while(1)                          // 主线程用于菜单管理操作 
    {
        first_menu() ;                  
    }
}
