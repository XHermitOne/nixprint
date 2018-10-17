/**
* Модуль функций разбора текстового файла печати
* @file
*/

#if !defined( __PARSER_H )
#define __PARSER_H

#include <time.h>

#include "ictypes.h"
#include "lprint.h"
#include "tools.h"
#include "blocks.h"
#include "lines.h"

BOOL AnalyzeDoc(PrintParam_t *P);
BOOL BuildPageForPrint(PrintParam_t *P, WORD &Dup, bool &isLastPage, bool &NeedInit );
void ChangeServiceChars(PrintParam_t *P );
WORD CheckDupMode( TextLine_t *L, WORD dupMode );
BOOL ClearDocFromPrintedLineAndBlock(PrintParam_t *P);
void DoAutoShrift(PrintParam_t *P, WORD DupMode );
void DoDateTime(PrintParam_t *P);
BOOL DoDuplicate(PrintParam_t *P, WORD &DupMode );
void DoNumeration(PrintParam_t *PP);
BOOL DoPart(PrintParam_t *P);
void DoRepeatName(PrintParam_t *P);
void DoSumColumns(PrintParam_t *P);
DWORD GetSignBlockHigh(PrintParam_t *P);
BOOL HaveDataInPage(PrintParam_t *P);
int isItRightDoc(PrintParam_t *P);
char LoadDoc(PrintParam_t *P );
BOOL PageCopy(PrintParam_t *P, BOOL DupMode );
BOOL SignCopy(PrintParam_t *P);
DWORD TryFitNextBlockToPage(PrintParam_t *P);
DWORD TryFitToPage(PrintParam_t *P, WORD &DupMode, BOOL &FormFeed, BOOL &NeedCancelDup );
void propis(TextLine_t *Dup);

#endif /*__PARSER_H*/
