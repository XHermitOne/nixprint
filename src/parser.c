/**
* Модуль функций разбора текстового файла печати
* @file
*/

#include "parser.h"

static int sam_itogo;  /**< Сумма "Итого по листу" */

BOOL NeedCancelDup=FALSE; /**< отменить режим дублирования */

/**
* Процедура загрузки документа для печати...
*/
char LoadDoc(PrintParam_t *P )
{
    if (DBG_MODE) LogAddLine("LoadDoc function START");
    
	//-- откроем файл для печати...
	FILE *in = NULL;
	if ( ( in = fopen( P->FileForPrint, "rb" ) ) == NULL )
    {
        if (DBG_MODE) LogAddLine("File: %s not opened.",P->FileForPrint);
		return 0;
    }

	//-- cur pos in file
	DWORD CurSize = 0;
	P->LinesInFile = 0;		// кол-во загруженных линий из файла

	//-- узнаем размер файл, MaxSize
	fseek(in, 0, SEEK_END);
	DWORD MaxSize = ftell(in);
	fseek(in, 0, SEEK_SET);
    
    if (DBG_MODE) LogAddLine("File %s size %u", P->FileForPrint, MaxSize);

	//-- установим длину буфера 32Kb для упреждающего чтения
	char Buffer[32 * 1024];
    //if (DBG_MODE) LogAddLine("Set Buffer...");
	if (setvbuf(in, Buffer, _IOFBF, sizeof(Buffer)) != 0 )
	{
        fclose(in);
        if (DBG_MODE) LogAddLine("Buffer Init. File %s is closed ..ok", P->FileForPrint);
		return 0;
	}
    //if (DBG_MODE) LogAddLine("...ok");

	//-- Инициализация блоков...
    //if (DBG_MODE) LogAddLine("New block...");
	TextBlock_t *CurB = P->Doc = BlkNew(bTEXT, NULL);
	if (CurB == NULL)
	{
		fclose(in);
        if (DBG_MODE) LogAddLine("New text block.File %s is closed ..ok", P->FileForPrint);
		return 0;
	}
    //if (DBG_MODE) LogAddLine("...ok");

	//-- Загрузим линии из файла, и расставим их по нужным блокам...
	DWORD oldAttr = bTEXT;
	int isFormFeed = 0;		// 1-конец страницы 2-конец страницы и следующий инициализировать шрифтом по умолчанию...
	do {
		TextLine_t *Line = new TextLine_t;
		if (Line == NULL)
		{
			fclose(in);
            if (DBG_MODE) LogAddLine("Load line. File %s is closed ..ok", P->FileForPrint);
			return 0;
		}

		Line->Attr = 0;
		Line->Len  = 0;
		Line->Data = NULL;
		Line->Font = NULL;
		Line->Next = NULL;

		if (LoadLine(in, Line, &isFormFeed) == NULL)
			break; // конец документа, выход из цикла...
        if (DBG_MODE) LogAddLine("Load line...ok");

		CurSize += Line->Len;

		// проверка на баг из clipper-а, когда первый символ formfeed...
		if (P->LinesInFile == 0 && isFormFeed == 1)
		{
            if (DBG_MODE) LogAddLine("Delete line");
			if (Line->Data != NULL)
				delete []Line->Data;
			delete Line;
            if (DBG_MODE) LogAddLine("...ok");
			continue;
		}

		P->LinesInFile++;

		// вырежим все шрифты...
        if (DBG_MODE) LogAddLine("Strip shrifts");
		StripShrifts(Line);
        if (DBG_MODE) LogAddLine("...ok");

		// заменим псевдографикой то что можно заменить...
        if (DBG_MODE) LogAddLine("Define pseudograph");
		DefinePseudograph(Line);
        if (DBG_MODE) LogAddLine("...ok");

		// описание плохих символов для вырезания...
		// "плохие" символы ...
		char *BAD_CHARS = "\xb0\xb1\xb2\xdb\xdc\xdd\xde\xdf";
		StripBadChars( Line, BAD_CHARS );
		
		BOOL AddLine = TRUE; // добавить линию в блок
		if ( Line && Line->Len > 0 )
		{
			if ( CurB->Attr != bSIGN )			// блок CИГНАТУРЫ ничем нельзя разорвать...
			{
				if ( Line->Data[ 0 ] == '~' )   // начало шапки
				{
					if ( CurB->Attr == bTEXT )
						CurB->Attr = bFIRST;

					if ( CurB->LineInBlock > 0 )
						CurB = BlkNew( bHEADER, CurB ); // блок-шапки
					else
						CurB->Attr = bHEADER;
				}

				if ( Line->Data[ 0 ] == '@' )	// конец шапки
				{
					BlkAddLine( CurB, Line );
					CurB->Attr = bHEADER;

					CurB = BlkNew( bDATA, CurB );	// следующий блок за шапкой
					AddLine = FALSE;				// блок данных...
				}
			}

			if ( Line->Data[ 0 ] == '$' )	// Сигнатура $B начало, $E - конец...
			{
				BYTE *Data = NULL;
				switch ( Line->Data[ 1 ] )
				{
					case 'B':				// начало 
						oldAttr = CurB->Attr;
						if ( CurB->LineInBlock > 0 )
							CurB = BlkNew( bSIGN|bNEEDINIT, CurB );
						else
							CurB->Attr = bSIGN|bNEEDINIT;

						Data = new BYTE[ Line->Len - 2 ];
						if ( Data )
						{
							Line->Len -= 2;
							memmove( Data, Line->Data+2, Line->Len );
							delete []Line->Data;
							Line->Data = Data;

						}
						break;

					case 'E':				// конец...
						if ( ( CurB->Attr & bSIGN ) == bSIGN )
						{
							Data = new BYTE[ Line->Len - 2 ];
							if ( Data )
							{
								Line->Len -= 2;
								memmove( Data, Line->Data+2, Line->Len );
								delete []Line->Data;
								Line->Data = Data;

							}
							Line->Attr |= aSIGNATURE;
							BlkAddLine( CurB, Line );
							CurB = BlkNew( oldAttr, CurB );
							AddLine = FALSE;
						}
						break;
						// SAM!!!!!!!!!!!!!!!!!!!
					case 'K':	//  $K  Установить "Книжную" ориентацию
						{
							P->Paper.Orientation=0;
						}
						break;
					case 'A':	// $A  Установить "Книжную" ориентацию
						{
							P->Paper.Orientation=1;
						}
						break;
				}
			}
		}

		// добавим линию в нужный блок
		if ( AddLine )
		{
			BlkAddLine( CurB, Line );
			if ( ( CurB->Attr & bSIGN ) == bSIGN )
				Line->Attr |= aSIGNATURE;

			if ( isFormFeed == 2 )
				Line->Attr |= aFORMFEEDDOC;
		}
		
		// нужно начать новый блок? да, если обнаружен FORMFEED, одиночный
		if ( isFormFeed == 1 ) 
			CurB = BlkNew(bTEXT, CurB);
		else if ( isFormFeed == 2 )
            CurB = BlkNew(bTEXT | bNEEDINIT, CurB);
	} while ( TRUE );
	
	fclose( in ); 
    if (DBG_MODE) LogAddLine("File %s is closed ..ok",P->FileForPrint);

	//-- проверим, чтобы не было блоков нулевой длины (LineInBlock==0)
    CurB = P->Doc;
    if (DBG_MODE) ShowBlockType( P->Doc, 1 );
    
    while ( CurB )
    {
        if ( CurB->Next && CurB->Next->LineInBlock == 0 ) 
        {
			TextBlock_t *needDel = CurB->Next;
			CurB->Next = CurB->Next->Next;		// CurB будет указывать на блок после нулевого
            delete needDel;						// удалим
        }
        CurB = CurB->Next;
    }

	return (P->LinesInFile > 0) ? 1:0;
}


