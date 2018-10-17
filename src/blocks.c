/**
* Модуль функций обработки блоков.
* @file
*/

#include "blocks.h"

/**
* создать новый блок, тип блока bTYPE
*/
TextBlock_t *BlkNew( DWORD bTYPE,TextBlock_t *ExistBlk )
{
	TextBlock_t *Blk = new TextBlock_t;
	if ( Blk == NULL )
		return NULL;

	Blk->Attr			= bTYPE;
	Blk->HaveActions	= 0;
	Blk->LineInBlock	= 0;
	Blk->BlockData		= NULL;
	Blk->Next			= NULL;

	if ( ExistBlk != NULL )
		ExistBlk->Next = Blk;

	return Blk;
}

/**
* new in this version of func:
*      - теперь не создаётся второй раз дупликат строчки...
* Добавляет линию к блоку...
*/
TextBlock_t *BlkAddLine(TextBlock_t *ExistBlk,TextLine_t *Line )
{
	if ( ExistBlk == NULL || Line == NULL )
		return NULL;

	Line->Next = NULL;

	struct TextLine_t *L = ExistBlk->BlockData;
	if ( L == NULL )
		ExistBlk->BlockData = Line;
	else
	{
		while ( L->Next != NULL )
			L = L->Next;

		L->Next = Line;
	}

	ExistBlk->LineInBlock++;
	return ExistBlk;
}


/**
* Ищёт блок с определённым типом...bTYPE
*/
TextBlock_t *BlkFind( DWORD bTYPE,TextBlock_t *Head )
{
	if ( Head == NULL )
		return NULL;

	TextBlock_t *Cur = Head;
	while ( Cur )
	{
		if ( Cur->Attr & bTYPE )
			break;
		else
			Cur = Cur->Next;
	}

	return Cur;
}

/**
* Ищет линию с определённым Action
*/
TextLine_t *BlkActionFind( DWORD Action,TextBlock_t *Head )
{
	if ( Head == NULL || Head->LineInBlock == 0 || Head->BlockData == NULL )
		return NULL;

	TextLine_t *L = Head->BlockData;
	while ( L )
	{
		if ( L->Attr & Action )
			break;
		L = L->Next;
	}
	return L;
}

/**
*
*/
TextLine_t *ActionFind( DWORD Action,TextLine_t *Head )
{
	if ( Head == NULL && Action == 0 )
		return NULL;

	TextLine_t *L = Head;
	while ( L )
	{
		if ( L->Attr & Action )
			break;
		L = L->Next;
	}
	return L;
}

/**
*
*/
DWORD FindMaxLen(TextLine_t *Head )
{
	if ( Head == NULL )
		return NULL;

	DWORD MaxLen = 0;
	TextLine_t *L = Head;
	while ( L )
	{
		if ( L->Len > MaxLen )
			MaxLen = L->Len;
		L = L->Next;
	}
	return MaxLen;
}

/**
*
*/
DWORD BlkSetLineID( DWORD aTYPE,TextBlock_t *ExistBlk, DWORD NumLines )
{
	DWORD SetLines = 0;

	if ( ExistBlk != NULL && ExistBlk->BlockData != NULL )
	{
		TextLine_t *L = ExistBlk->BlockData;
		while ( L && NumLines-- )
		{
			L->Attr |= aTYPE;
			L = L->Next;
			SetLines++;
		}
	}

	return SetLines;
}

/**
*
*/
DWORD BlkClearLineID( DWORD aTYPE,TextBlock_t *ExistBlk, DWORD NumLines )
{
	DWORD ClearLines = 0;

	if ( ExistBlk != NULL && ExistBlk->BlockData != NULL )
	{
		TextLine_t *L = ExistBlk->BlockData;
		while ( L && NumLines-- )
		{
			if ( L->Attr & aTYPE )
				L->Attr &= (~aTYPE);
			L = L->Next;
			ClearLines++;
		}
	}

	return ClearLines;
}

/**
*
*/
DWORD BlkDelLine(TextLine_t *L,TextBlock_t *ExistBlk )
{
	if ( ExistBlk != NULL && ExistBlk->BlockData != NULL )
	{
        TextLine_t *Prev = NULL;
		TextLine_t *CurLine = ExistBlk->BlockData;
		while ( CurLine )
		{
			if ( CurLine == L ) // типа нашли линию для удаления...
			{
				if ( CurLine == ExistBlk->BlockData && !Prev )
					ExistBlk->BlockData = CurLine->Next;
                else
                if ( Prev->Next == CurLine )
                    Prev->Next = CurLine->Next;

				DelLine( CurLine );
				ExistBlk->LineInBlock--;
				return ExistBlk->LineInBlock;
			}
            Prev = CurLine;
			CurLine = CurLine->Next;
		}
	}
	return 0;
}

/**
*
*/
void ShowBlockType(TextBlock_t *B, BOOL LineInfoShow = FALSE )
{

	char *BlkName[] = { "TEXT", "FIRST", "HEADER", "DATA", "SIGN" };

	while ( B )
	{
		char n;

		switch ( B->Attr )
		{
			case bTEXT: n=0;                break;
			case bTEXT | bNEEDINIT: n=0;    break;
			case bFIRST: n=1;               break;
			case bHEADER: n=2;              break;
			case bDATA: n=3;                break;
			case bSIGN: n=4;                break;
			case bSIGN | bNEEDINIT: n=4;    break;
//			case bSKIP: n=4;                break;
		}
	
		LogAddLine( "[Block: %10s - %3lu%s]\n", BlkName[ n ], B->LineInBlock, ( B->Attr&bNEEDINIT)?" init":"" );

		TextLine_t *L = ( LineInfoShow ) ? B->BlockData : NULL;
		while ( L != NULL )
		{
            char temp[ 1024 ]; temp[0]=0;
			if ( L->Attr & aNUM )           sprintf( temp, "[#]" );
			if ( L->Attr & aSKIP_NAME )     sprintf( temp, "[!]" );
			if ( L->Attr & aTOTAL )         sprintf( temp, "[%]" );
			if ( L->Attr & aHEAD_BEGIN )    sprintf( temp, "[~]" );
			if ( L->Attr & aHEAD_END )      sprintf( temp, "[@]" );
			if ( L->Attr & aREPEAT )        sprintf( temp, "[}]" );
			if ( L->Attr & aAUTOMODE )      sprintf( temp, "[^]" );
			if ( L->Attr & aSUMCOL )        sprintf( temp, "[+]" );
			if ( L->Attr & aDUPLICATE )     sprintf( temp, "[&]" );
			if ( L->Attr & aDATETIME )      sprintf( temp, "[D]" );
			if ( L->Attr & aSIGNATURE )     sprintf( temp, "[S]" );
			if ( L->Attr & aPORTRAIT_ORIENTATION )      sprintf( temp, "[P]" );
			if ( L->Attr & aLANDSCAPE_ORIENTATION )     sprintf( temp, "[L]" );
			//if ( L->Attr & aFIT_TO_WIDTH )              sprintf( temp, "[F]" );
            if ( strlen( temp ) )
                LogAddLine ( "%s\n", temp );

			L = L->Next;
		}
			

		B = B->Next;
	}
	LogAddLine( "\n\n" );

}
