#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "user.h"
#include "user_op.h"

#include <sys/socket.h>
#include <arpa/inet.h>

/*
 *  用户系统初始化
 * */
int  init_user_system(t_user_table ** table)
{
    return init_user_table(table) ;
}

/*
 *  用户系统退出
 * */
void exit_user_system(t_user_table ** table)
{
    assert(*table) ;

    destroy_table(table) ;
    return ;
}

/*
 *  添加用户
 * */
int add_user_op(t_user_table * table, t_user_desc * user)
{
    assert( user ) ;
    assert( table ) ;

    t_user_desc * index = NULL ;   

    index = (t_user_desc *) calloc(1, sizeof(t_user_desc)) ;
    if(NULL == index)
        return -1 ;
    
    memcpy(index, user, sizeof(t_user_desc)) ;

    return add_table(table, index) ;

}

/*
 *  根据关键字删除用户
 * */
int del_addr_user_op(t_user_table * table, char * nickname)
{
    assert( table ) ;
    assert( nickname ) ;
 
    t_user_desc * index = 0 ;
        
    index = find_user_op(table, nickname); 
    
    if(NULL == index)   //没有找ip源地址
        return -1 ;
    
    return del_table(table, index) ;
}

/*
    从表内删除节点
*/
int del_user_op( t_user_table * table, t_user_desc * index)
{
    return del_table(table , index) ;
}

/*
 *  根据地址来查找用户
 * */
t_user_desc * find_user_op(t_user_table * table, char * nickname)
{
    assert( nickname ) ;
    assert( table ) ;

    t_user_desc * index = NULL ;

    index = table->header ; 
    while(NULL != index)
    {
        if(0 == strcmp(nickname, index->nickname))    break ;
        index = index->next ;
    }   
    return index ;
}
