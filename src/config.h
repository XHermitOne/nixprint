/**
* Модуль для работа с конфигурации.
* сохранение и востановление параметров...
* @file
*/


#if !defined( __CONFIG_H )
#define __CONFIG_H
#include "lprint.h"

#define     STD_STR_LEN     80

/**
*полностью параметры прочитать, записать...
*/
void LoadConfig(PrintParam_t *P);
void SaveConfig(PrintParam_t *P);

/**
* прочитать, записать параметры для PrinterName...
*/
BOOL LoadConfigPrinter(PrintParam_t *P, char *PrinterName );
BOOL SaveConfigPrinter(PrintParam_t *P, char *PrinterName );

/** 
* сохраняет параметры Границы бумаги, меню Настройки...
*/
void SaveConfigPaperOptions(PrintParam_t *P );

/**
* Структура хранения данных в конфиге
*/
typedef struct Config_t
{
	const char	*name;  /**< Имя параметра */
	char    *location;  /**< Значение */
} Config_t;

BOOL saveConfig (char *CFGFileName,Config_t *Strings);
BOOL loadConfig(const char *CFGFileName,Config_t *Strings);

/**
* Путь к папке конфига
*/
char *GetCfgPath(void);

#endif

