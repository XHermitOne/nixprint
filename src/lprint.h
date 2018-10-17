/**
* Модуль главных структур программы и основных запускающих функций
* @file
*/
#if !defined( __LPRINT_H )
#define __LPRINT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <malloc.h>
#include <getopt.h>
#include <cairo.h>

#include "ictypes.h"
#include "tools.h"
#include "lines.h"
#include "blocks.h"
#include "texts.h"
#include "epson.h"
#include "paper.h"
#include "convert.h"
#include "printer.h"
#include "version.h"

#if defined( _DEBUG )
#define _SHOW_BLOCKS
#endif


/**
* Режим отладки
*/
extern BOOL DBG_MODE;

enum PaperSide_t { sTop, sBotton, sLeft, sRight };

/**
* так как в win32 для win 9x бага в GDI функциях, то есть нельзя 
*   выводить в координаты больше 32768, то пришлось ввести коэффициент
*/   
//#define _DIVISER 8.0
#define _DIVISER 4

/**
* для определения внутренних размеров...1см==1000 в программе
*/
#define _UNIT 634
//#define _UNIT 1000
#define _METRIC(n) ((n)/(_DIVISER))

#define MAX_PARTS 100
const float INCH = 2.54*((float)_METRIC(_UNIT));

#define MAX_PAGES 9999

/**
* структура принетров
*/
typedef struct Printer_t
{
	LPTSTR pPrinterName;
	LPTSTR pPortName;
} Printer_t;

/**
* структура для registry
*/
typedef struct Options_t
{
	char CfgLoaded;                 /**< 1-параметры загружены из registry 0-no */
    char printerName[ MAX_PATH ];   /**< название принтера для печати в случае, если CfgLoaded == 1 */

    char FirstPageForPrint[255];	/**< страницы для печати... */
    
    /**
    * тип бумаги 
    *      0 User Define
    *      1 A4-portrait
    *      2 A4-landscape
    *      3 A3-fanfold
    */    
    WORD PaperType;			        
    WORD AskCorrectPaper;           /**< 0 - не спрашивать ничего, >0 кол-во странич после скольки спрашивать....*/
    WORD LineAtPage;		        /**< кол-во строк на листе, надо ли??? */
	char EpsonFastPrint;	        /**< 0 - Win, 1 - Epson compitable */
	char PaperFromArg;		        /**< 0 - взять из конфига бумажку, 1 - из строчки argv[..] */
	WORD CopiesForPrint;	        /**< кол-во копий... */

    DWORD AutoDetectFastmode;       /**< автоматически определять режим печати для принтера */
    DWORD ShowShriftWidth;          /**< показывать кол-во печатаемых символов в меню... */
    DWORD EjectLastSheet;           /**< выбрасывать последний лист при печати в FASTMODE */
    DWORD ShowDialog;               /**< показывать диалог подготовьте бумагу, вставьте следующий лист... */
	DWORD SaveSelectedPaper;        /**< сохранять бумажку, на которую последний раз печатали... */
	DWORD UseOwnShrift;		        /**< загружать собственные шрифты...1-да, 0-нет... */
} Options_t;

/**
* структура для основных параметров...
*/
typedef struct PrintParam_t  
{
    Options_t Options;              /**< опции из менюхи... */
    char *FileForPrint;		        /**< указатель на файл для печати. */
	char fontShortArc[ MAX_PATH ];	/**< полное имя файла для шрифтов "path\lprintw.dat" */
    char *OutputFile;		        /**< указатель на выходной PDF файл. */

    
    void *HPRV;                     /**< handle для окна просмотра... */
	void *PDC;				        /**< handle для принтера. */
    cairo_surface_t *CairoSurface;  /**< Поверхность Cairo (для вывода в PDF) */
    cairo_t *CairoEngine;           /**< Основной управляющий объект Cairo (для вывода в PDF) */
	Printer_t *PrinterForPrint;	    /**< принтер для печати... */
	struct Paper_t Paper;			/**< бумага, на который будем печатать... */
	void *pDevMode;		            /**< указатель на установки принтера.. */

	struct cache_t *DefFont;        /**< текущий фонт */
	struct cache_t *OldFont;        /**< фонт по умолчанию в контексте */
    
	struct EpsonFont_t EFont;		/**< указатель на текущий фонт при печати... */
	struct EpsonFont_t EFontOld;	/**< старенький удаленький :) */
	BOOL FontNeedUpdate;	        /**< 1-надо обновить фонт...CreateFont... */

	DWORD LinesInFile;		        /**< кол-во линий после загрузки см. LoadDoc. */

	BYTE PagesForPrint[MAX_PAGES];  /**< страницы которые печатать... */
	struct TextLine_t *Page;		/**< блок в котором будет храниться страница... */
	struct TextBlock_t *Doc;		/**< указатель на загруженный документ. */
	struct TextBlock_t *BlkCur;	    /**< указатель на текущий печатаемый блок */
	struct TextBlock_t *Sign;		/**< указатель на блок подписи */
	struct TextLine_t *LineCur;	    /**< указатель на текущию линию в блоке при печати... */

	WORD AutoShrift;		        /**< '^', 1-yes 0-no */
	
	WORD FieldWidth;                /**< ширина поля '}' */
	WORD NumParts;                  /**< кол-во частей... */
    
    /** 
    * длина части для режима '}'
    * [ 0] - длина до фигурной скобки...
    *   	== 0 режим не активен...
    * [>0] - длина частей без длины фигурной скобки... 
    */
	DWORD Parts[MAX_PARTS];         
    
    /** 
    * файла со шрифтами LPRINTW.DAT существует там же где и LPRINTW.EXE
    * TRUE  - да, существует
    * FALSE - нет, не существует, не возможно использовать собственные шрифты
    */                                    
	BOOL FileFontsExist;	        
	BOOL PrintCanceled;	            /**< Если FALSE, то можно печатать и всё работает. */
    BOOL isPrinting;                /**< TRUE если печатается. а непропускается страница... */
 
    /** 
    * TRUE  если мы находимся в режиме просмотра страниц...
    * FALSE если просто печатаем текст...
    */
    BOOL isPreview;                

	BOOL isExternalPreview;	        /**< TRUE -- просмотр через LIST.COM, выбрали View */

	WORD CurPage;			        /**< номер страницы обрабатываемой страницы... */
	WORD MaxPageForPrint;	        /**< максимальное кол-во страниц в документе... */
	WORD CurPageForPrint;	        /**< номер текущей печатаемой страницы */
    WORD CurPartForPrint;           /**< номер текущей печатаемой части... */

    BOOL AppendToCurPage;           /**< TRUE допечатать к текущему листу... иначе на новый... */
    BOOL AppendNextBlockToCurPage;  /**< TRUE допечатать следующий блок к текущему листу... иначе на новый... */
} PrintParam_t;

/**
* глобальная структура, HEART OF LPRINTW :)
*/
extern PrintParam_t PrintParam;

/**
* Процедура загрузки документа для печати...
*/
char LoadDoc(PrintParam_t *P );

/**
* Основная запускаемая процедура
*/
int LPrintMain(int argc, char *argv[]);

#endif //__LPRINT_H
