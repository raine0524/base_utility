#include "stdafx.h"
#include "KBASE/IniFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale>
#include <iostream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------------------
KIniFile::KIniFile(void)
{
	m_pEntry      = NULL;
	m_pCurEntry   = NULL;
	m_result [0]  = 0;
	m_pIniFile    = NULL;
}

//---------------------------------------------------------------------------------------
KIniFile::~KIniFile(void)
{
	FreeAllMem ();
}

#ifdef DONT_HAVE_STRUPR
//---------------------------------------------------------------------------------------
void _strupr( char *str )
{
    // We dont check the ptr because the original also dont do it.
    while (*str != 0)
    {
        if ( islower( *str ) )
        {
		     *str = toupper( *str );
        }
        str++;
	}
}
#endif

//---------------------------------------------------------------------------------------
// Opens an ini file or creates a new one if the requested file doesnt exists.
bool KIniFile::Open(const char* pFileName)
{
	char Str[255];
	char* pStr;
	struct ENTRY* pEntry;

	FreeAllMem();

	if(pFileName == NULL)
	{
		return false;
	}
	if((m_pIniFile = fopen(pFileName, "r")) == NULL)
	{
		return false;
	}

	while(fgets(Str, 255, m_pIniFile) != NULL)
	{
		pStr = strchr (Str, '\n');
		if(pStr != NULL)
		{
			*pStr = 0;
		}
		pEntry = MakeNewEntry();
		if(pEntry == NULL)
		{
			return false;
		}

		#ifdef INI_REMOVE_CR
		int Len = strlen(Str);
		if( Len > 0 )
		{
			if ( Str[Len-1] == '\r' )
			{
				Str[Len-1] = '\0';
			}
		}
		#endif

		pEntry->pText = (char *)malloc(strlen(Str)+1);
		if(pEntry->pText == NULL)
		{
			FreeAllMem();
			return false;
		}
		strcpy(pEntry->pText, Str);
		 /* Cut all comments */
		pStr = strchr(Str,';');
		if(pStr != NULL)
		{
			*pStr = 0;
		}
		/* Is Section */
		if((strstr (Str, "[") > 0) && (strstr (Str, "]") > 0)) 
		{
			pEntry->Type = tpSECTION;
		}
		else
		{
			if (strstr (Str, "=") > 0)
			{
				pEntry->Type = tpKEYVALUE;
			}
			else
			{
				pEntry->Type = tpCOMMENT;
			}
		}
		m_pCurEntry = pEntry;
	}
	fclose (m_pIniFile);
	m_pIniFile = NULL;
	return true;
}

//---------------------------------------------------------------------------------------
// Closes the ini file without any modifications. If you want to write the file use WriteIniFile instead.
void KIniFile::Close(void)
{
	FreeAllMem ();
	if(m_pIniFile != NULL)
	{
		fclose(m_pIniFile);
		m_pIniFile = NULL;
	}
}

//---------------------------------------------------------------------------------------
// Writes the iniFile to the disk and close it. Frees all memory allocated by WriteIniFile
bool KIniFile::WriteClose(const char* pFileName)
{
	struct ENTRY *pEntry = m_pEntry;
	if(m_pIniFile != NULL)
	{
		fclose(m_pIniFile);
	}
	if ((m_pIniFile = fopen (pFileName, "wb")) == NULL)
	{
		FreeAllMem ();
		return false;
	}

	while(pEntry != NULL)
	{
		if(pEntry->Type != tpNULL)
		{
			#ifdef INI_REMOVE_CR
			fprintf (m_pIniFile,"%s\n",pEntry->pText);
			#else
			fprintf (m_pIniFile,"%s\r\n",pEntry->pText);
			#endif
		}
		pEntry = pEntry->pNext;
	}

	fclose (m_pIniFile);
	m_pIniFile = NULL;
	return true;
}

//---------------------------------------------------------------------------------------
void KIniFile::WriteString (const char* pSection,const char* pKey,const char *pValue)
{
	EFIND List;
	char Str [255];

	if(ArePtrValid(pSection,pKey,pValue) == false)
	{
		return;
	}
	if(FindKey(pSection,pKey,&List) == true)
	{
		sprintf(Str,"%s=%s%s",List.KeyText,pValue,List.Comment);
		FreeMem (List.pKey->pText);
		List.pKey->pText = (char*)malloc(strlen(Str)+1);
		strcpy(List.pKey->pText,Str);
	}
	else
	{
		/* section exist, Key not */
		if((List.pSec != NULL) && (List.pKey == NULL)) 
		{
			AddKey(List.pSec,pKey,pValue);
		}
		else
		{
			AddSectionAndKey(pSection,pKey,pValue);
		}
	}
}