/**
* Анализирует документ на всякие акшины для lprint, 
* так же разбивает на блоки: first, header, data, text...
*/
BOOL AnalyzeDoc(PrintParam_t *P)
{
	if ( P->Doc == NULL )
		return FALSE;

	DWORD LinesComplete = 0;

	for ( BYTE Passes = 1; Passes <= 2; Passes++ )
	{
		TextBlock_t *Start, *Blk;
		Blk = P->Doc;
		Start = Blk;
		DWORD WasIDinBlocks = 0;

		// Проставим атрибуты для ЛИНИЙ...в 2  прохода...
		while ( Blk ) 
		{
			// 1 pass...
			// пробежим по блоку и выделим все ACTIONS...
			// на первом проходе                          
			// установим все акшины не учитывая needAction
			TextLine_t *Ln = Blk->BlockData;
			while ( Ln && Passes == 1 )
			{
				DWORD ID = ( aTXT | ( Blk->Attr & ( bDATA | bHEADER | bTEXT | bFIRST ) ) );
				if ( Ln->Len )
				{
					for ( WORD i = 0; i < MAX_ACTIONS; i++ )
					{
						BOOL ActionFound = FALSE;

						//
						char Pos = Actions[ i ].Pos;
						char Ch = Actions[ i ].Ch;

						if ( Pos == 0 && memchr( Ln->Data, Ch, Ln->Len ) != NULL )
							ActionFound = TRUE;

						if ( Pos > 0 && Ln->Data[ Pos - 1 ] == Ch )
							ActionFound = TRUE;

                        BOOL FoundAct = ( memchr( Ln->Data + abs( Pos ) - 1, 
                                                  Ch, 
                                                  Ln->Len - ( abs( Pos ) - 1 ) ) != NULL );
						if ( Pos < 0 && FoundAct )
							ActionFound = TRUE;

						if ( ActionFound && ( Blk->Attr & Actions[ i ].MustBeInBlock ) )
							ID |= Actions[ i ].ID;
					} // for
				} // if len

				Ln->Attr = ID;
				Blk->HaveActions |= ID;
				Ln = Ln->Next;
			} // while

			// 2 pass...
			if ( Passes == 2 )
			{
				WasIDinBlocks |= Blk->HaveActions;
				if ( Blk->Attr & bDATA )
				{
					TextBlock_t *RepeatFrom = Start;

					while ( RepeatFrom )
					{
						TextLine_t *DL = RepeatFrom->BlockData; 
						while ( DL )
						{
							for ( WORD i = 0; DL && DL->Len && i < MAX_ACTIONS; i++ )
							{
								if ( ( DL->Attr & Actions[ i ].ID ) && Actions[ i ].needAction )
								{
									// проверка на отстуствие двух `}` вблоке шапки и данных...
									BOOL fakeAction = 
                                        (Actions[ i ].ID == aREPEAT && 
                                          (RepeatFrom->Attr & bHEADER) && 
										  (((RepeatFrom->HaveActions|RepeatFrom->Next->HaveActions )&aREPEAT) &&
										  (RepeatFrom->HaveActions & RepeatFrom->Next->HaveActions & aREPEAT)==0));

									fakeAction |= ( (Actions[ i ].ID&(aSUMCOL|aSKIP_NAME))&&Actions[ i ].needAction == aHEAD_END && !(DL->Attr&aHEAD_END));

									if ( ( WasIDinBlocks & Actions[ i ].needAction ) == 0 || fakeAction )
										DL->Attr &= ( ~Actions[ i ].ID );
								}
							} 

							DL = DL->Next;
						}

						if ( RepeatFrom == Blk )
							break;

						RepeatFrom = RepeatFrom->Next;
					} // while ( RepeatFrom ) ...

					// сбросим акшины какие встречались, так как новый документ...
					Start = Blk->Next;
					WasIDinBlocks = 0; 
				} // if ( Blk->Attr & bDATA ) ...
			} // if ( passes == 2 ) ...

			Blk = Blk->Next;
		} // while ( blk )
	} // for ( passes

	return TRUE;
}


/**
* проверка на DupMode режим '&'...
*/
WORD CheckDupMode( TextLine_t *L, WORD dupMode )
{
    if (DBG_MODE) LogAddLine("CheckDupMode function START");
    WORD DupMode = dupMode;
	TextLine_t *D = ActionFind( aDUPLICATE, L );
    if ( D && D->Data && D->Len >= 2 && D->Data[ 0 ] == '&' && strchr( "0123456789", D->Data[ 1 ] ) )
    {
        DupMode = atoi( (const char *)&D->Data[ 1 ] );
        if ( DupMode < 1 || DupMode > 9 )  // SAM !!!!
            DupMode = 0;
    }
    if (DBG_MODE) LogAddLine("DupMode=%i\nCheckDupMode function STOP",DupMode);
    return DupMode;
}

/**
* @return кол-во линий которые вошли на страничку...
*/
DWORD TryFitToPage(PrintParam_t *P, WORD &DupMode, BOOL &FormFeed, BOOL &NeedCancelDup )
{
    if (DBG_MODE) LogAddLine("TryFitToPage function START");
    
	NeedCancelDup = FALSE;
	DWORD FitLine = 0;      // сколько линий входит на страничку....
	DWORD LineInBlock = P->BlkCur->LineInBlock;
	TextLine_t *Line = P->BlkCur->BlockData;
	
	TextLine_t *EmptyLine = NULL;	// указатель на первую пустую строчку...
	WORD EmptyLineTotal = 0;

    WORD oldDupMode = DupMode;
    DupMode = CheckDupMode( Line, DupMode );

	WORD PlatInPage	= 0;			// кол-во платёжек которое вошло на лист
    TextLine_t *BeginPlat = NULL;	// начало платёжки
    DWORD cuttedLineInPlat = 0;		// кол-во строк в "обрезанной" платёжки

    BOOL dontStopRulon = (!P->Paper.H && P->Options.AskCorrectPaper == 0 ); // рулонная бумага...

	while ( LineInBlock && !FormFeed )
	{
		//-- высота линии
		DWORD Interval = GetInterval( Line );
		if ( Interval > 0 )					 // изменился межстрочный интервал?
			P->Paper.LineInterval = Interval; // да

		DWORD LineHigh = P->Paper.LineInterval;

		if ( dontStopRulon || P->Paper.FreeHigh > LineHigh )
		{
            if ( P->Paper.FreeHigh > LineHigh )
			    P->Paper.FreeHigh -= LineHigh;

			//-- проверка на платёжку...
			if ( Line->Len )
			{ 
                // проверка производится по ключевым словам из ресурсного файла lprintw.lng
                char *line_data=cp866_to_utf8((char*)Line->Data);
				char *d1 = strstr( line_data, GetStr( IDS_DUP1 ) ); //
				char *d2 = strstr( line_data, GetStr( IDS_DUP2 ) ); // по словам "Расчетный листок"
				char *d3 = strstr( line_data, GetStr( IDS_DUP3 ) ); // 
				char *d4 = strstr( line_data, GetStr( IDS_DUP4 ) ); // по словам "Лицевой счет"
				char *d5 = strstr( line_data, GetStr( IDS_DUP5 ) ); // по слову "СЧЕТ-КВИТАНЦИЯ"
				char *d6 = strstr( line_data, GetStr( IDS_DUP6 ) ); // по слову ".Организац"
                free(line_data);
                
				if ( DupMode && ((d1 && d2) || (d3 && d4 ) || (d5)|| (d6) ))	// Нашли начало платёжки?
				{
					BeginPlat = Line;		// да, нашли
					cuttedLineInPlat = 0; 
					PlatInPage++;
				}
				else 
				{
					//-- проверка на заверщение режима дублирования
					if ( (Line->Attr & aDUPLICATE) == aDUPLICATE && Line->Data[ 1 ] == '0' ) // &0? -- завершить...
					{
						NeedCancelDup = TRUE;
						cuttedLineInPlat = 0;	
						FitLine++;
    					LineInBlock--;
                        // if (DBG_MODE) LogAddLine("Break DupMode cancel");
						break;
					}
                    // ВНИМАНИЕ! При дублирующем режиме страница обрывается только если
                    // пустых строк 2 и больше -----------+
                    //                                    v
					else if ( DupMode && EmptyLineTotal > 1 && EmptyLine && EmptyLine->Font == NULL && !NeedCancelDup )
					{
						NeedCancelDup = TRUE;
						cuttedLineInPlat = 0;
                        // if (DBG_MODE) LogAddLine("Break DupMode EmptyLineTotal=%d", EmptyLineTotal);
						break;
					}
				}

				EmptyLineTotal = 0;		// 
				EmptyLine = NULL;
			}
			else
			{
				EmptyLineTotal++;
				EmptyLine = Line;
			}
            
			cuttedLineInPlat++;
			FitLine++;
    		LineInBlock--;

            if ( Line->Attr & aFORMFEED ) 
                FormFeed = TRUE;

			Line = Line->Next;
            if (DBG_MODE) LogAddLine("Line next LineInBlock <%d> FormFeed <%d>", LineInBlock, FormFeed);
		}
        else
            break;
	} // while ( LineInBlock...

	// проверим, оборвалась платёжка? 
	if ( ( DupMode || ( oldDupMode && DupMode == 0 ) ) && 
		 Line && Line->Len && PlatInPage > 1 && cuttedLineInPlat )
        FitLine -= cuttedLineInPlat;

    // if (DBG_MODE) LogAddLine("FitLine=%lu\tcutterLineInPlat=%lu\tTryFitToPage function STOP", FitLine, cuttedLineInPlat);

	return FitLine;
}

