/**
* Модуль функций печати сформированной страницы
* @file
*/

#include "printer.h"

#include <cairo.h>
#include <cairo-pdf.h>
#include <math.h>

#pragma warning( push )
#pragma warning( disable : 4244 4018 )


int LOGX = 0;     /**<  разрешения принтера в DPI... */
int LOGY = 0;     /**<  разрешения принтера в DPI... */

/**
* ~7 миллиметров сверху/снизу принтер 
* не может печатать, в режиме быстрой печати...
* может, если Ручками открутить назад бумажку, а 
* автоматическая загрузка оставляет ~7 мм... //726-66 lines >726-65 lines
* THINK: Может сделать опцией? тогда адвансед юзер сможет сам всё настроить :)
*/
const int DEF_FAST_BORDER_HIGH  = _METRIC( 178 ); 
const int DEF_FAST_BORDER_WIDTH = _METRIC( 100 ); //500 было  // 252

#define MAX_CACHE       1024
cache_t FontCache[ MAX_CACHE ];
int totalFontCache;

#define  BUFFER_MAX     60000
/**
* для буферизации символом на печать...
*/
static char buffer[ BUFFER_MAX ];
static WORD bufferLen = 0;

static char *GetPrintFilename(char *FileName=NULL)
{
    char *result=NULL;
    char *home_path=GetHomePath();
    
    if (FileName==NULL)
        FileName=PDF_PRINT_FILENAME;

    result=(char*)calloc(strlen(home_path)+1+strlen(FileName)+1,sizeof(char));
    strcpy(result,home_path);
    strcat(result,"/");
    strcat(result,FileName);
    
    return result;
}

int bufferAdd( char c )
{
    if ( bufferLen+1<BUFFER_MAX && c != 0 )
    {
        buffer[ bufferLen ] = c;
        bufferLen++;
        buffer[ bufferLen ] = 0;
        return c;
    }
    
    return 0;
}

int bufferAddStr( char *s )
{
	if ( !s ) return 0;

	int len = strlen( s );
    if ( s && bufferLen+len<BUFFER_MAX )
    {
		memcpy( &buffer[bufferLen], s, len );
		bufferLen += len;
        buffer[ bufferLen ] = 0;
        return len;
    }
    return 0;
}

BOOL UpdateFont(PrintParam_t *P);
void bufferFlush(PrintParam_t *P)
{
    int X = P->EFontOld.x + P->EFontOld.ofsX;
    int Y = P->EFontOld.y + P->EFontOld.ofsY + P->EFontOld.dY/2;	

    // сделаем так, тосимвол '_' не может быть UnderLine..., зачем? :)
    // специальная, проверка для символа '_'
    // -40 -70 2*U etcподоброно эмпирически..., то есть метод  тыка.:)
    int UnderLineShift = ( P->EFontOld.dY / 2 )-_METRIC(10);
    if ( buffer[ 0 ] == '_' )
        Y -= (UnderLineShift- _METRIC( 148 ) ); // was 150
    else
        if ( P->EFontOld.Underline )
        {   
            // нарисуем подчёркивание...
            cairo_move_to(P->CairoEngine, X, Y+(2*UnderLineShift)-_METRIC(280));
            cairo_set_line_width(P->CairoEngine,1.0);
            cairo_line_to(P->CairoEngine, X+(P->EFontOld.dX*bufferLen), Y+(2*UnderLineShift)-_METRIC(280));
        }

    // подсчитаем пробелы...
    WORD numSpace = 0;
    for ( WORD i = 0; i < bufferLen; i++ )
        if ( buffer[ i ] == ' ' ) numSpace++;

    // напечатаем, если это не пробелы и в буффере есть данные...
    if ( bufferLen > 0 && buffer[0] && ( numSpace < bufferLen ) )
    {

        cairo_move_to(P->CairoEngine, X, Y);
        char *s=cp866_to_utf8(buffer);
        cairo_show_text(P->CairoEngine, s);
        if (DBG_MODE) LogAddLine("Print string: %s point:\tX=%i Y=%i\tx=%i y=%i\tofsX=%i ofsY=%i",s,X,Y,P->EFontOld.x,P->EFontOld.y,P->EFontOld.ofsX,P->EFontOld.ofsY);
        free(s);        
        
        //delete []lpDx;
    }
    buffer[0]=0;
    bufferLen=0;

}

