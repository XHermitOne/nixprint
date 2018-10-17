/**
* Модуль функций работы с языковым файлом LNG
* @file
*/

#include "lprint.h"

/**
* Структура файла с языком
* 1 строка: .Lang=Russian setlocale
* n строка: UINT Text{/Text};HelpMsg;
*/ 
struct _Texts
{
	DWORD id;			/**< номер ресурса */
	char *txt;			/**< ссылка на текст */
	char *hlp;			/**< ссылка на хинт */
	_Texts *next;
} *Texts = NULL;

/**
* @return new buf pos...
*/
char *readStr( char *src, char *buf  )
{
    if ( !src )
        return NULL;

    buf[0]=0;

    if (src[0]=='\r'&&src[1]=='\n')
        return src+2;

    sscanf( src, "%[^\r]s", buf );
    if ( strlen( buf ) == 0 )
        return NULL;
    else
        return src+strlen( buf )+2;
}

/**
*
*/
BYTE *DEF_LANG_FILE=NULL; /** Файл языка LNG */

const char LangFileID[] = ".language=russian"; /**< Текущий язак */

/**
* Возможные имена файлов
*/
const char LNGFileName1[] = "/usr/share/nixprint/nixprint.lng";
const char LNGFileName2[] = "./nixprint.lng";

/**
* Инициализация языка
*/
BOOL InitLanguage()
{
	///??? вставить код для загрузки lng из той директории из которой запущен lprint.
    char *DefLang = (char *)DEF_LANG_FILE; //&DEF_LANG_FILE;
	char Buffer[10240];
    
	if( DefLang==NULL)
	{
		FILE *in=fopen(LNGFileName1, "rb" );
        
        //Если  токого нет, то поробовать его найти в другом месте
        if (in==NULL)
        {
            in=fopen(LNGFileName2, "rb" );
        }
        
		if (in!=NULL)
		{
			DefLang = new char[10240];
			fread( DefLang, 1, 10240, in );
			fclose( in );
		}
	}

	// читаем строчку...
    DefLang = readStr( DefLang, Buffer );

	// первая строка .Lang=Russian ???
	//if ( _stricmp( Buffer, LangFileID ) )
	if ( stricmp( Buffer, LangFileID ) )
        return FALSE; // нет

	while ( 1 )
	{
        if ( ( DefLang = readStr( DefLang, Buffer ) ) == NULL )
            break;

		if ( !Buffer[ 0 ] || Buffer[ 0 ] == ';' ) 
			continue; // пустая строчка, дальше...

		DWORD id;
		char Text[ 2048 ];
		char Help[ 2048 ];
		Text[0]=Help[0]='\0';

		sscanf( Buffer, "%lu %[^;]s", &id, Text );
        if ( id == 0 )
            continue;

		//-- найдём HELPMSG
		char *HlpPresent = strchr( Buffer, ';' );
		char *Rev		 = strrchr( Buffer, ';' );
		if ( HlpPresent && Rev && *(HlpPresent+1) != '\0' && HlpPresent != Rev )
			strncpy( Help, HlpPresent+1, Rev-HlpPresent-1 );
        
		_Texts *cur = new _Texts;
		if ( cur == NULL )
			return FALSE;
		cur->id = id;

		int len = strlen( Text );
		cur->txt = new char[  len + 1 ];
		if ( cur->txt == NULL )
		{
			delete cur;
			return FALSE;
		}
		strcpy( cur->txt, Text );
		cur->txt[len]=0;

		if ( strlen( Help ) )
		{
			cur->hlp = new char[ strlen( Help ) + 1 ];
			if ( cur->hlp == NULL )
				return FALSE;
			strcpy( cur->hlp, Help );
		}
		else
			cur->hlp = NULL;

		cur->next = NULL;
		if ( Texts == NULL )
			Texts = cur;
		else
		{
			_Texts *Head = Texts;
			while ( Head->next != NULL ) Head = Head->next;
			Head->next = cur;
		}
	}

	return TRUE;
}

/**
* Получить строку перевода по идентификатору
*/
char *GetStr( DWORD id )
{
	_Texts *Head = Texts;
    
    if ((Head==NULL) && (DBG_MODE)) LogAddLine("WARNING: Don't init language\n");
    
	while( Head )
	{
		if ( Head->id == id )
			return Head->txt;

		Head = Head->next;
	}
	return NULL;
}

/**
* Получить хинт( всплывающую подсказку) по идентификатору
*/
char *GetHlp( DWORD id )
{
	_Texts *Head = Texts;

    if ((Head==NULL) && (DBG_MODE)) LogAddLine("WARNING: Don't init language\n");
    
	while( Head )
	{
		if ( Head->id == id )
			return Head->hlp;

		Head = Head->next;
	}
	return NULL;
}


/**
* Деинициализация языка
*/
BOOL DoneLanguage()
{
	_Texts *Head = Texts;
	while ( Head )
	{
		_Texts *ptr = Head;
		Head = Head->next;
		if ( ptr->txt != NULL ) delete[] ptr->txt;
		if ( ptr->hlp != NULL )	delete[] ptr->hlp;
		delete ptr;
	}
	return TRUE;
}
