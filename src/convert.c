/**
* Модуль отрисовки таблиц линиями вместо набора псевдографических символов.
* В общем для конвертации из псевдо-графики в линии...
* @file
*/

#include "convert.h"
#include <vector>
#include <cairo.h>

struct gChar_t
{
	int LineNum;		/**< номер строчки в которой встретился символ */
	long X, Y;			/**< координаты символа во время печати */
	int DX, DY;			/**< ширина, высота символа */
	char Matrix[5];		/**< 0 - символ в таблице...смотри epson.c */
	bool InLine;		/**< занят в линии символ? */
	DWORD ShortLines;	/**< битовый массив линий из которых состои символ псевдо графики */
};

typedef std::vector<gChar_t *> gCharVector;

struct gLine_t
{
	gChar_t *CharOwner;		/**<  из какого графического символа линия */
	int		BID, EID;		/**<  номер строки начала и конца линии */
	int		BX, BY;			/**<  начало линии... */
	int		EX, EY;			/**<  конец линии... */
	char	Pen;			/**<  перо для линии... */
};

typedef std::vector<gLine_t *> gLineVector;

class ConvertLines
{
	PrintParam_t	*param;		/**< указатель на параметры */
	WORD			partNum;	/**< какая часть печатается */

	EpsonFont_t SavedEpsonFont;	/**< здесь сохраним фонт */
	Paper_t		SavedPaper;		/**< здесь сохраним бумажку */

	gCharVector		charVector;	/**< массив символов псевдографики */
	gLineVector		lineVector;	/**< массив линий */

public:

	/**
    * конструктор
    */
	ConvertLines( PrintParam_t *p, WORD partNumber )
	{
		param	= p;
		partNum = partNumber;

		// запомним фонт и бумага
		memmove( &SavedEpsonFont, &param->EFont, sizeof( SavedEpsonFont ) );
		memmove( &SavedPaper, &param->Paper, sizeof( SavedPaper ) );
	}

	/**
    * деструктор
    */
	~ConvertLines()
	{
		// востановим старый фонт и бумагу, после эмулятора
		memmove( &param->EFont, &SavedEpsonFont, sizeof( SavedEpsonFont ) );
		memmove( &param->Paper, &SavedPaper, sizeof( SavedPaper ) );
		param->FontNeedUpdate = 1;

		// очистим все символы...
		for ( gCharVector::iterator CharIterator = charVector.begin(); CharIterator != charVector.end(); CharIterator++ )
			delete *CharIterator;
		charVector.clear();

		// очистим все линии...
		for ( gLineVector::iterator LineIterator = lineVector.begin(); LineIterator != lineVector.end(); LineIterator++ )
			delete *LineIterator;
		lineVector.clear();
	}


	bool charAdd( char ch, long x, long y, int dx, int dy, int lineNum )
	{
		bool added = false;
		for ( int i = 0; i < MAX_TABLE_CHARS; i++ )
		{
			if ( GrTableChars[i].Matrix[ 0 ] == ch )
			{
				gChar_t *n = new gChar_t;
				if ( n == NULL )
					return false;

				n->X			= x;
				n->Y			= y;
				n->DX			= dx;
				n->DY			= dy;
				n->LineNum		= lineNum;
				n->ShortLines	= GrTableChars[ i ].Lines;
				memcpy( n->Matrix, GrTableChars[ i ].Matrix, sizeof(char) * 5 );
				charVector.push_back( n );
				added = true;
				break;
			}
		}

		return added;
	}