/**
* фун-ция проверяет войдёт ли след. HEADER+DATA на страницу...?
* return: кол-во линий которые войдут....
*/
DWORD TryFitNextBlockToPage(PrintParam_t *P)
{
    // сохраним страничку девственно не тронутой...
    TextBlock_t *oldBlk = P->BlkCur;
    DWORD oldFree = P->Paper.FreeHigh;
    DWORD pInterval = P->Paper.LineInterval;
    //
	bool NeedAppend = false;	// когда встречается вподряд ~~, значит просто смена шапки

	DWORD FitLine = 0; 
	DWORD LineInBlock = oldBlk->LineInBlock;
	TextLine_t *Line = oldBlk->BlockData;
    BOOL DataPresent = (( oldBlk->Attr & bDATA ) == bDATA);
	while ( LineInBlock )
	{
		DWORD Intrvl = GetInterval( Line );
		if ( Intrvl )	
			pInterval = Intrvl;

		if ( Line && Line->Attr & aHEAD_BEGIN && !NeedAppend )
			NeedAppend = ( Line->Len > 1 && Line->Data[1] == '~' );

		DWORD LineHigh = pInterval;
		if ( !oldFree || LineHigh <= oldFree )
		{
            if ( oldFree > 0 )
			    oldFree -= LineHigh;
			Line = Line->Next;
			LineInBlock--;
			FitLine++;
            if ( !LineInBlock )
            {
                if ( DataPresent )
                    break;

                if ( oldBlk->Attr & ( bFIRST | bHEADER ) )
                {
                    oldBlk = oldBlk->Next;
                    if ( !oldBlk )
                        break;

                    DataPresent = ( ( oldBlk->Attr & bDATA ) == bDATA);

                    Line = oldBlk->BlockData;
                    LineInBlock = oldBlk ->LineInBlock;
                }
                else
                    break;
            }
		}
		else
			break;
	}

    return ( LineInBlock && !NeedAppend ) ? 0 : FitLine;
}


/**
* возращает высоту блока подписи...
*/
DWORD GetSignBlockHigh(PrintParam_t *P)
{
	if ( P->Sign == NULL || P->Sign->LineInBlock == 0 )
		return 0;

	DWORD DefInt = P->Paper.LineInterval;
	DWORD BlockHigh = 0;
	TextLine_t *Line  = P->Sign->BlockData;
	DWORD LineInBlock = P->Sign->LineInBlock;
	while ( LineInBlock )
	{
		DWORD Interval = GetInterval( Line );
		if ( Interval>0 )	
			DefInt = Interval; // изменился
		BlockHigh += DefInt;

		LineInBlock--;
	}

	return BlockHigh;
}