/**
* @return Размер поля для стороны S...
*/
DWORD GetBorderWidth(PrintParam_t *P, PaperSide_t S )
{
    DWORD W = 0;                // ширина поля...
    int OfsX = 0, OfsY = 0;     // смещения слева, сверху
    int UnprintableArea = 0;    // физический отступ на который не может печатать
    int B = 0;                  // установленная граница пользователя

    switch( S )
    {
        case sTop:
        case sBotton:
			B = ( S == sTop ) ? P->Paper.BorderTop : P->Paper.BorderBotton;

            if ( B == 0 ) 
				B = DEF_FAST_BORDER_HIGH; 

            UnprintableArea = (float(OfsY)/float(LOGY)*INCH);
            if ( B < UnprintableArea )
                W += UnprintableArea;
            else
                W += B;
            if (DBG_MODE) LogAddLine("Get border width %i = %i %i %i ",S,W,UnprintableArea,B);
            break;

        case sLeft:
        case sRight:
			B = ( S == sLeft ) ? P->Paper.BorderLeft : P->Paper.BorderRight;
			if ( B == 0 )
				B = DEF_FAST_BORDER_WIDTH; 

            UnprintableArea = (float(OfsX)/float(LOGX)*INCH);
            if ( B < UnprintableArea )
                W += UnprintableArea;
            else
                W += B;
            break;
    }
    
    if (DBG_MODE) LogAddLine("Get border %i = %i",S,W);
    return W;
}

/**
* возвращает свободную рабочию зону для печати по вертикали...
*/
DWORD GetPaperFreeHigh(PrintParam_t *P)
{
    if ( P->CairoEngine == NULL )
    {
        if (DBG_MODE) LogAddLine("WARNING! Dont define P.CairoEngine");
        return 0;
    }
    
    
    DWORD FreeHigh = 0;
    // если рулон то сделать длину бумаги равной A4*накол-во страни для повтора
    if ( !P->Paper.H && P->Options.EpsonFastPrint )
        FreeHigh = (static_cast<DWORD>((float)29.7*(float)_METRIC( 1000 )))*P->Options.AskCorrectPaper;
    else // ниже для всей нерулонной бумаги...
    	FreeHigh = P->Paper.H-GetBorderWidth(P, sTop)-GetBorderWidth(P, sBotton);

    if (DBG_MODE) LogAddLine("GetPaperFreeHigh: Paper.H=%i Options.EpsonFastPrint=%i FreeHigh: %d ",P->Paper.H,P->Options.EpsonFastPrint,FreeHigh);

    return FreeHigh;
}

/**
* возвращает свободную рабочию зону для печати по горизонтали...
*/
DWORD GetPaperFreeWidth(PrintParam_t *P )
{
    if ( P->CairoEngine != NULL )
    { 
        DWORD FreeWidth = P->Paper.W - GetBorderWidth( P, sLeft  ) - GetBorderWidth( P, sRight );
        if (DBG_MODE) LogAddLine("GetPaperFreeWidth: Paper.W=%i FreeWidth: %d ", P->Paper.W, FreeWidth);
		return FreeWidth;
    }
	else
		return 0;
}

/**
*
*/
DWORD StartPrintFast(PrintParam_t *P, char *DocName, char *PrintToFile )
{
	LogAddLine( "---- StartPrintFast: docname={%s} prn2file={%s}", DocName, PrintToFile );
	LogAddLine( "       Printer Info: prnname={%s} portname={%s}", P->PrinterForPrint->pPrinterName, P->PrinterForPrint->pPortName );

	DWORD err=0;
	if ( err == 0 )
	{
		return err;
	}
	LogAddLine( "---- StartPrintFast: StartDocPrinter %s", (err!=0)?"OK":"error" );
	return err;
}

/**
*
*/
DWORD StartPrintWindows(PrintParam_t *P, char *DocName, char *PrintToFile )
{
    int Height,Width;
    
	if (DBG_MODE) LogAddLine( "---- StartPrintWindows: docname={%s} prn2file={%s}", DocName, PrintToFile );
    
    //--- установим бумажка...
    //P.PaperType ==
    //      0 User Define
    //      1 A4-portrait
    //      2 A4-landscape
    //      3 A3-fanfold
    switch ( P->Options.PaperType )
    {
        case 0: 
            if (DBG_MODE) LogAddLine( "     paperType: UserDefine = {%i, %i}", Width, Height );
            break;

        case 1:
			if (DBG_MODE) LogAddLine( "     paperType: A4 (portrait)" );
            break;

        case 2:
			if (DBG_MODE) LogAddLine( "     paperType: A4 (landscape)" );
            break;

        case 3:
			if (DBG_MODE) LogAddLine( "     paperType: A3" );
            break;
    }

    Height = P->Paper.H;
    Width = P->Paper.W;
    
	//--- установим кол-во копий...

	//--- выясним ориентацию для печати Portrait or Landscape???
	if ( P->Paper.Orientation )
	{
		if (DBG_MODE) LogAddLine( "     Set orientation: (%s)", ( P->Paper.Orientation == 1 ) ? "portrait":"landscape" );
	}

    //--- установим изменения...

    //--- создадим контекст для печати...
    char *filename=GetPrintFilename();
    P->CairoSurface = cairo_pdf_surface_create(filename, Width, Height);
    cairo_surface_set_device_offset(P->CairoSurface,DEVICE_OFFSET_X,DEVICE_OFFSET_Y);
    P->CairoEngine = cairo_create(P->CairoSurface);
    if (DBG_MODE) LogAddLine("Paper: %i x %i\tPrint file: %s", Width, Height, filename);
    free(filename);
    
	if ( P->CairoSurface == NULL )
    {
		if (DBG_MODE) LogAddLine( "---- StartPrintWindows: error (cant create dc)" );
		return 0;
    }

    //--- заполним структуру для спулера...

    //--- отступ на котором принтер не может печатать...
    POINT pt = {0, 0};

    // на флоат для точного попадания... :)
    float oX = (LOGX*((float)P->Paper.BorderLeft / INCH))-pt.x;
    P->Paper.ofsX = max( oX, 1.0 );

    float oY = (LOGY*((float)P->Paper.BorderTop / INCH))-pt.y;
    P->Paper.ofsY = max( oY, 1.0 ); 
    if (DBG_MODE) LogAddLine("--- Paper\t BorderLeft: %i BorderTop: %i\tofsX: %i ofsY: %i",P->Paper.BorderLeft,P->Paper.BorderTop,P->Paper.ofsX,P->Paper.ofsY);
    
	int err = 1;
	if (DBG_MODE) LogAddLine( "---- StartPrintWindows: StartDoc %s", (err>0)?"OK":"error" );

	return err;
}

