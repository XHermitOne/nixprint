/**
* Модуль функций обработки бумагой
* @file
*/

#if !defined( __PAPER_H )
#define __PAPER_H

#include "texts.h"
#include "ictypes.h"

#define PORTRAIT_ORIENTATION_CODE   1
#define LANDSCAPE_ORIENTATION_CODE  2

typedef struct Paper_t
{
	DWORD BorderLeft;		/**< отступ на бумаги слева */
	DWORD BorderRight;		/**< справа */
	DWORD BorderTop;		/**< сверху  */
	DWORD BorderBotton;		/**< снизу... в tenth's mm */

    /**
    * ширина и высота в десятых миллиметра... tenths of mm
	* A4 == 2100х2970 для PORTRAIT 
	* A4 == 2970x2100 для LANDSCAPE
	* Значение W & H зависит от ориентации при печати...
    */
	DWORD W, H;				

	DWORD FreeHigh;			/**< свободно на странице... */

	DWORD Orientation;		/**< 1 - PORTRAIT, 2 - LANDSCAPE */
	DWORD LineInterval;		/**< интервал между строчками... целой для того, чтобы не работать с дробными числами.... */
	DWORD LineInPage;		/**< 0 */
	int ofsX, ofsY;			/**< отступ слева и сверху на который принтер физически не может печатать */
}Paper_t;

typedef struct papers_t
{
    int borders[4];     /**< 0-top 1-right 2-botton 3-left... */
    POINT size;         /**< размер бумажки ala 210x297 */
    DWORD mode[2];      /**< доступная бумага, индекс 0-для обычной печати 1-для быстрой... */
}papers_t;

#define MAX_PAPERS          8          /**< кол-во бумаги в меню... */

typedef int papersWidth_t[8][4];
typedef struct PapersWidth_t 
{
        papersWidth_t Epson;
        papersWidth_t Win;
} PapersWidth_t;

extern papers_t Papers[MAX_PAPERS];
extern PapersWidth_t PapersWidth;

#endif /*__PAPER_H*/