/**
* пока есть строчки на листе, будем добавлять...
* здесь анализируем только межстрочные интервалы...
* @return  true  - сформированна страница  false - ничего больше нет
*/
BOOL BuildPageForPrint(PrintParam_t *P, WORD &Dup, bool &isLastPage, bool &NeedInit )
{
    if (DBG_MODE) LogAddLine("BuildPageForPrint function START");
    
	isLastPage = false;
	NeedInit = false;
    P->AppendNextBlockToCurPage = FALSE;

	//-- заполним страницу по высоте нужным кол-вом строчек...
	P->BlkCur = P->Doc;

	//-- блок подписи???
	DWORD SignHigh = 0;
	if ( P->BlkCur && ( P->BlkCur->Attr & bSIGN ) == bSIGN )
	{
		P->Sign = P->BlkCur;
		if ( (P->BlkCur->Attr&bNEEDINIT)==bNEEDINIT )
			NeedInit = true;
		P->BlkCur = P->BlkCur->Next;
		SignHigh = GetSignBlockHigh( P );
	}

    
    // установим свободную высоту...:)
	if ( !P->AppendToCurPage || ( P->AppendToCurPage && P->Paper.FreeHigh < SignHigh ) ) //NEW v0.50 добавлен полностью if
    {
        P->Paper.FreeHigh = GetPaperFreeHigh( P ) - SignHigh;
    }

    LogAddLine("SignHigh=%i AppendToCurPage=%i FreeHigh=%i",SignHigh,P->AppendToCurPage,P->Paper.FreeHigh);
    
	TextBlock_t *prevBlk = NULL;
	BOOL DataPresent = false;
    BOOL FormFeed = false;
	BOOL LastDataInBlock = false;
	while ( P->BlkCur ) 
	{
        DataPresent = ( ( P->BlkCur->Attr & bDATA ) == bDATA );

		if ( P->BlkCur->Attr & ( bFIRST | bHEADER | bDATA | bTEXT ) )
		{
			//-- инициализируем заново...
			if ( (P->BlkCur->Attr&bNEEDINIT)==bNEEDINIT )
				NeedInit = true;

            //-- кол-во линий помещённых из блока...
			DWORD NumFitLines = TryFitToPage( P, Dup, FormFeed, NeedCancelDup );
			if ( NumFitLines == 0 ) // 0-х блоков быть не может...
			{
				isLastPage = true;
				break;
			}
			else
			{
				//-- проверка на то, что шапку тоже печатать последний раз...
				if ( DataPresent && prevBlk && 
					 ( prevBlk->Attr & ( bFIRST | bHEADER ) ) && 
					 NumFitLines == P->BlkCur->LineInBlock )
				{
					LastDataInBlock = true;
					BlkClearLineID( aPRINT, prevBlk, P->Paper.LineInPage );
					BlkSetLineID( aPRINT_ONCE, prevBlk, P->Paper.LineInPage );
				}

				if (DBG_MODE) LogAddLine( "Fited for print:%lu", NumFitLines);
				DWORD Attr = ( ( P->BlkCur->Attr & bHEADER ) && P->BlkCur->Next ) ? aPRINT : aPRINT_ONCE;
				BlkSetLineID( Attr, P->BlkCur, NumFitLines );

                // проверка чтобы в случае режима DUPLICATE 
                // а режим дупликэйт только в bTEXT -)
                // линия с автошрифтов попала в каждую страницу...
                // или если не auto, то чтобы всё до &n попало...
                if ( ( P->BlkCur->Attr & bTEXT ) && NumFitLines < P->BlkCur->LineInBlock )
                {
                    TextLine_t *Prv; // 
                    TextLine_t *Du, *Beg = P->BlkCur->BlockData;
                    if ( ( Du = ActionFind( aDUPLICATE, Beg ) ) != NULL && Dup && !NeedCancelDup )
                    {
                        // повторить всё до &2 включая саму строку, 
                        // для того, что печатать не один раз...
                        do {
                            if ( Beg->Attr & aPRINT_ONCE ) 
                            {
                                Beg->Attr &= (~aPRINT_ONCE); //сбросим что единожды...
                                Beg->Attr |= aPRINT;
                            }

                            Prv = Beg;
                            Beg = Beg->Next;
                        } while ( !( Prv->Attr & aDUPLICATE ) );
                    }
                }
				P->Paper.LineInPage += NumFitLines;
			}
		}

		// последняя страница???
		if ( ( DataPresent && P->BlkCur->Next == NULL && LastDataInBlock ) ||
			 ( P->BlkCur->Next == NULL && P->Paper.LineInPage  == P->BlkCur->LineInBlock && (P->BlkCur->Attr&bTEXT)!=0) )
			isLastPage = true;

        // заполнили за один проход всю страницу.......?
        if ( P->BlkCur->LineInBlock > P->Paper.LineInPage || FormFeed )
            break;

		// запомним последний...
		prevBlk = P->BlkCur;
		P->BlkCur = P->BlkCur->Next;

        // проверка, на конец заполнения страницы...
		if ( DataPresent && P->BlkCur && ( P->BlkCur->Attr & ( bFIRST | bHEADER | bTEXT | bSIGN ) ) )
        {
            if ( P->Paper.H )
            {
                P->AppendNextBlockToCurPage = TryFitNextBlockToPage( P );
                if (DBG_MODE) LogAddLine( "Build New page: %d", P->AppendNextBlockToCurPage);
            }
			break;
        }

	} // while

	//-- блок подписи присоединим...
	if ( P->Doc && P->Sign && P->Paper.LineInPage )
		BlkSetLineID( ( LastDataInBlock ) ? aPRINT_ONCE : aPRINT, P->Sign, P->Sign->LineInBlock );

	// теперь проверим на aSUMCOL линии попавшии для печати...
	if ( P->Doc ) 
	{

		DWORD LineInPage = P->Paper.LineInPage;

		TextBlock_t *B = P->Doc;
		if ( B == P->Sign ) B = B->Next;
		TextLine_t *L = B->BlockData, *LastLineForPrint = NULL;
		BOOL SUMCOL = FALSE, AddITOGO = TRUE;

		while ( LineInPage ) 
		{
			if ( !L )
			{
				B = B->Next;
				L = B->BlockData;
			}

			// поиск сложения...
			if ( ( L->Attr & aPRINT ) == aPRINT && ( L->Attr & aSUMCOL ) == aSUMCOL )
				SUMCOL = TRUE;

			// поиск итого...
			if ( SUMCOL && ( L->Attr & aPRINT_ONCE ) == aPRINT_ONCE ) 
			{
				LastLineForPrint = L;
				if ( ( L->Attr & aTOTAL ) == aTOTAL )
				{
					AddITOGO = FALSE;
					break;
				}
			}
			if ( !B ) 
				break;
			L = L->Next;
			LineInPage--;
		}

		// решение об удалении линии..
		if ( SUMCOL && AddITOGO && P->Paper.LineInterval > P->Paper.FreeHigh )
		{
			if ( LastLineForPrint != NULL )
				LastLineForPrint->Attr = ( LastLineForPrint->Attr & (~aPRINT_ONCE) );
			P->Paper.LineInPage--;
		}
	}

    if (DBG_MODE) LogAddLine("Paper line in page = %i\tBuildPageForPrint function STOP",P->Paper.LineInPage);

	return ( P->Paper.LineInPage != 0 ) ? TRUE : FALSE;
}

/**
* копируем страничку в отделный блок...aPRINT или aPRINT_ONCE
*/
BOOL PageCopy(PrintParam_t *P, BOOL DupMode )
{
    if (DBG_MODE) LogAddLine("PageCopy function START");
    
	DWORD NeedLinePrint = P->Paper.LineInPage;
    BOOL EmptyPresent = TRUE;

	TextBlock_t *Blk = P->Doc;
	//-- блок подписи?
	if ( P->Doc == P->Sign && P->Doc != NULL )
		Blk = P->Doc->Next;

	TextLine_t *L = Blk->BlockData;

	while ( NeedLinePrint )
	{
        // закончились линии в текущем блоке? 
        // переход на следующий...
		if ( !L )
		{
			Blk = Blk->Next;
			L = Blk->BlockData;
		}

        // линия помечена для печати?
		if ( L && L->Attr & ( aPRINT | aPRINT_ONCE ) )
		{
            // сдублируем линию...
			TextLine_t *Dup = DupLine( L );
			if ( !Dup ) // not enough memory
            {
                if (DBG_MODE) LogAddLine("WARNING: PageCopy function STOP not enough memory");
				return FALSE;
            }

            if ( DupMode ) LineTrimRightSpace( Dup ); // обрубим пробелы справа...
            
            // добавление линии к странице...
			if ( !P->Page )
            {
				P->Page = Dup;
            }
			else 
			{
				TextLine_t *S = P->Page;
				while ( S->Next ) S = S->Next;
				S->Next = Dup;
			}

			NeedLinePrint--;
		}

		if ( !Blk ) 
			break;

		L = L->Next;
	}
	
    if (DBG_MODE) LogAddLine("PageCopy function STOP");
    
	return TRUE;
}

/**
* добавим блок подписи в конец листа P.Page
*/
BOOL SignCopy(PrintParam_t *P)
{
	unsigned char *tmp;
	if ( !P->Sign ) 
		return FALSE;

	TextLine_t *L = P->Sign->BlockData;

	while ( L )
	{
        // линия помечена для печати?
		if ( L && L->Attr & ( aPRINT | aPRINT_ONCE ) )
		{
            // сдублируем линию...
			TextLine_t *Dup = DupLine( L );

			tmp=Dup->Data;
			if(tmp[0]=='+' && tmp[1]=='+') // есть сочетание '++'
				propis(	Dup);
			
				

			if ( !Dup ) // not enough memory
				return FALSE;

            // добавление линии к странице...
			if ( !P->Page )
				P->Page = Dup;
			else 
			{
				TextLine_t *S = P->Page;
				while ( S->Next ) S = S->Next;
				S->Next = Dup;
			}
		}

		L = L->Next;
	}

	P->Paper.LineInPage += P->Sign->LineInBlock;
	
	return P->Sign != NULL;
}

