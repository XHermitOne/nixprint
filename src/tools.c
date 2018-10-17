/**
* Модуль сервисных функций
* @file
*/

#include "tools.h"

/**
* checkParm
*
* Checks for the given parameter in the program's command line arguments
* @return the argument number (1 to argc-1) or 0 if not present
*/

int checkParm (const char *check,int argc, char **argv)
{
    int i;

	for (i = 1; i < argc; i++)
	{
		if (!strcasecmp(check, argv[i]))
			return i;
	}

	return 0;
}

/**
* Вывод ошибок в виде окна сообщений
* Я использую в основном для отладки
*/
void ErrBox(char *Fmt,...)
{
	char buffer[128];
	va_list ap;

	va_start(ap,Fmt);
	vsprintf(buffer,Fmt,ap);
	
	//MessageBox(NULL,buffer,"Ошибка", MB_OK | MB_ICONSTOP | MB_TASKMODAL );
    LogAddLine("Error: ");
    LogAddLine(buffer);
    LogAddLine("\n");
}

char sayError(DWORD ErrorID) 
{ 
    LogAddLine("ErrorID: %lu\n",&ErrorID);
    return 0;
}

char sayError( char *msg ) 
{ 
    LogAddLine(msg);
    LogAddLine("\n");
    return 0;
}

char sayMessage( DWORD MsgID, int aOptions ) 
{ 
    return sayError(MsgID);
}

char sayMessage( char *Msg, int aOptions )
{
    return sayError(Msg);
}

static char buf[12];

char *i2a(int i)
{
    char *ptr = buf + sizeof(buf) - 1;
    unsigned int u;
    int minus = 0;

   if (i < 0) {
      minus = 1;
      u = ((unsigned int)(-(1+i))) + 1;
   }else
      u = i;
      *ptr = 0;
   do
      *--ptr = '0' + (u % 10);
   while(u/=10);

   if (minus)
      *--ptr = '-';
   return ptr;
}


static char integers[10]={'0','1','2','3','4','5','6','7','8','9'};
/**
* converts long to a string representation and returns it
*/
char *long2str(char *buf, long n)
{
    char tmp[22];/* This is a stack.64/Log_2[10] = 19.3, so this is enough forever...*/
    unsigned long u;
    char *bufptr=buf, *tmpptr=tmp+1;

   if(n<0){/*Swap the sign and store '-':*/
      /*INT_MIN workaround:*/
      u = ((unsigned long)(-(1+n))) + 1;
      *bufptr++='-';
   }else
       u=n;

   *tmp='\0';/*terminator*/

   /*Fill up the stack:*/
   do
     *tmpptr++=integers[u%10];
   while(u/=10);

   /*Copy the stack to the output buffer:*/
   while( (*bufptr++ = *--tmpptr)!='\0' );
   return buf;
}/*long2str*/

/**
* Min function required for Linux, although it is questionable
* TBD whether this is needed here or not.  It is not needed
* in the Linux build for DioCLI...
*/
LONG64 long_min(LONG64 a, LONG64 b)
{
  LONG64 c = ((a) < (b) ? (a) : (b));
  return c;
} // End min

/**
* C++ version char* style "itoa".  It would appear that
* itoa() isn't ANSI C standard and doesn't work with GCC on Linux
* @param nValue value to convert to ascii
* @param szResult buffer for the result
* @param nBase base for conversion
* @return value converted to ascii
*/
char* itoa( int nValue, char* szResult, int nBase )
{
	// check that the base if valid
	if (nBase < 2 || nBase > 16) {
		 *szResult = 0;
		 return szResult;
    }

	char* szOut = szResult;
	int nQuotient = nValue;

	do {
		//*szOut = "0123456789abcdef"[ std::abs( nQuotient % nBase ) ];
		*szOut = "0123456789abcdef"[ abs( nQuotient % nBase ) ];
		++szOut;
		nQuotient /= nBase;
	} while ( nQuotient );

	// Only apply negative sign for base 10
	if ( nValue < 0 && nBase == 10) *szOut++ = '-';

	//reverse( szResult, szOut );
	*szOut = 0;

	return szResult;

} // end itoa

