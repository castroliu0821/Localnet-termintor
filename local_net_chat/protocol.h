#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#define DATA_MAX    1024

/*
*   注册返回连接错误码
*/
#define ERR_REPEAT      2               // 用户名重复
#define ERR_NAME_LNG    3               // 用户太长
#define ERR_NAME_SHR    4               // 用户名太短
#define ERR_PWD_LNG     5               // 密码太长
#define ERR_PWD_SHR     6               // 密码太短
#define ERR_NAME_NOEXIT 7               // 用户名不存在
#define ERR_PWD         8               // 用户密码不匹配
#define ERR_OFFLINE     9               // 用户不在线
#define ERR_REENTER     10              // 用户重复登陆
#define ERR_CONNECT     11              // 与服务器断开

/*
 *  描述 : 报文类型
 *  TYPE_LOGIN : 登陆类型
 *  TYPE_REG   : 注册类型
 *  TYPE_MSG   : 消息类型
 *  TYPE_HEART : 心跳类型
 *  TYPE_CMD   : 命令类型
 *  TYPE_CHAT  : 聊天类型
 *  TYPE_EXIT  : 退出类型
 *  TYPE_OK    : 确认类型
 *  TYPE_ERR  　: 错误类型
 * */
typedef enum _t_data_type{
                            TYPE_LOGIN = 0 ,
                            TYPE_REG ,
                            TYPE_MSG ,
                            TYPE_HEART ,
                            TYPE_CMD ,
                            TYPE_CHAT ,
                            TYPE_EXIT ,
                            TYPE_OK ,
                            TYPE_ERR 
                        } t_data_type ;

/*
 *  描述 : 协议类型
 *      msg_type : 报文种类
 *      msg_size : 消息长度
 *      data     : 消息缓冲区
 * */
typedef struct _t_potol_type {
    t_data_type msg_type ;
        int     msg_size ;
        char    data[DATA_MAX] ;
}t_potol_type ;

#endif