/**
*
*/
void DoSumColumns(PrintParam_t *P)
{
	char flag='T';
	TextLine_t *Header = ActionFind( aSUMCOL, P->Page );
	if ( !Header || !Header->Next )
		return;

	// если есть линия с '+' и блок данных...
	// разберёмся с линией ИТОГО ПО ЛИСТУ:
	TextLine_t *ITOGO = NULL;
	TextLine_t *Total = ActionFind( aTOTAL, Header ); 
	if ( Total ) // есть такая линия... Итого по листу:
	{
		if ( Total->Data )
			delete []Total->Data;
		ITOGO = Total; 
		ITOGO->Attr = aPRINT_ONCE;
	}
	else  // нет линии "Итого по листу"
	{ 
		TextLine_t *T = P->Page;
		while ( T->Next ) T = T->Next;
		ITOGO = new TextLine_t;
		//VERIFY( ITOGO != NULL );
		ITOGO->Font = NULL;
		ITOGO->Next = NULL;
		T->Next = ITOGO;  
		ITOGO->Attr = aSKIP;
		flag='F';  // Итого не печатать
	}
	ITOGO->Len = Header->Len;
	ITOGO->Data = new BYTE[ Header->Len + 1 ];
	//VERIFY( ITOGO->Data != NULL );
	memset( ITOGO->Data, ' ', Header->Len );
	ITOGO->Data[ Header->Len ] = 0;

	// копируем "Итого по листу:" в строчку...
	{
		char *I = GetStr( IDS_ITOGO_PO_LISTU );

		WORD Ofs = 0;
		if ( Header->Len > 1 &&  Header->Data[1] == ' ' )
		{
			Ofs++;
			while ( Ofs + strlen( I ) < ITOGO->Len && Header->Data[ Ofs ] == ' ' )
				Ofs++;
		}

		if ( Ofs + strlen( I ) < ITOGO->Len )
			memmove( ITOGO->Data + Ofs, I, strlen( I ) );
	}

	// найдём все колонки в которых надо суммировать данные...
	for ( DWORD ColBegin = 0; ColBegin < Header->Len; ColBegin++ )
	{
		if ( Header->Data[ ColBegin ] != '+' )
			continue;

		// уберём знак '+' в колонке...
		Header->Data[ ColBegin ] = Header->Data[ ColBegin + 1 ];

		// посчитаем ширину колонки...
		WORD ColWidth = 0;
		char C;
		while ( ( C = Header->Data[ ColBegin + ColWidth ] ) != 0 && C != ' ' )
			ColWidth++;
		
		char *ColData = new char[ ColWidth + 1 ]; // строчка для числа из колонки.
		//VERIFY( ColData != NULL );
		__int64 ColTotal = 0; // сумма по колонке...
		__int64 ColCur;		  // текущее число ...	

		DWORD AddedCol = 0;

		// позиции символов заполнителей...
		char PointPos = -1;
		char CommaPos = -1;
		char CommaChar = 0;

		// отступ справа
		char RIndent = -1;
		BOOL Digits = FALSE;

		// на следующию строчку после шапки...
		TextLine_t *L = Header->Next;

		// проссумируем всю колонку до линии итого по листу...
		while ( L != ITOGO )
		{
			if ( L && L->Len > ColBegin )
			{
				ColCur = 0;
				memset( ColData, 0, ColWidth + 1 );

				// прочитаем в строчку значение колонки
				strncpy( ColData, (char *)&L->Data[ ColBegin ], ColWidth );

				while ( strlen( ColData ) < ColWidth ) strcat( ColData, " " );

				WORD i = 0;
				do {
                    // вырезаем из строчки целую часть числа...
					if ( !strlen( ColData ) )
						break;
					else
					{
						if ( ( i == 0 && strchr( "+-0123456789", ColData[ i ] ) ) ||
							 ( i > 0 && strchr( ".,'0123456789", ColData[ i ] ) ) )
						{
							if ( isdigit( ColData[ i++ ] ) && !Digits )
								Digits = TRUE;
						}
						else
						{
							if ( ColData[ i ] == ' ' )
							{
								if ( Digits && ColTotal == 0 && RIndent == -1 ) 
								{
									int spc = i;
									RIndent = 0;
									while ( ColData[ spc ] && ColData[ spc++ ] == ' ' )
										RIndent++;
								}
								DelChar( ColData, i, 1 );
							}
							else
								ColData[ 0 ] = 0;
						}
					}

				} while ( i < strlen( ColData ) );

				if ( strlen( ColData ) )
				{
                    // теперь вырезаем дробную часть числа...
					char *Point = strchr( ColData, '.' );
					if ( !Point ) 
						i = strlen( ColData );
					else
					{
						i = Point - ColData;
						if ( PointPos == -1 ) PointPos = strlen( ColData ) - ( i + 1 );
					}

					WORD j = i;
					while ( i && strlen( ColData ) )
					{
						if ( ColData[ i ] == ',' || ColData[ i ] == 0x27 )
						{
							if ( CommaPos == -1 )
							{
								CommaPos = j - i - 1;
								CommaChar = ColData[ i ];
							}
							DelChar( ColData, i, 1 );
						}
						if ( ColData[ i ] == '.' )
							DelChar( ColData, i, 1 );
						else
							i--;
					}
					// 	LogAddLine( "Число в колонке: [%s]", ColData ); getch();
					//ColCur = _atoi64( ColData );
					ColCur=atoi64( ColData );
					ColTotal += ColCur;
				} // if (strlen...
			} // if
			else
			if ( !L )
				break;

			AddedCol++;
			
			L = L->Next;
		} // while

		// число в строчку...
		sam_itogo=ColTotal;
		//_i64toa( ColTotal, ColData, 10 );
		i64toa(ColTotal,ColData);

		// добавим сумму по колонке в линию ИТОГО...
		int oldComma = CommaPos;
		int old = ( PointPos <= 0 ) ? 0 : PointPos - 1;

		if ( RIndent == -1 || RIndent > ColWidth )
			RIndent = 0;

		// добавим ведущие нули, чтобы 0.01 отображалось правильно...
		int needZero = ( PointPos>0 ) ? PointPos : ((CommaPos>0&&PointPos<0)?CommaPos:0);
		if ( needZero>0 && needZero<ColWidth && strlen( ColData ) <= needZero )
		{
			char tmp[ 1024 ];
			tmp[0]=0;

			// добавим нули ведущие...
			int i=(strlen( ColData ) == needZero ) ? 1:0;
			for ( ; i<needZero; i++ )
				strcat( tmp, "0" );

			strcat( tmp, ColData );
			strcpy( ColData, tmp );
		}

		DWORD LastNum = ColBegin + ColWidth - RIndent; // - ( ( old ) ? 1 : 0 );
		int j = 0;
		int i = strlen( ColData );
		while ( ColWidth - j > 0 && i )
		{
			// для точки...
			if ( PointPos >= 0 )
			{
				if ( PointPos == 0 )
				{
					j++;
					ITOGO->Data[ LastNum - j ] = '.'; 
				}
				PointPos--;
			}

			// для запятушки...
			if ( oldComma > 0 && PointPos < 0 )
			{
				if ( CommaPos == 0 )
				{
					CommaPos = oldComma;
					if ( i-1>=0 && strchr( "0123456789", ColData[ i - 1 ] )!=NULL )
					{
						ITOGO->Data[ LastNum - j - 1 ] = CommaChar;
						j++;
					}
				}
				CommaPos--;
			}

			// число...
			ITOGO->Data[ LastNum - j - 1 ] = ColData[ --i ];
			j++;
		} // while
		if ( ColData )
        {
            if (DBG_MODE) LogAddLine("DoSumColumn function ColData: '%s' ColWidth: %i",ColData,ColWidth);
			delete []ColData;
        }
	} // for
}

