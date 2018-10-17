/**
* Модуль функций обработки ESC последовательностей
* @file
*/

#include "lprint.h"

#pragma warning( push )
#pragma warning( disable : 4244 4018 )

// описание в epson.h
EpsonCode_t EpsonCode[] = 
{
/**< Команды управления шрифтами...	*/
	{ "((F))9",		6, 	0 }, // выбор чернового/качественного шрифта
	{ "\033x?",		3, 	0 }, // ..... черновой элит
	{ "\033I?",		3, 	0 }, //  
	{ "\033k?",		3, 	0 }, // выбор типа печати...
	{ "((F))?",		6, 	0 }, // выбор курьер-0 сансериф-1
	{ "((I))1",     6, 	Italic			}, // выбор курсива...
	{ "((I))0",     6,  Italic | Off	}, // выбор вертикального(нормального) шрифта

	{ "\0334",		2, 	Italic			}, // выбор italic \n
	{ "\0335",		2, 	Italic | Off	}, // отмена italic

	{ "\033E",		2, 	Bold			},
	{ "\033F",		2, 	Bold | Off		},

	{ "\033G",		2, 	DoubleStrike	   }, // двойной удар...
	{ "\033H",		2, 	DoubleStrike | Off },

	{ "((B))1",		6, 	DoubleStrike	   },
	{ "((B))0",		6, 	DoubleStrike | Off },

	{ "\033-1",		3,  UnderLine },
	{ "\033-\x1",	3,  UnderLine },
	{ "((-))1",		6,  UnderLine },
  
	{ "\033-0",		3,  UnderLine | Off },
	{ "\033-\x0",	3,  UnderLine | Off },
	{ "((-))0",		6,  UnderLine | Off },

	{ "\033_1",		3,  AboveLine }, // подчёркивание над линией...
	{ "\033_\x1",	3,  AboveLine },

	{ "\033_0",		3,  AboveLine | Off},
	{ "\033_\x0",	3,  AboveLine | Off},

	{ "\033S0",		3,  SuperScript },
	{ "\033S\x0",	3,  SuperScript },
	
	{ "\033S1",		3,  SubScript },
	{ "\033S\x1",	3,  SubScript },

	{ "\033T",		2,  SuperScript | Off },
	{ "\033T",		2,  SubScript | Off },

/**< Команды выбора набора знаков... */
	{ "\033t0",		3, 0 }, // выбор стандартного набора
	{ "\033t\x0",	3, 0 },

	{ "\033t1",		3, 0 },	// выбор набора знаков ibm
	{ "\033t\x1",	3, 0 },

	{ "\0337",		2, 0 }, // выбор таблицы №1 
	{ "\0336",		2, 0 }, // выбор таблицы №2

	{ "\033[T\x4\x0\x0\x0??", 9, 0 }, // выбор страницы кода ibm

	{ "\033\\??",	4, 0 }, // разрешает печать всех знаков...
	{ "\033^?",		3, 0 }, // печатает токо один символ..	

	{ "\033~1",		3, 0 }, // ноль с косой чертой...
	{ "\033~\x1",	3, 0 },

	{ "\033~0",		3, 0 }, // ноль без черты..
	{ "\033~\x0",	3, 0 },
	
/**< Команды шага и размера знаков... */
	{ "\033P",		2, Pica },
	{ "\x12",		1, Condensed | Off },

	{ "\033M",		2, Elita },
	{ "\033:",		2, Elita },

	{ "\x0f",		1, Condensed },
	{ "\033\x0f",	2, Condensed },

	{ "\033W1",		3, DoubleSize },
	{ "\033W\x1",	3, DoubleSize },

	{ "\033W0",		3, DoubleSize | Off },
	{ "\033W\x0",	3, DoubleSize | Off },

	{ "\x0e",		1, DoubleSizeLine },
	{ "\033\x0e",	2, DoubleSizeLine },

	{ "\x14",		1, DoubleSizeLine | Off },

	{ "\033p1",		3, Propor },
	{ "\033p\x1",	3, Propor },
	
	{ "\033p0",		3, Propor | Off },
	{ "\033p\x0",	3, Propor | Off },

	{ "\033!?",		3, Privel | Off },

	{ "\033 ?",		3, 0 },

	{ "\033h?",		3, DQSize },

	{ "((S))?",		6, CharSize },

	{ "\033w1",		3, DoubleHigh },
	{ "\033w\1",	3, DoubleHigh },

	{ "\033w0",		3, DoubleHigh | Off },
	{ "\033w\0",	3, DoubleHigh | Off },

	{ "\033[@\4\0\0\0??", 9, 0 },

/**< Команды позиции по вертикали */
	{ "\0330",		2, Inter1_8in },
	{ "\0331",		2, Inter7_72in },
	{ "\0332",		2, Inter1_6in },

	{ "\0333?",		3, InterN_216in },
	{ "\033A?",		3, InterN_72in },
	{ "\0332",		2, InterMacro },

	{ "\033J?",		3, 0 }, // эти интервалы пропустим вперёд
	{ "\033j?",		3, 0 },	// ------------------------ назад	

	{ "\033f1?",	4, SkipNLine },
	{ "\033f\x1?",	4, SkipNLine },

	{ "\0334",		2, SetStartPage },

	{ "\033C\x0?",	4, SetLengthPageInInch },
	{ "\033C?",		3, SetLengthPageInLine },

	{ "\033c?",		3, SetUpMargin },
	{ "\033N?",		3, SetDownMargin },
	{ "\033O",		2, CancelUpDownMargin },	

	{ "\0338",		2, 0 },
	{ "\0339",		2, 0 },

	{ NULL, 0, 0 }
};


