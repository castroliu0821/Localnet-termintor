#ifndef _SERVER_H_
#define _SERVER_H_

#include <sys/socket.h>

/*
 *  名称 : int initserver(...) 
 *  描述 : 初始化服务器
 *  参数 :
 *      type : 数据报类型   SOCK_STREAM/SOCK_DGRAM
 *      addr : 服务器地址信息
 *      alen : 地址类型长
 *      qlen : 接收请求数
 *  返回 :
 *      ０　成功
 *      -1　失败
 * */
int initserver(int type, const struct sockaddr *addr, socklen_t alen, int qlen) ;

#endif