//---------------------------------------------------------------------------------------
void KIniFile::WriteBool(const char* pSection,const char* pKey,bool bValue)
{
	char Val[2] = {'0',0};
	if(bValue)
	{
		Val[0] = '1';
	}
	WriteString(pSection,pKey,Val);
}

//---------------------------------------------------------------------------------------
void KIniFile::WriteInt(const char* pSection,const char* pKey, int nValue)
{
	char Val[12]; /* 32bit maximum + sign + \0 */
	sprintf(Val,"%d",nValue);
	WriteString(pSection,pKey,Val);
}

//---------------------------------------------------------------------------------------
void KIniFile::WriteDouble(const char* pSection,const char* pKey, double dValue)
{
	char Val[32]; /* DDDDDDDDDDDDDDD+E308\0 */
	sprintf(Val,"%1.10lE",dValue);
	WriteString(pSection,pKey,Val);
}

//---------------------------------------------------------------------------------------
const char* KIniFile::ReadString(const char* pSection,const char* pKey,const char* pDefault)
{
	EFIND List;
	if(ArePtrValid(pSection,pKey,pDefault)== false)
	{
		return pDefault;
	}
	if(FindKey(pSection,pKey,&List) == true)
	{
		strcpy (m_result, List.ValText);
		return m_result;
	}
	return pDefault;
}

//---------------------------------------------------------------------------------------
bool KIniFile::ReadBool (const char *pSection,const char *pKey,bool bDefault)
{
	char Val[2]={"0"};
	if(bDefault)
	{
		Val[0]='1';
	}
	const char* pVal=ReadString(pSection,pKey,Val);
	return pVal[0]=='1'?true:false;
}

//---------------------------------------------------------------------------------------
int KIniFile::ReadInt(const char* pSection,const char* pKey,int nDefault)
{
	char Val[12];
	sprintf(Val,"%d",nDefault);
	return (atoi(ReadString(pSection,pKey,Val)));
}

//---------------------------------------------------------------------------------------
double KIniFile::ReadDouble(const char* pSection,const char* pKey,double dDefault)
{
	double Val;
	sprintf(m_result,"%1.10lE",dDefault);
	sscanf(ReadString(pSection,pKey,m_result),"%lE",&Val);
	return Val;
}

