/**
* Модуль главных структур программы и основных запускающих функций
* @file
*/

#include "lprint.h"
#include "config.h"

/**
* Режим отладки
*/
BOOL DBG_MODE=FALSE;

/**
* Функция используется для отладки
*/
void sam_dbg(PrintParam_t *P ) 
{
	FILE *in = NULL;
	in = fopen( "d:\\sam.dbg", "wb");
	TextBlock_t *Blk = P->Doc;
	TextLine_t *L, *L_bak ;
	L=P->Page;
	L_bak=P->Page;

	while( L)
	{
        // линия помечена для печати?
		if ( L && L->Attr & ( aPRINT | aPRINT_ONCE ) )
			fprintf(in,">> ");

		fprintf(in,  (char * )L->Data );
		fprintf(in,  "\n" );
		L=L->Next;
	}
	L=L_bak;
	fclose( in );
}

/**
* глобальная структура, HEART OF LPRINTW :)
*/
PrintParam_t PrintParam;

/**
* Функция запуска печати
* argv[1] -- имя файла для печати...
*/
int LPrint( int argc, char *argv[], PrintParam_t *P )
{
	// обнулим структуру для печати....
	memset( P, 0, sizeof( PrintParam_t ) );

    //Загрузить конфигурационные параметры
    LoadConfig(P);
    
    //Установка параметров печати из коммандной строки
    int paper_format=0;
    int paper_orient=0;
    int opt;
    char *filename;
    char pages[256]="1-9999";
    int parity=0;
    const struct option long_opts[] = {
              { "file", required_argument, NULL, 'f' },
              { "out", required_argument, NULL, 'u' },
              { "A4", no_argument, NULL, '4' },
              { "A3", no_argument, NULL, '3' },
              { "portrait", no_argument, NULL, 'P' },
              { "landscape", no_argument, NULL, 'L' },
              { "width", required_argument, NULL, 'W' },
              { "height", required_argument, NULL, 'H' },
              { "all", no_argument, NULL, 'a' },
              { "even", no_argument, NULL, 'e' },
              { "odd", no_argument, NULL, 'o' },
              { "pages", required_argument, NULL, 'p' },
              { "copies", required_argument, NULL,'c' },
              { "left", required_argument, NULL, 'l' },
              { "right", required_argument, NULL, 'r' },
              { "top", required_argument, NULL, 't' },
              { "bottom", required_argument, NULL, 'b' },
              { "lines", required_argument, NULL, 'n' },
              { "use_font", no_argument, NULL, 'F' },
              { "debug", no_argument, NULL, 'd' },
              { "log", no_argument, NULL, 'j' },
              { "version", no_argument, NULL, 'v' },
              { "help", no_argument, NULL, 'h' },
              { NULL, 0, NULL, 0 }
       };
  
    if (DBG_MODE) LogAddLine("OPTIONS:");
    while ((opt = getopt_long(argc, argv, "fu43PLWHaeopclrtbnFdjvh:", long_opts, NULL)) != -1)
    {
        switch (opt) 
        {
            case 'f':
                filename = optarg;

                // файл для печати....
                P->FileForPrint = filename;
                
                // проверим существует ли файл для печати...
                if ( !fileExist( filename ) ) 
                    return 3;
                    
                if (DBG_MODE) LogAddLine("      --file = %s", filename);
                break;

            case 'u':
                filename = optarg;

                // выходной PDF файл
                P->OutputFile = filename;

                if (DBG_MODE) LogAddLine("      --out = %s",filename);
                break;

            case '4':
                paper_format=0;
                if (DBG_MODE) LogAddLine("      --A4");
                break;

            case '3': 
                paper_format=1;
                if (DBG_MODE) LogAddLine("      --A3");
                break;

            case 'P': 
                paper_orient=0;
                if (DBG_MODE) LogAddLine("      --portrait");
                break;

            case 'L': 
                paper_orient=1;
                if (DBG_MODE) LogAddLine("      --landscape");
                break;

            case 'W':
                P->Paper.W=atol(optarg)*_METRIC(100);
                if (DBG_MODE) LogAddLine("      --width = %i",P->Paper.W);
                break;
                
            case 'H':
                P->Paper.H=atol(optarg)*_METRIC(100);
                if (DBG_MODE) LogAddLine("      --height = %i",P->Paper.H);
                break;

            case 'a':
                parity=0;
                if (DBG_MODE) LogAddLine("      --all");
                break;
                
            case 'e':
                parity=2;
                if (DBG_MODE) LogAddLine("      --even");
                break;
                
            case 'o':
                parity=1;
                if (DBG_MODE) LogAddLine("      --odd");
                break;
                
            case 'p':
                strcpy(pages,optarg);
                if (DBG_MODE) LogAddLine("      --pages = \"%s\"",pages);
                break;
                
            case 'c':
                P->Options.CopiesForPrint=atoi(optarg);
                if (DBG_MODE) LogAddLine("      --copies = %i",P->Options.CopiesForPrint);
                break;

            case 'n':
                P->Options.LineAtPage=atoi(optarg);
                if (DBG_MODE) LogAddLine("      --lines = %i",P->Options.LineAtPage);
                break;
                
            case 'F':
                P->Options.UseOwnShrift=1;
                if (DBG_MODE) LogAddLine("      --use_font");
                break;

            case 'l':
                P->Paper.BorderLeft=atol(optarg)*_METRIC(100);
                if (DBG_MODE) LogAddLine("      --left = %i",P->Paper.BorderLeft);
                break;
                                
            case 'r':
                P->Paper.BorderRight=atol(optarg)*_METRIC(100);
                if (DBG_MODE) LogAddLine("      --right = %i",P->Paper.BorderRight);
                break;
                                
            case 't':
                P->Paper.BorderTop=atol(optarg)*_METRIC(100);
                if (DBG_MODE) LogAddLine("      --top = %i",P->Paper.BorderTop);
                break;
                                
            case 'b':
                P->Paper.BorderBotton=atol(optarg)*_METRIC(100);
                if (DBG_MODE) LogAddLine("      --bottom = %i",P->Paper.BorderBotton);
                break;
                                
            case 'd':
                DBG_MODE=TRUE;
                if (DBG_MODE) LogAddLine("      --debug");
                break;

            case 'j':
                DBG_MODE=TRUE;
                if (DBG_MODE) LogAddLine("      --log");
                break;
                
            case 'h':
                printHelp();
                if (DBG_MODE) LogAddLine("      --help");
                break;
                
            case 'v':
                printVersion();
                if (DBG_MODE) LogAddLine("      --version");
                break;
                
            case '?':
                printHelp();
                return 1;

            default:
                fprintf(stderr, "Unknown parameter: \'%c\'",opt);
                return 1;
        }
    }
            
    if (DBG_MODE) LogAddLine("Start print ...");
    
    P->Options.PaperType=0;
    P->Paper.LineInterval=Fonts[0].dY; // установим интервал, 1/6Inch
    
    if (!paper_format && !paper_orient)
    {
        //A4 portrait
        P->Options.PaperType=1;
        P->Paper.W=Papers[0].size.x*_METRIC(100);
        P->Paper.H=Papers[0].size.y*_METRIC(100);
    }
    if (!paper_format && paper_orient)
    {
        //A4 landscape
        P->Options.PaperType=2;
        P->Paper.W=Papers[1].size.x*_METRIC(100);
        P->Paper.H=Papers[1].size.y*_METRIC(100);
    }
    else
        if (paper_format)
        {
            //A3
            P->Options.PaperType=3;
            P->Paper.W=Papers[4].size.x*_METRIC(100);
            P->Paper.H=Papers[4].size.y*_METRIC(100);
        }

    
	// загрузим документ, преобразуем tab to space, вырежим esc-code
	// разобъём документ на блоки, на шапку, текс, 
    // на данные перед шапкой
	char Done = LoadDoc( P );
	if ( Done != 1 )
		return ( Done == 2 ) ? 5 : 3; // опять user передумал печатать, но позже...

	// найдём все управляющие символы такие как:
    // нумерация, сложение столбиков, етк...
	//initAbortCallBack();// GetStr( IDS_ANALYZETITLE ) 
	//clock_t start = clock();
    //ShowBlockType( P.Doc, 1 ); getch();

	bool isOk = AnalyzeDoc( P );
	if ( !isOk )
		return 5;	// отменил пользователь или p.doc==null что не должно быть...

    // раскомментарить для показки инфы о блоках...
    //ShowBlockType( P.Doc, 1 ); //getch();

	int err;
    if ( ( err=isItRightDoc( P ) ) != 0 )
    {
        // say wrong block....
        sayError( IDS_ERROR_INTERNAL+err );
        return 5;
    }

  
    BOOL errInLoop = FALSE;
    while ( !errInLoop )
    {
        InitPages(P,P->Options.FirstPageForPrint,pages,parity);
        
        //-- печать, просмотр документа...
		P->CurPage = 0;
        P->MaxPageForPrint = 0;
        errInLoop = PrintDoc( P );
		break;
    }


    //Сохранить конфигурационные параметры
    SaveConfig(P);

    if (DBG_MODE) LogAddLine("... End print");
    
	return ( errInLoop || P->PrintCanceled ) ? 5 : 0;
}