	bool lineAdd( int bx, int by, 
		  		  int ex, int ey, 
				  int bID, int eID, 
				  gChar_t *charOwner )
	{
		for ( gLineVector::iterator i = lineVector.begin(); i != lineVector.end(); i++ )
		{
			if (by==ey) // add to hor line
			{
				if ( abs( (*i)->EX - bx ) <= 50 && (*i)->BY == by && (*i)->EY == ey )
				{
					(*i)->EX = ex;
					return true;
				}
			}
			else
			if (bx==ex) // add to ver line		 //TODO:DEPEND from 1000
			{
				if ( abs( (*i)->EY - by ) <= 200 && 
					 (*i)->EID+1 == bID &&
					 (*i)->BX == bx && 
					 (*i)->EX == ex &&
					 (*i)->CharOwner->Matrix[3]>0 && 
					 (*i)->CharOwner->Matrix[3] == charOwner->Matrix[1] )
				{
					(*i)->EID = bID;
					(*i)->EY = ey;
					return true;
				}
            
			}
		}

		// не нашли совпадающию? значит добавим в массив
		gLine_t *l = new gLine_t;
		if ( l == NULL )
			return false;

		l->BX  = bx;
		l->BY  = by;
		l->EX  = ex;
		l->EY  = ey;
		l->BID = bID;
		l->EID = eID;
		l->CharOwner = charOwner;
		l->Pen = 1;
		lineVector.push_back( l );

		return true;
	}

	bool convertCharToLine()
	{
		if ( charVector.empty() )
			return false;

		//--- конвертнём каждый символ псевдо графики в набор маленьких линий...
		for ( gCharVector::iterator p = charVector.begin(); p != charVector.end(); p++ )
		{
			gChar_t *C = *p;
			int x = C->X;						// начало символа во внутренних координатах...
			int y = C->Y;
			int w = C->DX;						// ширина символа
			int h = C->DY;						// высота символа
			int xc = x+(w>>1);					// центр Х
			int yc = y+(h>>1);					// центр Y

			//int o = _METRIC( 35 );              // смещение от центра...
			int o = _METRIC( 100 );              // смещение от центра...

			struct Line_t { 
				long bx, by, ex, ey; 
			} Lines[] = {
				// вертикальные линии...
				{ xc-o, y, xc-o, yc-o },
				{ xc,	y, xc,	 yc-o },
				{ xc+o, y, xc+o, yc-o },

				{ xc-o, yc-o, xc-o, yc },
				{ xc,	yc-o, xc,	yc },
				{ xc+o, yc-o, xc+o, yc },

				{ xc-o, yc, xc-o, yc+o },
				{ xc,	yc, xc,	  yc+o },
				{ xc+o, yc, xc+o, yc+o },

				{ xc-o, yc+o, xc-o, y+h },
				{ xc,	yc+o, xc,	y+h },
				{ xc+o, yc+o, xc+o, y+h },

				//горизонтальные...
				{ x,	yc-o,	xc-o, yc-o },
				{ x,	yc,		xc-o, yc   },
				{ x,	yc+o,	xc-o, yc+o },

				{ xc-o,	yc-o,	xc,	yc-o },
				{ xc-o,	yc,		xc,	yc	 },
				{ xc-o,	yc+o,	xc,	yc+o },

				{ xc,	yc-o,	xc+o,	yc-o },
				{ xc,	yc,		xc+o,	yc	 },
				{ xc,	yc+o,	xc+o,	yc+o },

				{ xc+o,	yc-o,	x+w,	yc-o },
				{ xc+o,	yc,		x+w,	yc	 },
				{ xc+o,	yc+o,	x+w,	yc+o },
				{ 0, 0, 0, 0 }
			};

			for ( int i = 0; i < 32; i++ )
			{
				if ( (C->ShortLines & DWORD( 1 << i ))!=0 ) // есть линия?
					lineAdd( Lines[i].bx, Lines[i].by, 
							 Lines[i].ex, Lines[i].ey,
							 C->LineNum, C->LineNum, *p );
			}
		}
	}

	bool optimizeLines( long Limit )
	{
        gCharVector::iterator i;
		if ( charVector.empty() )
			return false;

		// обнулим все линии...
		for ( i = charVector.begin(); i != charVector.end(); i++ )
			(*i)->InLine = false;

		// найдём первый символ с вертикальным направлением...
		for ( i = charVector.begin(); i != charVector.end(); i++ )
		{
			gChar_t *S = *i;
			if ( S->Matrix[ 3 ]>0 && !S->InLine )
			{
				S->InLine = true;
				char t=S->Matrix[ 3 ];
				int lastLine = S->LineNum;
				for ( gCharVector::iterator j = i+1; j != charVector.end(); j++ )
				{
					gChar_t *C = *j;

					if ( C->InLine )
						continue;

					if ( labs(S->X - C->X) <= Limit && 
						 C->Matrix[ 1 ] == 0 ) // вверх нет линии???
						continue;			   // посмотрим следующий...

					if ( labs(S->X - C->X) <= Limit )
					{
						if ( C->Matrix[ 1 ] != t )
							break;
						else
						if ( labs(C->LineNum-lastLine) == 1 )
						{
							lastLine = C->LineNum;
							C->InLine = true;
							C->X      = S->X;
							C->DX     = S->DX;
							if ( C->Matrix[ 3 ] == 0 )
								break;
						}
						else
							break;
					}
				}
			}
		}
		return true;
	}

