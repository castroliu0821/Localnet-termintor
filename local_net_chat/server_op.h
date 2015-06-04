#ifndef _SERVER_OP_H_
#define _SERVER_OP_H_

#define FILE_SERVER_CONFIG     "server.cfg"    // 服务器配置文件
#define FILE_USER_SAVE  "server.usr"           // 服务器用户管理文件

#define BUFF_SIZE        1024           // 缓冲区最大长度
#define CONN_MAX         30             // 最大连接数
#define OFFTIME_SEC      8              // 脱机时间 　单位　秒
#define SCAN_FEQ_SEC     1              // 扫描用户表　频率　单位　秒

#include "protocol.h"
#include "user_op.h"
#include "pthread.h"

/*
        描述:　服务所操作的表项
*/
extern t_user_table * table ;
extern pthread_mutex_t table_mutex ;

int init_server_op(void) ;

/*
       描述 : 　处理用户请求
*/
int user_register_op(int fd, t_potol_type * dat) ;
/*
        描述：　用户登陆程式
        返回：
            1: 尚未登录成功
            0: 登录成功
*/
int user_login_op(int fd, t_potol_type * dat) ;

/*
        描述：　处理用户请求命令函数
        返回 :    结果值
            ０　成功
            -1   失败 
*/
int user_cmd_op(int fd, t_potol_type * dat) ;

/*
    描述 : 　处理客户端的心跳信号
    返回值
        成功
*/
int user_live_op(t_potol_type * dat) ;

/*
    描述: 　处理客户端的聊天请求信号
*/
int user_chat_op(int fd, t_potol_type * dat);

/*
    描述 : 客户退出操作
*/
int user_exit_op( void ) ;
#endif  // end of _SERVER_OP_H