/**
* Функция подготовки перед печатью и анализ возвращаемого кода ошибки
*/
int LPrintMain( int argc, char *argv[])
{
	char Msg[ 1024 ];
	char FileName[ MAX_PATH ];
	int i;

    // инициализируем библиотеку сообщений... SetConsoleTitle
    InitLanguage();

    // собственно всё начинается тут ...... и заканчивается тоже :-)
    int exitCode = LPrint( argc, argv, &PrintParam );
	switch ( exitCode )
	{
		case 0: // всё в ёлку напечаталось :-)
			break;

		case 1:
			sayMessage( IDS_HELP1 ,0);
			break;

		case 2: // пользователь передумал печатать
			break;

		case 3: // не смогли открыть файл для печати....
            if (DBG_MODE) LogAddLine("ERROR! File for print not found");
            
			i = strlen( PrintParam.FileForPrint );
			if ( i <= 45 )
				strcpy( FileName, PrintParam.FileForPrint );
			else
				sprintf(FileName, "...%s", &PrintParam.FileForPrint[ i - 45 ]);

			sprintf(Msg, GetStr( IDS_ERROR_CANT_OPEN_FILE ), FileName);
			sayError( Msg );
			break;

		case 4:
			sayError( IDS_ERROR_CANT_CREATE_DC );
			break;

		case 5:
			// какие-то другие ошибки...
			// но мы их не будем обрабатывать....
			break;
	}

	DoneLanguage();
	return exitCode;
}