static void *oldDC;

/**
*
*/
DWORD StartPrintDisplay(PrintParam_t *P, char *DocName, char *PrintToFile )
{
    if ( P->PDC == NULL )
        return FALSE;
    oldDC = P->PDC;
    return TRUE;
}

/**
*
*/
BOOL GetLOGXY(PrintParam_t *P) 
{
    void *hdc = NULL;
    if ( P->isPreview )
        ;
    else
       ;

	if ( !hdc )
        return FALSE;

    LOGX=600;
    LOGY=600;

    return TRUE;
}

/**
*
*/
BOOL StartPrint(PrintParam_t *P)
{
	DWORD Error = 0;

    totalFontCache=0;
	P->pDevMode   = NULL;
    P->PDC        = NULL;

    char prn2file[1024]; prn2file[0]=0;

    //--- узнаем физ-кое разрешения принтера...LOGX & LOGY

    //--- создание Device Context...P.PDC
    if ( P->isPreview )
        Error = StartPrintDisplay( P, P->FileForPrint, prn2file );
    else if ( P->Options.EpsonFastPrint )
        Error = StartPrintFast( P, P->FileForPrint, prn2file );
    else 
        Error = StartPrintWindows( P, P->FileForPrint, prn2file );

    //--- установим обработчик ошибок для принтера

	//--- установим собственные шрифты если нужны...

    P->AppendToCurPage           = FALSE;
    P->AppendNextBlockToCurPage  = FALSE;
	P->BlkCur		  = P->Doc;
	P->CurPageForPrint = 0;

	//--- Подготовим бумагу к печати
    //    для печати доступна вся страница
	P->Paper.FreeHigh = GetPaperFreeHigh( P );	

    // установим шрифт по умолчанию, 1-ПИКА
   	InitDefEpsonFont( P, 1 );

	return Error;
}

/**
* завершение печати
*/
BOOL EndPrint(PrintParam_t *P )
{
	// восстановим фонт по умолчанию...

    // удалим все фонты...
    for (int i=0; i<totalFontCache; i++)
        ;
    totalFontCache=0;
	
    BOOL ret = 0;
    if ( P->PrintCanceled )
        ;

	// удалим ресурсы: фонты...


	if ( P->pDevMode )
		delete P->pDevMode;

	// удалить контекст для принтера...

    if (P->CairoSurface)
    {
        cairo_surface_destroy(P->CairoSurface);
        P->CairoSurface=NULL;
    }
    
    if (P->CairoEngine)
    {
        cairo_destroy(P->CairoEngine);
        P->CairoEngine=NULL;
    }

    //Задублируем выходной PDF файл если надо
     if(P->OutputFile && (!strempty(P->OutputFile)))
     {
        char *filename=GetPrintFilename();

        if (fileExist(P->OutputFile))
        {
            delFile(P->OutputFile);
            if (DBG_MODE) LogAddLine("Delete file: %s", P->OutputFile);
        }
        copyFile(P->OutputFile,filename);
        if (DBG_MODE) LogAddLine("Copy file: %s -> %s", filename, P->OutputFile);
        free(filename);
     }

	return ret;
}

/**
*
*/
void *CreateEMF( void *RefDC, RECT *Rect, WORD CurPage, WORD CurPart )
{
    char FileName[ 1024 ];
    sprintf( FileName, "page%u-%u.emf",  CurPage, CurPart );
    return NULL;
}

