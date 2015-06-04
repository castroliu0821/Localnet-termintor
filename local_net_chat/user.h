/*
 *  描述用户的信息，及用户链表的操作
 * */
#ifndef _USER_H_
#define _USER_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include <pthread.h>

#define NICKNAME_MAX 20
#define NICKNAME_MIN 7

#define PASSWD_MAX   20
#define PASSWD_MIN   7

#define UNKWON      2
#define ON_LINE     1 
#define OFF_LINE    0

//#ifndef _USER_OP_H_
//    #warning "using <user_op.h> instead <user.h>"
//#endif

/*
 *  描述 : 用户信息
 *  成员 :     
 *     ip_family : 用户成员的ip类型 AF_INET 或 AF_INET6 支持
 *          tid  : 在线用户的线程持有tid
 *     life_time : 用户心跳剩余时间
 *     run_time  : 用户在线时间
 *     passwd    : 用户名密码
 *     nickname  : 用户名呢称
 *     addr      : 用户的主机地址
 *     prev      : 上个用户节点
 *     next      : 下个用户节点
 *     TODO 添加其它属性
 * */
typedef struct _t_user_desc{
    int  ip_family ;
    pthread_t tid  ;
    int  life_time ;
    long run_time ;
    int  status ; 
    int  sock_fd ;
    char passwd [ PASSWD_MAX ];
    char nickname [ NICKNAME_MAX ] ;
    char addr[ INET6_ADDRSTRLEN ] ;
    struct _t_user_desc * prev ;
    struct _t_user_desc * next ;
}t_user_desc ;

/*
 *  描述 : 在线用户表
 *  成员 :
 *      cnt : 目前在线人数
 *   header : 用户头节点
 *
 * */
typedef struct _tuser_table{
    int cnt ;
    t_user_desc * header ;
}t_user_table ;

/*
 *  名称 : int init_user_table(t_user_table * table)
 *  描述 : 初始化用户表
 *  参数 :
 *      table :　传入指针地址
 *  返回 :
 *      0 成功
 *     -1 失败
 * */
int init_user_table(t_user_table ** table) ;

/*
 *  名称 : void destroy_table(t_user_table * table) ;
 *  描述 : 销毁用户表
 *  参数 : 
 *      无
 *  返回 : 
 *      无
 * */
void destroy_table(t_user_table ** table) ;

/*
 *  名称 : int add_table(t_user_table * table, t_user_desc * user) ;
 *  描述 : 添加用户到表内
 *  参数 :
 *      user :　用户指针
 *  返回 : 
 *      0 成功
 *     -1 失败
 * */
int add_table(t_user_table * table, t_user_desc * user) ;

/*
 *  名称 : int del_table(t_user_table *table ,t_user_desc * user) ;
 *  描述 : 删除用户从表
 *  参数 : 
 *      user : 用户指针
 *  返回 :
 *      0 成功
 *     -1 失败
 * */
int del_table(t_user_table *table ,t_user_desc * user) ;
#endif
