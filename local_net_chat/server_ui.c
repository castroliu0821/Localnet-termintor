#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <pthread.h>
#include <cdk.h>
#include <curses.h>
#include <signal.h>

#include "server_op.h"
#include "user_op.h"
#include "server_ui.h"
#include "log.h"

#define BUFF_MAX 1024

WINDOW      * cursesWin	= 0 ;
CDKSCREEN   * cdkscreen	= 0 ;
CDKSCROLL   * cmd_widget  = 0;
CDKCALENDAR * calendar_widget = 0 ;
CDKSWINDOW  * log_widget      = 0 ;

char * cmd_item[6]= { 
                      "</B/32> CALENDER " ,
                      "</B/32> LIVEUSER " ,
                      "</B/32> RESISTER " ,
                      "</B/32> CLEARSRC " ,
                      "</B/32> CHECKLOG " ,
                      "</B/32>   EXIT   " ,
                     };

static int create_calendar( void ) ;
static int create_swindow( void ) ;
static int create_radio( void ) ;

static void * thread_ui(void * args) ;
static void exit_handle(int signo) ;
static void exit_window_ui(void) ;
/*
    初始化窗口文件
*/
int init_window_ui( void )
{
    pthread_t tid ;
    pthread_attr_t attr ; 
    
    cursesWin = initscr ();
    cdkscreen = initCDKScreen (cursesWin);
    initCDKColor ();
 
    signal(SIGUSR1, exit_handle);
 
    if( 0 != create_calendar())      //　创建日历控件
         return (-1) ;
    if( 0 != create_swindow())       //　创建日志滑动显示文件
         return (-1) ;
    if( 0 != create_radio())
         return (-1) ;
    pthread_attr_init( &attr ) ;
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);    //设置可以分享线程
    pthread_create(&tid, &attr, thread_ui, NULL) ;       //创建　“客户端”　处理线程
    pthread_attr_destroy( &attr ) ;
    return 0 ;
}

static void exit_window_ui( void )
{
   destroyCDKCalendar (calendar_widget);
   destroyCDKSwindow (log_widget) ;
   destroyCDKScreen (cdkscreen);
   endCDK ();
   fflush (stdout);
}

static int create_calendar(void)
{
   struct tm *dateInfo;
   time_t clck;

   char *title = "<C></B/40>C A L E N D E R";
   time (&clck);
   dateInfo	= gmtime (&clck);

   calendar_widget = newCDKCalendar (cdkscreen ,
                RIGHT, TOP ,
			      title, dateInfo->tm_mday, dateInfo->tm_mon+1, dateInfo->tm_year+1900 ,
			      COLOR_PAIR (16) | A_BOLD ,
			      COLOR_PAIR (24) | A_BOLD ,
			      COLOR_PAIR (32) | A_BOLD ,
			      COLOR_PAIR (40) | A_REVERSE ,
			      TRUE ,
                FALSE );

   if (calendar_widget == 0)
      return -1 ;
   calendar_widget->weekBase = 'w';
   drawCDKCalendar (calendar_widget, ObjOf (calendar_widget)->box);

   return 0;
}

static int create_swindow(void)
{
   log_widget = newCDKSwindow (cdkscreen,
                               LEFT , TOP ,
                               LINES, COLS-25 ,
        			               "<C></B/40> S y s t e m  L o g ", 1000,
                               TRUE ,
                               FALSE );
   if (log_widget == 0)
        return (-1) ;
        
   drawCDKSwindow (log_widget, ObjOf (log_widget)->box);
    
   return 0 ;
}

static int create_radio( void )
{
    char *title = "<C></B/40>O P T I O N" ;
    
    cmd_widget = newCDKScroll ( cdkscreen ,
                               RIGHT ,13 , RIGHT ,
                               LINES-12 , 23,
                               title, cmd_item, 6,
                               TRUE, A_REVERSE ,
                               TRUE, FALSE);
    
    if(NULL == cmd_widget)
        return -1 ;
    return 0 ;
}

static void exit_handle(int signo)
{
    pthread_mutex_lock(&table_mutex) ;
    exit_user_system(&table) ;              //删除链表
    pthread_mutex_unlock(&table_mutex) ;    
    exit_window_ui();                       //退出界面系统
    exit(0) ;
}

static void * thread_ui(void * args)
{
    int selection ;
    while(1){       
        selection = activateCDKScroll(cmd_widget, NULL) ;
        switch(selection)
        {
        case 0 :                                            // 结束服务器
                activateCDKCalendar(calendar_widget, 0) ;
            break ;
        case 1 :                                            // 激活日历窗口
                live_user();                                // 显示在线用户
            break ;
        case 2 :                                            
                check_user();                               //查看已注册用户
            break ;
        case 3 : 
                clear_window() ;           
            break ;     
        case 4 : 
                activateCDKSwindow(log_widget, NULL) ;
            break ;     
        case 5 :
                raise(SIGUSR1) ;                            // 发送异步信号，标明结束当前服务器
                pthread_exit(NULL);
            break;
        default :
            break ;
        }        
   }
   return NULL ;
}

int check_user(void)
{
    char buff [BUFF_MAX] ;
    char name_buff [NICKNAME_MAX+1] ;
    FILE * fl ;
    int i = 0;
    fl = fopen(FILE_USER_SAVE, "r+") ;
    if(NULL == fl)
        return -1 ;    

    pthread_mutex_lock(&log_mutex) ;
    while(NULL != fgets(name_buff, BUFF_MAX, fl))
    {
        ++i ;
        *(strchr(name_buff, ':')) = '\0' ;
        sprintf(buff,"<C></B/24>%d:%-20s", i, name_buff) ;
        addCDKSwindow(log_widget, buff, BOTTOM);        
    }
    pthread_mutex_unlock(&log_mutex) ;    
    fclose(fl) ;

    activateCDKSwindow(log_widget, NULL) ;
    return 0 ;
}
void live_user(void)
{
    int i =0 ;
    char buff[BUFF_MAX] = {0} ;
    t_user_desc * index ;

    pthread_mutex_lock(&table_mutex) ;
    pthread_mutex_lock(&log_mutex) ;

    index = table->header ;
    while(NULL != index)
    {
        if(ON_LINE == index->status)
        {
            ++i;
            sprintf(buff, "<C></B/24>%d:%s--%s", i, index->nickname, index->addr);
        }
        addCDKSwindow(log_widget, buff, BOTTOM) ;
        index = index->next ;
    }
    if(0 == i)
        addCDKSwindow(log_widget, "<C></B/24>none user login", BOTTOM) ;    
    pthread_mutex_unlock(&log_mutex) ;
    pthread_mutex_unlock(&table_mutex) ;
    
    activateCDKSwindow(log_widget, NULL) ;
}

void clear_window(void)
{
    cleanCDKSwindow(log_widget) ; 
}
