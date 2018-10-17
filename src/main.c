/**
* Модуль функции MAIN
* @file
*/
#include "lprint.h"

/**
* Основная функция MAIN. Анализ параметров коммандной строки и 
* установка этих параметров. 
*
* Параметры коммандной строки:
* @param \--help|-h|-? Напечатать строки помощи
* @param \--version|-v  Напечатать версию программы
* @param \--debug|-D Включить режим отладки
* @param \--log|-L Включить режим журналирования
* @param \--file|-f Файл печати
* @param \--out|-u Выходной PDF файл
* @param \--A4|\--a4 Размер бумаги A4
* @param \--A3|\--a3 Размер бумаги A3
* @param \--portrait|-P Портретная ориентация листа
* @param \--landscape|-L Ландшафтная ориентация листа
* @param \--width=|-w= Ширина листа
* @param \--height=|-h= Высота листа
* @param \--all Печать всех страниц
* @param \--even Печать четных страниц
* @param \--odd Печать нечетных страниц
* @param \--pages= Диапазон печатных страниц
* @param \--copies= Количество копий
* @param \--use_font Использование собственного шрифта
* @param \--left=|-l= Размер левого поля печати
* @param \--right=|-r= Размер правого поля печати
* @param \--top=|-t= Размер верхнего поля печати
* @param \--bottom=|-b= Размер нижнего поля печати
* @param \--lines= Количество строк на странице
*/
int main (int argc, char **argv)
{

	if (argc==1 || checkParm("--help",argc,argv) || checkParm("-h",argc,argv) || checkParm("-?",argc,argv))
	{
		printHelp();
		return 0;
	}

    if (checkParm("--version",argc,argv) || checkParm("-v",argc,argv))
    {
        printVersion ();
        return 0;
    }

	if (checkParm("--debug",argc,argv) || checkParm("-d",argc,argv))
        DBG_MODE=TRUE;
    else
        DBG_MODE=FALSE;

	if (checkParm("--log",argc,argv) || checkParm("-j",argc,argv))
        DBG_MODE=TRUE;
    else
        DBG_MODE=FALSE;

	return LPrintMain(argc,argv);
}
