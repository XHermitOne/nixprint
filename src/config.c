/**
* Модуль для работа с конфигурации.
* сохранение и востановление параметров...
* @file
*/

#include "lprint.h"
#include "config.h"
#include "paper.h"

/**
* Основные настройки
*/
char LDefPrinter[STD_STR_LEN];
char LPapersBorderAndSize[STD_STR_LEN];
char LAutoDetect[STD_STR_LEN];
char LShowShrift[STD_STR_LEN];
char LEjectLastSheet[STD_STR_LEN];
char LShowDialog[STD_STR_LEN];
char LSavePaper[STD_STR_LEN];
char LUseOwnShrift[STD_STR_LEN];

/**
* Настройки принтера
*/
char LDefPaper[STD_STR_LEN];
char LEpsonFastPrint[STD_STR_LEN];

char CfgPath[] = ".nixprint";           /**< Папка конфигурациронных файлов*/
char CfgFileName[] = "nixprint.cfg";    /**< Имя файла конфигурации основных параметров*/
static char FullCfgFileName[MAX_PATH];

#define I_DEFPRINTER            0
#define I_PAPERSBORDERANDSIZE   1
#define I_AUTODETECT            2
#define I_SHOWSHRIFT            3
#define I_EJECTLASTSHEET        4
#define I_SHOWDIALOG            5
#define I_SAVEPAPER             6
#define I_USEOWNSHRIFT          7

Config_t config_strings[] =
{
	{ "DefaultPrinter",                     LDefPrinter},
	{ "PapersBorderAndSize",                LPapersBorderAndSize},
	{ "AutoDetectEpsonCompatiblePrinter",   LAutoDetect},
	{ "ShowShriftWidthInMenu",              LShowShrift},
	{ "EjectLastSheet",                     LEjectLastSheet},
	{ "ShowDialogAboutChangePaper",         LShowDialog},
	{ "SaveSelectedPaper",                  LSavePaper},
	{ "UseOwnShrift",                       LUseOwnShrift},
};

#define I_DEFPAPER               0
#define I_EPSONFASTPRINT         1

Config_t printer_cfg_strings[] =
{
	{ "DefaultPaper",           LDefPaper},
	{ "EpsonCompatiblePrinter", LEpsonFastPrint},
};

char PaperCfgFormat[]="BORDER: %i,%i,%i,%i SIZE: %ix%i MODE: %i %i";

BOOL saveConfig (char *CFGFileName,Config_t *Strings,int Count)
{
	int	i;
	FILE	*f;

	if(DBG_MODE) LogAddLine("Saving config settings %s %i strings.\n",CFGFileName,Count);
    
	f = fopen (CFGFileName, "wt");
	if (!f)
    {
        if(DBG_MODE) LogAddLine("ERROR: Can't write the file %s \n",FullCfgFileName);
		return FALSE;	// can't write the file, but don't complain
    }

	for (i = 0; i < Count; i++)
	{
		fprintf(f, "%s\t\t\"%s\"\n", Strings[i].name,Strings[i].location);
	}

	fclose (f);
    return TRUE;
}


BOOL loadConfig(const char *CFGFileName,Config_t *Strings,int Count)
{
	int i; 
	int len;
	FILE *f;
	char def[STD_STR_LEN];
	char strparm[100];
	int parm;

	if(DBG_MODE) LogAddLine("Loading config settings %s %i strings.\n",CFGFileName,Count);

    f = fopen(CFGFileName, "rt");
	if (f)
	{
		while (!feof(f))
		{
			if (fscanf(f, "%79s %[^\n]\n", def, strparm) == 2)
			{
                if(DBG_MODE) LogAddLine("Line: %s %s.\n",def,strparm);                
				if (strparm[0] == '"') /* string values */
				{
                    for (i = 0; i<Count; i++)
                    {
                        if (!strcmp(def, Strings[i].name))
                        {
                            len = (int)strlen(strparm) - 2;
                            if (len <= 0)
                            {
                                Strings[i].location[0] = '\0';
                                break;
                            }
                            if (len > 79)  len = 79;
                            strncpy(Strings[i].location, strparm + 1, len);
                            Strings[i].location[len] = '\0';
                            if(DBG_MODE) LogAddLine("Name: %s Value: %s.\n",Strings[i].name,Strings[i].location);                
                            break;
                        }
                    }
                continue;
				}

			}
		}
        fclose (f);
	}
    else
    {
        if(DBG_MODE) LogAddLine("ERROR: Cant read file %s \n",CFGFileName);
        return FALSE;
    }
    return TRUE;
}