/**
* заменять в колонке повторяющиеся названия если в конце колонки есть '!'
*/
void DoRepeatName(PrintParam_t *P)
{
    const char *FILL_PATTERN = "\xFA";  // точка по середине
    const WORD FILL_STEP = 3;			// каждые FILL_STEP

	TextLine_t *Header = ActionFind( aSKIP_NAME, P->Page);
	if ( Header && Header->Next && ( Header->Next->Attr & aPRINT_ONCE ) == aPRINT_ONCE )
	{
		// найдём все колонки в которых надо пропустить одинаковые названия
		for ( DWORD i = 0; i < Header->Len; i++ )
		{
			if ( Header->Data[ i ] != '!' )
				continue;

			// заменим знак '!' в колонке, не предыдущий символ...
			Header->Data[ i ] = Header->Data[ i - 1 ];

			// найдём начало колонки
			DWORD ColBegin = i; 
			while ( ColBegin && Header->Data[ ColBegin ] != ' ' ) ColBegin--;
			if ( Header->Data[ ColBegin ] == ' ' ) ColBegin++;
			
			// ширина колонки
			DWORD ColWidth = i - ColBegin + 1; 

			// установим L на следующию строчку после шапки...
			TextLine_t *L = Header->Next;

			// данные для первой строчки...
			char *ColData = new char[ ColWidth + 1 ];
			memset( ColData, 0, ColWidth + 1 );

			// данные для повтороющийся
			char *ColCur = new char[ ColWidth + 1 ];
			memset( ColCur, 0, ColWidth + 1 );

			// прочитаем в строчку значение колонки
			strncpy( ColData, (char *)&L->Data[ ColBegin ], ColWidth );
			DelAllSpace( ColData );

			// в первой строчке надо всё оставить как есть, проверку со следующей...
			L = L->Next;
			
			// проанализируем всю колонку на одинаковые значения поля...
			while ( L )
			{
				strncpy( ColCur, (char *)&L->Data[ ColBegin ], ColWidth ); // текущая колонка
				DelAllSpace( ColCur );

				// если не равно предыдущему, то обновить...
				if ( strcmp( ColData, ColCur ) ) 
					strcpy( ColData, ColCur );
				else 
				if ( strlen( ColData ) )
				{
					// линия меньше начало колонки для повтора?
					if ( ColBegin+ColWidth > L->Len ) PadRightSpace( L, Header->Len ); // добавим справа пробелов до ширины шапки

					// ровно предыдущей, заменить на FILL_PATTERN... 
					strnset( (char *)&L->Data[ ColBegin ], ' ', ColWidth );
					DWORD Len = strlen( (char *)&L->Data[ ColBegin ] );
					Len = ( Len > ColWidth ) ? ColWidth : Len;
					for ( WORD j = 1 ; j < Len; j += FILL_STEP )
						memmove( &L->Data[ ColBegin + j ], FILL_PATTERN, strlen( FILL_PATTERN ) );
				}
				L = L->Next;
			}

			delete []ColCur;
			delete []ColData;

		}
	}
}

/**
*   Подбор автоматического шрифта
*/
void DoAutoShrift(PrintParam_t *P, WORD DupMode )
{
	TextLine_t *A = ActionFind( aAUTOMODE, P->Page );
	if ( !A || !A->Next ) 
		return;

	//-- удалим '^'
    if ( A->Len && A->Data[0]=='^' )
    {
        DelChar( (char *)A->Data, 0, 1 ); 
        A->Len--;

        if ( A->Font )
        {
            TextFont_t *F = A->Font;
            while ( F )
            {
                if ( F->Pos ) F->Pos--;
                F = F->Next;
            }
        }
    }

    if ( !DupMode )
        DupMode = 1;

	//-- найдём линию с '@'
	TextLine_t *H = ActionFind( aHEAD_END, P->Page );
	DWORD MaxLen = ( H ) ? H->Len : ( FindMaxLen( P->Page ) * DupMode );
	DWORD FreeW = GetPaperFreeWidth( P );

    //Искуственно увеличим ширину линии
    if (DBG_MODE) LogAddLine("DoAutoShrift:\tFreeW=%u\tMaxLen=%u", FreeW, MaxLen);
    
	//-- весь документ входит простым шрифтом без деления на части?
	for ( WORD i = 0; i < 4; i++ )
	{

		if ( ( MaxLen * Fonts[ i ].dX ) <= FreeW )
		{
			P->AutoShrift = Fonts[ i ].dX;
            if (DBG_MODE) LogAddLine("%u. AutoShrift:\t%u", i, P->AutoShrift);
			return;
		}
	}

	if ( !H ) // нет шапки, режим отменяется!!!
		return;

	//-- теперь попробуем разбивку страницы с частями...
	DWORD PartLen = MaxLen;
	WORD NumParts = 1;
	while ( 1 )
	{
		for ( WORD i = 0; i < 4; i++ )
		{
            //-- самым узким шрифтов не входит? 
            //   увеличиваем кол-во частей и заново...
			if ( ( PartLen * Fonts[ i ].dX ) > FreeW )
			{
				if ( i == 3 ) NumParts++;
			}
			else
			{
				P->AutoShrift = Fonts[ i ].dX;
				return;
			}
		}

        // пытаемся поделить на предпологаемое кол-во частей...
		do {
			BOOL SpaceFound = FALSE;

			//-- найдём конец колонки, чтобы колонка полностью вошла на лист...
			for ( PartLen = MaxLen / NumParts; PartLen < MaxLen && !SpaceFound; PartLen++ )
				SpaceFound = ( H->Data[ PartLen ] == ' ' );

			// не входит? увеличим кол-во частей...
			if ( !SpaceFound )
			{
				NumParts++;
				PartLen = 0;
                
                //ВНИМАНИЕ! Проверка чтобы не возникало деления на 0
                if (NumParts == (WORD_MAX-1))
                    break;
			}
		} while ( PartLen == 0 );
	}
}

/**
* нумерация страниц...
*/
void DoNumeration(PrintParam_t *PP)
{
	TextLine_t *N = ActionFind( aNUM, PP->Page);
	if (!N) 
        return;

	char *P = strchr((char *)N->Data, '#');
	if (!P) 
        return; // ??? странно, так вроде быть не должно...
	*P = 0;

	char PageNum[ 6 ]; 
    sprintf(PageNum, "%d", PP->CurPageForPrint);

	DWORD NewLen = N->Len + strlen(PageNum) - 1; // -1 без символа решётка...

	BYTE *NewLine = new BYTE[NewLen + 1];
	if ( !NewLine ) return; // not enough memory...

	memset( NewLine, 0, NewLen + 1 );

	// скопируем до символа '#'...
	P = (char *)N->Data;
	for (DWORD p = 0; *P; p++)
		NewLine[p] = *P++;

	// скопируем номер страницы...
	strcat((char *)NewLine, PageNum);

	// скопируем после номера страницы...
	if (*(++P))
		strcat( (char *)NewLine, P );	

	delete []N->Data;
	N->Data = NewLine;
	N->Len = NewLen;
}

/**
* вставляет дату и время...
*/
void DoDateTime(PrintParam_t *P)
{
	TextLine_t *A = ActionFind( aAUTOMODE, P->Page );
	if ( A ) A = A->Next; // на следующию в строке с ^ может быть дата и время

	TextLine_t *D = ActionFind( aDATETIME, P->Page);
	if ( !D ) return;

	//-- если автошрифт есть, то проверит чтобы в автошрифте Время и Дата до него должна быть
	while ( A )
	{
		if ( A == D )
			return;
		else
			A = A->Next;
	}

	char *Dat = strchr( (char *)D->Data, '\\' );
	if ( !Dat || 
		 ( Dat == (char *)D->Data && Dat[1]!=' ' && Dat[1]!=0 ) ||						// в начале строчке после пробел
		 ( Dat >  (char *)D->Data && Dat[1]!=' ' && *(Dat-1) != ' ')	// в середине до и после пробел
	   ) return; 

	*Dat = 0;

	time_t loc_time;
	time( &loc_time );

	tm *today = localtime( &loc_time );

	char DateTime[ 128 ]; 
	strftime( DateTime, 128, "%d/%m/%y (%H:%M)", today );

	DWORD NewLen = D->Len + strlen( DateTime ) - 1; // -1 без символа '\'

	BYTE *NewLine = new BYTE[ NewLen + 1 ];
	if ( !NewLine ) return; // not enough memory...

	memset( NewLine, 0, NewLen + 1 );

	// скопируем до символа '\'...
	Dat = (char *)D->Data;
	for ( DWORD p = 0; *Dat; p++ )
		NewLine[ p ] = *Dat++;

	// скопируем время дату...
	strcat( (char *)NewLine, DateTime );

	// скопируем после '\'
	if ( *(++Dat) )
		strcat( (char *)NewLine, Dat );	

	delete []D->Data;
	D->Data = NewLine;
	D->Len  = NewLen;
}

