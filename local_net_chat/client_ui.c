#include <ctype.h>
#include <cdk.h>
#include <curses.h>

#include <sys/select.h>

#include "user.h"
#include "protocol.h"
#include "pthread.h"

#define     BUFF_MAX 1024

pthread_mutex_t swin_mutex = PTHREAD_MUTEX_INITIALIZER;

WINDOW      * cursesWin	= 0 ;
CDKSCREEN   * cdkscreen	= 0 ;

CDKSWINDOW   * msg_widget =0 ;             //　聊天窗口控件
CDKSCROLL    * cmd_widget = 0 ;            //　命令控件 
CDKMENTRY    * chat_widget = 0 ;           // 字符内容控件
CDKENTRY     * name_widget = 0 ;           //　发送名称控件
CDKCALENDAR  * calendar_widget = 0 ;       //　日历窗口控件

static CDKLABEL     * status_widget = 0 ;  // 用户在线状态控件

char * init_item[4]= {
                    "</B/32> CALENDER " ,
                    "</B/32> REGISTER " ,
                    "</B/32>  LOGIN   " ,
                    "</B/32>   EXIT   " ,
                     };
char * login_item[5]= {
                    "</B/32> CALENDER  " ,
                    "</B/32> CHECKLOG  " ,
                    "</B/32>  COMMAND  " ,
                    "</B/32>  DIALOG   " ,
                    "</B/32>  LOGOUT   " ,
                     };
char * status_item[3]={
                    "</D/B/40>*****UNKWON****" ,
                    "</D/B/40>*****ONLINE****" ,
                    "</D/B/40>****OFFLINE****" ,
                     };

char * error_item[] ={
                    "</B/24>Register User Already  Exist" ,
                    "</B/24>Register Username Is Invalid" ,
                    "</B/24>Register Passwd Is Invalid" ,
                    "</B/24>User Is Not Exist" , 
                    "</B/24>Password Is Error" ,
                    "</B/24>Peer User Is Offline" ,
                    "</B/24>User Is Already Login" ,
                    "</B/24>UNCONECT SERVER" ,
                    "</B/24>ERROR UNKOWN"  ,                   
};


static int create_status( void );       //  创建状态标签
static int create_dialog( void ) ;      //　创建聊天窗口
static int create_swindow( void ) ;     //　聊天结果窗口
static int create_calendar( void ) ;    //　创建日历窗口
static int create_scroll( void ) ;      //　命令选择窗口

static BINDFN_PROTO (entryCB) ;         //　捆绑函数密码输入函数
static BINDFN_PROTO (waringCB) ;        //　用户输入检查函数

void sleep_us(int usec)
{
    struct timeval val ={0} ;
    val.tv_sec = usec/1000000 ;
    val.tv_usec = usec % 1000000 ;
    
    select(0 , NULL, NULL ,NULL, &val) ;
}

/*
    初始化窗口文件
*/
int init_window_ui( void )
{
    
    cursesWin = initscr ();
    cdkscreen = initCDKScreen (cursesWin);
    initCDKColor () ;
   
    if(-1 == create_calendar())
        return (-1) ;
    if(-1 == create_dialog())
        return (-1) ;
    if(-1 == create_scroll())
        return (-1) ;
    if(-1 == create_swindow())
        return (-1) ;

    return 0 ;
}

/*
    描述　: 退出ui时的操作
*/
void exit_window_ui( void )
{
    destroyCDKCalendar(calendar_widget) ;
    destroyCDKMentry(chat_widget) ;
    destroyCDKEntry(name_widget) ;
    destroyCDKSwindow(msg_widget) ;
    destroyCDKScroll(cmd_widget) ;
    destroyCDKEntry(status_widget) ;
    endCDK ();
    fflush (stdout);
}

static int create_status( void )
{
    status_widget = newCDKLabel( cdkscreen ,
                              LEFT, LINES-6 ,
                              (CDK_CSTRING2)status_item, 1 ,
                              TRUE, FALSE
                              );
    if(NULL == status_widget)
        return (-1);
    
    eraseCDKLabel(status_widget) ;
    return 0 ;
}

/*
    描述　: 建立日历控件
*/
static int create_calendar( void )
{
   struct tm *dateInfo;
   time_t clck;

   char *title = "<C></B/24> CALENDER";
   time (&clck);
   dateInfo	= gmtime (&clck);

   calendar_widget = newCDKCalendar (cdkscreen ,
                                     RIGHT, TOP ,
                                     title, dateInfo->tm_mday, 
                                     dateInfo->tm_mon+1, 
                                     dateInfo->tm_year+1900 ,
                                     COLOR_PAIR (16) | A_BOLD ,
                                     COLOR_PAIR (24) | A_BOLD ,
                                     COLOR_PAIR (32) | A_BOLD ,
                                     COLOR_PAIR (40) | A_REVERSE ,
                                     TRUE, FALSE );
   if (NULL == calendar_widget)
      return -1 ;

   calendar_widget->weekBase = 'w';
   drawCDKCalendar (calendar_widget, ObjOf (calendar_widget)->box); //立即显示日历窗口

   return 0;
}

