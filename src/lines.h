/**
* Модуль функций обработки линий
* @file
*/

#if !defined( __ACTIONS )
#define __ACTIONS

#include "ictypes.h"

/** 
* описание акшинов при печати... 
*/
const DWORD aTXT		= 0;			/**< по-умолчанию */
const DWORD aNUM		= ( 1 << 0 );	/**< # нумерация страниц */
const DWORD aSKIP_NAME	= ( 1 << 1 );	/**< ! пропускать одинаковые имена */
const DWORD aTOTAL		= ( 1 << 2 );	/**< % добавить строку ИТОГО по листу:... */
const DWORD aHEAD_BEGIN	= ( 1 << 3 );	/**< ~ начало шапки */
const DWORD aHEAD_END	= ( 1 << 4 );	/**< @ конец шапки */
const DWORD aREPEAT		= ( 1 << 5 );	/**< } повторять левое поле... */
const DWORD aAUTOMODE	= ( 1 << 6 );	/**< ^ автоматический подбор шрифта... */
const DWORD aSUMCOL		= ( 1 << 7 );	/**< + суммировать колонки */
const DWORD aDUPLICATE	= ( 1 << 8 );	/**< & повторять документ */
const DWORD aDATETIME	= ( 1 << 9 );	/**< \ печатает дату и время */
const DWORD aFORMFEED   = ( 1 << 10 );  /**< символ конца страницы... */
const DWORD aFORMFEEDDOC= ( 1 << 11 );	/**< \f\f, конец документа... */
const DWORD aSIGNATURE	= ( 1 << 12 );	/**< линия из подписи к документу... */
const DWORD aPORTRAIT_ORIENTATION	= ( 1 << 13 );	/**< Портретная ориентация */
const DWORD aLANDSCAPE_ORIENTATION	= ( 1 << 14 );	/**< Ландшафтная ориентация */
//const DWORD aPAGE_SEGMENT	= ( 1 << 19 );	/**< Заполнение листа по ширине печати. При отключенной автоматичесской разбивке на страницы */

const DWORD aSKIP   	= ( 1 << 15 );	/**< будем обозначать напечатанные линнии... */
const DWORD aPRINT		= ( 1 << 16 );  /**< нужно напечатать, и не один раз */
const DWORD aPRINT_ONCE = ( 1 << 17 );  /**< нужно напечать, потом сразу удалить... */
const DWORD aTOTAL_PRO  = ( 1 << 18 );  /**<  % добавить строку ИТОГО по листу:...(Прописью) */

#endif //__ACTIONS

#if !defined( __LINES_H )
#define __LINES_H

#include <stdio.h>

#include "ictypes.h"

/**
* структура для описание Epson Esc последовательностей...
*/
typedef struct TextFont_t
{
	BOOL Skip;			        /**< TRUE-не учитывать, FALSE всё пучком, брать в расчёт */
	DWORD Attr;			        /**< тип шрифта, Pica, Elite, Condensed, etc... */
	DWORD Pos;	                /**< позиция в строке, где начинается. */
	WORD DataLen;		        /**< длина данных в Epson-ESC последовательности... */
	char *Data;			        /**< указатель на последовательность... */
	char *EpsonData;	        /**< указатель на оригинальную Esc последовательность в файле... */
    WORD EpsonDataLen;          /**< из Epson[].Len... */
	struct TextFont_t *Next;	/**< указатель на следующий фонт. */
} TextFont_t;

/**
* структуря для линий...
*/
typedef struct TextLine_t
{
    DWORD Attr;			        /**< тип линии: Skip, etc */
	DWORD Len;			        /**< кол-во символов в Data; */
	BYTE *Data;			        /**< указатель на строчку, БЕЗ фонтов */
	struct TextFont_t *Font;	/**< указатель на шрифты, какие установленны. */
	struct TextLine_t *Next;	/**< указатель на следующию строку. */
} TextLine_t;

/**
* структура для акшинов...
*/
typedef struct Action_t 
{
	char Ch;			    /**< символ для акшина */
	DWORD ID;			    /**< акшин */
	DWORD needAction;	    /**< какие акшины должны быть в файлике */
	DWORD MustBeInBlock;    /**< в каком блоке может быть */
	char Pos;			    /**< позиция в которой должен быть */
                            /**< =0 в любой */
                            /**< >0 только в этой */
                            /**< <0 начиная с abs(Pos) */
} Action_t;


#define MAX_ACTIONS 15

/**
* массив акшинов описан в lines.c
*/
extern struct Action_t Actions[];

/**
* функции для работы с линиями...
*/
BOOL LoadLine( FILE *in,TextLine_t *Line, int *FormFeed );

/**
* создаём структуру(ы) TextFont_t в линии Line 
*/
BOOL StripShrifts(TextLine_t *Line );

/**
* вырзаем символы псевд-графики, зачем они нам :) 
*/
WORD StripBadChars(TextLine_t *Line, char *BadChars );

/**
* удаляем линию..., высвобождаем память и всё такое
*/
BOOL DelLine(TextLine_t *Line );

/**
* дублировать фонт F
*/
TextFont_t *DupFont(TextFont_t *F );

/**
* дублировать линию L
*/
TextLine_t *DupLine(TextLine_t *L );

/**
* удалить Count символ, в строке S, в позиции Pos
*/
void DelChar( char *S, WORD Pos, WORD Count );

/**
* удалить все пробелы и символы табуляции
*/
void DelAllSpace( char *S );

/**
* удалить все хвостовые пробелы в линии
*/
void LineTrimRightSpace(TextLine_t *L );

/**
* дополнить справа строку пробелами до длины NewLen
*/
void PadRightSpace(TextLine_t *L, DWORD NewLen );

/**
* вставляет часть строчки с позиции SrcPos шириной SrcWidth в позицию DstPos
*/
void InsPartLine(TextLine_t *L, DWORD SrcPos, DWORD SrcWidth, DWORD DstPos );

/**
* Заменить символы = в Line на псевдографику
*/
BOOL DefinePseudograph(TextLine_t *Line);

#endif //__LINES_H