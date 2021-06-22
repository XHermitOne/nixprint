/**
* Модуль функций обработки блоков.
* @file
*/

#include <string.h>

#include "ictypes.h"
#include "lines.h"
#include "log.h"


#if !defined( __BLOCKS_TYPES )
#define __BLOCKS_TYPES
const DWORD bTEXT     = ( 1 << 20 );    /**< Тип блока: просто текст */
const DWORD bFIRST    = ( 1 << 21 );    /**< Тип блока: данные перед шапкой */
const DWORD bHEADER   = ( 1 << 22 );    /**< Тип блока: шапка */
const DWORD bDATA     = ( 1 << 23 );    /**< Тип блока: данные после шапки */
const DWORD bNEEDINIT  = ( 1 << 24 );   /**< Тип блока: нужно инизиализировать всё как для первой страницы...*/
const DWORD bSIGN     = ( 1 << 25 );    /**< Тип блока: блок подписи */
#endif //__BLOCKS_TYPES

#if !defined( __BLOCKS_H )
#define __BLOCKS_H

/**
* Cтруктура описание блока
*/

typedef struct TextBlock_t 
{
    DWORD Attr;                     /**< тип блока ( text, table, etc ) */
    DWORD HaveActions;              /**< какие акшины(see lines.h) */
                                    /**< присутсвуют в блоке... */
    DWORD LineInBlock;              /**< кол-во строчек в блоке */
    struct TextLine_t *BlockData;   /**< указатель на строчки текста */

    struct TextBlock_t *Next;       /**< указатель на следующий блок, 
                                    * если последний то NULL
                                    */
} TextBlock_t;

/**
* функции для работы с блоками
*/

/**
* создать новый блок, тип блока bTYPE
*/
TextBlock_t *BlkNew( DWORD bTYPE,TextBlock_t *ExistBlk );

/**
* добавить линию-Line к блоку-ExistBlk
*/
TextBlock_t *BlkAddLine(TextBlock_t *ExistBlk,TextLine_t *Line );

/**
* поиск блока с определённым типом...bTYPE начиная с Head
*/
TextBlock_t *BlkFind( DWORD bTYPE,TextBlock_t *Head );

/**
* поиск линии с Action в блоке Head
*/
TextLine_t *BlkActionFind( DWORD Action,TextBlock_t *Head );

/**
* поиск в цепочке линии Head акшина Action...
*/
TextLine_t *ActionFind( DWORD Action,TextLine_t *Head );

/**
* поиск максимальной по длине линии в блоке Head
*/
DWORD FindMaxLen(TextLine_t *Head );

/**
* установка акшина aTYPE для кол-вол линии Numlines в блоке ExistBlk..
*/
DWORD BlkSetLineID( DWORD aTYPE,TextBlock_t *ExistBlk, DWORD NumLines );
/**
* снятие акшина aTYPE для кол-вол линии Numlines в блоке ExistBlk..
*/
DWORD BlkClearLineID( DWORD aTYPE,TextBlock_t *ExistBlk, DWORD NumLines );

/**
* удалить линии в блоке...
*/
DWORD BlkDelLine(TextLine_t *L,TextBlock_t *ExistBlk );

/**
*DEBUG: Показывает информацию по блокам....через printf()...
*/
void ShowBlockType(TextBlock_t *B, BOOL LineInfoShow );

#endif //__BLOCKS_H