/*
    描述 　:　创建结果显示窗口
*/
static int create_swindow(void)
{
   msg_widget = newCDKSwindow (cdkscreen,
                               LEFT , TOP ,
                               LINES-8, COLS-24 ,
        			           "<C></B/24> SYSTEM MESSAGE ", 1000,
                               TRUE ,
                               FALSE );
   if (msg_widget == 0)
        return (-1) ;

   drawCDKSwindow (msg_widget, ObjOf (msg_widget)->box);    // 立即显示消息框

   return 0 ;
}

/*
    描述　：　创建命令显示窗口
*/
static int create_scroll( void )
{
    char *title = "<C></B/24>OPTION" ;

    cmd_widget = newCDKScroll ( cdkscreen ,
                                RIGHT ,13 , RIGHT ,
                                LINES-12 , 23 ,
                                title, init_item, 4,
                                TRUE, A_REVERSE ,
                                TRUE, FALSE);       
    if(NULL == cmd_widget)
        return -1 ;
        
    drawCDKScroll(cmd_widget, TRUE) ;
    return 0 ;
}

/*
    描述 : 创建聊天输入窗口
*/
static int create_dialog(void)
{
   create_status();    
    chat_widget = newCDKMentry( cdkscreen,
                                17, BOTTOM ,
                                NULL, "</B/24>Send :", A_BOLD, '_',vMIXED,
                                COLS-49, 4, 10, 0,
                                TRUE, FALSE);
    if(NULL == chat_widget)
        return (-1) ;

    name_widget = newCDKEntry(cdkscreen ,
                               LEFT, BOTTOM,
                               NULL, "</B/24>Name :", A_NORMAL, '_', vMIXED ,
                               9, 4, 20, TRUE, FALSE );
    if(NULL == name_widget)
        return (-1) ;

    eraseCDKEntry(name_widget) ;
    eraseCDKMentry(chat_widget) ;
    return 0 ;
}

/*获取姓名及密码
    注意: Name 输入框绑定较多的按键回调，可能考虑不全需要在实践中找到有那些丢失内容
    另外姓名框内是包括检错非法字符的，只能使用大小字母，数字，及下划线 ,出错其它内容
    时会出现提示错误标签。
*/
    CDKBUTTONBOX * login_widget = 0 ;
    CDKENTRY     * passwd_widget = 0 ;
    CDKENTRY     * account_widget = 0 ;
int login_input(int opt, char *name, char * passwd)
{
    int res = 0 ;
    char * info = NULL;

    char * button[] = { " OK " ,
                        " Cancel "
                      } ;

    account_widget = newCDKEntry(cdkscreen ,            //创建　Ｎame　输入框
                               COLS/2-10, LINES/2-4,
                               NULL, "</B/24>Name :", A_NORMAL, ' ', vMIXED ,
                               15, 7, 20, TRUE, FALSE );

    passwd_widget = newCDKEntry(cdkscreen ,             //创建　Code 输入框
                               COLS/2-10, LINES/2-1,
                               NULL, "</B/24>Code :", A_NORMAL, ' ', vHMIXED ,
                               15, 6, 20, TRUE, FALSE );
    
    info = (opt ? "<C></B/24>Login" : "<C></B/24>Register") ;
    login_widget = newCDKButtonbox( cdkscreen ,         //创建　容器　包含 Name 及　Ｃode 输入框
                                    getbegx(account_widget->win)-2, 
                                    getbegy(account_widget->win)-3,
                                    8, account_widget->boxWidth+3 ,
                                    info, 1, 2 ,
                                    (CDK_CSTRING2)button, 2, A_REVERSE ,
                                    TRUE, FALSE);

    if(NULL == passwd_widget || NULL == account_widget || NULL == login_widget)
    {
        res =  -1 ;
        goto error ;
    }

    setCDKEntryPreProcess(account_widget, waringCB, 0) ;
    setCDKEntryHiddenChar(passwd_widget, '*') ;       //设置隐藏属性
    bindCDKObject(vENTRY, passwd_widget, KEY_TAB, entryCB, login_widget) ;

    drawCDKButtonbox(login_widget, TRUE);    
    drawCDKEntry(account_widget, TRUE);
    drawCDKEntry(passwd_widget, TRUE);

    info = activateCDKEntry(account_widget, NULL);  //操作姓名数据
    if(vNORMAL != account_widget->exitType)
    {
        res = -1 ;
        goto error ;
    }
    strcpy(name , info) ;
    
    info = activateCDKEntry(passwd_widget, NULL);   //操作密码数据
    if(vNORMAL != passwd_widget->exitType)
    {
        res = -1 ;
        goto error ;
    }
    if(0 != login_widget->currentButton)            //检测账号密码是否为确认
    {
        res = -1 ;
        goto error ;
    }   
    strcpy(passwd, info) ;
    res = 0 ;
error :
    destroyCDKEntry(account_widget);
    destroyCDKEntry(passwd_widget);
    destroyCDKButtonbox(login_widget);
    account_widget = NULL ;
    passwd_widget = NULL ; 
    login_widget = NULL ;
    return res ;
}