	void printLines()	// печатает линии
	{
        if (DBG_MODE) LogAddLine("printLines function START\n");
        
		if ( lineVector.empty() )
			return;

		for ( gLineVector::iterator l = lineVector.begin(); l != lineVector.end(); l++ )
		{
			int bx = (*l)->BX;
			int by = (*l)->BY;
			int ex = (*l)->EX;
			int ey = (*l)->EY;

            if (DBG_MODE) LogAddLine("cairo line: bx=%i by=%i ex=%i ey=%i\n",bx,by,ex,ey);
            cairo_move_to(param->CairoEngine, bx, by);
            cairo_set_line_width(param->CairoEngine,1.0);
            cairo_line_to(param->CairoEngine, ex, ey);
            
		}

        if (DBG_MODE) LogAddLine("printLines function STOP\n");
	}

    /**
    * конвертируем
    */
	int	doConvert()	
	{
		DWORD Start = 0;
		for ( int i = 0; i < partNum; i++ )	Start += param->Parts[ i ];

		BOOL LastPart = ( partNum + 1 == param->NumParts );

		TextLine_t *L = param->Page;

		// сбросим все установки шрифтов, установим пику...!!!!!!

		int grCharID = 0; // индентификатор графического символа, будем являтся номером строки
		while ( L )
		{
			if ( L->Attr & ( aPRINT | aPRINT_ONCE ) )
			{
				// чтобы всёбыло по настоящёму... -)
				if ( L->Attr & aAUTOMODE )
					InitDefEpsonFont( param, 0 );

				DWORD partLen = 0;
				if ( Start + param->Parts[ partNum ] < L->Len )
					partLen = param->Parts[ partNum ];
				else
				if ( L->Len && L->Len > Start )
					partLen = ( L->Len - Start );

				for ( DWORD p = 0; p < partLen; p++ )
				{
					// эмуляция установки ESC шрифтов под WindЫЫЫ :)
					SetFont( param, L->Font, Start + p );

					// символ для печати...
					char Ch = L->Data[ Start + p ];
                
					// должен быть самый большой интервал. 
					// если больше строчный интервал, то высота псевдографики этот интервал иначе высота символ...
					int Interval = (param->EFont.dY<param->EFont.Interval)?(param->EFont.dY):(param->EFont.Interval);

					if ( Ch !=' ' && charAdd( Ch, param->EFont.x, param->EFont.y, param->EFont.dX, Interval, grCharID ) )
						L->Data[ Start + p ] = ' '; // заменим напробел....
					param->EFont.x += param->EFont.dX;
    			}

				// учитываем фонты в конце линии...
				SetFont( param, L->Font, Start + partLen );

			}
			// new-line...
			grCharID++;				// увеличим счётчик линий...
			PrintNewLine( param );	// эмулируем новую строчку в эмуляторе
			L = L->Next;			// на следующие строчку указатель на линии
		}

		// двух проходная оптимизация...с большим лимитом, потом с маленьким...
		optimizeLines( _METRIC( 120 ) );	// второй прИход :)
		convertCharToLine();				// теперь объеденим всё в линии
		printLines();						// теперь наконец-то напечатаем

		return (charVector.empty())?0:charVector.size();
	}
};

/**
* Функция конвертации таблицы псевдографики в линии
*/
int ConvTableToLines( PrintParam_t *P, WORD partNum )
{
    if (DBG_MODE) LogAddLine("ConvTableToLines function START\n");

    int R;
	ConvertLines Convert( P, partNum );
	R = Convert.doConvert();
    
    if (DBG_MODE) LogAddLine("Result: %i\tConvTableToLines function STOP\n",R);
    return R;
}
