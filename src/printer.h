/**
* Модуль функций печати сформированной страницы
* @file
*/

#if !defined( __PRINTER_H )
#define __PRINTER_H

#include "lprint.h"
#include "log.h"
#include "tools.h"
#include "parser.h"

#define PDF_PRINT_FILENAME ".nixprint/print.pdf"

//#define _DIV_FONT_SIZE 0.6
//#define _DIV_FONT_SIZE 1

#define DEVICE_OFFSET_X 30.0
#define DEVICE_OFFSET_Y 50.0

typedef struct printer_t
{
    char fastPrint;     /**< быстрая печать...1-yes, 0-no \ когда нажали кнопку "Печать"... */
    int defaultPaper;   /**< выбранная бумага в последний раз */
}printer_t;

/**
* для буфферезации FONT HANDLEs ...      createfont see CacheCreatefont...
*/
typedef struct cache_t
{
    int H, W, Weight;
    DWORD Italic;
} cache_t;

int SetFont(PrintParam_t *P,struct TextFont_t *F,DWORD p);
DWORD GetPaperFreeHigh(PrintParam_t *P);
DWORD GetPaperFreeWidth(PrintParam_t *P);
BOOL SetMapModeForPrinter(PrintParam_t *P);
BOOL StartPrint(PrintParam_t *P);
BOOL EndPrint(PrintParam_t *P);
BOOL PagePrint(PrintParam_t *P,BOOL isLastPage,BOOL NeedInit);
void PrintChar(PrintParam_t *P,char *s,int count);
void PrintNewLine(PrintParam_t *P);
BOOL PrintDoc(PrintParam_t *P );
int InitPages( PrintParam_t *P, char *data, char *strPage, int pageParity);
void SetOrientationPage(PrintParam_t *P, DWORD orientation);

#endif /*__PRINTER_H*/