/**
* может изменять P.AutoShrift...if DupMode > 0 && !autoMode;
*/
BOOL DoDuplicate(PrintParam_t *P, WORD &DupMode )
{
    if ( !DupMode )
        return FALSE;

	TextLine_t *L = P->Page;
	TextLine_t *D = ActionFind( aDUPLICATE, L );
    if ( !D || !L )
        return FALSE;

    if ( D->Data && D->Len >= 2 && D->Data[ 0 ] == '&' && strchr( "0123456789", D->Data[ 1 ] ) )
    {
	    DelChar( (char *)D->Data, 0, 2 ); 
	    D->Len -= 2;
        if ( !D->Len )
        {
            delete []D->Data;
            D->Data = NULL;
        }
    }

    DWORD MaxLen = FindMaxLen( L ); // найдём максимально ширукую строчку на листе

    // проверка, на то войдёт ли столько колонок, сколько попросили?
    DWORD AutoShrift = 0;
    if ( P->AutoShrift ) 
        AutoShrift = P->AutoShrift;
    else
    {
        TextLine_t *Beg = L;
        while ( Beg != D )
        {
            TextFont_t *F = Beg->Font;
            while ( F )
            {
                AnalyzeFont(P,F,0);
                F = F->Next;
            }
            Beg = Beg->Next;
        }
        // установим... 
        P->AutoShrift = AutoShrift = P->EFont.dX;
    }

    DWORD FreeW = GetPaperFreeWidth( P );
    while ( DupMode && ( AutoShrift * ( MaxLen * DupMode ) ) > FreeW )
        DupMode--;

    if ( !DupMode )
        return FALSE;

	while ( L )
	{
        // отрубим режим повтора....
        if ( ( L->Attr & aDUPLICATE ) && L->Len == 1 && L->Data[1]==0 )
        {   
            D->Attr &= ( ~aPRINT );
            D->Attr |= ( aPRINT_ONCE );
            DelChar( ( char *)L->Data, 0, 1 );
            L->Len--;
            DupMode = 0;
            break;
        }
        
        // для.............!!!!!!!!! !!!!!! !!!!!!!!!! - не дублировать часть смотри if ниже...
		if ( !( L->Attr & (aAUTOMODE|aPRINT|aDUPLICATE)) ) 
        {
            for ( int i = 1; i < DupMode; i++ )
		        InsPartLine( L, 0, MaxLen, MaxLen+1 );
        }

		L = L->Next;
	}
	return TRUE;
}

/**
*   Функция разбивки страницы по частям
*/
BOOL DoPart(PrintParam_t *P)
{
	TextLine_t *H = ActionFind( aHEAD_END, P->Page );
	
	//-- нет шапки, значит просто текст...
	if ( !H || !H->Next )
	{
		P->NumParts = 1;
		P->Parts[ 0 ] = FindMaxLen( P->Page );
		return TRUE;
	}

	DWORD Pole = 0;
	TextLine_t *R = ActionFind( aREPEAT, P->Page);
	if ( R ) 
	{
        //-- заменим на следуюший символ...
        BYTE *RSk = (BYTE *)memchr( R->Data, '}', R->Len );
        if ( RSk )
        {
            Pole = RSk - R->Data;
            *RSk = ( R->Len > Pole ) ? ( *(RSk+1) ) : ' ';
        }
	}

	DWORD MaxLen = H->Len;
	DWORD AutoShrift = ( P->AutoShrift ) ? P->AutoShrift : Fonts[ 0 ].dX;
	DWORD FreeW = GetPaperFreeWidth( P );
	DWORD PartW = FreeW;

	if ( MaxLen * AutoShrift <= PartW )
	{
		P->NumParts = 1;
		P->Parts[ 0 ] = MaxLen;
	}
	else
	{
		BOOL Done = FALSE;
		DWORD Part = 0, Field = 0;

		while ( MaxLen )
		{
			if ( !Done )
			{
				Part = ( ( PartW / AutoShrift ) - Field ) + 1;
				if ( Part > MaxLen ) 
					Part = MaxLen;
				
				// подсчитаем начало следующей части...
				DWORD Begin = 0;
				for ( DWORD i = 0; i < P->NumParts; i++ )
					Begin += P->Parts[ i ];

				// коррекция начала следующей части если есть повторяющееся скобка...
				if ( P->NumParts > 1 && Field )
					Begin -= ( Field * ( P->NumParts - 1 ) );

                // включаем _только_ полную колонку...
				if ( ( Part + Field ) * AutoShrift > PartW )
                {
                    // ВНИМАНИЕ!
                    // в данном цикле происходит контроль включения 
                    // только полной колонки при разбивке листа на части
                    DWORD PartBak = Part;
					while ( Begin + Part > Begin && H->Data[ Begin + Part ] != ' ' )
					{
                        //if (DBG_MODE) LogAddLine("DoPart:\tPart=%u\tField=%u\tPartW=%u\tBegin=%u\t<%c>", Part, Field, PartW, Begin, H->Data[ Begin + Part ]);
						Part--;
						if ( Part <=0 )
                        {
							Part = PartBak;
							break;
                        }
					}
				} 
			}

            //--- проверка, если с данным режимом ^ } отчёт не помешается на данную бумагу...
            // в том случае, если Part == 0
            if ( Part == 0 )
            {
                if ( P->AutoShrift == 0 )
                    return FALSE; // всё, реально не могу напечатать...
                else
                if ( P->AutoShrift > Fonts[ 3 ].dX )
                {
                    for ( int i = 0; i < 4; i++ )
                        if ( Fonts[ i ].dX == P->AutoShrift )
                        {
                            if ( i == 3 ) return FALSE;
                            P->AutoShrift = Fonts[ i+1 ].dX;
                            break;
                        }
                    if ( Pole )
                        R->Data[ Pole ] = '}';
                    P->NumParts = 0;
                    return DoPart( P );
                }
            }

			MaxLen -= Part;

			P->Parts[ P->NumParts ] = Part + Field;
			P->NumParts++;

			Field = Pole;
		}

		//---  размножить на странице колонку...
		if ( Field )
		{
			TextLine_t *L = R;

			while ( L )
			{
				for ( WORD i = 0; i < P->NumParts; i++ )
				{
					DWORD Pos = 0;
					for ( WORD j = 0; j < i; j++ )
						Pos += P->Parts[ j ];

					if ( i ) //для всех, кроме 0==первая часть...
                    {
						InsPartLine( L, 0, Field, Pos );
                        if ( L->Data[ Pos ] == '@' && L->Data[ Pos+1 ] )
                            L->Data[ Pos ] = L->Data[ Pos+1 ];
                    }
				}

				if ( L && ( L->Attr & aREPEAT ) == aREPEAT && L != R )
                {
                    if ( L->Data[ Pole ] == '}' )
					{
						if ( L->Data[ Pole+1 ] != 0 )
							L->Data[ Pole ] = L->Data[Pole+1];
						else
							L->Data[ Pole ] = ' ';
					}
					break;
                }

				L = L->Next;
			}
		}

	}

    return TRUE;
}