/**
* define для установки бита n в состояние 1
*/
#define L(n) (DWORD(1)<<(n-1))

/**
* массив определяет битиками матрицу которую использовать для рисования псевдографики
*
* Описывает символы в табличках:
* 0 - код символа 
* 
*	  1
*	  |
* 4 --+-- 2
* 	  |
*	  3
* Значение указывает какой может быть возле символ:
* 0 - не может быть, 1 - с одинарной, 2 с двойной, етк
*/
GrTableChar_t GrTableChars[ MAX_TABLE_CHARS ] = {
// все однинарные
	{{ '\xDA', 0, 1, 1, 0 }, L(20)|L(23)|L(8)|L(11) }, 
	{{ '\xC2', 0, 1, 1, 1 }, L(20)|L(23)|L(8)|L(11)|L(14)|L(17) },
	{{ '\xBF', 0, 0, 1, 1 }, L(14)|L(17)|L(8)|L(11) }, 
	 					    
	{{ '\xC3', 1, 1, 1, 0 }, L(2)|L(5)|L(20)|L(23)|L(8)|L(11)},
	{{ '\xC5', 1, 1, 1, 1 }, L(2)|L(5)|L(20)|L(23)|L(8)|L(11)|L(14)|L(17)},
	{{ '\xB4', 1, 0, 1, 1 }, L(2)|L(5)|L(8)|L(11)|L(14)|L(17)},
	 					    
	{{ '\xC0', 1, 1, 0, 0 }, L(2)|L(5)|L(20)|L(23)},
	{{ '\xC1', 1, 1, 0, 1 }, L(2)|L(5)|L(20)|L(23)|L(14)|L(17)},
	{{ '\xD9', 1, 0, 0, 1 }, L(2)|L(5)|L(14)|L(17)},
	 					    
	{{ '\xC4', 0, 1, 0, 1 }, L(14)|L(17)|L(20)|L(23)},	// -
	{{ '\xB3', 1, 0, 1, 0 }, L(2)|L(5)|L(8)|L(11)},		// |
	 					    
// все двойные			    
	{{ '\xC9', 0, 2, 2, 0 }, L(16)|L(19)|L(22)|L(4)|L(7)|L(10)|L(24)|L(12)},
	{{ '\xCB', 0, 2, 2, 2 }, L(13)|L(16)|L(19)|L(22)|L(15)|L(10)|L(12)|L(24)},
	{{ '\xBB', 0, 0, 2, 2 }, L(13)|L(16)|L(19)|L(6)|L(9)|L(12)|L(15)|L(10)},
	 					    
	{{ '\xCC', 2, 2, 2, 0 }, L(1)|L(4)|L(7)|L(10)|L(3)|L(22)|L(24)|L(12)},
	{{ '\xCE', 2, 2, 2, 2 }, L(1)|L(3)|L(13)|L(22)|L(15)|L(10)|L(24)|L(12)},
	{{ '\xB9', 2, 0, 2, 2 }, L(1)|L(3)|L(6)|L(9)|L(12)|L(10)|L(15)|L(13)},
	 					    
	{{ '\xC8', 2, 2, 0, 0 }, L(1)|L(4)|L(7)|L(18)|L(21)|L(24)|L(3)|L(22)},
	{{ '\xCA', 2, 2, 0, 2 }, L(1)|L(3)|L(13)|L(22)|L(15)|L(18)|L(21)|L(24)},
	{{ '\xBC', 2, 0, 0, 2 }, L(1)|L(3)|L(6)|L(9)|L(21)|L(18)|L(15)|L(13)},
	 					    
	{{ '\xCD', 0, 2, 0, 2 }, L(13)|L(16)|L(19)|L(22)|L(15)|L(18)|L(21)|L(24)}, // =
	{{ '\xBA', 2, 0, 2, 0 }, L(1)|L(3)|L(4)|L(7)|L(10)|L(3)|L(6)|L(9)|L(12)}, // ||
	 					    
// двойная верт, однойная гориз.
	{{ '\xD6', 0, 1, 2, 0 }, L(17)|L(20)|L(23)|L(7)|L(10)|L(9)|L(12)},
	{{ '\xD2', 0, 1, 2, 1 }, L(17)|L(20)|L(23)|L(7)|L(10)|L(9)|L(12)|L(14)},
	{{ '\xB7', 0, 0, 2, 1 }, L(17)|L(20)|L(7)|L(10)|L(9)|L(12)|L(14)},
	 					    
	{{ '\xC7', 2, 1, 2, 0 }, L(1)|L(3)|L(4)|L(7)|L(10)|L(3)|L(6)|L(9)|L(12)|L(23) },
	{{ '\xD7', 2, 1, 2, 1 }, L(1)|L(3)|L(4)|L(7)|L(10)|L(3)|L(6)|L(9)|L(12)|L(23)|L(14)},
	{{ '\xB6', 2, 0, 2, 1 }, L(1)|L(3)|L(4)|L(7)|L(10)|L(3)|L(6)|L(9)|L(12)|L(14)},
	 					    
	{{ '\xD3', 2, 1, 0, 0 }, L(1)|L(4)|L(17)|L(20)|L(3)|L(6)|L(23)},
	{{ '\xD0', 2, 1, 0, 1 }, L(1)|L(4)|L(17)|L(20)|L(3)|L(6)|L(23)|L(14)},
	{{ '\xBD', 2, 0, 0, 1 }, L(1)|L(4)|L(17)|L(20)|L(3)|L(6)|L(14)},
	 					    
// двойная гориз, однойная верт.
	{{ '\xD5', 0, 2, 1, 0 }, L(19)|L(5)|L(8)|L(11)|L(21)|L(22)|L(24)},
	{{ '\xD1', 0, 2, 1, 2 }, L(13)|L(16)|L(19)|L(22)|L(15)|L(18)|L(21)|L(24)|L(11)},
	{{ '\xB8', 0, 0, 1, 2 }, L(13)|L(16)|L(15)|L(18)|L(5)|L(8)|L(11)},
	 					    
	{{ '\xC6', 1, 2, 1, 0 }, L(2)|L(5)|L(8)|L(11)|L(19)|L(21)|L(22)|L(24)},
	{{ '\xD8', 1, 2, 1, 2 }, L(2)|L(11)|L(19)|L(21)|L(22)|L(24)|L(13)|L(15)|L(16)|L(18)},
	{{ '\xB5', 1, 0, 1, 2 }, L(2)|L(5)|L(8)|L(11)|L(16)|L(18)|L(13)|L(15)},
	 					    
	{{ '\xD4', 1, 2, 0, 0 }, L(2)|L(5)|L(8)|L(19)|L(21)|L(22)|L(24)},
	{{ '\xCF', 1, 2, 0, 2 }, L(2)|L(13)|L(16)|L(19)|L(22)|L(15)|L(18)|L(21)|L(24)},
	{{ '\xBE', 1, 0, 0, 2 }, L(2)|L(5)|L(8)|L(13)|L(16)|L(15)|L(18)},
};	 
	 

