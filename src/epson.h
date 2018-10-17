/**
* Модуль функций обработки ESC последовательностей
* @file
*/
#if !defined( __EPSON_CONST )
#define __EPSON_CONST

const DWORD SkipSeq				= 0;

const DWORD Italic				= 1;
const DWORD Bold				= 2;
const DWORD DoubleStrike		= 3;
const DWORD UnderLine			= 4;
const DWORD AboveLine			= 5;
const DWORD SuperScript			= 6;
const DWORD SubScript			= 7;
const DWORD Pica				= 8;
const DWORD Elita				= 9;
const DWORD	DoubleSize			= 10;
const DWORD DoubleSizeLine		= 11;
const DWORD Propor				= 12; // пока пофиг
const DWORD Privel				= 13; // пока пофиг
const DWORD DQSize				= 14; // пока пофиг
const DWORD CharSize			= 15; // пока пофиг
const DWORD DoubleHigh			= 16;
const DWORD Inter1_8in			= 17;
const DWORD Inter7_72in			= 18;
const DWORD Inter1_6in			= 19;
const DWORD InterN_216in		= 20;
const DWORD InterN_72in			= 21;
const DWORD InterMacro			= 22;
const DWORD SkipNLine			= 23;
const DWORD SetLengthPageInLine = 24;
const DWORD SetLengthPageInInch = 25;
const DWORD SetUpMargin			= 26;
const DWORD SetDownMargin		= 27;
const DWORD CancelUpDownMargin	= 28;
const DWORD Condensed			= 29;
const DWORD SetStartPage		= 30;

const DWORD Off					= ((DWORD)1<<31);

#endif //__EPSON_CONST

#if !defined( __EPSON_H )
#define __EPSON_H

#include "ictypes.h"
#include "lprint.h"

#define _INTERVAL_COEFF 0.5 /**< Для подбора межстрочного интервала */

/**
* Структура для объявления всех esc-последовательностей
*/
typedef struct EpsonCode_t
{
	char *Seq;			/**< ESC последовательность */
	WORD Len;			/**< длина последовательности */
	DWORD Alias;		/**< ID для шрифта, 0-нет. не обрабатывается lprintом а просто вырезается */
} EpsonCode_t;

/**
* Структура для фонтов...
*/
typedef struct Font_t {	WORD W, H, dX, dY; } Font_t;
extern Font_t Fonts[];

/**
* Структура для символов псевдографики...
*/
typedef struct GrTableChar_t
{
	char Matrix[ 5 ];
	DWORD Lines;
} GrTableChar_t;

/**
* ссылка на стандартную. таблицу epson кодов 
*/
extern EpsonCode_t EpsonCode[];

#define MAX_TABLE_CHARS 40
extern GrTableChar_t GrTableChars[MAX_TABLE_CHARS];

/**
* Структура для состояние фонта во времени печати
*/
typedef struct EpsonFont_t
{
	BOOL Pica, Elita;
	BOOL SupScript, SubScript;
	BOOL DoubleSize;
    BOOL DoubleSizeLine;
	BOOL Condensed;
	BOOL Italic;
	BOOL Bold;		        /**< тоже самое что и Doublestrike*/
	BOOL Underline;
	BOOL DoubleHigh;        /**< двойная высота шрифта... */

	int W, H, dX, dY, Weight;
	int ofsX, ofsY;         /**< смещение для sub & sup script */

	int x, y;		        /**< coordinates current output position */
	DWORD Interval, IntervalN_72;	/**< межстрочный интервал... */
} EpsonFont_t;

extern char FORMFEED[];		/**< сивол перевода страницы для епсона */
extern char DEF_INIT[];		/**< строка инициализации по умолчанию */
extern char *DEF_FONTS[];	/**< фонты: Пика, Элит, Пика Конд, Элит Конд... */

char InitDefEpsonFont(struct PrintParam_t *P, char autoShrift);
DWORD GetInterval(TextLine_t *L );
BOOL AnalyzeFont(struct PrintParam_t *P,TextFont_t *F,DWORD attr);

#endif //__EPSON_H

