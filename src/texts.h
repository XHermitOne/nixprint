/**
* Модуль функций работы с языковым файлом LNG
* @file
*/

#if !defined( __TEXTS_DATA )
#define __TEXTS_DATA

#include "ictypes.h"

extern BYTE *DEF_LANG_FILE;

#endif //__TEXTS_DATA


#if !defined( __TEXTS_IDS )
#define __TEXTS_IDS

#define IDS_SELECT_PRINTER          1
#define IDS_SELECT_PAPER            2
#define IDS_PRINTER                 3
#define IDS_PAPER                   4
#define IDS_FIRST_PAGE_FOR_PRINT_I  5
#define IDS_NUM_COPIES              55  //!
#define IDS_FAST_PRINT_FOR_EPSON    6
#define IDS_FAST_PRINT_FOR_EPSON_AUTO 66 //!
#define IDS_BUTTON_PRINT            7
#define IDS_BUTTON_CANCEL           8
#define IDS_PARAMETER_OF_PRINT      9
#define IDS_PAPER_WIDTH             10
#define IDS_PAPER_HIGH              11
#define IDS_TITLE_PAPER_USER        12
#define IDS_BUTTON_OK               13
#define IDS_BUTTON_BORDER           14
#define IDS_BORDER_LEFT             15
#define IDS_BORDER_RIGHT            16
#define IDS_BORDER_TOP              17
#define IDS_BORDER_BOTTON           18
#define IDS_BORDER_TITLE            19
#define IDS_TOO_MANY_COPIES         20
#define IDS_BUTTON_EXTERNALVIEW     21
#define IDS_PRINTPAGE_ALL           22
#define IDS_PRINTPAGE_ODD           23
#define IDS_PRINTPAGE_EVEN          24 

/**
* По этим "хитрым строчкам" :[]~ определяется габариты "платежек"
*  список "хитрык строк" добавил
*/
#define IDS_DUP1                    50 /**< две хитрые строчки, для поиска "асчетны"  Расчетный листок */
#define IDS_DUP2                    51 /**< разрыва платёжек                "исто"  */
#define IDS_DUP3                    52 /**<  "ицевой" */
#define IDS_DUP4                    53 /**<  "счет" */
#define IDS_DUP5                    54 /**<  "СЧЕТ-КВИТАНЦ" */
#define IDS_DUP6                    55 /**<  ".Организац" */

#define IDS_MSG_WARNING             100
#define IDS_MSG_ERROR               101
#define IDS_MSG_INFORMATION         102
#define IDS_MSG_CONFIRMATION        103

#define IDS_BUTTON_OPTIONS          150
#define IDS_OPTIONS_TITLE           200
#define IDS_OPTIONS_COMMONTITLE     201
#define IDS_OPTIONS_CAUTO           202
#define IDS_OPTIONS_CSHOWSHRIFT     203
#define IDS_OPTIONS_CSAVEPAPER      204
#define IDS_OPTIONS_CUSEOWNSHRIFT   205
#define IDS_LPRINTW_DOESNT_FOUND    206

#define IDS_OPTIONS_FASTTITLE       211
#define IDS_OPTIONS_FLASTEJECT      212
#define IDS_OPTIONS_FSHOWDIALOG     213

#define IDS_BUTTON_SAVE             220

#define IDS_PAPER1                  500 //
#define IDS_PAPER2                  501 //
#define IDS_PAPER3                  502 //
#define IDS_PAPER4                  503 //
#define IDS_PAPER5                  504 //
#define IDS_PAPER6                  505 //
#define IDS_PAPER7                  506 /**< бумага определяемая пользователем */
#define IDS_PAPER8                  507 /**< бумага определяемая пользователем */
#define IDS_PAPERINFO1              508
#define IDS_PAPERINFO2              509
#define IDS_PAPERINFOANDREEV        510 /**< Андрееву нравится в одну строчку -) */
#define IDS_PAPER3_P                511 /**< portrait ориентация для бумаги А3 */

#define IDS_NO_PRINTER_INSTALLED    1000
#define IDS_SMALL_PAGE_NUMBER       1001
#define IDS_INCORRECT_WIDTH         1002
#define IDS_INCORRECT_HIGH          1003
#define IDS_INCORRECT_BORDERWIDTH   1004
#define IDS_INCORRECT_BORDERHIGH    1005
#define IDS_CORRECTPAPER            1006
#define IDS_CORRECTPAPERMSG1        1007
#define IDS_CORRECTPAPERMSG2        1008
#define IDS_CORRECTPAPERMSG3        1009
#define IDS_CORRECTPAPERMSG4        1010
#define IDS_CORRECTPAPERMSG5        1050
#define IDS_CORRECTPAPERMSG6        1060
#define IDS_INVALIDNUMCOPIES        1070

#define IDS_APPENDMSG1              1500
#define IDS_APPENDMSG2              1501
#define IDS_APPENDBUTTON1           1502
#define IDS_APPENDBUTTON2           1503

#define IDS_ITOGO_PO_LISTU          2000  

#define IDS_HELP1                   2500

#define IDS_PRINT_MSG               2600

#define IDS_ERROR                   2999 
#define IDS_ERROR_CANT_CREATE_DC    3001 /**< не вызывается CreateDC; */
#define IDS_ERROR_CANT_OPEN_FILE    3002
#define IDS_ERROR_REPORT_TOO_WIDE   3003

#define IDS_ERROR_TWO_HEADERS       3100
#define IDS_ERROR_INTERNAL          3100
#define IDS_ERROR_INTERNAL1         3101 // смотри для комментария сообщение выше
#define IDS_ERROR_INTERNAL2         3102 // смотри для комментария сообщение выше
#define IDS_ERROR_INTERNAL3         3103 // смотри для комментария сообщение выше

#define IDS_LOADTITLE               3500
#define IDS_ANALYZETITLE            3501

#define IDS_LOADDOC                 4000
#define IDS_ANALYZE_DOC             4001
#define IDS_PAGECOPY                4002
#define IDS_CHECKPAGE               4003
#define IDS_PAGEPRINT               4010
#define IDS_PAGESKIP                4011
#define IDS_PAGEPRINTPART           4012
#define IDS_PAGEPARTSKIP            4013

#endif //__TEXTS_IDS

#if !defined( __TEXTS_H )
#define __TEXTS_H

BOOL InitLanguage();
BOOL DoneLanguage();
char *GetStr( DWORD id );
char *GetHlp( DWORD id );

#endif //__TEXTS_H