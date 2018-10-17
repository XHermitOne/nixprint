/**
* Модуль функций обработки линий
* @file
*/

#include <stdio.h>
#include <string.h>

#include "lprint.h"
#include "lines.h"
#include "blocks.h"
#include "epson.h"

struct Action_t Actions[MAX_ACTIONS] = 
{
	{ '#', aNUM,		0,			bFIRST | bHEADER | bTEXT,	 0 }, // +
	{ '%', aTOTAL,		aSUMCOL,	bDATA,						 1 }, // ++
	{ '+', aTOTAL_PRO,	aSUMCOL,	bDATA,						 1 }, // ++  SAM ADDED
	{ '~', aHEAD_BEGIN, 0,			bHEADER,					 1 }, // +
	{ '@', aHEAD_END,	0,			bHEADER,					 1 }, // +
	{ '}', aREPEAT,		aHEAD_END,	bHEADER | bDATA,		     -2 }, // +
	{ '^', aAUTOMODE,	0,			bFIRST | bHEADER | bTEXT,	 1 }, // +
	{ '&', aDUPLICATE,	0,			bTEXT,						 1 }, // + 
	{ '\\', aDATETIME,	0,			bFIRST | bHEADER,	         0 }, // +
	{ '+', aSUMCOL,		aHEAD_END,	bHEADER,				     -2 }, // +
	{ '!', aSKIP_NAME,	aHEAD_END,	bHEADER,				     -2 }, // +
    { '\x0C', aFORMFEED, 0,         bFIRST | bHEADER | bDATA | bTEXT, 0 },
    { '[', aPORTRAIT_ORIENTATION,  0,   bTEXT, 1 },
    { ']', aLANDSCAPE_ORIENTATION, 0,   bTEXT, 1 }
//    { '$', aPAGE_SEGMENT, 0,   bTEXT, 1 }
};

const WORD MAX_LINE_LEN = 60000;
const WORD TAB_SIZE = 8;

/**
* Загрузить линию
* INPUT:
*		in   -- файл для загрузки
* OUTPUT:
*		Line     -- линия из файла
*     	FormFeed -- 0 - нет в линии
*					1 - конец страницы, для разделения одного отчёта.
*					2 - конец документа, для разделения документов, чтобы в одном файле моно напечатать несколько документов
*/
BOOL LoadLine( FILE *in,TextLine_t *Line, int *FormFeed )
{
	if ( in == NULL ) 
		return FALSE;

	*FormFeed = 0; // нет ничего...
	int ch = -1;	
	WORD i;								    
	BYTE Buffer[ MAX_LINE_LEN ];		
	memset( Buffer, 0, MAX_LINE_LEN ); 	
	for ( i = 0; ( i < MAX_LINE_LEN ) && ( ch = fgetc( in ) ) != EOF && ( ch != '\r' ); i++ )
	{
		if ( ch == (char)0x9 ) 
		{
			// заменим сразу табуляцию
			WORD SpaceNeed = TAB_SIZE - ( i % TAB_SIZE );
			memset( &Buffer[ i ], 0x20, SpaceNeed );
			i += ( SpaceNeed - 1 );
		}
		else 
		if ( ( Buffer[ i ] = (char)ch ) == '\f' )
		{
			*FormFeed = 1;
			// проверим нет ли второго \f?
			long OldPos = ftell( in );
			if ( fgetc( in ) != '\f' )
				fseek( in, OldPos, SEEK_SET );
			else
				*FormFeed = 2;
		}
	}

    // конец строчки?
	if ( ch == '\r' )
	{
		// проверим нет ли после <CR> символа <LF> ?
		long OldPos = ftell( in );
		if ( fgetc( in ) != '\n' )
			fseek( in, OldPos, SEEK_SET );
	}

	if ( i == MAX_LINE_LEN || ( i == 0 && ch == EOF ) )
		return FALSE;	// PANIC: линия больше 60000 символов!

	Line->Data = new BYTE[ i + 1 ];
	if ( Line->Data == NULL )
		return FALSE;	// нет памяти для строки!

	Line->Len = i;
	memmove( Line->Data, Buffer, i + 1 ); // +1 - включай последний ноль в строке!

	return TRUE;
}

/**
* для сортировки EpsonCode
*/
int EpsonCompare( const void *a1, const void *a2 )
{
	WORD l1 = (*(EpsonCode_t *)a1).Len;
	WORD l2 = (*(EpsonCode_t *)a2).Len;

	if ( l1 == l2 )	return 0; else return ( l1 > l2 ) ? -1 : 1;
}