BOOL DefInited = FALSE;

/**
*
*/
BOOL BeginPage(PrintParam_t *P, bool NeedInit )
{
    if (DBG_MODE) LogAddLine("BeginPage function START");
    P->Paper.FreeHigh = GetPaperFreeHigh( P );

    //Установка динамической ориентации листа при печати пакета документов
    if (P->BlkCur)
    {
        if ( P->BlkCur->HaveActions & aPORTRAIT_ORIENTATION)
            SetOrientationPage(P, PORTRAIT_ORIENTATION_CODE);
        if ( P->BlkCur->HaveActions & aLANDSCAPE_ORIENTATION)
            SetOrientationPage(P, LANDSCAPE_ORIENTATION_CODE);
        //if ( P->BlkCur->HaveActions & aFIT_TO_WIDTH)
        //    SetFitToWidthPage(P);
    }

    if ( P->isPreview )
    {
        RECT R = { 0, 0, P->Paper.W, P->Paper.H }; //TODO!!!
        void *HEMF = CreateEMF( NULL, &R, P->CurPageForPrint, P->CurPartForPrint);
        if ( HEMF == NULL )
            return FALSE;
        else
            P->PDC = HEMF;
    }

    BOOL R = FALSE;
	if ( P->Options.EpsonFastPrint )
    {
        if ( !DefInited || NeedInit )
        {
            PrintChar( P, DEF_INIT,  strlen( DEF_INIT ) );
            DefInited = TRUE;
        }
    }
	else
	{
		SetMapModeForPrinter( P );
	}

    if (DBG_MODE) LogAddLine("Result: %i\tBeginPage function STOP",R);
    return R;
}

/**
*   Признак конца страницы
*/
BOOL EndPage(PrintParam_t *P, bool isLastPage )
{
    BOOL Cancel = 0;
    int isEndPage = -1;
	if ( P->Options.EpsonFastPrint )
    {
       	//--- конец страницы, но НЕ РУЛОНА для епсона...
        if ( P->Paper.H )
        {
			if ( !isLastPage || ( isLastPage && P->Options.EjectLastSheet ) )
				PrintChar( P, FORMFEED, 1 );

        }

    }
	else
    if ( P->isPreview )
    {
        P->PDC = oldDC;
        isEndPage = 1;
    }
    else
    {
        cairo_stroke(P->CairoEngine);
        cairo_show_page(P->CairoEngine);
        isEndPage=1;
    }

    return ( !Cancel && isEndPage > 0 );
}

/**
*
*/
BOOL SetMapModeForPrinter(PrintParam_t *P)
{
	//--- для быстрой печати не надо, то есть всё установленно...
	if ( P->Options.EpsonFastPrint )
		return TRUE;

    //--- посчитаем ширину в лог. единицах f(x) = кол-воТочекНаДюйм * ( lenInCm / 2540 ); 2540 == 1 inch

    //--- установим свой режим отображения...

	//--- установка размера окна логического

	// установка размера окна физического...

	//--- установка начало координат
	// на логическом уровне

	// на физическом уровне

	// установим прозрачный фон

    return TRUE;
}

/**
*
*/
BOOL SelectFont( PrintParam_t *P, int H, int W, int Weight, DWORD Italic)
{
	char FamilyName[1024];

    double font_size=H-_METRIC(80);
    
	strcpy( FamilyName, "Courier New" );
	if ( P->Options.UseOwnShrift ) 
		strcpy( FamilyName, "Iourier New" );	// использовать собственный шрифт для печати...
    if ( P->EFont.Condensed )
    {
        font_size=H; //-_METRIC(16);
        strcpy( FamilyName, "Courier New Condensed" ); //Сжатый шрифт
    }
    
    cairo_font_slant_t slant=(Italic)?CAIRO_FONT_SLANT_ITALIC:CAIRO_FONT_SLANT_NORMAL;
    cairo_font_weight_t weight=(Weight<=300)?CAIRO_FONT_WEIGHT_NORMAL:CAIRO_FONT_WEIGHT_BOLD;
    cairo_select_font_face(P->CairoEngine,FamilyName,slant,weight);
    cairo_set_font_size(P->CairoEngine,font_size);
    
    return TRUE;
}

/**
* 
*/
cache_t *CacheCreateFont( PrintParam_t *P, int H, int W, int Weight, DWORD Italic )
{
    BOOL font_selected;
    
    for ( int i=0; i<totalFontCache;i++ )
    {
        if ( FontCache[i].H==H && 
			 FontCache[i].W==W &&
             FontCache[i].Weight==Weight && 
			 FontCache[i].Italic==Italic )
        {
            return &FontCache[i];
        }
    }

    if ( totalFontCache+1<MAX_CACHE )
    {
        FontCache[totalFontCache].H		 = H;
        FontCache[totalFontCache].W		 = W;
        FontCache[totalFontCache].Weight = Weight;
        FontCache[totalFontCache].Italic = Italic;
        totalFontCache++;
    }

    return &FontCache[totalFontCache-1];
}