/**
*-- courier, описание метрики для шрифтов.
*/
Font_t Fonts[] = {	 
	//WORD W,			   H,			   dX,			   dY;
	{ _METRIC( 232 ), _METRIC( 428 ), _METRIC( 244 ), _METRIC( 428 ) }, // pica   
	{ _METRIC( 188 ), _METRIC( 428 ), _METRIC( 204 ), _METRIC( 428 ) }, // elita 
	{ _METRIC( 132 ), _METRIC( 428 ), _METRIC( 142 ), _METRIC( 428 ) }, // pica cond   
	{ _METRIC( 116 ), _METRIC( 428 ), _METRIC( 128 ), _METRIC( 428 ) }  // elita cond   //Dx was 126
};


/**
* межстрочные инт-лы
*/
// межстрочные инт-лы 1_8			  7_72			  1_6 
DWORD Intervals[] = { _METRIC( 318 ), _METRIC( 246 ), _METRIC( 428 ) };

/**
* символ конец страницы... alt(12)
*/
char FORMFEED[] = "\x0C";

/**
* Строчка инициализации по умолчанию для Epson-совместимых принтеров...
*  <-T добавленаа из за "багов" в EPSON FX-1170
*/
char DEF_INIT[] = "\0332\033P\x12\033T"; 