/**
* C++ version char* style "i64toa".  It would appear that
* i64toa() isn't ANSI C standard and doesn't work with GCC on Linux
* @param l64Value value to convert to ascii
* @param szResult buffer for the result
* @return value converted to ascii
*/
char* i64toa(LONG64 l64Value, char* szResult)
{
    //sprintf(szResult,"%lld", l64Value);
    char *result=l64a(l64Value);
    strcpy(szResult,result);
    return szResult;
    
} // end i64toa


__int64 atoi64(const char* str) 
{
    __int64 lValue = 0;
	int lFlag = 1;
	int nSize = strlen(str);
		
	for(size_t i = 0; i < nSize; i++)
	{
		if (i == 0)
		{
			if (str[i] == '-') //
			{
				lFlag = -1;
				continue;
			}
		}

		if ((str[i] >= '0') && (str[i] <= '9'))
		{
			lValue = lValue * 10 + str[i] - 48;
		}
		else //
		{
			break;
		}
	}
	  
	return lValue*lFlag;
}

char* strlwr(char* pstr)
{
	char* p = pstr;
	while(*p)
	{
		if(*p >= 'A' && *p <= 'Z')
			*p = 'a' + (*p - 'A');
		p++;
	}
	return pstr;
}

char* strupr(char* pstr)
{
	char* p = pstr;
	while(*p)
	{
		if(*p >= 'a' && *p <= 'z')
			*p = 'A' + (*p - 'a');
		p++;
	}
	return pstr;
}

char *strnset(char *s, int ch, size_t n)
{  
    /* mimic strnset command in dos/ os2/ win / ... */
    for(int i=0; i< (int) n; i++)
    {
        if(s[i] == STR_NULL ) return(s); // return when find null
        s[i] = ch;
    }
    return  (s);
}


/**
* Проверка на пустую строку
*/
BOOL strempty(char *s)
{
    int len=strlen(s);
    if(len == 0)
        return TRUE;
    else
        return FALSE;
}

/**
* Найти слово в строке
*/
BOOL find_word(char *source, char *search)
{
    int i=0;                //  Position inside source
    int search_length;      //  Length of string search
    int source_length;      //  Length of string source
    BOOL found=FALSE;       //  Flag 1-> found the value

    search_length=strlen(search);
    source_length=strlen(source);

    /*  While we haven't found it and we haven't readched the      */
    /*  point where our current position plus the length of the    */
    /*  string we are looking for is longer than the string itself,*/
    /*  check the next search_length characters for a match        */
    while (!found && ((i+search_length)<=(source_length)))
    {
        found = (strncasecmp(source+i, search, search_length)==0);
        i++;
    }
 
    return(found);
}

DWORD GetFullPathName(char *lpFileName, DWORD nBufferLength,
                char *lpBuffer,char **lpFilePart)
{
    //realpath("../../",lpBuffer);
    return 0;
}

DWORD GetShortPathName(char *lpszLongPath,char *lpszShortPath,DWORD cchBuffer)
{
 return 0;   
}

char *GetHomePath(void)
{
    return getenv("HOME");
}

static	char	*buff;		/**< buffer for strings */
static	char	**ptrs;		/**< buffer for pointers to strings */

static	char	*bufp;		/**< current buffer pointer */
static	char	**ptrp;		/**< next word address */

static	int	words;		    /**< number of words in string */
static	int	space;		    /**< number of characters to store */

static const char *skip(const char *string,const char *white)
{
	while (*string && strchr(white,*string) != CNULL)
		string++;

	return string;
}

static void copy(char ch)
{
	if (buff != CNULL)
		*bufp++ = ch;
	else
	space++;
}

static void newword(char *cp)
{
	if (buff != CNULL)
		*ptrp++ = cp;
	else
	words++;
}

