/**
* Модуль функций записи в лог
* @file
*/

#include "lprint.h"
#include "log.h"
#include "config.h"
#include <stdio.h>
#include <time.h>

/**
* Текущее время-дата
*/
char TimeDate[128];
char *GetTimeDate()
{
	TimeDate[0]=0;
	time_t loc_time;
	time( &loc_time );

	tm *today = localtime( &loc_time );
	strftime( TimeDate, 128, "%d/%m/%y (%H:%M:%S)", today );

	return TimeDate;
}

/**
* Класс менеджера лога
*/
class LogInit
{
public:
	FILE *out;
	bool isNew;

	LogInit( char *LogName )
	{
		out = NULL;
		isNew = false;
        //char *home_path=GetHomePath();
        char *cfg_path=GetCfgPath();
    
        char *full_log_filename = (char*)calloc(strlen(cfg_path)+strlen(LogName)+1,sizeof(char));
        strcpy(full_log_filename,cfg_path);
        strcat(full_log_filename,LogName);
        
        out = fopen(full_log_filename, "a");
        fprintf( out, "[START LOG] %s - - - - - - - - - - - - - - - - - - - - -", GetTimeDate() );

        free(full_log_filename);            
	}

	~LogInit()
	{
		if ( out )
		{
			fprintf( out, "[STOP LOG] %s - - - - - - - - - - - - - - - - - - - - -", GetTimeDate() );
			fclose( out );
            out = NULL;
		}
	}
};

/**
* Лог файл
*/
class LogInit LogFile( "/nixprint.log" );

/**
* Добавить строчку в лог
*/
void LogAddLine( char *S, ... )
{
	if ( LogFile.out )
	{
		va_list argptr;
		va_start( argptr, S );

		char msg[4096];
		vsnprintf( msg, sizeof(msg), S, argptr );
		va_end( argptr );

		fprintf( LogFile.out, "    %s %s\n", GetTimeDate(), msg );
		printf( "%s %s\n", GetTimeDate(), msg );
		fflush( LogFile.out );
	}
}