static BOOL InitStripFirst = TRUE;
static WORD TOTAL_EPSON_CODE = 0;

/**
* только первый раз, найдём самую длинную последовательность... 
* oтсортируем в уменьшающемся порядке... 9, 8, 7, 6 ,5...
*/
class InitStripShrifts_t
{
public:

    InitStripShrifts_t()
    {
	    for ( WORD i = 0; EpsonCode[ i ].Seq != NULL; i++ )
		    TOTAL_EPSON_CODE++;

	    qsort( EpsonCode, TOTAL_EPSON_CODE, sizeof( EpsonCode_t ), EpsonCompare );
    };

    ~InitStripShrifts_t()
    {};
} InitStripShrifts; // тут и вызовем конструктор...

/**
* поиск всех epson-последовательностей в строчке и их вырезание в структуру...
* (optimized) ... найдём все последовательности...
*/
BOOL StripShrifts(TextLine_t *Line )
{
	if ( Line->Data == NULL || Line->Len == 0 )
		return TRUE;

	TextFont_t *Last = NULL;

    // буфер и его длина.... а что, стэк большой :)
	char L[ 60000 ]; 
	WORD LLen = 0;

    // длина для всех EpsonSeq в строке
	DWORD SeqLen = 0; 

	// цикл для каждого символа в строке..
    for ( DWORD Pos = 0; Pos < Line->Len; Pos++ ) 
	{
		BOOL AddSeq = TRUE;
		BOOL SeqAdded = FALSE;
		
        // для каждой проследовательности
		for ( WORD SeqNum = 0; SeqNum < TOTAL_EPSON_CODE; SeqNum++ ) 
		{
            WORD ECLen = 0;

            // эти коды точно могут быть в строке?
			if ( ( ECLen = EpsonCode[ SeqNum ].Len ) + Pos > Line->Len ) 
				continue; // не подходят по длине, next seq..

            BYTE *LData = Line->Data + Pos;
            char *ECSeq = EpsonCode[ SeqNum ].Seq;

            // проверка на совпадение, учитывая символы маски '?' 
            char *FntData = (char *)memchr( ECSeq, '?', ECLen );
            WORD FntMaskLen = ( FntData == NULL ) ? 0 : strlen( FntData );
            WORD FntHardLen = ECLen - FntMaskLen;

            AddSeq = (memcmp( LData, ECSeq, FntHardLen ) == 0); // 0-equal

            // нашли? добавим последовательность...
			if ( AddSeq ) 
			{
				TextFont_t *Fnt = new TextFont_t;
				Fnt->Next = NULL;
				Fnt->Skip = ( EpsonCode[ SeqNum ].Alias == 0 ) ? TRUE : FALSE;
				Fnt->Attr = EpsonCode[ SeqNum ].Alias;
				Fnt->Pos = ( Pos - SeqLen );
				Fnt->DataLen = 0;
				Fnt->Data = NULL;
                Fnt->EpsonDataLen = ECLen;

                // сохраним, оригинальную esc последовательность из файда для быстро печати...
				Fnt->EpsonData = new char[ ECLen + 1 ];
                memmove( Fnt->EpsonData, LData, ECLen );
				Fnt->EpsonData[ ECLen ] = 0;

                // символы из под маски ...
				if ( ( Fnt->DataLen = FntMaskLen ) > 0 )
				{
					Fnt->Data = new char[ Fnt->DataLen + 1 ];
                    memmove( Fnt->Data, &LData[ FntHardLen ], Fnt->DataLen );
					Fnt->Data[ Fnt->DataLen ] = 0;
				}

                // увелим кол-во епсон последовательностей в строчке...
				SeqLen += ECLen;

                // установим уазатель Pos на следующию позицию, после шрифта...
				Pos += ( ECLen - 1 );

                // Добавим последовательность...								
				if ( Last == NULL )
					Line->Font = Fnt; 
				else
					Last->Next = Fnt;
				Last = Fnt;
				SeqAdded = TRUE;
				break;		
			}
			else
				AddSeq = TRUE;
		} // for ( SeqNum...
		if ( !SeqAdded ) 
            L[ LLen++ ] = Line->Data[ Pos ];
	} // for ( Pos...

	if ( Line->Font != NULL ) // значит есть шрифты, надо делать новую строку...
	{
        // не будем переудалять блок, так быстрей будет
		L[ LLen ] = 0;
		delete []Line->Data;
		Line->Data = (BYTE *)new char[ LLen + 1 ];
		memmove( (char *)Line->Data, &L[ 0 ], LLen + 1 );
		Line->Len = LLen;
	}

	return TRUE;
}

