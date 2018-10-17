/**
* Модуль сервисных функций
* @file
*/

#if !defined( __TOOLS_H )
#define __TOOLS_H

#include <stdio.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <iconv.h>

#include <fcntl.h>              /* O_RDONLY */
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/param.h>          /* MAXPATHLEN */

#include "ictypes.h"
#include "log.h"

int checkParm (const char *check,int argc, char **argv);

void ErrBox(char *Fmt,...);

char sayError( DWORD ErrorID );
char sayError( char *msg );

char sayMessage( DWORD MsgID, int aOptions );
char sayMessage( char *Msg, int aOptions );

char *i2a(int i);
char *long2str(char *buf, long n);

LONG64 long_min(LONG64 a, LONG64 b);
char* itoa( int nValue, char* szResult, int nBase );
char* i64toa(LONG64 l64Value, char* szResult);
__int64 atoi64(const char* str);

char* strlwr(char* pstr);
char* strupr(char* pstr);
char *strnset(char *s, int ch, size_t n);
BOOL strempty(char *s);

/**
* Найти слово в строке
*/
BOOL find_word(char *source, char *search);

#define stricmp strcasecmp

DWORD GetFullPathName(const char *lpFileName, DWORD nBufferLength,
                char *lpBuffer,char **lpFilePart);
DWORD GetShortPathName(char *lpszLongPath,char *lpszShortPath,DWORD cchBuffer);

char *GetHomePath(void);       
                
#define		SLOSH		'\\'
#define		DEFQUOTES	"\"'"	/**< default quote characters */
#define     CNULL       0
#define     CPNULL      0
//#define     FALS      0
//#define     TRUE      1
#define		vnew(object,n)  ((object *)malloc((size_t)((n)*sizeof(object))))
                
static  const char * skip(const char *string, const char *white);
static  void copy(char	ch);
static  void newword(char *cp);
static  void subsplit(const char *string, const char *delim,const char *quotes);

char **strsplit(char *string, char *delim);
                                  
typedef struct
{
	char* search;
	char* replace;
} sr;

sr* parse_path_string(char* str);
char *replace(const char *src, const char *from, const char *to);
char* replace_all(char* src, sr* r);

char *check_cfg_param(char *key,char *value,char *name);

BOOL dir_exists(char *path);
int mkpath(const char *path, mode_t mode);

BOOL fileExist( char *FileForPrint );
int copyFile(char *to, char *from);
BOOL delFile( char *Filename);

char *konk(char *a1, char *a2);
char *_3( int num, char *_1,char *n1,char *_2,char *n2,char *nn );
char *trim(char *str);
char *propisStr(int summa);
char *cp1251_to_utf8(char *from);
char *cp866_to_utf8(char *from);

double max(double a,double b);
double min(double a,double b);

#endif /*__TOOLS_H*/