static void subsplit(const char	*string,const char *delim,const char *quotes)
{
	int	sloshed;
	char	quotec;
	
	words=0;
	space=0;
	string=skip(string,delim);
	if (*string)
		newword(bufp);

	for (quotec='\0', sloshed=FALSE;
	     *string;
	     string++)
		{ if (quotec != '\0')
			/* in quotes */
			if (sloshed)
				/* in quotes after a slosh */
				{ if (*string != quotec && *string != SLOSH)
					/* not something that's escaped */
					copy(SLOSH);

				  copy(*string);

				  /* forget slosh */
				  sloshed=FALSE;
				}
			else
			/* in quotes not after a slosh */
			if (*string == quotec)
				/* leave quotes */
				quotec='\0';
			else
			if ((sloshed=(*string == SLOSH)))
				/* notice slosh */
				;
			else
			copy(*string);
		  else
		  /* not in quotes */
		  if (sloshed)
			/* not in quotes after a slosh */
			{ if (*string != SLOSH
			   && strchr(quotes,*string) == CNULL
			   && strchr(delim,*string) == CNULL)
				copy(SLOSH);

			  copy(*string);

			  /* forget slosh */
			  sloshed=FALSE;
			}
		  else
		  /* not in quotes not after a slosh */
		  { if (strchr(quotes,*string) != CNULL)
			/* enter quotes */
			quotec = *string;
		    else
		    if (strchr(delim,*string) != CNULL)
			/* find next word */
			{ string=skip(string,delim);
			  if (*string)
				{ copy('\0');
				  newword(bufp);
				}

			  string--;
			}
		    else
		    if ((sloshed=(*string == SLOSH)))
			;
		    else
		    copy(*string);
		  }
		}

	/* catch trailing sloshes */
	if (sloshed)
		copy(SLOSH);

	copy('\0');
	newword(CNULL);
}

char **qstrsplit(const char	*string,const char *delim,const char *quotes)
{
	/* default delimiters */
	if (delim == CNULL)
		delim=" \t\n";

	/* mark pass one */
	buff=CNULL;

	/* count words and characters */
	subsplit(string,delim,quotes);

	/* allocate room for characters */
	if ((buff=vnew(char, space)) == CNULL)
		return CPNULL;

	/* allocate room for words */
	if ((ptrs=vnew(char *,words+1)) == CPNULL)
		{ free(buff);
		  return CPNULL;
		}

	/* initialise pointers */
	bufp=buff;
	ptrp=ptrs;

	/* point to allocated space */
	*ptrp++ = buff;

	/* copy words into buffer */
	subsplit(string,delim,quotes);

	return &ptrs[1];	/* return pointer to words */
}

char **strsplit(char *string,char *delim)
{
	return qstrsplit(string,delim,DEFQUOTES);
}

sr* parse_path_string(char* str)
{
	char* s;
	int numreps=1;
	sr* r;
	char** ss;
	char* rtk;
	char* xtk;
	int i;

	for(s = str; *s; s++)
		if(*s==';')
			numreps++;

	numreps++;

	r = (sr*) calloc(numreps, sizeof(sr));
	ss = (char**) calloc(numreps-1, sizeof(char*));

	rtk = strtok(str,";");

	i = 0;
	while(rtk)
	{
	    ss[i] = strdup(rtk);
	    rtk = strtok(NULL,";");
	    i++;
	}

	for(i=0;i<(numreps-1);i++)
	{
		xtk = strtok(ss[i],"=");
		r[i].search = strdup(xtk);
		r[i].replace = strdup(strtok(NULL,"="));
	}

	r[i+1].search=NULL;
	r[i+1].replace=NULL;

//	for(i=0;i<numreps-1;i++)
//		free(ss[i]);

//	free(ss);

	return r;

}

char *replace(const char *src, const char *from, const char *to)
{
   /*
    * Find out the lengths of the source string, text to replace, and
    * the replacement text.
    */
   size_t size    = strlen(src) + 1;
   size_t fromlen = strlen(from);
   size_t tolen   = strlen(to);
   /*
    * Allocate the first chunk with enough for the original string.
    */
   char *value = (char *) malloc(size);
   /*
    * We need to return 'value', so let's make a copy to mess around with.
    */
   char *dst = value;
   /*
    * Before we begin, let's see if malloc was successful.
    */
   if ( value != NULL )
   {
      /*
       * Loop until no matches are found.
       */
      for ( ;; )
      {
	 /*
	  * Try to find the search text.
	  */
	 const char *match = strstr(src, from);
	 if ( match != NULL )
	 {
	    /*
	     * Found search text at location 'match'. :)
	     * Find out how many characters to copy up to the 'match'.
	     */
	    size_t count = match - src;
	    /*
	     * We are going to realloc, and for that we will need a
	     * temporary pointer for safe usage.
		 */
	    char *temp;
	    /*
	     * Calculate the total size the string will be after the
	     * replacement is performed.
	     */
	    size += tolen - fromlen;
	    /*
		 * Attempt to realloc memory for the new size.
	     */
	    temp = (char *)realloc(value, size);
	    if ( temp == NULL )
	    {
	       /*
		* Attempt to realloc failed. Free the previously malloc'd
		* memory and return with our tail between our legs. :(
		*/
	       free(value);
	       return NULL;
	    }
		/*
	     * The call to realloc was successful. :) But we'll want to
	     * return 'value' eventually, so let's point it to the memory
	     * that we are now working with. And let's not forget to point
	     * to the right location in the destination as well.
	     */
	    dst = temp + (dst - value);
	    value = temp;
		/*
	     * Copy from the source to the point where we matched. Then
	     * move the source pointer ahead by the amount we copied. And
	     * move the destination pointer ahead by the same amount.
	     */
	    memmove(dst, src, count);
	    src += count;
	    dst += count;
	    /*
	     * Now copy in the replacement text 'to' at the position of
	     * the match. Adjust the source pointer by the text we replaced.
	     * Adjust the destination pointer by the amount of replacement
		 * text.
	     */
	    memmove(dst, to, tolen);
	    src += fromlen;
	    dst += tolen;
	 }
	 else /* No match found. */
	 {
		/*
	     * Copy any remaining part of the string. This includes the null
	     * termination character.
	     */
	    strcpy(dst, src);
	    break;
	 }
      }
   }
   return value;
}