/**
* включение фонтов для епсон совместимых принтеров...
*/
char *DEF_FONTS[] = { "\x12\x1bP", "\x12\x1bM", "\x12\x1bP\x0f", "\x12\x1bM\x0f" };

/**
* Определение межстрочного интервала линии
*/
DWORD GetInterval(TextLine_t *L )
{
    if (DBG_MODE) LogAddLine("GetInterval function START");
    
	DWORD Interval = 0;

	if ( L && L->Font )
	{
		TextFont_t *F = L->Font;
		while ( F )
		{
			if ( F->Attr == Inter1_8in ) 
            {
				Interval = Intervals[ Inter1_8in - Inter1_8in ];
            }
			else 
            if ( F->Attr == Inter7_72in ) 
            {
				Interval = Intervals[ Inter7_72in - Inter1_8in ];
            }
			else 
            if ( F->Attr == Inter1_6in || F->Attr == InterMacro ) 
            {
				Interval = Intervals[ Inter1_6in - Inter1_8in ];
            }
			else 
            if ( ( F->Attr == InterN_216in || F->Attr == InterN_72in ) && F->DataLen ) 
			{
				DWORD Param = (BYTE)F->Data[ 0 ];
				if ( F->Attr == InterN_216in )
                {
					Interval = (INCH * (float)Param) / (float)216 + _METRIC(100);
                }
				else
                {
					Interval = (INCH * (float)Param) / (float)72 + _METRIC(100);
					//Interval = (INCH * (float)Param) / (float)36;
                }
			}
			F = F->Next;
		}
	}

    if (DBG_MODE) LogAddLine("Interval=%lu\tGetInterval function STOP",Interval);    
	return Interval;
}