/**
*
*/
BOOL UpdateFont(PrintParam_t *P )
{
    BOOL result;
    
    // а не надо ли нам его удалить???
    if ( P->DefFont )
        P->DefFont = NULL;

    if (DBG_MODE) LogAddLine("UpdateFont:\t \
    Pica=%i Elite=%i\t \
    Sup=%i Sub=%i\t \
    DblSize=%i DblSizeLine=%i\t \
    Cond=%i\t \
    Italic=%i Bold=%i\t \
    Under=%i\t \
    DblHigh=%i \
    W=%i H=%i dX=%i dY=%i Weight=%i\t \
    ofsX=%i ofsY=%i\t \
    x=%i y=%i\t \
    I=%i I_72=%i\t \
    ",P->EFont.Pica,P->EFont.Elita,P->EFont.SupScript,P->EFont.SubScript,
    P->EFont.DoubleSize,P->EFont.DoubleSizeLine,
    P->EFont.Condensed,P->EFont.Italic,P->EFont.Bold,
    P->EFont.Underline,P->EFont.DoubleHigh,
    P->EFont.W,P->EFont.H,P->EFont.dX,P->EFont.dY,P->EFont.Weight,
    P->EFont.ofsX,P->EFont.ofsY,P->EFont.x,P->EFont.y,
    P->EFont.Interval,P->EFont.IntervalN_72);
    
    P->DefFont = CacheCreateFont(P,P->EFont.H,P->EFont.W,P->EFont.Weight,P->EFont.Italic);
        
	if ( P->OldFont == NULL )
        P->OldFont = P->DefFont;
    result=SelectFont(P,P->DefFont->H,P->DefFont->W,P->DefFont->Weight,P->DefFont->Italic);
    
    P->FontNeedUpdate = FALSE;
    
    return result;
}

/**
*
*/
void PrintCharEpson(PrintParam_t *P, char *s, int count )
{
	if ( count <= 0 )
		return;

	char *d=new char[count];
	if ( d == NULL )
		return;

	memcpy( d, s, count*sizeof(char));


	DWORD Written;
	if ( d ) delete []d;
}

/**
*
*/
void PrintCharWin(PrintParam_t *P, char *s, int count )
{
	if ( count<=0 )
		return;

	// конверт из DOS->WIN кодировку...
    char *prn_s=cp866_to_utf8(s);

    //--- символ '_' отдельная группа...
    BOOL isSpecialChars = 
        ( ( bufferLen>0 && buffer[0] == '_' && prn_s[0] != '_' )  ||
          ( bufferLen>0 && buffer[0] != '_' && prn_s[0] == '_' ) );

    //--- символы ' ','_','-' отдельная группа тоже...
    BOOL isSpace = FALSE;
    if ( !isSpecialChars )
         isSpace =  
        ( ( bufferLen>0 && buffer[0] == ' ' && prn_s[0] != ' ' )  ||
          ( bufferLen>0 && buffer[0] != ' ' && prn_s[0] == ' ' ) );

    BOOL isMinus = FALSE;
    if ( !isSpecialChars && !isSpace )
         isMinus =  
        ( ( bufferLen>0 && buffer[0] == '-' && prn_s[0] != '-' )  ||
          ( bufferLen>0 && buffer[0] != '-' && prn_s[0] == '-' ) );

    // обновим фонт если надо...
	if ( P->FontNeedUpdate || isSpecialChars || isSpace || isMinus )
    {
        bufferFlush( P );

        if ( P->FontNeedUpdate )
            UpdateFont( P );

        memmove( &P->EFontOld, &P->EFont, sizeof( P->EFont ) );
    }

	bufferAdd( s[0] );
    free(prn_s);
}

/**
*
*/
void PrintChar(PrintParam_t *P, char *s, int count )
{
    char *prn_s=cp866_to_utf8(s);
    free(prn_s);
    
    if ( P->isPrinting )
    {
        // для епсона(быстрая печать) пуляем всё как есть...
	    if ( P->Options.EpsonFastPrint )
			PrintCharEpson( P, s, count );
	    else // маленько покэшируем...
			PrintCharWin( P, s, count );
    } // if ( isPrinting )....  memory

    P->EFont.x += ( P->EFont.dX*count);

}

