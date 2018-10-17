#include <stdio.h>

#include "config.h"
#include "texts.h"
#include "parser.h"

int main (int argc, char **argv)
{
    int i;

    printf("Start testing\n");
    
    printf("Test tools.c\n");

    printf("Test cp1251_to_utf8 function\n");
    char *utf8str=cp1251_to_utf8("ABCD ¿¡¬√ƒ");
    printf("UTF8 string: %s\n",utf8str);
    free(utf8str);
    printf("Test cp1251_to_utf8 function...OK\n");

    printf("Test cp866_to_utf8 function\n");
    utf8str=cp866_to_utf8("ABCD ÄÅëÑ");
    printf("UTF8 string: %s\n",utf8str);
    free(utf8str);
    printf("Test cp866_to_utf8 function...OK\n");
    
    printf("Test mkpath function\n");
    mkpath("./testdir/test/test",S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    rmdir("./testdir/test/test");
    rmdir("./testdir/test");
    rmdir("./testdir");
    printf("Test mkpath function...OK\n");
    
    printf("Test tools.c...OK\n\n");
    
    PrintParam_t P;
    memset( &P, 0, sizeof( PrintParam_t ) );
    
    strcpy(P.Options.printerName,"DefaultTestPrinter");

    char test_prn_file[]="./tst/doctxt01.txt";
    //char test_prn_file[]="./tst/udfrep.rep";
    P.FileForPrint=(char*)calloc(strlen(test_prn_file)+1,sizeof(char));
    strcpy(P.FileForPrint,test_prn_file);

    P.Paper.W=Papers[1].size.x *_METRIC(100);
    P.Paper.H=Papers[1].size.y *_METRIC(100);

    printf("Set Paper size: width=%i height=%i\n",P.Paper.W,P.Paper.H);

    P.Options.PaperType=1;
    printf("Set Paper type: %i\n",P.Options.PaperType);
    
    printf("Set default P Options\n");
    printf("PrinterName: %s\n",P.Options.printerName);
    printf("PaperType: %i\n",P.Options.PaperType);
    printf("EpsonFastPrint: %i\n",P.Options.EpsonFastPrint);
    printf("AutoDetectFastmode: %i\n",P.Options.AutoDetectFastmode);
    printf("EjectLastSheet: %i\n",P.Options.EjectLastSheet);
    printf("ShowDialog: %i\n",P.Options.ShowDialog);
    printf("SaveSelectedPaper: %i\n",P.Options.SaveSelectedPaper);
    printf("UseOwnShrift: %i\n",P.Options.UseOwnShrift);
    printf("FileForPrint: %s\n",P.FileForPrint);
    printf("\n");
    
    printf("Test config.c\n");
    printf("Test SaveConfig in config.c \n");
    SaveConfig(&P);
    printf("Test SaveConfig in config.c...OK\n");
    
    printf("Test LoadConfig in config.c \n");
    LoadConfig(&P);
    printf("Test LoadConfig in config.c...OK\n");
    printf("Test config.c ...OK\n\n");

    printf("Test texts.c\n");
    printf("Test InitLanguage in texts.c \n");
    BOOL result=InitLanguage();
    printf("result=%i\n",result);
    printf("Test InitLanguage in texts.c...OK\n");
    
    printf("Test GetStr/GetHlp in texts.c \n");
    int id[]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,55,66,
	    50,51,52,53,54,55,100,101,102,103,
	    150,200,201,202,203,204,205,206,211,212,213};
    int id_count=47;
    char *str;
    char *hlp;
    for (i=0;i<id_count;i++)
    {
        str=GetStr(id[i]);
	hlp=GetHlp(id[i]);
        printf("ID=%i GetStr: %s GetHlp: %s \n",id[i],str,hlp);
    }
    printf("Test GetStr/GetHlp in texts.c...OK\n");

    printf("Test DoneLanguage in texts.c \n");
    //result=DoneLanguage();
    printf("result=%i\n",result);
    printf("Test DoneLanguage in texts.c...OK\n");
    printf("Test texts.c ...OK\n\n");

    //InitLanguage();
    printf("Test parser.c\n");
    printf("Test LoadDoc in parser.c \n");
    LoadDoc(&P);
    printf("Test LoadDoc in parser.c...OK\n");
    printf("Test AnalyzeDoc in parser.c \n");
    AnalyzeDoc(&P);
    printf("Test AnalyzeDoc in parser.c...OK\n");
    printf("Test parser.c ...OK\n\n");
    
    printf("Test printer.c\n");

    printf("Test InitPages in printer.c \n");
    char buff[255]="1-9999";
    InitPages( &P, P.Options.FirstPageForPrint,buff,0);
    printf("Test InitPages in printer.c...OK\n");
    printf("Test PrintDoc in printer.c \n");
    P.CurPage = 0;
    P.MaxPageForPrint = 0;
    PrintDoc(&P);
    printf("Test PrintDoc in printer.c...OK\n");
    printf("Test printer.c ...OK\n\n");
    DoneLanguage();

    printf("Stop testing\n\n");
    return 0;
}