/**
* Вырезает символы в Line, которые входят в BadChars
*/
WORD StripBadChars(TextLine_t *Line, char *BadChars )
{
	if ( Line->Data == NULL || Line->Len == 0 )
		return 0;

	WORD NumBadChars = 0;

	for ( DWORD i = 0; i < Line->Len; i ++ )
	{
		if ( strchr( BadChars, Line->Data[ i ] ) )
		{
			Line->Data[ i ] = ' ';
			NumBadChars++;
		}
	}

	return NumBadChars;
}

/**
* освободим память...
*/
BOOL DelLine(TextLine_t *Line )
{
	if ( !Line )
		return FALSE;

	delete []Line->Data;
	
	TextFont_t *F = Line->Font; 
	while ( F )
	{
		TextFont_t *Prev = F;
		F = F->Next;
		if ( Prev->Data )		delete []Prev->Data;
		if ( Prev->EpsonData )	delete []Prev->EpsonData;
		delete Prev;
	}
	delete Line;
	return TRUE;
}

/**
* дублировать фонт F
*/
TextFont_t *DupFont(TextFont_t *F )
{
	if ( !F ) return NULL;

	TextFont_t *N = new TextFont_t;
	if ( !N ) return NULL;

	N->Skip = F->Skip;
	N->Attr = F->Attr;
	N->Pos  = F->Pos;
	N->Next = NULL;
	N->Data = NULL;
	N->DataLen = F->DataLen;
	N->EpsonData = NULL;
    N->EpsonDataLen = F->EpsonDataLen;
	if ( F->DataLen )
	{
		N->Data = new char[ F->DataLen + 1 ];
		if ( !N->Data )
		{
			delete N;
			return NULL;
		}
		memmove( N->Data, F->Data, F->DataLen + 1 );
	}

	if ( F->EpsonData )
	{
		N->EpsonData = new char [ F->EpsonDataLen + 1 ];
		if ( !N->EpsonData )
		{
			delete []N->Data;
			delete N;
			return NULL;
		}
		memmove( N->EpsonData, F->EpsonData, F->EpsonDataLen + 1 );
	}

	return N;
}

/**
* дублировать линию L
*/
TextLine_t *DupLine(TextLine_t *L )
{
	TextLine_t *N = new TextLine_t;
	if ( !N )
    {
        if (DBG_MODE) LogAddLine("WARNING: DupLine function STOP error new line memory");
        return NULL;
    }
    
	N->Attr = L->Attr;
	N->Font = NULL;
	N->Next = NULL;
	N->Len = L->Len;
	N->Data = new BYTE[ L->Len + 1 ];
	if ( !N->Data )
	{
		delete N;
        if (DBG_MODE) LogAddLine("WARNING: DupLine function STOP error line data memory");
		return NULL;
	}
	memmove( N->Data, L->Data, L->Len + 1 );

	if ( L->Font )
	{
		TextFont_t *Start = DupFont( L->Font );
		TextFont_t *LFnt = L->Font->Next;
		if ( Start )
		{
			TextFont_t *F = Start;
			while ( LFnt )
			{
				F->Next = DupFont( LFnt );
				F = F->Next;
				LFnt = LFnt->Next;
			}
		}
		N->Font = Start;
	}

	return N;
}

/**
* Удаляет в строке S с Pos, Count символов...
*/
void DelChar( char *S, WORD Pos, WORD Count )
{
	if (!S || Count == 0) return;
	WORD Len = strlen( S );
	if (Pos > Len) return;

	if ( Pos + Count - 1 > Len )
		Count = Len - Pos + 1;

	memmove( &S[ Pos ], &S[ Pos + Count ], Len - Pos - Count + 1 );
}

/**
* удалить все пробелы и символы табуляции
*/
void DelAllSpace( char *S )
{
	if ( !S ) return;
	WORD j = 0;
	while ( S[ j ] )
		if ( S[ j ] == ' ' || S[ j ] == 9 ) 
			DelChar( S, j, 1 );	
		else 
			j++;
}

