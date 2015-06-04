#include <stdlib.h>
#include <assert.h>

#include "user.h"


/*初始化用户表*/
int init_user_table(t_user_table ** table)
{
    *table = (t_user_table *) calloc(1, sizeof(t_user_table)) ;
    if(NULL == *table)
        return (-1) ;
    (*table)->cnt = 0 ;
    (*table)->header = NULL;
    return (0);
}

/*销毁用户表*/
void destroy_table(t_user_table ** table) 
{
    assert( *table ) ;

    t_user_desc * index = NULL ;
    t_user_desc * index2 = NULL ;    

    index = (*table)->header ;
    while(NULL != index)
    {
        index2 = index->next ;
        free(index) ;
        index = index2 ;
    }

    free(*table) ;
    
    *table = NULL ;
}

/*添加用户表操作*/
int add_table(t_user_table * table, t_user_desc * user) 
{

    assert( user ) ;
    assert( table ) ;
    
    ++(table->cnt) ;   //在线用户加1

    if(NULL == table->header)
    {
        user->next = NULL ;
        user->prev = NULL ;
        table->header = user ;
        return 0;
    }

    //头插节点法
    user->next = table->header ;
    table->header->prev = user ;
    user->prev = NULL ;
    table->header = user ;
    return 0 ;
}

/*
 *  删除用户节点
 * */
int del_table(t_user_table *table ,t_user_desc * user) 
{
    assert( user ) ;
    assert( table ) ;

    t_user_desc * del_node = NULL ;

    del_node = table->header ;

    while(NULL != del_node)
    {
        if(user == del_node)
            break ;
        del_node = del_node->next ;
    }
    
    if(NULL == del_node)
    {
        return -1 ;
    }
    
    --(table->cnt) ; //在线用户减1
   
    if(del_node == table->header)
    {
        table->header = del_node->next ;   //删除头节点
    }
    else if(NULL == del_node->next)
    {
        del_node->prev->next = NULL ;           //尾节点
    }
    else
    {
        del_node->prev->next = del_node->next ;
        del_node->next->prev = del_node->prev ;
    }

    free(del_node) ;

    return 0 ;
}