/**
* print new line..
* перевод на следующию срочку...
*/
void PrintNewLine(PrintParam_t *P )
{
	DWORD intr = P->EFont.Interval;

    if ( P->Paper.FreeHigh > intr ) 
        P->Paper.FreeHigh -= intr;

	P->EFont.y += intr;
	P->EFont.x = 0;

    if (DBG_MODE) LogAddLine("PrintNewLine:\tInterval: %i",intr);
    
    // отключим режим расширенной печати для одной линии...ascii(14)
    if ( P->EFont.DoubleSizeLine )
        AnalyzeFont( P, NULL, DoubleSizeLine | Off );

    memmove( &P->EFontOld, &P->EFont, sizeof( P->EFont ) );

	if ( P->Options.EpsonFastPrint )
	{
		char CRLF[] = "\r";
		PrintChar( P, CRLF, 2 );
	}

}


/**
* возвращает длину текста без последовательностей, с позиции p
*/
int LenPureText(PrintParam_t *P, TextFont_t *F, DWORD p )
{
	int lenPureText = 0;
	// найти фонт в текущей позиции...
	while ( F && F->Pos < p ) F = F->Next;

	while ( F && F->Pos == p )
		F = F->Next;

	if ( F && F->Pos > p )
		lenPureText = F->Pos - p - 1;

	return lenPureText;
}

/**
*
*/
int SetFont(PrintParam_t *P, TextFont_t *F, DWORD p )
{
	int FontSet = 0;
	// найти фонт в текущей позиции...
	while ( F && F->Pos < p ) F = F->Next;

	// ни пуха, ни пера... только фонт -)
	while ( F && F->Pos == p )
	{
		FontSet++;
		AnalyzeFont( P, F, 0 );

        // печатает Epson шрифт сразу...
		if ( P->isPrinting && P->Options.EpsonFastPrint && F->EpsonData )	
		    PrintChar( P, F->EpsonData, F->EpsonDataLen );
		F = F->Next;
	}

	return FontSet;
}

/**
*   Печать страницы.
*/
BOOL PagePrint(PrintParam_t *P, bool isLastPage, bool NeedInit )
{
    if (DBG_MODE) LogAddLine("PagePrint function START LineInPage: %i NumParts: %i", P->Paper.LineInPage, P->NumParts);
    
	DWORD curLine = 0;
	DWORD totalLine = ( P->Paper.LineInPage * P->NumParts );

    // текущая позиция печати в строке...
	DWORD Start = 0;
	bool HaveData = (P->isPrinting>0);
	
    // цикл для печати по частям...
	for ( WORD i = 0; i < P->NumParts; i++ )
	{
		//-- печатать ли страницу?
		P->isPrinting = ( HaveData && P->PagesForPrint[P->CurPage++]>0 ) ? TRUE : FALSE	;
        if (DBG_MODE) LogAddLine("PagePrint\tHaveData: %i\t isPrinting: %i",HaveData,P->isPrinting);

        //-- инициализируем сообщение в диалоги печати...
	    char title[ 100 ];
        char *s = NULL;
        if ( P->NumParts > 1 )
            s = GetStr( IDS_PAGEPRINTPART );
        else
            s = GetStr( IDS_PAGEPRINT );

        if ( !P->isPrinting )
            s = GetStr( ( P->NumParts > 1 ) ? IDS_PAGEPARTSKIP: IDS_PAGESKIP );

        P->CurPartForPrint = i+1;
	    sprintf( title, s, P->CurPageForPrint, P->CurPartForPrint );

		//--
        if ( P->isPrinting )
		{
			// добавить текущию печатаемую часть к предыдущей ?
			if ( !P->AppendToCurPage )
			{
				BeginPage( P, NeedInit ); // не инициализировать

				// обнулим координату печати...
				P->EFont.x = P->EFont.y = 0;
				P->MaxPageForPrint++;
			}
        
			// преобразование и рисование линий, если не включен быстрый режим печати...
			if ( !P->Options.EpsonFastPrint )
				ConvTableToLines( P, i );
		}

        // цикл по всем линиям страницы...
		TextLine_t *L = P->Page;
		while ( L )
		{
            // обработчик ESC и кнопка "Отмена"
            BOOL abort = FALSE;
            if ( abort )
            {
                P->PrintCanceled = TRUE;
				break;
            }

            // пропустить линию..
			if ( L->Attr & aSKIP )
            {
                L = L->Next;
                continue;
            }

            // ширина печатаемой части на странице...
            DWORD partLen = 0;
            if ( Start + P->Parts[ i ] < L->Len ) 
                partLen = P->Parts[ i ];
            else
                if ( L->Len && L->Len > Start )
                    partLen = ( L->Len - Start );

            // у линии атрибу для печати?
			if ( L->Attr & ( aPRINT | aPRINT_ONCE ) )
			{
                // проверка на линию для замены на подобранный шрифт...
                if ( L->Attr & aAUTOMODE ) 
                {
                    char AutoShrift = InitDefEpsonFont(P,0);
                    if ( P->Options.EpsonFastPrint )
						PrintChar( P, DEF_FONTS[ AutoShrift ], strlen( DEF_FONTS[ AutoShrift ] ) );
                }

                // заменим здесь символ перевода страницы на пробел, напечатаем сами в EndPage()
                if ( L->Attr & aFORMFEED )
                {
                    char *FF = strchr( (char *)L->Data, FORMFEED[0] );
                    if ( FF ) *FF = ' ';
                }

    			for ( DWORD p = 0; p < partLen; p++ )
				{
				    // эмуляция установки ESC шрифтов под WindЫЫЫ :)
                    SetFont( P, L->Font, Start + p );
	
	    		    // печать символа в строчке...
					char str[ 2 ] = { L->Data[ Start + p ], 0 };

					PrintChar( P, str, 1 ); // was len
				} // for

				// учитываем фонты в конце линии...
                SetFont( P, L->Font, Start + partLen );

                // сбрасываем буффер...
                bufferFlush( P );

                // перевод строки...
				PrintNewLine( P );
			} // if 

            // следующая линия...
			L = L->Next;
		} // while ( L )

        if ( P->PrintCanceled )	// прервали печать?
            break;				// да...

        // проверим сколько места осталось на странице....
        if ( P->Paper.H )
        {
            int usedHigh = P->Paper.H - P->Paper.FreeHigh;	// сколько использованно бумаги

            // если войдёт ещё часть...то спросит куда печатать....
            if ( ( usedHigh > 0 && usedHigh <= P->Paper.FreeHigh && 
				   i+1<P->NumParts && P->PagesForPrint[P->CurPage]>0 ) 
				 || P->AppendNextBlockToCurPage )
            {
                // ВНИМАНИЕ! Если здесь поставить P->AppendToCurPage = TRUE;
                // то по ~~ не будет переводить вывод на следующую страницу
                P->AppendNextBlockToCurPage = FALSE;
            }
            else
                P->AppendToCurPage = FALSE;
        }

        if ( P->isPrinting && !P->AppendToCurPage )
        {
            P->PrintCanceled = !EndPage( P, isLastPage && P->CurPartForPrint == P->NumParts );
            if ( P->PrintCanceled )
                break;
        }

		// на следующию часть...
		Start += P->Parts[ i ];
	}

    if (DBG_MODE) LogAddLine("PagePrint function STOP");
    
	return !P->PrintCanceled;
}