void LineTrimRightSpace(TextLine_t *L )
{
    if (DBG_MODE) LogAddLine("LineTrimRightSpace function START");
    
    if ( !L || !L->Len )
    {
        if (DBG_MODE) LogAddLine("WARNING: LineTrimRightSpace function STOP error arguments");
        return;
    }

    char *E = strrchr((char *)L->Data, 0);
    if ( !(*E) )
    {
        E--;
        while ( *E == ' ' && L->Len )
        {
            L->Len--;
            *E-- = 0;
        }
    }

    if (DBG_MODE) LogAddLine("LineTrimRightSpace function STOP");
}

void PadRightSpace(TextLine_t *L, DWORD NewLen )
{
	if ( !L || L->Len >= NewLen )
		return;

	BYTE *S = new BYTE[ NewLen + 1 ];
	if ( !S )
		return;

	memset( S, ' ', NewLen );
	S[ NewLen ] = 0;

	memcpy( S, L->Data, L->Len );
	delete []L->Data;
	L->Data = S;
	L->Len = NewLen;
}

/**
* вставляет часть строчки с позиции SrcPos шириной SrcWidth в позицию DstPos 
*/
void InsPartLine(TextLine_t *L, DWORD SrcPos, DWORD SrcWidth, DWORD DstPos )
{
	if ( !L || !L->Len || SrcPos > L->Len )	return;

	DWORD newLen = SrcWidth + ( ( DstPos > L->Len ) ? DstPos : L->Len );
	BYTE *newStr = new BYTE[ newLen + 1 ];
	if ( !newStr ) return;

	// инициализация строки пробелами...
	memset( newStr, ' ', newLen );
	newStr[ newLen ] = 0;

	// до...
	DWORD Count = ( L->Len > DstPos ) ? DstPos : L->Len;
	memmove( newStr, L->Data, Count );

	// что повторить..
	memmove( &newStr[ DstPos ], &L->Data[ SrcPos ], ( SrcWidth > L->Len ) ? L->Len : SrcWidth );

	// после...
	Count = ( L->Len > DstPos ) ? ( L->Len - DstPos + 1 ) : 0;
	memmove( &newStr[ DstPos + SrcWidth ], &L->Data[ DstPos ], Count );

	delete []L->Data;
	L->Data = newStr;
	L->Len = newLen;

	// повторить шрифты...
	if ( !L->Font )	return;			// нет в линии шрифтов...

	// откуда повторять...
	TextFont_t *SF = L->Font;
	while ( SF && SF->Pos < SrcPos ) SF = SF->Next;
	if ( !SF ) return;				// нет после данной позиции шрифтов...

	// куда повторять...
	TextFont_t *DF = L->Font;
	while ( DF && DF->Pos < DstPos ) DF = DF->Next;
	if ( !DF )
	{
		DF = L->Font;
		while ( DF && DF->Next ) DF = DF->Next;
	}
	else
	{
		TextFont_t *prevDF = L->Font;
		while ( prevDF && prevDF->Next != DF )	prevDF = prevDF->Next;
		if ( prevDF && prevDF->Next == DF )	DF = prevDF;
	}

	// собственно повтор...
	while ( SF && SF->Pos <= SrcPos + SrcWidth )
	{
		TextFont_t *Start = DupFont( SF );
		if ( Start )
		{
			Start->Pos += ( DstPos );

			if ( !DF->Next )
				DF->Next = Start;
			else
			{
				Start->Next = DF->Next;
				DF->Next = Start;
			}
			DF = DF->Next;
		}
		SF = SF->Next;
	}

	// остаток...
	DF = DF->Next;	
	while ( DF )
	{
		DF->Pos += SrcWidth;
		DF = DF->Next;
	}
}

/**
* Заменить символы = в Line на псевдографику
*/
BOOL DefinePseudograph(TextLine_t *Line)
{
    BOOL is_replace=FALSE;

    if ( Line->Data == NULL || Line->Len < 2 )
		return FALSE;

    if (DBG_MODE) LogAddLine("DefinePseudograph Data: %s buff: %s",Line->Data);
    
    if (find_word((char *)Line->Data,"==") || find_word((char *)Line->Data,"|"))
        is_replace=TRUE;
        
    if (is_replace)
    {
        for ( DWORD i = 0; i < Line->Len; i ++ )
        {
            if (Line->Data[ i ] == '=')
                Line->Data[ i ] = '\xC4';
            if (Line->Data[ i ] == '|')
                Line->Data[ i ] = '\xB3';
        }
    }

	return TRUE;
}