static BOOL CreateCfgPath(char *Path);
char *GetCfgPath(void)
{
    char *result=NULL;
    char *home_path=GetHomePath();
    
    result=(char*)calloc(strlen(home_path)+1+strlen(CfgPath)+1,sizeof(char));
    strcpy(result,home_path);
    strcat(result,"/");
    strcat(result,CfgPath);

    if (!dir_exists(result)) 
        CreateCfgPath(result);

    if(DBG_MODE) LogAddLine("GetCfgPath: %s\n",result);
    
    return result;
}

static BOOL CreateCfgPath(char *Path=NULL)
{
    int do_free=0;
    char *conf_path=Path;
    if (conf_path==NULL)
    {
        conf_path=GetCfgPath();
        do_free=1;
    }
        
        
    BOOL result=(BOOL) mkpath(conf_path,S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    if(DBG_MODE) LogAddLine("Create path: %s\n",conf_path);

    if (do_free)
        free(conf_path);    
        
    return result;
}

static char *GetCfgFileName(char *FileName)
{
    char *filename=NULL;
    char *conf_path=GetCfgPath();

    if (FileName!=NULL)
    {
        filename=(char*)calloc(strlen(FileName)+1,sizeof(char));
        strcpy(filename,FileName);
    }
    else
    {
        filename=(char*)calloc(strlen(conf_path)+1+strlen(CfgFileName)+1,sizeof(char));
        strcpy(filename,conf_path);
        strcat(filename,"/");
        strcat(filename,CfgFileName);
    }

    free(conf_path);
    
    if(DBG_MODE) LogAddLine("GetCfgFileName: %s\n",filename);
    return filename;
}

static char *GetPrinterCfgFileName(char *PrinterName)
{
    if ((PrinterName==NULL)||(!strcmp("",PrinterName)))
    {
        //if(DBG_MODE) LogAddLine("WARNING: Dont define printer name\n");
        return NULL;
    }
    char *filename=NULL;
    char *conf_path=GetCfgPath();

    filename=(char*)calloc(strlen(conf_path)+1+strlen(PrinterName)+5,sizeof(char));
    strcpy(filename,conf_path);
    strcat(filename,"/");
    strcat(filename,PrinterName);
    strcat(filename,".cfg");
    
    free(conf_path);
    return filename;
}

BOOL LoadConfigPrinter(PrintParam_t *P, char *PrinterName )
{
    char *printer_cfg_filename=GetPrinterCfgFileName(PrinterName);
    
    if (printer_cfg_filename==NULL)
    {
        if(DBG_MODE) LogAddLine("WARNING! Load config printer not define.\n");
        return FALSE;
    }
    
    BOOL result=loadConfig(printer_cfg_filename,printer_cfg_strings,2);
    if(DBG_MODE) LogAddLine("Load config printer %s.\n",printer_cfg_filename);
    free(printer_cfg_filename);
    
    if (! result)  return FALSE;

    P->Options.EpsonFastPrint = ( !strcmp("1",printer_cfg_strings[I_EPSONFASTPRINT].location) ) ? 1 : 0;
    

    //--- ...Номер бумажки на которую последний раз печатали...
    P->Options.PaperType = atoi(printer_cfg_strings[I_DEFPAPER].location);

    return TRUE;
}

BOOL SaveConfigPrinter(PrintParam_t *P, char *PrinterName )
{
    //--- возьмём ключик для принтера по умолчанию...
    char *printer_cfg_filename=GetPrinterCfgFileName(PrinterName);
    
    if (printer_cfg_filename==NULL)
    {   
        if(DBG_MODE) LogAddLine("WARNING! Save config printer not define.\n");
        return FALSE;
    }

    if(DBG_MODE) LogAddLine("Save config printer %s.\n",printer_cfg_filename);
    //--- fast print...
    DWORD FastPrint = (P->Options.EpsonFastPrint)?1:0;
    printer_cfg_strings[I_EPSONFASTPRINT].location=strdup(i2a(FastPrint));

    //--- paper type...
    DWORD PaperType = P->Options.PaperType;
    printer_cfg_strings[I_DEFPAPER].location=strdup(i2a(PaperType));

    if (! saveConfig(printer_cfg_filename,printer_cfg_strings,2))
    {
        free(printer_cfg_filename);
        return FALSE;
    }

    free(printer_cfg_filename);
    return TRUE;
}

void LoadConfig(PrintParam_t *P)
{
    //--- ничего не загрузили...
    P->Options.CfgLoaded = 0;

    //--- проверим, а запускались ли мы тут вообще?

    //--- считаем принтер, который был выбран в последний раз для печати....
    char *cfg_filename=GetCfgFileName(NULL);
    BOOL result=loadConfig(cfg_filename,config_strings,8);
    if(DBG_MODE) LogAddLine("Load config: %s\n",cfg_filename);
    free(cfg_filename);
    
    if (! result)  return;
    
    strcpy(P->Options.printerName,config_strings[I_DEFPRINTER].location);

    //--- ... paper & border...
    sscanf(config_strings[I_PAPERSBORDERANDSIZE].location,PaperCfgFormat,
        &Papers[0].borders[0],&Papers[0].borders[1],&Papers[0].borders[2],&Papers[0].borders[3],
        &Papers[0].size.x,&Papers[0].size.y,
        &Papers[0].mode[0],&Papers[0].mode[1]);

    //--- "AutoDetectEpsonCompatiblePrinter"
    P->Options.AutoDetectFastmode=atol(config_strings[I_AUTODETECT].location);

    //--- "ShowShriftWidthInMenu"
    P->Options.ShowShriftWidth=atol(config_strings[I_SHOWSHRIFT].location);

    //--- "SaveSelectedPaper"
    P->Options.SaveSelectedPaper=atol(config_strings[I_SAVEPAPER].location);

    //--- "UseOwnShrift"
    P->Options.UseOwnShrift=atol(config_strings[I_USEOWNSHRIFT].location);

    //--- "EjectLastSheet"
    P->Options.EjectLastSheet=atol(config_strings[I_EJECTLASTSHEET].location);
    
    //--- "ShowDialogAboutChangePaper"
    P->Options.ShowDialog=atol(config_strings[I_SHOWDIALOG].location);

    if (P->Options.printerName)
        if (!LoadConfigPrinter( P, P->Options.printerName ) )
            return;

    //--- удачно всё загрузили
    P->Options.CfgLoaded = 1;
}

/**
* сохраняет выбранные в меню параметры печати...
*/
void SaveConfigPaperOptions(PrintParam_t *P)
{
    config_strings[I_DEFPRINTER].location=strdup(P->Options.printerName);
    sprintf(config_strings[I_PAPERSBORDERANDSIZE].location,PaperCfgFormat, 
        Papers[0].borders[0],Papers[0].borders[1],Papers[0].borders[2],Papers[0].borders[3],
        Papers[0].size.x,Papers[0].size.y,
        Papers[0].mode[0],Papers[0].mode[1]);
    config_strings[I_AUTODETECT].location=strdup(i2a(P->Options.AutoDetectFastmode));
    config_strings[I_SHOWSHRIFT].location=strdup(i2a(P->Options.ShowShriftWidth));
    config_strings[I_SAVEPAPER].location=strdup(i2a(P->Options.SaveSelectedPaper));
    config_strings[I_USEOWNSHRIFT].location=strdup(i2a(P->Options.UseOwnShrift));
    config_strings[I_EJECTLASTSHEET].location=strdup(i2a(P->Options.EjectLastSheet));
    config_strings[I_SHOWDIALOG].location=strdup(i2a(P->Options.ShowDialog));
    
    char *cfg_filename=GetCfgFileName(NULL);
    saveConfig(cfg_filename,config_strings,8);
    free(cfg_filename);
}

void SaveConfig(PrintParam_t *P)
{
    CreateCfgPath();
    
    //--- сохранимся...
    SaveConfigPaperOptions( P );

    //--- printername...
    if (P->Options.printerName)
        if ( !SaveConfigPrinter( P, P->Options.printerName ) )
            return;
}