/**
* инициализирует шрифт для печати...
* если autoShrift > 0 то устнавалвивает fonts[ autoShrift ]
* если есть '^'-autoshrift в документе, то устанавливаем его иначе просто пику... -)
*/
char InitDefEpsonFont(PrintParam_t *P, char autoShrift )
{
    if (DBG_MODE) LogAddLine("InitDefEpsonFont function START");
    
    if ( autoShrift )
    {
	    memset( &P->EFont, 0, sizeof( P->EFont ) );
	    memset( &P->EFontOld, 0, sizeof( P->EFontOld ) );
    }

	char AutoShrift = ( autoShrift ) ? autoShrift-1 : 0;

	for ( int i = 0; !autoShrift && P->AutoShrift && i < 4; i++ )
		if ( Fonts[ i ].dX == P->AutoShrift )
		{
			AutoShrift = i;
			break;
		};
	
	P->EFont.Pica =	( AutoShrift == 0 || AutoShrift == 2 ) ? TRUE : FALSE;
	P->EFont.Elita = ( AutoShrift == 1 || AutoShrift == 3 ) ? TRUE : FALSE;
	P->EFont.Condensed = ( AutoShrift > 1 ) ? TRUE : FALSE;
	P->EFont.W =	Fonts[ AutoShrift ].W;
	P->EFont.H = Fonts[ AutoShrift ].H;
	P->EFont.dX = Fonts[ AutoShrift ].dX;
	P->EFont.dY = Fonts[ AutoShrift ].dY;
	P->EFont.Weight = 300;
	P->EFont.DoubleHigh = FALSE;

    if ( autoShrift )
    {
	    P->EFont.x = P->EFont.y = 0;
	    P->EFont.Interval = Intervals[ 2 ];
    }

	P->EFont.IntervalN_72 = 0;
	P->FontNeedUpdate = TRUE;

    if (DBG_MODE) LogAddLine("AutoShrift=%i\tInitDefEpsonFont function STOP",(int)AutoShrift);
    
	return AutoShrift;
}


int oldInter=-1;

