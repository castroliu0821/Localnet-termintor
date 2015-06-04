#ifndef _USER_OP_H_
#define _USER_OP_H_

#include "user.h"

/*
 *  名称 : int init_user_system( void )
 *  描述 : 用户初始化操作
 *  参数 :
 *      user : 用户初始化参数
 *  返回 :
 *      ０ 成功
 *     -1  失败
 * */
int  init_user_system(t_user_table ** table) ;
/*
 *  名称 : void exit_user_system( void )
 *  描述 : 退出用户系统相应操作
 *  参数 : 
 *      无
 *  返回
 *      无
 * */
void exit_user_system(t_user_table ** table) ;

/*
 *  名称 : int add_user_op( t_user_desc * user)
 *  描述 : 添加用户
 *  参数 : 
 *      user : 添加用户指针
 *  返回 :
 *      0 成功
 *     -1 失败
 * */
int add_user_op(t_user_table * table, t_user_desc * user) ;

/*
 *  名称 : int del_user_op( t_user_table * table, t_user_desc * index)
 *  描述 : 根据用户地址来删除
 *  参数 : 
 *      addr : 地址指针
 * ip_family : 地址类型
 *  返回 : 
 *      1 错误的地址类型
 *      0 成功
 *     -1 失败
 * */
int del_user_op( t_user_table * table, t_user_desc * index);

/*
 *  名称 : int del_addr_user_op(t_user_table * table, char * nickname);
 *  描述 : 根据用户地址来删除
 *  参数 : 
 *      addr : 地址指针
 * ip_family : 地址类型
 *  返回 : 
 *      1 错误的地址类型
 *      0 成功
 *     -1 失败
 * */
int del_addr_user_op(t_user_table * table, char * nickname);

/*
 *  名称 : t_user_desc * find_user_op( char *addr ,int ip_family)
 *  描述 : 根据地址addr来查找用户
 *  参数 : 
 *      addr : 地址指针
 * ip_family : 地址类型
 *  返回 : 
 *      用户指针 成功
 *      NULL     失败
 * */
t_user_desc * find_user_op(t_user_table * table, char * nickname) ;

#endif