char* replace_all(char* src, sr* r)
{
	char* ret = src;
	int i;


	for(i=0;r[i].search;i++)
		ret = replace(ret,r[i].search,r[i].replace);

	ret = replace(ret,"\\","/");
	return ret;
}

char *check_cfg_param(char *key,char *value,char *name)
{
    char *result=NULL;
    
    if(!strcmp(key,name))
    {
        result=(char*)calloc(strlen(value)+1,sizeof(char));
        strcpy(result,value);
    }
    
    return result;    
}

/**
* Проверить существует директория
*/
BOOL dir_exists(char *path)
{
    struct stat            st;
    BOOL result=(BOOL)(stat(path, &st) == 0);
    return result;
}

static int do_mkdir(const char *path, mode_t mode)
{
    struct stat            st;
    int             status = 0;

    if (stat(path, &st) != 0)
    {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0) // && errno != EEXIST)
            status = -1;
    }
    else if (!S_ISDIR(st.st_mode))
    {
        LogAddLine("ERROR: %s is not directory",path);
        status = -1;
    }

    return(status);
}

/**
* mkpath - ensure all directories in path exist
* Algorithm takes the pessimistic view and works top-down to ensure
* each directory in path exists, rather than optimistically creating
* the last element and working backwards.
*/
int mkpath(const char *path, mode_t mode)
{
    char           *pp;
    char           *sp;
    int             status;
    char           *copypath = strdup(path);

    status = 0;
    pp = copypath;
    while (status == 0 && (sp = strchr(pp, '/')) != 0)
    {
        if (sp != pp)
        {
            /* Neither root nor double slash in path */
            *sp = '\0';
            status = do_mkdir(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (status == 0)
        status = do_mkdir(path, mode);
    free(copypath);
    return (status);
}

/**
* существует ли файл для печати...
* @return TRUE  - exist / FALSE - don't exist
*/
BOOL fileExist( char *FileForPrint )
{
    BOOL Exist = FALSE;
    if ( FileForPrint )
    {
		FILE *in = fopen( FileForPrint, "rb" );
		if ( in != NULL )
        {
            Exist = TRUE;
			fclose( in );
        }
    }
    return Exist;
}

/**
* Копирование файла
*/
int copyFile(char *to, char *from)
{
    char newname[MAXPATHLEN+1];
    // char answer[20];
    struct stat stf, stt;
    int fdin, fdout;
    int n, code = 0;
    char iobuf[64 * 1024];
    char *dirname = NULL, *s;

    if((fdin = open(from, O_RDONLY)) < 0)
    {
        LogAddLine("Cannot read %s", from);
        return (-1);
    }
    fstat(fdin, &stf);
    if((stf.st_mode & S_IFMT) == S_IFDIR)
    {
        close(fdin);
        LogAddLine("%s is a directory", from);
        return (-2);
    }

    if(stat(to, &stt) >= 0)
    {
        /* Файл уже существует */
        if((stt.st_mode & S_IFMT) == S_IFDIR)
        {
            /* И это каталог */

            /* Выделить последнюю компоненту пути from */
            if((s = strrchr(from, '/')) && s[1])
                s++;
            else    s = from;

            dirname = to;

            /* Целевой файл - файл в этом каталоге */
            sprintf(newname, "%s/%s", to, s);
            to = newname;

            if(stat(to, &stt) < 0)
                goto not_exist;
        }

        if(stt.st_dev == stf.st_dev && stt.st_ino == stf.st_ino)
        {
            LogAddLine("%s: cannot copy file to itself", from);
            return (-3);
        }
        switch(stt.st_mode & S_IFMT)
        {
            case S_IFBLK:
            case S_IFCHR:
            case S_IFIFO:
                break;

            default:
                // LogAddLine("%s already exists, overwrite ? ", to);
                // fflush(stdout);

                // *answer = '\0';
                // gets(answer);

                // if(*answer != 'y')
                // {     /* NO */
                //    close(fdin);
                //    return (-4);
                // }
                break;
        }
    }

not_exist:

    LogAddLine("COPY %s TO %s\n", from, to);

    if((stf.st_mode & S_IFMT) == S_IFREG)
    {
        /* Проверка наличия свободного места в каталоге dirname */
        struct statvfs fs;
        char tmpbuf[MAXPATHLEN+1];

        if(dirname == NULL)
        {
            /* То 'to' - это имя файла, а не каталога */
            strcpy(tmpbuf, to);
            if(s = strrchr(tmpbuf, '/'))
            {
                if(*tmpbuf != '/' || s != tmpbuf)
                {
                    /* Имена "../xxx"
                    * и второй случай:
                    * абсолютные имена не в корне,
                    * то есть не "/" и не "/xxx"
                    */
                    *s = '\0';
                }
                else
                {
                    /* "/" или "/xxx" */
                    if(s[1]) s[1] = '\0';
                }
                dirname = tmpbuf;
            }
            else
                dirname = ".";
        }

        if(statvfs(dirname, &fs) >= 0)
        {
            size_t size = (geteuid() == 0 ) ?
                /* Доступно суперпользователю: байт */
                fs.f_frsize * fs.f_bfree :
                /* Доступно обычному пользователю: байт */
                fs.f_frsize * fs.f_bavail;

            if(size < stf.st_size)
            {
                LogAddLine("Not enough free space on %s: have %lu, need %lu", dirname, size, stf.st_size);
                close(fdin);
                return (-5);
            }
        }
    }

    if((fdout = creat(to, stf.st_mode)) < 0)
    {
        LogAddLine("Can't create %s", to);
        close(fdin);
        return (-6);
    }
    else
    {
        fchmod(fdout, stf.st_mode);
        fchown(fdout, stf.st_uid, stf.st_gid);
    }

    while (n = read (fdin, iobuf, sizeof iobuf))
    {
        if(n < 0)
        {
            LogAddLine("read error");
            code = (-7);
            goto done;
        }
        if(write (fdout, iobuf, n) != n)
        {
            LogAddLine("write error");
            code = (-8);
            goto done;
        }
    }

done:
    close (fdin);
    close (fdout);

    /* Проверить: соответствует ли результат ожиданиям */
    if(stat(to, &stt) >= 0 && (stt.st_mode & S_IFMT) == S_IFREG)
    {
        if(stf.st_size < stt.st_size)
        {
            LogAddLine("File has grown at the time of copying");
        }
        else
            if(stf.st_size > stt.st_size)
            {
                LogAddLine("File too short, target %s removed", to);
                unlink(to);
                code = (-9);
            }
    }
    return code;
}

/**
*   Удалить файл
*/
BOOL delFile( char *Filename)
{
    if (unlink(Filename) >= 0 )
        return TRUE;
    else
    {
        LogAddLine("Delete file %s error",Filename);
        return FALSE;
    }
}

char *ones[11],*teen[11] ,*decade[11],*hundred[11];

/**
*
*/
char *propisStr(int summa)
{
	char *str, *bak;
	int i,tmp;

  ones[10]="";teen[10]="";decade[10]="";hundred[10]="";
  ones[1]="один";ones[2]="два";
  ones[3]="три";ones[4]="четыре";ones[5]="пять";ones[6]="шесть";ones[7]="семь";
  ones[8]="восемь";ones[9]="девять";
  teen[1]="одинадцать";teen[2]="двенадцать";teen[3]="тринадцать";teen[4]="четырнадцать";
  teen[5]="пятнадцать";teen[6]="шестнадцать";teen[7]="семнадцать";teen[8]="восемнадцать";teen[9]="девятнадцать";
  decade[1]="десять";decade[2]="двадцать";decade[3]="тридцать";decade[4]="сорок";decade[5]="пятьдесят";decade[6]="шестьдесят";
  decade[7]=" семьдесят";decade[8]=" восемьдесят";decade[9]=" девяносто";
  hundred[1]="сто";hundred[2]="двести";hundred[3]="триста";hundred[4]="четыреста";hundred[5]="пятьсот";
  hundred[6]="шестьсот";hundred[7]="семьсот";hundred[8]="восемьсот";hundred[9]="девятьсот";


	i=summa/1000000;
	tmp=summa%1000000;
	str= new char[100];
	str[0]='\x00';
	konk(str,"Итого по листу:");
	bak=	_3(i , "один","миллион","два","миллиона","миллионов" );
	konk(str,bak);
    delete bak;

	i=tmp/1000;
	tmp=summa%1000;
	konk(str," ");
    bak= _3(i , "одна","тысяча","две","тысячи","тысяч" );
    konk(str,bak);
	delete bak;


	i=tmp;
	konk(str," ");
	bak=   _3(i, "одна","","две","","")  ;
	konk(str,  bak  );
	delete bak;

    str=trim(str); // 
	return str;
}


char *_3( int num, char *_1,char *n1,char *_2,char *n2,char *nn )
{
	char *ret;
	int i;

	ret=new char[500];
	ret[0]='\x00';

	i=num/100;
	if(i==0) i=10;
	konk(ret,hundred[i]);

	if( ((num%100)<20) &&((num%100)>10))
	{
		i=num%100-10;
		if(i==0) 
			i=10;
		konk(ret," ");
		konk(ret,teen[i]);
		konk(ret," ");
		konk(ret, nn);
		return( ret);
	}
	else
	{
		i=num/10;
		i=i%10;
		if(i==0)
			i=10;
		konk(ret," ");
		konk(ret,decade[i]);
	}
	i=num%10;
	if(i==0) i=10;
	if(num%10==0)
	{
	}else
	if(num%10==1)
	{
		konk(ret," ");
		konk(ret,_1);
		konk(ret," ");
		konk(ret,n1);
	} else 
	if(num%10==2)
	{
		konk(ret," ");
		konk(ret,_2);
		konk(ret," ");
		konk(ret,n2);
	} else
	if( (num%10>=3) && (num%10<=4)  )
	{
		konk(ret," ");
		konk(ret, ones[i]);
		konk(ret," ");
		konk(ret, n2);
	} else
	{
		konk(ret," ");
		konk(ret,ones[i]);
		konk(ret," ");
		konk(ret,nn);
	}
	return(ret);
}


/**
* Слияние строк
*/
char *konk(char *a1, char *a2)
{
	int i,len,len2;

	len=strlen(a1);
	len2=strlen(a2);
	for(i=0; i<len2; i++)
	{
		a1[len+i]=a2[i];
	}
	a1[len+i]='\x00';
	return(a1);
}

/**
* Усечение строки
*/
char *trim(char *str)
{
	int len;
	//unsigned i;
	char *ret;

	len=strlen(str);
	ret=new char[len+1];
	ret[0]='\x00';
	konk(ret,str);
/*	int i,j,flag=0;
	for(  i=0 ; i<len ; i++) // недоделано
	{
		if()  */
	delete str;
	return (ret);
}

static int to_utf8(char *from, char *to,char *codepage)
{
    size_t Lfrom, Lto;
    int ret;
    iconv_t d;
 
    d = iconv_open("UTF-8", codepage);
    Lfrom = (strlen(from)+1);
    Lto = 2 * Lfrom;
    ret = iconv(d, &from, &Lfrom, &to, &Lto);
    //ret = to_utf8(from, to, d);
    iconv_close(d);
    return ret;
}

char *cp1251_to_utf8(char *from)
{
    char *result;
    result=(char *)calloc(strlen(from)*2+1,sizeof(char));
    to_utf8(from,result,"CP1251");
    return result;
}

char *cp866_to_utf8(char *from)
{
    char *result;
    result=(char *)calloc(strlen(from)*2+1,sizeof(char));
    to_utf8(from,result,"CP866");
    return result;
}
   
double max(double a,double b)
{
    return (((a) > (b)) ? (a) : (b));
}

double min(double a,double b)
{
    return (((a) < (b)) ? (a) : (b));
}