/**
*
*/
BOOL PrintDoc(PrintParam_t *P )
{
    if (DBG_MODE) LogAddLine("PrintDoc function START");
    
    WORD DupMode = 0; // специальный режим '&n' n==0 оменить
	BOOL errInLoop = FALSE;

	//--- создадим контекст. установим всё что надо для печати
	if ( !StartPrint( P ) )
    {
        return TRUE;
    }

	//--- основной цикл
	while ( !errInLoop && !P->PrintCanceled )
	{
		bool isLastPage = false;	// если true - печатается последняя страница
		bool NeedInit = false;		// если true - инициализировать установки по умолчанию...
        
        if ( !BuildPageForPrint( P, DupMode, isLastPage, NeedInit ) )
            break;

		P->CurPageForPrint++;

 		PageCopy( P, DupMode );     // скопируем страницу...теперь работаем только с P.Page...
		DoNumeration( P );          // заменим '#' на текущий номер страницы...
		DoDateTime( P );            // заменим '\' на текущию дату...
		DoAutoShrift( P, DupMode ); // '^' подберём шрифт...

		if ( DupMode )
		{
			DoDuplicate( P, DupMode );  // '&' дублирование, размножим страничку...
			if ( FALSE )		        // отменить?
				DupMode = 0;			// да...
		}
		else
        {
		    DoSumColumns( P );      // '+' посчитать сумму в колонках...
    	    DoRepeatName( P );      // '!' заменить одинаковые имена в колонках...
        }

		//-- добавим блок подписи если нужен...
		SignCopy( P );

		//-- '}' повторим части... 
		if ( !DoPart( P ) ) 
        {
            sayError( IDS_ERROR_REPORT_TOO_WIDE );
            errInLoop = TRUE;
            ClearDocFromPrintedLineAndBlock( P );
            break;
        }

		//-- заменим '@', '~', '~~', '&0' 
        ChangeServiceChars( P ); 

        //--
		// если после шапки только пустые линии 
        // _И_ первая печатаемая страницы равна текущей => не печатать...
        P->isPrinting = HaveDataInPage( P );

		//-- Печать страницы...
        if ( PagePrint( P, isLastPage, NeedInit ) )
        {
            //Перевод на новую страницу
            //cairo_show_page(P->CairoEngine);
        }
        else
        {
		    errInLoop = TRUE;
            ClearDocFromPrintedLineAndBlock( P );
            break;
        }

        //-- очистим P.Page...
		if ( !ClearDocFromPrintedLineAndBlock( P ) )
		{
			errInLoop = TRUE;
			break;
		}
	}

    // завершим печать...
	EndPrint( P );

    return errInLoop;
}