/**
* заменим '@' и '~' на следующий символ...
*/
void ChangeServiceChars(PrintParam_t *P )
{
    TextLine_t *H = ActionFind( aHEAD_END, P->Page );
    if ( H && H->Data ) H->Data[ 0 ] = H->Data[ 1 ];

    H = ActionFind( aHEAD_BEGIN, P->Page );
    if ( H && H->Data ) H->Data[ 0 ] = H->Data[ 1 ];

	H = P->Page;
	while ( H )
	{
		if ( (H->Attr & aDUPLICATE) && H->Data && H->Len > 1 )
		{
			if ( H->Data[0] == '&' && H->Data[1] == '0' )
				H->Data[0]=H->Data[1]='*';
		}

		if ( (H->Attr & aHEAD_BEGIN ) && H->Data && H->Len > 1 )
		{
			if ( H->Data[0] == '~' && H->Data[1] == '~' )
				H->Data[0]=H->Data[1]=' ';
		}

        //Удалить символы смены ориентации из печатных символов
        //символ смены ориентации на портретную
		if ( (H->Attr & aPORTRAIT_ORIENTATION ) && H->Data && H->Len > 1 )
		{
			if ( H->Data[0] == '[' && H->Data[1] == ' ' )
				H->Data[0]=H->Data[1]=' ';
		}

        //символ смены ориентации на ландшафтную
		if ( (H->Attr & aLANDSCAPE_ORIENTATION ) && H->Data && H->Len > 1 )
		{
			if ( H->Data[0] == ']' && H->Data[1] == ' ' )
				H->Data[0]=H->Data[1]=' ';
		}

		H = H->Next;
	}
}

/**
* @return True если в блоке данных не только пустые линии...
*/
BOOL HaveDataInPage(PrintParam_t *P)
{
	DWORD OnceLineInData = 0;
	DWORD EmptyLineInData = 0;
	TextLine_t *L = P->Page;
	
	while ( L ) 
	{
		// поиск пустых линий...
		if ( ( L->Attr & aPRINT_ONCE ) == aPRINT_ONCE ) 
		{
			OnceLineInData++;
            DWORD spc=0;
            DWORD i=0;
			for ( spc = 0, i = 0; i < L->Len; i++ )
			{
				if ( L->Data[ i ] == ' ' || L->Data[ i ] == 9 || L->Data[ i ] == 0 )
					spc++;
			}

			if ( spc == L->Len )
				EmptyLineInData++;
		}

		L = L->Next;
	}

	return ( OnceLineInData > EmptyLineInData ) ? TRUE : FALSE;
}

/**
* освободить всю память во всех блоках и линии в этих блоках...
*/
BOOL ClearDocFromPrintedLineAndBlock(PrintParam_t *P)
{
	TextBlock_t *Blk = P->Doc;
	TextLine_t *L = Blk->BlockData;

	// режим подбора шрифта в bFIRST блоке...
	bool isAutoModeInFirst = false;

	// очистим линии в документе...
	while ( P->Paper.LineInPage && Blk && L )
	{
		//-- '^' в блоке bFIRST
		if ( ( Blk->Attr & bFIRST ) != 0 && ( L->Attr & aAUTOMODE ) != 0 )
			isAutoModeInFirst = true;

		if ( L->Attr & aPRINT )
		{
			P->Paper.LineInPage--;
            L->Attr &= (~aPRINT);
			L = L->Next;
			if ( !L && P->Paper.LineInPage )
			{
				Blk = Blk->Next;
				if ( Blk )
				{
					L = Blk->BlockData;
					continue;
				}

				return FALSE; //Очень странно, ещё должы быть линии а уже нет больше блоков...
			}
			else
			if ( !L )
				break;
		}
		else if ( L->Attr & (aPRINT_ONCE|aSKIP) ) // EXCEPTION WAS HERE!!!
		{
			P->Paper.LineInPage--;
			TextLine_t *NextLine = L->Next;

			if ( BlkDelLine( L, Blk ) )		// != 0 в блоке есть линии..
				L = NextLine;
			else
			{								
				DWORD pAttr = Blk->Attr;
				// нету больше в блоке линий, надо удалить блок...
				if ( Blk == P->Doc )
					P->Doc = Blk->Next;
				else
				{
					TextBlock_t *Prev = P->Doc;
					while ( Prev && Prev->Next != Blk ) Prev = Prev->Next;

					if ( Blk->Next == NULL )
						Prev->Next = NULL;
					else
						Prev->Next = Blk->Next;
				}

				delete Blk;
				Blk = P->Doc;

				if ( Blk )
				{
					L = Blk->BlockData;			// смотрим следующий блок

					// Отменяем режим AUTOSHRIFT '^' если он был включен
					if ( Blk->Attr & ( bFIRST | bHEADER | bTEXT ) )
					{
						if ( ( pAttr & bFIRST ) != 0 && ( Blk->Attr & bHEADER ) != 0 && P->AutoShrift && isAutoModeInFirst )
							L->Attr |= aAUTOMODE;
						else
							P->AutoShrift = 0;
					}
				}
				else
					L = NULL;					// Больше нет блоков
			}
		}
		else
		{
			Blk = Blk->Next;
			L = Blk->BlockData;
		}
	}

	// очистим линии в странице .Page
	L = P->Page;
	while ( L )
	{
		TextLine_t *Next = L->Next;
		DelLine( L );
		L = Next;
	}
	P->Page = NULL;

	// очистим кол-во частей...
	P->NumParts = 0;

	return TRUE;
}

/**
* проверка на то, что нет две вподряд идущих шапки, так как этого не должно быть...
*/
int isItRightDoc(PrintParam_t *P)
{
    TextBlock_t *B = P->Doc;
    WORD H = 0;

	//-- две вподряд шапки?
	while ( B )
	{
        H = ( B->Attr == bHEADER ) ? (H+1) : 0;
        if ( H > 1 )
            return 1;
		B = B->Next;
	}

	//-- только одна шапка?
	B = P->Doc;
	while ( B )
	{
		if ( ( B->Attr & bHEADER ) == bHEADER && B->Next == NULL )
			return 2;
		B = B->Next;
	}

	//-- символ '~' идёт после '@' БАГ разработчиков!
	B = P->Doc;
	while ( B )
	{
		bool cB = ( ( B->Attr & bHEADER ) == bHEADER );
		bool nB = ( B->Next && ( B->Next->Attr & bTEXT ) == bTEXT );
		if ( cB && nB )
			return 3;
		B = B->Next;
	}


    return 0;
}

void propis( TextLine_t *Dup)
{
	char *str;
	static int pos=0;
	int len,i,tmp;
	char *place;

	place=(char *) Dup->Data;
	str=propisStr( sam_itogo); //sam_itogo );
	len=strlen((char *)place); // разбить строку на несколько
	tmp=strlen(str);
	for(i=pos; (i<(len+pos))&&(i<strlen(str))  ; i++)
	{
       place[i-pos]=str[i];
	}
	place[i-pos]='\x00';

	if((len+pos)< strlen(str)) // вернуться назад и выровнять на границе слова
	{
		for(i=strlen((char *)place) ; i>pos ; i--)
		{
			if(place[i]==' ')
			{
				place[i]='\x00';
				place=trim(place);
				pos=i;
				i=-1;
			}
		}
	} else
		pos=i;
	
    Dup->Data=(unsigned char*)place;
    Dup->Len=strlen(place);

	delete str;
} 