/*
    描述　: 检测账号输入的合法性
*/
static int waringCB(EObjectType cdktype GCC_UNUSED,
                void * object GCC_UNUSED ,
                void * clientData, chtype key )
{
      char *mesg ;
    if(   KEY_TAB == key   || KEY_ESC == key  ||
          KEY_UP == key    || KEY_DOWN == key ||
          KEY_RIGHT == key || KEY_LEFT == key ||      
          KEY_ENTER == key || KEY_BACKSPACE == key )
         return 1 ;
         
    if(!isalpha(key) && !isdigit(key) && '_'!=key)
     {
        mesg= "<C></B/2> ONLY ALPHA , DIGIT AND'_' SUPPORT !!!" ;
        popupLabel(cdkscreen, &mesg, 1) ;
        drawCDKButtonbox(login_widget, TRUE) ;        
        drawCDKEntry(passwd_widget, TRUE) ;
        drawCDKEntry(account_widget, TRUE) ;
        eraseCDKMentry(chat_widget) ;
        eraseCDKEntry(name_widget) ;
        eraseCDKLabel(status_widget) ;
        return 0 ;
     }
    return 1 ;
}

static int entryCB(EObjectType cdktype GCC_UNUSED,
                void * object GCC_UNUSED ,
                void * clientData, chtype key )
{
    CDKBUTTONBOX *buttonbox = (CDKBUTTONBOX *)clientData ;  //TAB 回调函数
    (void)injectCDKButtonbox( buttonbox, key) ;
    return (TRUE) ;
}

/*
    描述 ： 向消息控件内显示消息 
    TODO BUGS : SWINDOW 打印时出现乱码， 但是又没有出现其它提示
    性的错误。 经发现 在其前后各 添加 延时效果 其本可以解决这种问题
*/
void add_msg_widget(char * buff)
{
    sleep_us(2000) ;
    pthread_mutex_lock( &swin_mutex) ;
    addCDKSwindow(msg_widget, buff, BOTTOM) ;
    pthread_mutex_unlock( &swin_mutex) ;
    sleep_us(2000) ;
}

/*
    描述 ： 显示聊天对方的状态
*/
void set_user_status( int status)
{
    eraseCDKLabel(status_widget);
    switch(status)
    {
        case UNKWON :
            setCDKLabelMessage(status_widget, &status_item[0], 1) ;     //状态显示 未知
        break;
        case ON_LINE :
            setCDKLabelMessage(status_widget, &status_item[1], 1) ;     //状态显示 在线    
        break ;
        case OFF_LINE:
            setCDKLabelMessage(status_widget, &status_item[2], 1) ;     //状态显示 离线   
        break ;
        default:
        break;
    }
    drawCDKLabel(status_widget, TRUE) ;
}

void err_log(int err)
{
    int i = 0 ;
    switch (err)
    {
        case ERR_REPEAT :
            i=0 ;
        break ;
        case ERR_NAME_LNG :
        case ERR_NAME_SHR :
            i=1 ;
        break ;
        case ERR_PWD_LNG :
        case ERR_PWD_SHR :
            i=2 ;
        break ;
        case ERR_NAME_NOEXIT :
            i=3 ;
        break ;
        case ERR_PWD :
            i=4 ;
        break ;
        case ERR_OFFLINE :
            i=5 ;
        break ;
        case ERR_REENTER :
            i=6 ;
        break ;
        case ERR_CONNECT :
            i=7 ;
        break ;
        default :
            i=8 ;
        break ;
    }
            add_msg_widget( error_item[i] ) ;
}

int client_recv_msg( t_potol_type * dat) 
{
    char buff[ BUFF_MAX ];
    t_user_desc index = {0} ;
    
    memcpy(&index, dat->data, sizeof(t_user_desc)) ;
 
    memcpy(buff, "</B/24>", sizeof("</B/24>")) ;   
    strcat(buff, index.nickname) ;
    strcat(buff, " > ") ;
    strcat(buff, dat->data+sizeof(t_user_desc)) ;
    add_msg_widget( buff ) ;

    return 0 ;
}

/*
        描述 : 　将菜单设置为登陆前菜单
*/
int show_init_cmd( void )
{
    eraseCDKScroll(cmd_widget) ;
    setCDKScrollItems(cmd_widget, init_item, 4, TRUE);
    drawCDKScroll(cmd_widget, TRUE) ;
    eraseCDKLabel(status_widget) ;
    eraseCDKEntry(name_widget) ;
    eraseCDKMentry(chat_widget) ;
    return 0 ;
}

/*
        描述 : 　将菜单刷新为登录后的菜单
*/
int show_login_cmd( void )
{
    eraseCDKScroll(cmd_widget) ;
    setCDKScrollItems(cmd_widget, login_item, 5, TRUE);
    drawCDKScroll(cmd_widget, TRUE) ;
    drawCDKLabel(status_widget, TRUE) ;
    drawCDKEntry(name_widget, TRUE) ;
    drawCDKMentry(chat_widget, TRUE) ;
    return 0 ;
}
