/**
* Модуль функций всё что связано с версией...
* в этом модуле будет хранится версия nixprint
*
* version - "vXX.YY [DD.MM.YEAR]
*   XX - старший разряд версии 
*   YY - младший, реально кол-во фиксов, исправленных багов в версии XX
*        смотри файлик NIXPRINT.HST
* @file
*/
#include "version.h"

const int XV=3;  /**< XX */
const int YV=7;  /**< YY */

static char version[100];
char *getNixPrintVersion(void)
{
    char build_time[100];
    strcpy(build_time,__TIMESTAMP__);
    //char temp[100];
    //char month[100];
    //int  day,i,year;
    //sscanf(ver,"%s %s %i %i:%i:%i %i",temp,month,&day,&i,&i,&i,&year);
    sprintf(version,"v%i.%2i [%s]",XV,YV,build_time);
    return version;
}

/**
* Вывести на экран версию программы
*/
void printVersion(void)
{
    printf("NixPrint version: ");
	printf(getNixPrintVersion());
    printf("\n");
}

static char HelpTxt[]="\n\
Параметры коммандной строки:\n\
\n\
    ./nixprint [Параметры печати]\n\
\n\
Параметры печати:\n\
\n\
    [Файл]\n\
        --file=|-f=        Файл печати\n\
\n\
    [Помощь и отладка]\n\
        --help|-h|-?        Напечатать строки помощи\n\
        --version|-v        Напечатать версию программы\n\
        --debug|-D          Включить режим отладки\n\
\n\
    [Размер бумаги]\n\
        --A4|--a4           Размер бумаги A4\n\
        --A3|--a3           Размер бумаги A3\n\
        --portrait|-P       Портретная ориентация листа\n\
        --landscape|-L      Ландшафтная ориентация листа\n\
        --width=|-w=        Ширина листа\n\
        --height=|-h=       Высота листа\n\
\n\
    [Диапазон печати]\n\
        --all               Печать всех страниц\n\
        --even              Печать четных страниц\n\
        --odd               Печать нечетных страниц\n\
        --pages=            Диапазон печатных страниц\n\
        --copies=           Количество копий\n\
\n\
    [Поля]\n\
        --left=|-l=         Размер левого поля печати\n\
        --right=|-r=        Размер правого поля печати\n\
        --top=|-t=          Размер верхнего поля печати\n\
        --bottom=|-b=       Размер нижнего поля печати\n\
\n\
    [Дополнительные параметры]\n\
        --lines=            Количество строк на странице\n\
        --use_font          Использование собственного шрифта\n\
";

/**
* Вывести на экран помощь
*/
void printHelp(void)
{
    printf("NixPrint программа вывода текстовых файлов с ESC последовательностями в PDF файл: \n");
    printf(HelpTxt);
    printf("\n");
}
