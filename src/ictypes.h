/**
* Модуль определения дополнительных типов
* @file
*/

#include <limits.h>

#if !defined( __ICTYPES_H )
#define __ICTYPES_H

#define FAR		far
#define NEAR	near
#define VOID	void
#ifndef CONST
#define CONST   const
#endif

#define CHAR	char		/* ch  */
#define SHORT	short		/* s   */
#define LONG	long		/* l   */

#define STR_NULL   ((char) 0)

//Предельные значения типов
#define WORD_MAX    USHRT_MAX
#define DWORD_MAX   ULONG_MAX

typedef unsigned long ULONG;
typedef unsigned short USHORT;
typedef unsigned char UCHAR;

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;

typedef int                 INT;
typedef unsigned int        UINT;

typedef signed long         __int64;
typedef __int64             LONG64;

typedef unsigned short WCHAR;   

typedef WCHAR *PWCHAR;
typedef WCHAR *LPWCH, *PWCH;
typedef CONST WCHAR *LPCWCH, *PCWCH;
typedef WCHAR *NWPSTR;
typedef WCHAR *LPWSTR, *PWSTR;

typedef CONST CHAR *LPCWSTR, *PCWSTR;

typedef CHAR *PCHAR;
typedef CHAR *LPCH, *PCH;

typedef CONST CHAR *LPCCH, *PCCH;
typedef CHAR *NPSTR;
typedef CHAR *LPSTR, *PSTR;
typedef CONST CHAR *LPCSTR, *PCSTR;

#ifdef  UNICODE                    

#ifndef _TCHAR_DEFINED
typedef WCHAR TCHAR, *PTCHAR;
typedef WCHAR TBYTE , *PTBYTE ;
#define _TCHAR_DEFINED
#endif

typedef LPWSTR LPTCH, PTCH;
typedef LPWSTR PTSTR, LPTSTR;
typedef LPCWSTR PCTSTR, LPCTSTR;
typedef LPWSTR LP;
#define __TEXT(quote) L##quote     

#else   /* UNICODE */               

#ifndef _TCHAR_DEFINED
typedef char TCHAR, *PTCHAR;
typedef unsigned char TBYTE , *PTBYTE ;
#define _TCHAR_DEFINED
#endif /* !_TCHAR_DEFINED */

typedef LPSTR LPTCH, PTCH;
typedef LPSTR PTSTR, LPTSTR;
typedef LPCSTR PCTSTR, LPCTSTR;
#define __TEXT(quote) quote         

#endif /* UNICODE */                
#define TEXT(quote) __TEXT(quote)   

#define MAX_PATH          260

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

typedef struct RECT
{
    LONG    left;
    LONG    top;
    LONG    right;
    LONG    bottom;
}RECT;

typedef struct POINT
{
    LONG  x;
    LONG  y;
}POINT;

typedef struct SIZE
{
    LONG        cx;
    LONG        cy;
}SIZE;


#endif /*__ICTYPES_H*/