/**
* strPage -- n1-n2, n3,n4, n5-n5
*/
int InitPages( PrintParam_t *P, char *data, char *strPage, int pageParity)
{
    if (DBG_MODE) LogAddLine("InitPages function START");
    
    const char *SEP = ",";
    int totalPage=0;
    char temp[255];
    char *t;

    char strBackup[255];
    strcpy( strBackup, strPage );

    //--- обнулим что печатать...
    if ( P )
        memset( P->PagesForPrint, 0, sizeof( BYTE ) * MAX_PAGES );
    *data=0;
    char *start=data;

    if ( P != NULL && pageParity == 1 || pageParity == 2 )
    {
        // pageParity == 1 -- печатать только нечётные
        // pageParity == 2 -- печатать только чётные
        for ( int p=(pageParity==1)?0:1; p<MAX_PAGES; p+=2 )
        {
            P->PagesForPrint[p] = 1;
            totalPage++;
        }
        if (DBG_MODE) LogAddLine("totalPage: %i\tInitPages function STOP",totalPage);
        return totalPage;
    }

    if (DBG_MODE) LogAddLine("Parsing: %s separator: %s",strPage,SEP);
    t = strtok( strPage, SEP );
    while ( t != NULL )
    {
        WORD S=0, E=0;
        char *isItRange = strchr( t, '-' );
        if ( isItRange )
        {
            if ( isdigit( *(isItRange+1) ) > 0 )
                //ВНИМАНИЕ!!! В формате необходимо ставить
                //%hu а не просто %u
                sscanf( t, "%hu-%hu", &S, &E ); // n1-n2
            else
            {
                if (DBG_MODE) LogAddLine("It is not digits\tInitPages function STOP");
                return -1;
            }
        }
        else
            S = atoi( t );                            // n3

        // последняя больше первой...
        if ( E>=S && S>=1 && E<=MAX_PAGES && S!=E )
        {
            sprintf( temp, "%u-%u,", S, E );
            strcpy( data, temp );
            data += strlen( temp );
            if ( P )
                memset( P->PagesForPrint+S-1, 1, sizeof( BYTE ) * ( E-S+1 ) );
            totalPage += ( E-S+1 );

        }
        else if ( S>0 && S<=MAX_PAGES )
        {
            sprintf( temp, "%u,", S );
            strcpy( data, temp );
            data += strlen( temp );
            if ( P )
                P->PagesForPrint[ S-1 ] = 1;
            totalPage++;
        }

        if ( S==0&&E==0 )
        {
            if (DBG_MODE) LogAddLine("S==0&&E==0\tInitPages function STOP");
            return -1;
        }

        t = strtok( NULL, SEP );
    }

    // последяя запятая не нужна...
    if ( *data == 0 && *(data-1)==',' ) *(data-1)=0;

    //-- на начало строки...
    data=start;
    strcpy( strPage, strBackup );

    if (DBG_MODE) LogAddLine("totalPage: %i\tInitPages function STOP",totalPage);
    return totalPage;
}

/**
* Установка ориентации страницы
*/
void SetOrientationPage(PrintParam_t *P, DWORD orientation)
{
    if (DBG_MODE) LogAddLine("Block actions orientation: %s\tPaper orientation: %d\tSet orientation: %d",(P->BlkCur)?((P->BlkCur->HaveActions&aLANDSCAPE_ORIENTATION)?"landscape":"portrait"):"NO BLOCK",P->Paper.Orientation,orientation);

    //Надо сначала проверить надо ли вообще устанавливать ориентацию
    //может она уже соответствует текущей
    if (P->Paper.Orientation != orientation)
    {
        //Портретная ориентация
        if (orientation == PORTRAIT_ORIENTATION_CODE)
        {
        cairo_translate( P->CairoEngine, P->Paper.H - DEF_FAST_BORDER_WIDTH*2, 0 );
        cairo_rotate( P->CairoEngine, M_PI/2 );
        }
        //Ландшафтная ориентация
        if (orientation == LANDSCAPE_ORIENTATION_CODE)
        {
        cairo_translate( P->CairoEngine, 0, P->Paper.H - DEF_FAST_BORDER_HIGH*2 );
        cairo_rotate( P->CairoEngine, -M_PI/2 );
        }

        P->Paper.Orientation = orientation;
    }
}


/**
* Установка режима заполнения листа по ширине печати страницы без переноса страницы
*/
//void SetFitToWidthPage(PrintParam_t *P)
//{
//    if (DBG_MODE) LogAddLine("Block actions Fit to width");
//    
//}

#pragma warning( pop )