/**
* F указатель на фонт который анализируется if NULL, то анализируется attr
* смотри в printer.c... 
*/
BOOL AnalyzeFont(PrintParam_t *P, TextFont_t *F, DWORD attr )
{
	DWORD Param=0;
    float fInter=0.0;

    
    // если, NULL то хотим установить фонт ручками...иначе из F->Attr
    if ( F ) 
    {
        attr = F->Attr;
        if (DBG_MODE) LogAddLine("AnalyzeFont: Font Attr: %u ", F->Attr);
    }
    else
    {
        if (DBG_MODE) LogAddLine("AnalyzeFont: Font not define");
    }
    

	BOOL state = ( attr & Off ) ? FALSE : TRUE;
	DWORD Attr = attr; 
	Attr &= (~Off);
	switch ( Attr )
	{
		case SkipSeq:
			break;

		case Italic:
			P->EFont.Italic = state;
			break;

		case Bold:
		case DoubleStrike:
			P->EFont.Bold = state;
			break;

		case UnderLine:
			P->EFont.Underline = state;
			break;

		case AboveLine:
			break;

		case SuperScript:
			if ( state )
			{
				P->EFont.SupScript = TRUE;
				P->EFont.SubScript = FALSE;
			}
			else
			{
				P->EFont.SupScript = FALSE;
				P->EFont.SubScript = FALSE;
			}
			break;

		case SubScript:
			if ( state )
			{
				P->EFont.SubScript = TRUE;
				P->EFont.SupScript = FALSE;
			}
			else
			{
				P->EFont.SupScript = FALSE;
				P->EFont.SubScript = FALSE;
			}
			break;

		case Pica: 
			P->EFont.Pica  = state;
			P->EFont.Elita = !state;
			break;

		case Elita:
			P->EFont.Elita = state;
			P->EFont.Pica  = !state;
			break;

		case DoubleSize:
			P->EFont.DoubleSize = state;

            if ( !state && P->EFont.DoubleSizeLine )
                P->EFont.DoubleSizeLine = state;
			break;

		case DoubleSizeLine:
            P->EFont.DoubleSizeLine = state;
			break;

		case DoubleHigh:
			P->EFont.DoubleHigh = state;
			if ( state )
			{
				oldInter = P->EFont.Interval;
				P->EFont.Interval = P->EFont.dY+(P->EFont.dY*1/6);
			}
			else
			{
				P->EFont.Interval = oldInter;
				P->EFont.ofsY = 0;
			}
			break;

		case Inter1_8in:
			P->EFont.Interval = Intervals[ Inter1_8in - Inter1_8in ];
			break;

		case Inter7_72in:
			P->EFont.Interval = Intervals[ Inter7_72in - Inter1_8in ];
			break;

		case InterMacro:	// одно и тоже...
		case Inter1_6in:	// 
			P->EFont.Interval = Intervals[ Inter1_6in - Inter1_8in ];
			break;

		case InterN_216in:
			Param = (BYTE)F->Data[ 0 ];
            fInter = (Param * INCH) / 216.0 + _METRIC(100);
			P->EFont.Interval = fInter;
			break;

		case InterN_72in:
			Param = (BYTE)F->Data[ 0 ];
            fInter = (Param * INCH) / 72.0  + _METRIC(100);
			P->EFont.Interval = P->EFont.IntervalN_72 = fInter;
			break;

		case Condensed:
			P->EFont.Condensed = state;
			break;

		// эти последовательности никак не обрабатываются...
		case Propor:			
		case Privel:
		case DQSize:
		case CharSize:
		case SkipNLine:
		case SetLengthPageInLine:
		case SetLengthPageInInch:
		case SetUpMargin:
		case SetDownMargin:
		case CancelUpDownMargin:
		case SetStartPage:
			break;

		default:
			break;
	}

	if ( P->EFont.Pica )
	{
		WORD index = ( ( P->EFont.Condensed && !P->EFont.DoubleHigh ) ? 2 : 0 );
		P->EFont.W = Fonts[ index ].W;	P->EFont.H = Fonts[ index ].H;
		P->EFont.dX = Fonts[ index ].dX;	P->EFont.dY = Fonts[ index ].dY;
	}

	if ( P->EFont.Elita )
	{
		WORD index = ( ( P->EFont.Condensed  && !P->EFont.DoubleHigh ) ? 3 : 1 );
		P->EFont.W = Fonts[ index ].W; P->EFont.H = Fonts[ index ].H;
		P->EFont.dX = Fonts[ index ].dX;	P->EFont.dY = Fonts[ index ].dY;
	}

    if ( P->EFont.DoubleSize || P->EFont.DoubleSizeLine )
    {
        P->EFont.W  <<= 1;
        P->EFont.dX <<= 1;
    }

	if ( P->EFont.DoubleHigh ) // отменяет действие SupScript и SubScript
	{
		P->EFont.H  <<= 1;
		P->EFont.W  -= 11;
		P->EFont.ofsY = -(P->EFont.dY/2);
		P->EFont.dY = (P->EFont.dY<<1);
	}
	else
	{
		if ( P->EFont.SupScript )
		{
			P->EFont.H >>= 1; 
			P->EFont.ofsX = 0;				
			P->EFont.ofsY = 10;
		}

		if ( P->EFont.SubScript )
		{
			P->EFont.H >>= 1; 
			P->EFont.ofsX = 0;
			P->EFont.ofsY = (P->EFont.dY>>1)+10;
		}
	}

	P->EFont.Weight = ( P->EFont.Bold ) ? 750 : 300;

	P->FontNeedUpdate = ( P->FontNeedUpdate          || /* до сих пор не обновили... */
        P->EFont.H          != P->EFontOld.H         || 
        P->EFont.W          != P->EFontOld.W         ||
        P->EFont.dX         != P->EFontOld.dX        || /* new 3 line...теперь учитываем и ширину... */
        P->EFont.ofsX       != P->EFontOld.ofsX      || /*                                           */
        P->EFont.ofsY       != P->EFontOld.ofsY      || /*                                           */
        P->EFont.Interval   != P->EFontOld.Interval  ||
		P->EFont.Weight	    != P->EFontOld.Weight    || 
        P->EFont.Italic     != P->EFontOld.Italic    ||
		P->EFont.Underline  != P->EFontOld.Underline ||
		P->EFont.DoubleHigh != P->EFontOld.DoubleHigh ) ? TRUE : FALSE;

	return TRUE;
}