//---------------------------------------------------------------------------------------
bool KIniFile::DeleteKey(const char* pSection,const char* pKey)
{
	EFIND List;
	struct ENTRY* pPrev;
	struct ENTRY* pNext;

	if(FindKey(pSection,pKey,&List) == true)
	{
		pPrev = List.pKey->pPrev;
		pNext = List.pKey->pNext;
		if (pPrev)
		{
			pPrev->pNext=pNext;
		}
		if (pNext)
		{ 
			pNext->pPrev=pPrev;
		}
		FreeMem(List.pKey->pText);
		FreeMem(List.pKey);
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------------------
void KIniFile::FreeMem(void* pPtr)
{
	if(pPtr != NULL)
	{
		free (pPtr);
	}
}

//---------------------------------------------------------------------------------------
void KIniFile::FreeAllMem (void)
{
	struct ENTRY* pEntry;
	struct ENTRY* pNextEntry;
	pEntry = m_pEntry;
	while(true)
	{
		if(pEntry == NULL)
		{
			break;
		}
		pNextEntry = pEntry->pNext;
		/* Frees the pointer if not NULL */
		FreeMem(pEntry->pText);
		FreeMem(pEntry);
		pEntry=pNextEntry;
	}
	m_pEntry = NULL;
	m_pCurEntry = NULL;
}

//---------------------------------------------------------------------------------------
struct ENTRY* KIniFile::FindSection(const char* pSection)
{
	char Sec[130];
	char iSec[130];
	struct ENTRY* pEntry;
	sprintf(Sec,"[%s]",pSection);
	_strupr(Sec);

	/* Get a pointer to the first Entry */
	pEntry = m_pEntry; 
	while(pEntry != NULL)
	{
		if(pEntry->Type == tpSECTION)
		{
			strcpy(iSec,pEntry->pText);
			_strupr(iSec);
			if(strcmp(Sec,iSec) == 0)
			{
				return pEntry;
			}
		}
		pEntry = pEntry->pNext;
	}
	return NULL;
}

//---------------------------------------------------------------------------------------
bool KIniFile::FindKey(const char* pSection,const char* pKey, EFIND* pList)
{
	char Search[130];
	char Found[130];
	char Text[255];
	char* pText;
	struct ENTRY* pEntry;
	pList->pSec = NULL;
	pList->pKey = NULL;

	pEntry = FindSection(pSection);
	if(pEntry == NULL)
	{
		return false;
	}
	pList->pSec = pEntry;
	pList->KeyText[0] = 0;
	pList->ValText[0] = 0;
	pList->Comment[0] = 0;
	pEntry = pEntry->pNext;
	if(pEntry == NULL)
	{
		return false;
	}
	sprintf(Search, "%s",pKey);
	_strupr(Search);
	while(pEntry != NULL)
	{
		/* Stop after next section or EOF */
		if((pEntry->Type == tpSECTION) || (pEntry->Type == tpNULL))
		{
			return false;
		}
		if(pEntry->Type == tpKEYVALUE)
		{
			strcpy(Text,pEntry->pText);
			pText = strchr(Text, ';');
			if(pText != NULL)
			{
				strcpy(pList->Comment,pText);
				*pText = 0;
			}
			pText = strchr(Text, '=');
			if(pText != NULL)
			{
				*pText = 0;
				strcpy(pList->KeyText,Text);
				strcpy(Found,Text);
				*pText = '=';
				_strupr(Found);

				/* printf ("%s,%s\n", Search, Found); */
				if(strcmp(Found,Search) == 0)
				{
				   strcpy (pList->ValText, pText+1);
				   pList->pKey = pEntry;
				   return true;
				}
			}
		}
		pEntry = pEntry->pNext;
	}
	return false;
}

//---------------------------------------------------------------------------------------
bool KIniFile::AddItem(char Type,const char* pText)
{
	struct ENTRY* pEntry = MakeNewEntry();
	if(pEntry == NULL)
	{
		return false;
	}
	pEntry->Type = Type;
	pEntry->pText = (char*)malloc(strlen(pText)+1);
	if(pEntry->pText == NULL)
	{
		free (pEntry);
		return false;
	}
	strcpy(pEntry->pText,pText);
	pEntry->pNext = NULL;
	if(m_pCurEntry != NULL)
	{
		m_pCurEntry->pNext = pEntry;
	}
	m_pCurEntry = pEntry;
	return true;
}

//---------------------------------------------------------------------------------------
bool KIniFile::AddItemAt (struct ENTRY* pEntryAt,char Mode, const char* pText)
{
	struct ENTRY* pNewEntry;
	if(pEntryAt == NULL)
	{
		return false;
	}
	pNewEntry = (struct ENTRY*)malloc(sizeof(ENTRY));
	if(pNewEntry == NULL)
	{
		return false;
	}
	pNewEntry->pText = (char *)malloc(strlen(pText)+1);
	if(pNewEntry->pText == NULL)
	{
		free(pNewEntry);
		return false;
	}
	strcpy(pNewEntry->pText,pText);
	/* No following nodes. */
	if(pEntryAt->pNext == NULL)
	{
		pEntryAt->pNext  = pNewEntry;
		pNewEntry->pNext = NULL;
	}
	else
	{
		pNewEntry->pNext = pEntryAt->pNext;
		pEntryAt->pNext  = pNewEntry;
	}
	pNewEntry->pPrev = pEntryAt;
	pNewEntry->Type  = Mode;
	return true;
}

//---------------------------------------------------------------------------------------
bool KIniFile::AddSectionAndKey(const char* pSection,const char* pKey,const char* pValue)
{
	char Text[255];
	sprintf(Text,"[%s]",pSection);
	if(AddItem(tpSECTION, Text) == false)
	{
		return false;
	}
	sprintf(Text,"%s=%s",pKey,pValue);
	return AddItem(tpKEYVALUE,Text)?1:0;
}

//---------------------------------------------------------------------------------------
void KIniFile::AddKey(struct ENTRY* pSecEntry,const char* pKey,const char* pValue)
{
	char Text[255];
	sprintf(Text,"%s=%s",pKey,pValue);
	AddItemAt(pSecEntry,tpKEYVALUE,Text);
}

//---------------------------------------------------------------------------------------
struct ENTRY* KIniFile::MakeNewEntry (void)
{
	struct ENTRY* pEntry;
	pEntry = (struct ENTRY*)malloc(sizeof(ENTRY));
	if(pEntry == NULL)
	{
		FreeAllMem ();
		return NULL;
	}
	if(m_pEntry == NULL)
	{
		m_pEntry = pEntry;
	}
	pEntry->Type  = tpNULL;
	pEntry->pPrev = m_pCurEntry;
	pEntry->pNext = NULL;
	pEntry->pText = NULL;
	if(m_pCurEntry != NULL)
	{
		m_pCurEntry->pNext = pEntry;
	}
	return pEntry;
}
