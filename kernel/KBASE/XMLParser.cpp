/***********************************************************
 * Name: KKXMLParser.cpp
 * Author: zhouxiaochuan@gmail.com
 * KDateTime: 2006-10-09
 *
 * NOTE:
 *   If you add "#define STRICT_PARSING", on the first line of this file
 *   the parser will see the following XML-stream:
 *      <a><b>some text</b><b>other text    </a>
 *   as an error. Otherwise, this tring will be equivalent to:
 *      <a><b>some text</b><b>other text</b></a>
 *
 * NOTE:
 *   If you add "#define APPROXIMATE_PARSING" on the first line of this file
 *   the parser will see the following XML-stream:
 *     <data name="n1">
 *     <data name="n2">
 *     <data name="n3" />
 *   as equivalent to the following XML-stream:
 *     <data name="n1" />
 *     <data name="n2" />
 *     <data name="n3" />
 *   This can be useful for badly-formed XML-streams but prevent the use
 *   of the following XML-stream (problem is: tags at contiguous levels
 *   have the same names):
 *     <data name="n1">
 *        <data name="n2">
 *            <data name="n3" />
 *        </data>
 *     </data>
 *
 ***********************************************************/

#include "stdafx.h"
#include "KBASE/XMLParser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <memory.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

inline int GetMinValue( const int t1, const int t2 ) { return t1 < t2 ? t1 : t2; }

// You can modify the initialization of the variable "g_XMLAllClearTags" below
// to change the clearTags that are currently recognized by the library.
// The number on the second columns is the length of the string inside the
// first column.
XML_ALLCLEARTAG g_XMLAllClearTags[] =
{
    {    "<![CDATA[",9,  "]]>"      },
    {    "<PRE>"    ,5,  "</PRE>"   },
    {    "<Script>" ,8,  "</Script>"},
    {    "<!--"     ,4,  "-->"      },
    {    "<!DOCTYPE",9,  ">"        },
    {    NULL       ,0,  NULL       }
};

// You can modify the initialization of the variable "XMLEntities" below
// to change the character entities that are currently recognized by the library.
// The number on the second columns is the length of the string inside the
// first column. Additionally, the syntaxes "&#xA0;" and "&#160;" are recognized.
typedef struct { const char* s; int l; char c;} XMLCharacterEntity;
static XMLCharacterEntity XMLEntities[] =
{
    { "&amp;" , 5, '&' },
    { "&lt;"  , 4, '<' },
    { "&gt;"  , 4, '>' },
    { "&quot;", 6, '\"'},
    { "&apos;", 6, '\''},
    { NULL    , 0, '\0'}
};

// When rendering the XMLNode to a string (using the "CreateXMLString" function),
// you can ask for a beautiful formatting. This formatting is using the
// following indentation character:
#define INDENTCHAR '\t'

//---------------------------------------------------------------------------------------
// The following function parses the XML errors into a user friendly string.
// You can edit this to change the output language of the library to something else.
const char* XMLNode::getError(XML_ERR xerror)
{
    switch (xerror)
    {
    case eXMLErrorNone:								return "No error";
    case eXMLErrorMissingEndTag:					return "Warning: Unmatched end tag";
    case eXMLErrorEmpty:							return "Error: No XML data";
    case eXMLErrorFirstNotStartTag:					return "Error: First token not start tag";
    case eXMLErrorMissingTagName:					return "Error: Missing start tag name";
    case eXMLErrorMissingEndTagName:				return "Error: Missing end tag name";
    case eXMLErrorNoMatchingQuote:					return "Error: Unmatched quote";
    case eXMLErrorUnmatchedEndTag:					return "Error: Unmatched end tag";
    case eXMLErrorUnmatchedEndClearTag:				return "Error: Unmatched clear tag end";
    case eXMLErrorUnexpectedToken:					return "Error: Unexpected token found";
    case eXMLErrorInvalidTag:						return "Error: Invalid tag found";
    case eXMLErrorNoElements:						return "Error: No elements found";
    case eXMLErrorFileNotFound:						return "Error: File not found";
    case eXMLErrorFirstTagNotFound:					return "Error: First Tag not found";
    case eXMLErrorUnknownEscapeSequence:			return "Error: Unknown character entity";
    case eXMLErrorCharConversionError:				return "Error: unable to convert between UNICODE and MultiByte chars";
    case eXMLErrorCannotOpenWriteFile:				return "Error: unable to open file for writing";
    case eXMLErrorCannotWriteFile:					return "Error: cannot write into file";
    case eXMLErrorBase64DataSizeIsNotMultipleOf4:	return "Warning: Base64-string length is not a multiple of 4";
    case eXMLErrorBase64DecodeTruncatedData:		return "Warning: Base64-string is truncated";
    case eXMLErrorBase64DecodeIllegalCharacter:		return "Error: Base64-string contains an illegal character";
    case eXMLErrorBase64DecodeBufferTooSmall:		return "Error: Base64 Decode output buffer is too small";
    };
    return "Unknown";
}

//---------------------------------------------------------------------------------------
// If "strictUTF8Parsing=0" then we assume that all characters have the same length of 1 byte.
// If "strictUTF8Parsing=1" then the characters have different lengths (from 1 byte to 4 bytes).
// This table is used as lookup-table to know the length of a character (in byte) based on the
// content of the first byte of the character.
// (note: if you modify this, you must always have XML_utf8ByteTable[0]=0 ).
static const char XML_utf8ByteTable[256] =
{
//  0 1 2 3 4 5 6 7 8 9 a b c d e f
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,// 0x00
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,// 0x10
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,// 0x20
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,// 0x30
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,// 0x40
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,// 0x50
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,// 0x60
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,// 0x70End of ASCII range
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,// 0x80 0x80 to 0xc1 invalid
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,// 0x90
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,// 0xa0
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,// 0xb0
    1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,// 0xc0 0xc2 to 0xdf 2 byte
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,// 0xd0
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,// 0xe0 0xe0 to 0xef 3 byte
    4,4,4,4,4,1,1,1,1,1,1,1,1,1,1,1 // 0xf0 0xf0 to 0xf4 4 byte, 0xf5 and higher invalid
};

//---------------------------------------------------------------------------------------
// Here is an abstraction layer to access some common string manipulation functions.
// The abstraction layer is currently working for gcc, Microsoft Visual Studio 6.0,
// Microsoft Visual Studio .NET, CC (sun compiler) and Borland C++.
// If you plan to "port" the library to a new system/compiler, all you have to do is
// to edit the following lines.

//char myIsTextUnicode(const void *b, int len) // inspired by the Wine API: RtlIsTextUnicode
//{
//    const wchar_t *s=(const wchar_t*)b;
//
//    // buffer too small:
//    if (len<(int)sizeof(wchar_t)) return 0;
//
//    // odd length test
//    if (len&1) return 0;
//
//    /* only checks the first 256 characters */
//    len=GetMinValue(256,len/sizeof(wchar_t));
//
//    // Check for the special byte order:
//    if (*s == 0xFFFE) return 0;     // IS_TEXT_UNICODE_REVERSE_SIGNATURE;
//    if (*s == 0xFEFF) return 1;      // IS_TEXT_UNICODE_SIGNATURE
//
//    // checks for ASCII characters in the UNICODE stream
//    int i,stats=0;
//    for (i=0; i<len; i++) if (s[i]<=(unsigned short)255) stats++;
//    if (stats>len/2) return 1;
//
//    // Check for UNICODE NULL chars
//    for (i=0; i<len; i++) if (!s[i]) return 1;
//
//    return 0;
//}
//
//char *myWideCharToMultiByte(const wchar_t *s, int l)
//{
//    const wchar_t *ss=s;
//    int i=(int)wcsrtombs(NULL,&ss,0,NULL);
//    if (i<0) return NULL;
//    char *d=(char *)malloc(i+1);
//    wcsrtombs(d,&s,i,NULL);
//    d[i]=0;
//    return d;
//}

//FILE *_tfopen(const char* filename,const char* mode) { return fopen(filename,mode); }
//int strlen(const char* c)   { return strlen(c); }
//int _strnicmp(const char* c1, const char* c2, int l) { return strncasecmp(c1,c2,l);}

//char* strstr(const char* c1, const char* c2) { return (char*)strstr(c1,c2); }
//char* strcpy(char* c1, const char* c2) { return (char*)strcpy(c1,c2); }

#ifndef WIN32
int _stricmp(const char* c1, const char* c2) { return strcasecmp(c1,c2); }
int _strnicmp(const char *c1,const char *c2, int l) { return strncasecmp(c1,c2,l);}
#endif

/////////////////////////////////////////////////////////////////////////
//      Here start the core implementation of the KXMLParser library    //
/////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// You should normally not change anything below this point.
// For your own information, I suggest that you read the OpenFile below:
XMLNode XMLNode::OpenFile(const char* filename, const char* tag)
{
	XMLNode xnode;
    XML_RESULTS rResults;

	// guess the value of the global parameter "strictUTF8Parsing"
    // (the guess is based on the first 200 bytes of the file).
    FILE* fp=fopen(filename,"rb");
    if(fp==NULL)
	{
		return xnode;
	}
	else
    {
        char bb[205];
        int l=(int)fread(bb,1,200,fp);
        SetGlobalOptions(1,GuessUTF8ParsingParameterValue(bb,l,1));
        fclose(fp);
		xnode=XMLNode::ParseFile(filename,tag,&rResults);
    }

    // display error message (if any)
    if (rResults.error != eXMLErrorNone)
    {
        // create message
        char message[2000],*s1=(char*)"",*s3=(char*)""; const char* s2="";
        if(rResults.error==eXMLErrorFirstTagNotFound)
		{
			s1=(char*)"First Tag should be '";
			s2=tag;
			s3=(char*)"'.\n";
		}
        sprintf(message,"XML Parsing error inside file '%s'.\n%s\nAt line %i, column %i.\n%s%s%s",filename,XMLNode::getError(rResults.error),rResults.nLine,rResults.nColumn,s1,s2,s3);
        // display message
#if defined(WIN32) && !defined(_CONSOLE) 
		TRACE(message);
#else
        printf("%s",message);
#endif
        //exit(255);
    }
    return xnode;
}

static char guessUnicodeChars=1;

static const char XML_asciiByteTable[256] =
{
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};
static const char *XML_ByteTable=(const char *)XML_utf8ByteTable; // the default is "strictUTF8Parsing=1"

//---------------------------------------------------------------------------------------
XML_ERR XMLNode::WriteToFile(const char* filename, const char *encoding, char nFormat) const
{
    int i;
    char* t=CreateXMLString(nFormat,&i);
    FILE *f=fopen(filename,"wb");
    if (!f) return eXMLErrorCannotOpenWriteFile;
    if (!IsDeclaration())
    {
        if ((!encoding)||(XML_ByteTable==XML_utf8ByteTable))
        {
            // header so that windows recognize the file as UTF-8:
            unsigned char h[3]={0xEF,0xBB,0xBF};
            if (!fwrite(h,3,1,f)) return eXMLErrorCannotWriteFile;
            if (!fwrite("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n",39,1,f)) return eXMLErrorCannotWriteFile;
        }
        else
            if (fprintf(f,"<?xml version=\"1.0\" encoding=\"%s\"?>\n",encoding)<0) return eXMLErrorCannotWriteFile;
    } else
    {
        if (XML_ByteTable==XML_utf8ByteTable) // test if strictUTF8Parsing==1"
        {
            unsigned char h[3]={0xEF,0xBB,0xBF}; if (!fwrite(h,3,1,f)) return eXMLErrorCannotWriteFile;
        }
    }

	if (!fwrite(t,sizeof(char)*i,1,f)) return eXMLErrorCannotWriteFile;
    if (fclose(f)!=0) return eXMLErrorCannotWriteFile;
    free(t);
    return eXMLErrorNone;
}

//---------------------------------------------------------------------------------------
// Duplicate a given string.
char* StringDup(const char* lpszData, int cbData)
{
    if (lpszData==NULL) return NULL;

    char* lpszNew;
    if (cbData==0) cbData=(int)strlen(lpszData);
    lpszNew = (char*)malloc((cbData+1) * sizeof(char));
    if (lpszNew)
    {
        memcpy(lpszNew, lpszData, (cbData) * sizeof(char));
        lpszNew[cbData] = (char)NULL;
    }
    return lpszNew;
}

XMLNode XMLNode::emptyXMLNode;
XML_CLEAR XMLNode::emptyXMLClear={ NULL, NULL, NULL};
XML_ATTRIB XMLNode::emptyXMLAttribute={ NULL, NULL};

//---------------------------------------------------------------------------------------
// Enumeration used to decipher what type a token is
typedef enum XMLTokenTypeTag
{
    eTokenText = 0,
    eTokenQuotedText,
    eTokenTagStart,         /* "<"            */
    eTokenTagEnd,           /* "</"           */
    eTokenCloseTag,         /* ">"            */
    eTokenEquals,           /* "="            */
    eTokenDeclaration,      /* "<?"           */
    eTokenShortHandClose,   /* "/>"           */
    eTokenClear,
    eTokenError
} XMLTokenType;

// Main structure used for parsing XML
typedef struct XML
{
    const char*                lpXML;
    int                    nIndex,nIndexMissigEndTag;
    enum XML_ERR          error;
    const char*                lpEndTag;
    int                    cbEndTag;
    const char*                lpNewElement;
    int                    cbNewElement;
    int                    nFirst;
} XML;

typedef struct
{
    XML_ALLCLEARTAG *pClr;
    const char*     pStr;
} NextToken;

// Enumeration used when parsing attributes
typedef enum Attrib
{
    eAttribName = 0,
    eAttribEquals,
    eAttribValue
} Attrib;

// Enumeration used when parsing elements to dictate whether we are currently
// inside a tag
typedef enum Status
{
    eInsideTag = 0,
    eOutsideTag
} Status;

//---------------------------------------------------------------------------------------
// private (used while rendering):
char* ToXMLString(char* dest,const char* source)
{
    char* dd=dest;
    char ch;
    XMLCharacterEntity *entity;
    while ((ch=*source))
    {
        entity=XMLEntities;
        do
        {
            if (ch==entity->c) {strcpy(dest,entity->s); dest+=entity->l; source++; goto out_of_loop1; }
            entity++;
        } while(entity->s);
        switch(XML_ByteTable[(unsigned char)ch])
        {
        case 4: *(dest++)=*(source++);
        case 3: *(dest++)=*(source++);
        case 2: *(dest++)=*(source++);
        case 1: *(dest++)=*(source++);
        }
out_of_loop1:
        ;
    }
    *dest=0;
    return dd;
}

//---------------------------------------------------------------------------------------
// private (used while rendering):
int lengthXMLString(const char* source)
{
    int r=0;
    XMLCharacterEntity *entity;
    char ch;
    while ((ch=*source))
    {
        entity=XMLEntities;
        do
        {
            if (ch==entity->c) { r+=entity->l; source++; goto out_of_loop1; }
            entity++;
        } while(entity->s);
        ch=XML_ByteTable[(unsigned char)ch]; r+=ch; source+=ch;
out_of_loop1:
        ;
    }
    return r;
}

//---------------------------------------------------------------------------------------
char* ToXMLString(const char* source)
{
    char* dest=(char*)malloc((lengthXMLString(source)+1)*sizeof(char));
    return ToXMLString(dest,source);
}

//---------------------------------------------------------------------------------------
char* ToXMLStringFast(char* *dest,int *destSz, const char* source)
{
    int l=lengthXMLString(source)+1;
    if (l>*destSz) { *destSz=l; *dest=(char*)realloc(*dest,l*sizeof(char)); }
    return ToXMLString(*dest,source);
}

//---------------------------------------------------------------------------------------
// private:
char* fromXMLString(const char* s, int lo, XML *pXML)
{
    // This function is the opposite of the function "ToXMLString". It decodes the escape
    // sequences &amp;, &quot;, &apos;, &lt;, &gt; and replace them by the characters
    // &,",',<,>. This function is used internally by the XML Parser. All the calls to
    // the XML library will always gives you back "decoded" strings.
    //
    // in: string (s) and length (lo) of string
    // out:  new allocated string converted from xml
    if (!s) return NULL;

    int ll=0,j;
    char* d;
    const char* ss=s;
    XMLCharacterEntity *entity;
    while ((lo>0)&&(*s))
    {
        if (*s=='&')
        {
            if ((lo>2)&&(s[1]=='#'))
            {
                s+=2; lo-=2;
                if ((*s=='X')||(*s=='x')) { s++; lo--; }
                while ((*s)&&(*s!=';')&&((lo--)>0)) s++;
                if (*s!=';')
                {
                    pXML->error=eXMLErrorUnknownEscapeSequence;
                    return NULL;
                }
                s++; lo--;
            } else
            {
                entity=XMLEntities;
                do
                {
                    if ((lo>=entity->l)&&(_strnicmp(s,entity->s,entity->l)==0)) { s+=entity->l; lo-=entity->l; break; }
                    entity++;
                } while(entity->s);
                if (!entity->s)
                {
                    pXML->error=eXMLErrorUnknownEscapeSequence;
                    return NULL;
                }
            }
        } else
        {
            j=XML_ByteTable[(unsigned char)*s]; s+=j; lo-=j; ll+=j-1;
        }
        ll++;
    }

    d=(char*)malloc((ll+1)*sizeof(char));
    s=d;
    while (ll-->0)
    {
        if (*ss=='&')
        {
            if (ss[1]=='#')
            {
                ss+=2; j=0;
                if ((*ss=='X')||(*ss=='x'))
                {
                    ss++;
                    while (*ss!=';')
                    {
                        if ((*ss>='0')&&(*ss<='9')) j=(j<<4)+*ss-'0';
                        else if ((*ss>='A')&&(*ss<='F')) j=(j<<4)+*ss-'A'+10;
                        else if ((*ss>='a')&&(*ss<='f')) j=(j<<4)+*ss-'a'+10;
                        else { free(d); pXML->error=eXMLErrorUnknownEscapeSequence;return NULL;}
                        ss++;
                    }
                } else
                {
                    while (*ss!=';')
                    {
                        if ((*ss>='0')&&(*ss<='9')) j=(j*10)+*ss-'0';
                        else { free(d); pXML->error=eXMLErrorUnknownEscapeSequence;return NULL;}
                        ss++;
                    }
                }
                (*d++)=(char)j; ss++;
            } else
            {
                entity=XMLEntities;
                do
                {
                    if (_strnicmp(ss,entity->s,entity->l)==0) { *(d++)=entity->c; ss+=entity->l; break; }
                    entity++;
                } while(entity->s);
            }
        } else
        {
            switch(XML_ByteTable[(unsigned char)*ss])
            {
            case 4: *(d++)=*(ss++); ll--;
            case 3: *(d++)=*(ss++); ll--;
            case 2: *(d++)=*(ss++); ll--;
            case 1: *(d++)=*(ss++);
            }
        }
    }
    *d=0;
    return (char*)s;
}

#define XML_isSPACECHAR(ch) ((ch=='\n')||(ch==' ')||(ch== '\t')||(ch=='\r'))

//---------------------------------------------------------------------------------------
// private:
char myTagCompare(const char* cclose, const char* copen)
// !!!! WARNING strange convention&:
// return 0 if equals
// return 1 if different
{
    if (!cclose) return 1;
    int l=(int)strlen(cclose);
    if (_strnicmp(cclose, copen, l)!=0) return 1;
    const char c=copen[l];
    if (XML_isSPACECHAR(c)||
        (c=='/' )||
        (c=='<' )||
        (c=='>' )||
        (c=='=' )) return 0;
    return 1;
}

//---------------------------------------------------------------------------------------
// Obtain the next character from the string.
static inline char getNextChar(XML *pXML)
{
    char ch = pXML->lpXML[pXML->nIndex];
    pXML->nIndex+=XML_ByteTable[(unsigned char)ch];
    return ch;
}

//---------------------------------------------------------------------------------------
// Find the next token in a string.
// pcbToken contains the number of characters that have been read.
static NextToken GetNextToken(XML *pXML, int *pcbToken, enum XMLTokenTypeTag *pType)
{
    NextToken        result;
    char            ch;
    char            chTemp;
    int              indexStart,nFoundMatch,nIsText=0;
    result.pClr=NULL; // prevent warning

    // Find next non-white space character
    do { indexStart=pXML->nIndex; ch=getNextChar(pXML); } while XML_isSPACECHAR(ch);

    if (ch)
    {
        // Cache the current string pointer
        result.pStr = &pXML->lpXML[indexStart];

        // First check whether the token is in the clear tag list (meaning it
        // does not need formatting).
        XML_ALLCLEARTAG *ctag=g_XMLAllClearTags;
        do
        {
            if (_strnicmp(ctag->lpszOpen, result.pStr, ctag->openTagLen)==0)
            {
                result.pClr=ctag;
                pXML->nIndex+=ctag->openTagLen-1;
                *pType=eTokenClear;
                return result;
            }
            ctag++;
        } while(ctag->lpszOpen);

        // If we didn't find a clear tag then check for standard tokens
        switch(ch)
        {
        // Check for quotes
        case '\'':
        case '\"':
            // Type of token
            *pType = eTokenQuotedText;
            chTemp = ch;

            // Set the size
            nFoundMatch = 0;

            // Search through the string to find a matching quote
            while((ch = getNextChar(pXML)))
            {
                if (ch==chTemp) { nFoundMatch = 1; break; }
                if (ch=='<') break;
            }

            // If we failed to find a matching quote
            if (nFoundMatch == 0)
            {
                pXML->nIndex=indexStart+1;
                nIsText=1;
                break;
            }

//  4.02.2002
//            if (FindNonWhiteSpace(pXML)) pXML->nIndex--;

            break;

        // Equals (used with attribute values)
        case '=':
            *pType = eTokenEquals;
            break;

        // Close tag
        case '>':
            *pType = eTokenCloseTag;
            break;

        // Check for tag start and tag end
        case '<':

            // Peek at the next character to see if we have an end tag '</',
            // or an xml declaration '<?'
            chTemp = pXML->lpXML[pXML->nIndex];

            // If we have a tag end...
            if (chTemp == '/')
            {
                // Set the type and ensure we point at the next character
                getNextChar(pXML);
                *pType = eTokenTagEnd;
            }

            // If we have an XML declaration tag
            else if (chTemp == '?')
            {

                // Set the type and ensure we point at the next character
                getNextChar(pXML);
                *pType = eTokenDeclaration;
            }

            // Otherwise we must have a start tag
            else
            {
                *pType = eTokenTagStart;
            }
            break;

        // Check to see if we have a short hand type end tag ('/>').
        case '/':

            // Peek at the next character to see if we have a short end tag '/>'
            chTemp = pXML->lpXML[pXML->nIndex];

            // If we have a short hand end tag...
            if (chTemp == '>')
            {
                // Set the type and ensure we point at the next character
                getNextChar(pXML);
                *pType = eTokenShortHandClose;
                break;
            }

            // If we haven't found a short hand closing tag then drop into the
            // text process

        // Other characters
        default:
            nIsText = 1;
        }

        // If this is a TEXT node
        if (nIsText)
        {
            // Indicate we are dealing with text
            *pType = eTokenText;
            while((ch = getNextChar(pXML)))
            {
                if XML_isSPACECHAR(ch)
                {
                    indexStart++; break;

                } else if (ch=='/')
                {
                    // If we find a slash then this maybe text or a short hand end tag
                    // Peek at the next character to see it we have short hand end tag
                    ch=pXML->lpXML[pXML->nIndex];
                    // If we found a short hand end tag then we need to exit the loop
                    if (ch=='>') { pXML->nIndex--; break; }

                } else if ((ch=='<')||(ch=='>')||(ch=='='))
                {
                    pXML->nIndex--; break;
                }
            }
        }
        *pcbToken = pXML->nIndex-indexStart;
    } else
    {
        // If we failed to obtain a valid character
        *pcbToken = 0;
        *pType = eTokenError;
        result.pStr=NULL;
    }

    return result;
}

//---------------------------------------------------------------------------------------
const char* XMLNode::UpdateName_WOSD(const char* lpszName)
{
	if (d->lpszName&&(lpszName!=d->lpszName)) free((void*)d->lpszName);
	d->lpszName=lpszName;
    return lpszName;
}

//---------------------------------------------------------------------------------------
// private:
XMLNode::XMLNode(PXML_NODEDATA p){ d=p; (p->ref_count)++; }
XMLNode::XMLNode(PXML_NODEDATA pParent, const char* lpszName, char IsDeclaration)
{
    d=(XML_NODEDATA*)malloc(sizeof(XML_NODEDATA));
    d->ref_count=1;

	d->lpszName=NULL;
    d->nChild= 0;
    d->nText = 0;
    d->nClear = 0;
    d->nAttribute = 0;

    d->IsDeclaration = IsDeclaration;

    d->pParent = pParent;
    d->pChild= NULL;
    d->pText= NULL;
    d->pClear= NULL;
    d->pAttribute= NULL;
    d->pOrder= NULL;

	UpdateName_WOSD(lpszName);
}

//---------------------------------------------------------------------------------------
XMLNode XMLNode::CreateXMLTopNode_WOSD(const char* lpszName, char IsDeclaration) { return XMLNode(NULL,lpszName,IsDeclaration); }
XMLNode XMLNode::CreateXMLTopNode(const char* lpszName, char IsDeclaration) { return XMLNode(NULL,StringDup(lpszName),IsDeclaration); }

#define MEMORYINCREASE 50
static int memoryIncrease=0;

//---------------------------------------------------------------------------------------
static inline void *myRealloc(void *p, int newsize, int memInc, int sizeofElem)
{
    if (p==NULL) { if (memInc) return malloc(memInc*sizeofElem); return malloc(sizeofElem); }
    if ((memInc==0)||((newsize%memInc)==0)) p=realloc(p,(newsize+memInc)*sizeofElem);
//    if (!p)
//    {
//        printf("KXMLParser Error: Not enough memory! Aborting...\n"); exit(220);
//    }
    return p;
}

//---------------------------------------------------------------------------------------
// private:
int XMLNode::FindPosition(PXML_NODEDATA d, int index, XML_ELEMENTTYPE xtype)
{ 
    if (index<0) return -1;
    int i=0,j=(int)((index<<2)+xtype),*o=d->pOrder; while (o[i]!=j) i++; return i;
}

//---------------------------------------------------------------------------------------
// private:
// update "order" information when deleting a content of a XMLNode
int XMLNode::RemoveOrderElement(PXML_NODEDATA d, XML_ELEMENTTYPE t, int index)
{
    int n=d->nChild+d->nText+d->nClear, *o=d->pOrder,i=FindPosition(d,index,t);
    memmove(o+i, o+i+1, (n-i)*sizeof(int));
    for (;i<n;i++)
        if ((o[i]&3)==(int)t) o[i]-=4;
    // We should normally do:
    // d->pOrder=(int)realloc(d->pOrder,n*sizeof(int));
    // but we skip reallocation because it's too time consuming.
    // Anyway, at the end, it will be free'd completely at once.
    return i;
}

//---------------------------------------------------------------------------------------
void *XMLNode::AddToOrder(int *_pos, int nc, void *p, int size, XML_ELEMENTTYPE xtype)
{
    //  in: *_pos is the position inside d->pOrder ("-1" means "EndOf")
    // out: *_pos is the index inside p
    p=myRealloc(p,(nc+1),memoryIncrease,size);
    int n=d->nChild+d->nText+d->nClear;
    d->pOrder=(int*)myRealloc(d->pOrder,n+1,memoryIncrease*3,sizeof(int));
    int pos=*_pos,*o=d->pOrder;

    if ((pos<0)||(pos>=n)) { *_pos=nc; o[n]=(int)((nc<<2)+xtype); return p; }

    int i=pos;
    memmove(o+i+1, o+i, (n-i)*sizeof(int));

    while ((pos<n)&&((o[pos]&3)!=(int)xtype)) pos++; 
    if (pos==n) { *_pos=nc; o[n]=(int)((nc<<2)+xtype); return p; }

    o[i]=o[pos];
    for (i=pos+1;i<=n;i++) if ((o[i]&3)==(int)xtype) o[i]+=4;

    *_pos=pos=o[pos]>>2;
    memmove(((char*)p)+(pos+1)*size,((char*)p)+pos*size,(nc-pos)*size);

    return p;
}

//---------------------------------------------------------------------------------------
// Add a child node to the given element.
XMLNode XMLNode::AddChild_WOSD(const char* lpszName, char IsDeclaration, int pos)
{
    if (!lpszName) return emptyXMLNode;
    d->pChild=(XMLNode*)AddToOrder(&pos,d->nChild,d->pChild,sizeof(XMLNode),eNodeChild);
    d->pChild[pos].d=NULL;
    d->pChild[pos]=XMLNode(d,lpszName,IsDeclaration);
    d->nChild++;
    return d->pChild[pos];
}

//---------------------------------------------------------------------------------------
// Add an attribute to an element.
PXML_ATTRIB XMLNode::AddAttribute_WOSD(const char* lpszName, const char* lpszValuev)
{
    if (!lpszName) return &emptyXMLAttribute;
    int nc=d->nAttribute;
    d->pAttribute=(PXML_ATTRIB)myRealloc(d->pAttribute,(nc+1),memoryIncrease,sizeof(XML_ATTRIB));
    PXML_ATTRIB pAttr=d->pAttribute+nc;
    pAttr->lpszName = lpszName;
    pAttr->lpszValue = lpszValuev;
    d->nAttribute++;
    return pAttr;
}

//---------------------------------------------------------------------------------------
// Add text to the element.
const char* XMLNode::AddText_WOSD(const char* lpszValue, int pos)
{
    if (!lpszValue) return NULL;
    d->pText=(const char**)AddToOrder(&pos,d->nText,d->pText,sizeof(char*),eNodeText);
    d->pText[pos]=lpszValue;
    d->nText++;
    return lpszValue;
}

//---------------------------------------------------------------------------------------
// Add clear (unformatted) text to the element.
XML_CLEAR *XMLNode::AddClear_WOSD(const char* lpszValue, const char* lpszOpen, const char* lpszClose, int pos)
{
    if (!lpszValue) return &emptyXMLClear;
    d->pClear=(XML_CLEAR *)AddToOrder(&pos,d->nClear,d->pClear,sizeof(XML_CLEAR),eNodeClear);
    XML_CLEAR *pNewClear=d->pClear+pos;
    pNewClear->lpszValue = lpszValue;
    pNewClear->lpszOpenTag = lpszOpen;
    pNewClear->lpszCloseTag = lpszClose;
    d->nClear++;
    return pNewClear;
}

//---------------------------------------------------------------------------------------
// Trim the end of the text to remove white space characters.
static void FindEndOfText(const char* lpszToken, int *pcbText)
{
    char   ch;
    int     cbText;
    assert(lpszToken);
    assert(pcbText);
    cbText = (*pcbText)-1;
    for(;;)
    {
        assert(cbText >= 0);
        ch = lpszToken[cbText];
        if XML_isSPACECHAR(ch) cbText--;
        else { *pcbText = cbText+1; return; }
    }
}

//---------------------------------------------------------------------------------------
// private:
// Parse a clear (unformatted) type node.
int XMLNode::ParseClearTag(void *px, void *pa)
{
    XML *pXML=(XML *)px;
    XML_ALLCLEARTAG *pClear=(XML_ALLCLEARTAG *)pa;
    int cbTemp = 0;
    const char* lpszTemp;
	const char* lpXML=&pXML->lpXML[pXML->nIndex];

    // Find the closing tag
    lpszTemp = strstr(lpXML, pClear->lpszClose);

    // Iterate through the tokens until we find the closing tag.
    if (lpszTemp)
    {
        // Cache the size and increment the index
        cbTemp = (int)(lpszTemp - lpXML);

        pXML->nIndex += cbTemp+(int)strlen(pClear->lpszClose);

        // Add the clear node to the current element
        AddClear_WOSD(StringDup(lpXML,cbTemp), pClear->lpszOpen, pClear->lpszClose);
        return 1;
    }

    // If we failed to find the end tag
    pXML->error = eXMLErrorUnmatchedEndClearTag;
    return 0;
}

//---------------------------------------------------------------------------------------
void XMLNode::ExactMemory(PXML_NODEDATA d)
{
    if (memoryIncrease<=1) return;
    if (d->pOrder)     d->pOrder=(int*)realloc(d->pOrder,(d->nChild+d->nAttribute+d->nText+d->nClear)*sizeof(int));
    if (d->pChild)     d->pChild=(XMLNode*)realloc(d->pChild,d->nChild*sizeof(XMLNode));
    if (d->pAttribute) d->pAttribute=(PXML_ATTRIB)realloc(d->pAttribute,d->nAttribute*sizeof(XML_ATTRIB));
    if (d->pText)      d->pText=(const char**)realloc(d->pText,d->nText*sizeof(char*));
    if (d->pClear)     d->pClear=(XML_CLEAR *)realloc(d->pClear,d->nClear*sizeof(XML_CLEAR));
}

//---------------------------------------------------------------------------------------
// private:
// Recursively parse an XML element.
int XMLNode::ParseXMLElement(void *pa)
{
    XML *pXML=(XML *)pa;
    int cbToken;
    enum XMLTokenTypeTag type;
    NextToken token;
    const char* lpszTemp=NULL;
    int cbTemp=0;
    char nDeclaration;
    const char* lpszText=NULL;
    XMLNode pNew;
    enum Status status; // inside or outside a tag
    enum Attrib attrib = eAttribName;

    assert(pXML);

    // If this is the first call to the function
    if (pXML->nFirst)
    {
        // Assume we are outside of a tag definition
        pXML->nFirst = 0;
        status = eOutsideTag;
    } else
    {
        // If this is not the first call then we should only be called when inside a tag.
        status = eInsideTag;
    }

    // Iterate through the tokens in the document
    for(;;)
    {
        // Obtain the next token
        token = GetNextToken(pXML, &cbToken, &type);

        if (type != eTokenError)
        {
            // Check the current status
            switch(status)
            {

            // If we are outside of a tag definition
            case eOutsideTag:

                // Check what type of token we obtained
                switch(type)
                {
                // If we have found text or quoted text
                case eTokenText:
                case eTokenCloseTag:          /* '>'         */
                case eTokenShortHandClose:    /* '/>'        */
                case eTokenQuotedText:
                case eTokenEquals:
                    if (!lpszText)
                    {
                        lpszText = token.pStr;
                    }

                    break;

                // If we found a start tag '<' and declarations '<?'
                case eTokenTagStart:
                case eTokenDeclaration:

                    // Cache whether this new element is a declaration or not
                    nDeclaration = (type == eTokenDeclaration);

                    // If we have node text then add this to the element
                    if (lpszText)
                    {
                        cbTemp = (int)(token.pStr - lpszText);
                        FindEndOfText(lpszText, &cbTemp);
                        lpszText=fromXMLString(lpszText,cbTemp,pXML);
                        if (!lpszText) return 0;
                        AddText_WOSD(lpszText);
                        lpszText=NULL;
                    }

                    // Find the name of the tag
                    token = GetNextToken(pXML, &cbToken, &type);

                    // Return an error if we couldn't obtain the next token or
                    // it wasnt text
                    if (type != eTokenText)
                    {
                        pXML->error = eXMLErrorMissingTagName;
                        return 0;
                    }

                    // If we found a new element which is the same as this
                    // element then we need to pass this back to the caller..

#ifdef APPROXIMATE_PARSING
                    if (d->lpszName &&
                        myTagCompare(d->lpszName, token.pStr) == 0)
                    {
                        // Indicate to the caller that it needs to create a
                        // new element.
                        pXML->lpNewElement = token.pStr;
                        pXML->cbNewElement = cbToken;
                        return 1;
                    } else
#endif
                    {
                        // If the name of the new element differs from the name of
                        // the current element we need to add the new element to
                        // the current one and recurse
                        pNew = AddChild_WOSD(StringDup(token.pStr,cbToken), nDeclaration);

                        while (!pNew.IsEmpty())
                        {
                            // Callself to process the new node.  If we return
                            // 0 this means we dont have any more
                            // processing to do...

                            if (!pNew.ParseXMLElement(pXML)) return 0;
                            else
                            {
                                // If the call to recurse this function
                                // evented in a end tag specified in XML then
                                // we need to unwind the calls to this
                                // function until we find the appropriate node
                                // (the element name and end tag name must
                                // match)
                                if (pXML->cbEndTag)
                                {
                                    // If we are back at the root node then we
                                    // have an unmatched end tag
                                    if (!d->lpszName)
                                    {
                                        pXML->error=eXMLErrorUnmatchedEndTag;
                                        return 0;
                                    }

                                    // If the end tag matches the name of this
                                    // element then we only need to unwind
                                    // once more...

                                    if (myTagCompare(d->lpszName, pXML->lpEndTag)==0)
                                    {
                                        pXML->cbEndTag = 0;
                                    }

                                    return 1;
                                } else
                                    if (pXML->cbNewElement)
                                    {
                                        // If the call indicated a new element is to
                                        // be created on THIS element.

                                        // If the name of this element matches the
                                        // name of the element we need to create
                                        // then we need to return to the caller
                                        // and let it process the element.

                                        if (myTagCompare(d->lpszName, pXML->lpNewElement)==0)
                                        {
                                            return 1;
                                        }

                                        // Add the new element and recurse
                                        pNew = AddChild_WOSD(StringDup(pXML->lpNewElement,pXML->cbNewElement));
                                        pXML->cbNewElement = 0;
                                    }
                                    else
                                    {
                                        // If we didn't have a new element to create
                                        pNew = emptyXMLNode;

                                    }
                            }
                        }
                    }
                    break;

                // If we found an end tag
                case eTokenTagEnd:

                    // If we have node text then add this to the element
                    if (lpszText)
                    {
                        cbTemp = (int)(token.pStr - lpszText);
                        FindEndOfText(lpszText, &cbTemp);
                        lpszText=fromXMLString(lpszText,cbTemp,pXML);
                        if (!lpszText) return 0;
                        AddText_WOSD(lpszText);
                        lpszText = NULL;
                    }

                    // Find the name of the end tag
                    token = GetNextToken(pXML, &cbTemp, &type);

                    // The end tag should be text
                    if (type != eTokenText)
                    {
                        pXML->error = eXMLErrorMissingEndTagName;
                        return 0;
                    }
                    lpszTemp = token.pStr;

                    // After the end tag we should find a closing tag
                    token = GetNextToken(pXML, &cbToken, &type);
                    if (type != eTokenCloseTag)
                    {
                        pXML->error = eXMLErrorMissingEndTagName;
                        return 0;
                    }

                    // We need to return to the previous caller.  If the name
                    // of the tag cannot be found we need to keep returning to
                    // caller until we find a match
                    if (myTagCompare(d->lpszName, lpszTemp) != 0)
#ifdef STRICT_PARSING
                    {
                        pXML->error=eXMLErrorUnmatchedEndTag;
                        pXML->nIndexMissigEndTag=pXML->nIndex;
                        return 0;
                    }
#else
                    {
                        pXML->error=eXMLErrorMissingEndTag;
                        pXML->nIndexMissigEndTag=pXML->nIndex;
                        pXML->lpEndTag = lpszTemp;
                        pXML->cbEndTag = cbTemp;
                    }
#endif

                    // Return to the caller
                    ExactMemory(d);
                    return 1;

                // If we found a clear (unformatted) token
                case eTokenClear:
                    // If we have node text then add this to the element
                    if (lpszText)
                    {
                        cbTemp = (int)(token.pStr - lpszText);
                        FindEndOfText(lpszText, &cbTemp);
                        AddText_WOSD(StringDup(lpszText,cbTemp));
                        lpszText = NULL;
                    }

                    if (!ParseClearTag(pXML, token.pClr))
                    {
                        return 0;
                    }
                    break;

                default:
                    break;
                }
                break;

            // If we are inside a tag definition we need to search for attributes
            case eInsideTag:

                // Check what part of the attribute (name, equals, value) we
                // are looking for.
                switch(attrib)
                {
                // If we are looking for a new attribute
                case eAttribName:

                    // Check what the current token type is
                    switch(type)
                    {
                    // If the current type is text...
                    // Eg.  'attribute'
                    case eTokenText:
                        // Cache the token then indicate that we are next to
                        // look for the equals
                        lpszTemp = token.pStr;
                        cbTemp = cbToken;
                        attrib = eAttribEquals;
                        break;

                    // If we found a closing tag...
                    // Eg.  '>'
                    case eTokenCloseTag:
                        // We are now outside the tag
                        status = eOutsideTag;
                        break;

                    // If we found a short hand '/>' closing tag then we can
                    // return to the caller
                    case eTokenShortHandClose:
                        ExactMemory(d);
                        return 1;

                    // Errors...
                    case eTokenQuotedText:    /* '"SomeText"'   */
                    case eTokenTagStart:      /* '<'            */
                    case eTokenTagEnd:        /* '</'           */
                    case eTokenEquals:        /* '='            */
                    case eTokenDeclaration:   /* '<?'           */
                    case eTokenClear:
                        pXML->error = eXMLErrorUnexpectedToken;
                        return 0;
                    default: break;
                    }
                    break;

                // If we are looking for an equals
                case eAttribEquals:
                    // Check what the current token type is
                    switch(type)
                    {
                    // If the current type is text...
                    // Eg.  'Attribute AnotherAttribute'
                    case eTokenText:
                        // Add the unvalued attribute to the list
                        AddAttribute_WOSD(StringDup(lpszTemp,cbTemp), NULL);
                        // Cache the token then indicate.  We are next to
                        // look for the equals attribute
                        lpszTemp = token.pStr;
                        cbTemp = cbToken;
                        break;

                    // If we found a closing tag 'Attribute >' or a short hand
                    // closing tag 'Attribute />'
                    case eTokenShortHandClose:
                    case eTokenCloseTag:
                        // If we are a declaration element '<?' then we need
                        // to remove extra closing '?' if it exists
                        if (d->IsDeclaration &&
                            (lpszTemp[cbTemp-1]) == '?')
                        {
                            cbTemp--;
                        }

                        if (cbTemp)
                        {
                            // Add the unvalued attribute to the list
                            AddAttribute_WOSD(StringDup(lpszTemp,cbTemp), NULL);
                        }

                        // If this is the end of the tag then return to the caller
                        if (type == eTokenShortHandClose)
                        {
                            ExactMemory(d);
                            return 1;
                        }

                        // We are now outside the tag
                        status = eOutsideTag;
                        break;

                    // If we found the equals token...
                    // Eg.  'Attribute ='
                    case eTokenEquals:
                        // Indicate that we next need to search for the value
                        // for the attribute
                        attrib = eAttribValue;
                        break;

                    // Errors...
                    case eTokenQuotedText:    /* 'Attribute "InvalidAttr"'*/
                    case eTokenTagStart:      /* 'Attribute <'            */
                    case eTokenTagEnd:        /* 'Attribute </'           */
                    case eTokenDeclaration:   /* 'Attribute <?'           */
                    case eTokenClear:
                        pXML->error = eXMLErrorUnexpectedToken;
                        return 0;
                    default: break;
                    }
                    break;

                // If we are looking for an attribute value
                case eAttribValue:
                    // Check what the current token type is
                    switch(type)
                    {
                    // If the current type is text or quoted text...
                    // Eg.  'Attribute = "Value"' or 'Attribute = Value' or
                    // 'Attribute = 'Value''.
                    case eTokenText:
                    case eTokenQuotedText:
                        // If we are a declaration element '<?' then we need
                        // to remove extra closing '?' if it exists
                        if (d->IsDeclaration &&
                            (token.pStr[cbToken-1]) == '?')
                        {
                            cbToken--;
                        }

                        if (cbTemp)
                        {
                            // Add the valued attribute to the list
                            if (type==eTokenQuotedText) { token.pStr++; cbToken-=2; }
                            const char* attrVal=token.pStr;
                            if (attrVal)
                            {
                                attrVal=fromXMLString(attrVal,cbToken,pXML);
                                if (!attrVal) return 0;
                            }
                            AddAttribute_WOSD(StringDup(lpszTemp,cbTemp),attrVal);
                        }

                        // Indicate we are searching for a new attribute
                        attrib = eAttribName;
                        break;

                    // Errors...
                    case eTokenTagStart:        /* 'Attr = <'          */
                    case eTokenTagEnd:          /* 'Attr = </'         */
                    case eTokenCloseTag:        /* 'Attr = >'          */
                    case eTokenShortHandClose:  /* "Attr = />"         */
                    case eTokenEquals:          /* 'Attr = ='          */
                    case eTokenDeclaration:     /* 'Attr = <?'         */
                    case eTokenClear:
                        pXML->error = eXMLErrorUnexpectedToken;
                        return 0;
                        break;
                    default: break;
                    }
                }
            }
        }
        // If we failed to obtain the next token
        else
        {
            if ((!d->IsDeclaration)&&(d->pParent))
            {
#ifdef STRICT_PARSING
                pXML->error=eXMLErrorUnmatchedEndTag;
#else
                pXML->error=eXMLErrorMissingEndTag;
#endif
                pXML->nIndexMissigEndTag=pXML->nIndex;
            }
            return 0;
        }
    }
}

//---------------------------------------------------------------------------------------
// Count the number of lines and columns in an XML string.
static void CountLinesAndColumns(const char* lpXML, int nUpto, PXML_RESULTS pResults)
{
    char ch;
    assert(lpXML);
    assert(pResults);

    struct XML xml={ lpXML, 0, 0, eXMLErrorNone, NULL, 0, NULL, 0, 1 };

    pResults->nLine = 1;
    pResults->nColumn = 1;
    while (xml.nIndex<nUpto)
    {
        ch = getNextChar(&xml);
        if (ch != '\n') pResults->nColumn++;
        else
        {
            pResults->nLine++;
            pResults->nColumn=1;
        }
    }
}

//---------------------------------------------------------------------------------------
// Parse XML and return the root element.
XMLNode XMLNode::ParseString(const char* lpszXML, const char* tag, PXML_RESULTS pResults)
{
    if (!lpszXML)
    {
        if (pResults)
        {
            pResults->error=eXMLErrorNoElements;
            pResults->nLine=0;
            pResults->nColumn=0;
        }
        return emptyXMLNode;
    }

    XMLNode xnode(NULL,NULL,0);
    struct XML xml={ lpszXML, 0, 0, eXMLErrorNone, NULL, 0, NULL, 0, 1 };

    // Create header element
    memoryIncrease=MEMORYINCREASE; xnode.ParseXMLElement(&xml); memoryIncrease=0;
    enum XML_ERR error = xml.error;
    if ((xnode.nChildNode()==1)&&(xnode.nElement()==1)) xnode=xnode.GetChildNode(); // skip the empty node

    // If no error occurred
    if ((error==eXMLErrorNone)||(error==eXMLErrorMissingEndTag))
    {
        if (tag&&strlen(tag)&&_stricmp(xnode.GetName(),tag))
        {
            XMLNode nodeTmp;
            int i=0;
            while (i<xnode.nChildNode())
            {
                nodeTmp=xnode.GetChildNode(i);
                if (_stricmp(nodeTmp.GetName(),tag)==0) break;
                if (nodeTmp.IsDeclaration()) { xnode=nodeTmp; i=0; } else i++;
            }
            if (i>=xnode.nChildNode())
            {
                if (pResults)
                {
                    pResults->error=eXMLErrorFirstTagNotFound;
                    pResults->nLine=0;
                    pResults->nColumn=0;
                }
                return emptyXMLNode;
            }
            xnode=nodeTmp;
        }
    } else
    {
        // Cleanup: this will destroy all the nodes
        xnode = emptyXMLNode;
    }


    // If we have been given somewhere to place results
    if (pResults)
    {
        pResults->error = error;

        // If we have an error
        if (error!=eXMLErrorNone)
        {
            if (error==eXMLErrorMissingEndTag) xml.nIndex=xml.nIndexMissigEndTag;
            // Find which line and column it starts on.
            CountLinesAndColumns(xml.lpXML, xml.nIndex, pResults);
        }
    }
    return xnode;
}

//---------------------------------------------------------------------------------------
XMLNode XMLNode::ParseFile(const char* filename, const char* tag, PXML_RESULTS pResults)
{
	if (pResults) { pResults->nLine=0; pResults->nColumn=0; }
    FILE *f=fopen(filename,"rb");
    if (f==NULL) { if (pResults) pResults->error=eXMLErrorFileNotFound; return emptyXMLNode; }
    fseek(f,0,SEEK_END);

    int l=ftell(f),headerSz=0;

    if (!l) { if (pResults) pResults->error=eXMLErrorEmpty; return emptyXMLNode; }
    fseek(f,0,SEEK_SET);
    unsigned char *buf=(unsigned char*)malloc(l+1);
    fread(buf,l,1,f);
    fclose(f);
    buf[l]=0;

    if (guessUnicodeChars)
    {
        //if (myIsTextUnicode(buf,l))
        //{
        //    l/=sizeof(wchar_t);
        //    if ((buf[0]==0xef)&&(buf[1]==0xff)) headerSz=2;
        //    if ((buf[0]==0xff)&&(buf[1]==0xfe)) headerSz=2;
        //    char *b2=myWideCharToMultiByte((const wchar_t*)(buf+headerSz),l-headerSz);
        //    free(buf); buf=(unsigned char*)b2; headerSz=0;
        //} else
        //{
            if ((buf[0]==0xef)&&(buf[1]==0xbb)&&(buf[2]==0xbf)) headerSz=3;
        //}
    }

    if (!buf) { if (pResults) pResults->error=eXMLErrorCharConversionError; return emptyXMLNode; }
    XMLNode x=ParseString((char*)(buf+headerSz),tag,pResults);
    free(buf);
    return x;
}

//---------------------------------------------------------------------------------------
static inline void charmemset(char* dest,char c,int l) { while (l--) *(dest++)=c; }

//---------------------------------------------------------------------------------------
// private:
// Creates an user friendly XML string from a given element with
// appropriate white space and carriage returns.
//
// This recurses through all subnodes then adds contents of the nodes to the
// string.
int XMLNode::CreateXMLStringR(PXML_NODEDATA pEntry, char* lpszMarker, int nFormat)
{
    int nResult = 0;
    int cb;
    int cbElement;
    int nChildFormat=-1;
    int nElementI=pEntry->nChild+pEntry->nText+pEntry->nClear;
    int i,j;

    assert(pEntry);

#define LENSTR(lpsz) (lpsz ? strlen(lpsz) : 0)

    // If the element has no name then assume this is the head node.
    cbElement = (int)LENSTR(pEntry->lpszName);

    if (cbElement)
    {
        // "<elementname "
        cb = nFormat == -1 ? 0 : nFormat;

        if (lpszMarker)
        {
            if (cb) charmemset(lpszMarker, INDENTCHAR, sizeof(char)*cb);
            nResult = cb;
            lpszMarker[nResult++]='<';
            if (pEntry->IsDeclaration) lpszMarker[nResult++]='?';
            strcpy(&lpszMarker[nResult], pEntry->lpszName);
            nResult+=cbElement;
            lpszMarker[nResult++]=' ';

        } else
        {
            nResult+=cbElement+2+cb;
            if (pEntry->IsDeclaration) nResult++;
        }

        // Enumerate attributes and add them to the string
        PXML_ATTRIB pAttr=pEntry->pAttribute;
        for (i=0; i<pEntry->nAttribute; i++)
        {
            // "Attrib
            cb = (int)LENSTR(pAttr->lpszName);
            if (cb)
            {
                if (lpszMarker) strcpy(&lpszMarker[nResult], pAttr->lpszName);
                nResult += cb;
                // "Attrib=Value "
                if (pAttr->lpszValue)
                {
                    cb=(int)lengthXMLString(pAttr->lpszValue);
                    if (lpszMarker)
                    {
                        lpszMarker[nResult]='=';
                        lpszMarker[nResult+1]='"';
                        if (cb) ToXMLString(&lpszMarker[nResult+2],pAttr->lpszValue);
                        lpszMarker[nResult+cb+2]='"';
                    }
                    nResult+=cb+3;
                }
                if (lpszMarker) lpszMarker[nResult] = ' ';
                nResult++;
            }
            pAttr++;
        }

        if (pEntry->IsDeclaration)
        {
            if (lpszMarker)
            {
                lpszMarker[nResult-1]='?';
                lpszMarker[nResult]='>';
            }
            nResult++;
            if (nFormat!=-1)
            {
                if (lpszMarker) lpszMarker[nResult]='\n';
                nResult++;
            }
        } else
            // If there are child nodes we need to terminate the start tag
            if (nElementI)
            {
                if (lpszMarker) lpszMarker[nResult-1]='>';
                if (nFormat!=-1)
                {
                    if (lpszMarker) lpszMarker[nResult]='\n';
                    nResult++;
                }
            } else nResult--;
    }

    // Calculate the child format for when we recurse.  This is used to
    // determine the number of spaces used for prefixes.
    if (nFormat!=-1)
    {
        if (cbElement&&(!pEntry->IsDeclaration)) nChildFormat=nFormat+1;
        else nChildFormat=nFormat;
    }

    // Enumerate through remaining children
    for (i=0; i<nElementI; i++)
    {
        j=pEntry->pOrder[i];
        switch((XML_ELEMENTTYPE)(j&3))
        {
        // Text nodes
        case eNodeText:
            {
                // "Text"
                const char* pChild=pEntry->pText[j>>2];
                cb = (int)lengthXMLString(pChild);
                if (cb)
                {
                    if (nFormat!=-1)
                    {
                        if (lpszMarker)
                        {
                            charmemset(&lpszMarker[nResult],INDENTCHAR,sizeof(char)*(nFormat + 1));
                            ToXMLString(&lpszMarker[nResult+nFormat+1],pChild);
                            lpszMarker[nResult+nFormat+1+cb]='\n';
                        }
                        nResult+=cb+nFormat+2;
                    } else
                    {
                        if (lpszMarker) ToXMLString(&lpszMarker[nResult], pChild);
                        nResult += cb;
                    }
                }
                break;
            }

        // Clear type nodes
        case eNodeClear:
            {
                XML_CLEAR *pChild=pEntry->pClear+(j>>2);
                // "OpenTag"
                cb = (int)LENSTR(pChild->lpszOpenTag);
                if (cb)
                {
                    if (nFormat!=-1)
                    {
                        if (lpszMarker)
                        {
                            charmemset(&lpszMarker[nResult], INDENTCHAR, sizeof(char)*(nFormat + 1));
                            strcpy(&lpszMarker[nResult+nFormat+1], pChild->lpszOpenTag);
                        }
                        nResult+=cb+nFormat+1;
                    }
                    else
                    {
                        if (lpszMarker)strcpy(&lpszMarker[nResult], pChild->lpszOpenTag);
                        nResult += cb;
                    }
                }

                // "OpenTag Value"
                cb = (int)LENSTR(pChild->lpszValue);
                if (cb)
                {
                    if (lpszMarker) strcpy(&lpszMarker[nResult], pChild->lpszValue);
                    nResult += cb;
                }

                // "OpenTag Value CloseTag"
                cb = (int)LENSTR(pChild->lpszCloseTag);
                if (cb)
                {
                    if (lpszMarker) strcpy(&lpszMarker[nResult], pChild->lpszCloseTag);
                    nResult += cb;
                }

                if (nFormat!=-1)
                {
                    if (lpszMarker) lpszMarker[nResult] = '\n';
                    nResult++;
                }
                break;
            }

        // Element nodes
        case eNodeChild:
            {
                // Recursively add child nodes
                nResult += CreateXMLStringR(pEntry->pChild[j>>2].d, lpszMarker ? lpszMarker + nResult : 0, nChildFormat);
                break;
            }
        default: break;
        }
    }

    if ((cbElement)&&(!pEntry->IsDeclaration))
    {
        // If we have child entries we need to use long XML notation for
        // closing the element - "<elementname>blah blah blah</elementname>"
        if (nElementI)
        {
            // "</elementname>\0"
            if (lpszMarker)
            {
                if (nFormat != -1)
                {
                    if (nFormat)
                    {
                        charmemset(&lpszMarker[nResult], INDENTCHAR,sizeof(char)*nFormat);
                        nResult+=nFormat;
                    }
                }

                strcpy(&lpszMarker[nResult], "</");
                nResult += 2;
                strcpy(&lpszMarker[nResult], pEntry->lpszName);
                nResult += cbElement;

                if (nFormat == -1)
                {
                    strcpy(&lpszMarker[nResult], ">");
                    nResult++;
                } else
                {
                    strcpy(&lpszMarker[nResult], ">\n");
                    nResult+=2;
                }
            } else
            {
                if (nFormat != -1) nResult+=cbElement+4+nFormat;
                else nResult+=cbElement+3;
            }
        } else
        {
            // If there are no children we can use shorthand XML notation -
            // "<elementname/>"
            // "/>\0"
            if (lpszMarker)
            {
                if (nFormat == -1)
                {
                    strcpy(&lpszMarker[nResult], "/>");
                    nResult += 2;
                }
                else
                {
                    strcpy(&lpszMarker[nResult], "/>\n");
                    nResult += 3;
                }
            }
            else
            {
                nResult += nFormat == -1 ? 2 : 3;
            }
        }
    }

    return nResult;
}

#undef LENSTR

//---------------------------------------------------------------------------------------
// Create an XML string
// @param       int nFormat             - 0 if no formatting is required
//                                        otherwise nonzero for formatted text
//                                        with carriage returns and indentation.
// @param       int *pnSize             - [out] pointer to the size of the
//                                        returned string not including the
//                                        NULL terminator.
// @return      char*                  - Allocated XML string, you must free
//                                        this with free().
char* XMLNode::CreateXMLString(int nFormat, int *pnSize) const
{
    if (!d) { if (pnSize) *pnSize=0; return NULL; }

    char* lpszResult = NULL;
    int cbStr;

    // Recursively Calculate the size of the XML string
    nFormat = nFormat ? 0 : -1;
    cbStr = CreateXMLStringR(d, 0, nFormat);
    assert(cbStr);
    // Alllocate memory for the XML string + the NULL terminator and
    // create the recursively XML string.
    lpszResult=(char*)malloc((cbStr+1)*sizeof(char));
    CreateXMLStringR(d, lpszResult, nFormat);
    if (pnSize) *pnSize = cbStr;
    return lpszResult;
}

//---------------------------------------------------------------------------------------
XMLNode::~XMLNode() { DeleteNodeContent(); }

//---------------------------------------------------------------------------------------
int XMLNode::DetachFromParent(PXML_NODEDATA d)
{
    XMLNode *pa=d->pParent->pChild;
    int i=0;
    while (((void*)(pa[i].d))!=((void*)d)) i++;
    d->pParent->nChild--;
    if (d->pParent->nChild) memmove(pa+i,pa+i+1,(d->pParent->nChild-i)*sizeof(XMLNode));
    else { free(pa); d->pParent->pChild=NULL; }
    return RemoveOrderElement(d->pParent,eNodeChild,i);
}

//---------------------------------------------------------------------------------------
void XMLNode::DeleteNodeContent(char force)
{
    if (!d) return;
    (d->ref_count) --;
    if ((d->ref_count==0)||force)
    {
        int i;
        if (d->pParent) DetachFromParent(d);
        for(i=0; i<d->nChild; i++) { d->pChild[i].d->pParent=NULL; d->pChild[i].DeleteNodeContent(force); }
        free(d->pChild);
        for(i=0; i<d->nText; i++) free((void*)d->pText[i]);
        free(d->pText);
        for(i=0; i<d->nClear; i++) free((void*)d->pClear[i].lpszValue);
        free(d->pClear);
        for(i=0; i<d->nAttribute; i++)
        {
            free((void*)d->pAttribute[i].lpszName);
            if (d->pAttribute[i].lpszValue) free((void*)d->pAttribute[i].lpszValue);
        }
        free(d->pAttribute);
        free(d->pOrder);
        free((void*)d->lpszName);
        free(d);
        d=NULL;
    }
}

//---------------------------------------------------------------------------------------
XMLNode XMLNode::AddChild(XMLNode childNode, int pos)
{
    PXML_NODEDATA dc=childNode.d;
    if ((!dc)||(!d)) return childNode;
    if (dc->pParent) { if ((DetachFromParent(dc)<=pos)&&(dc->pParent==d)) pos--; } else dc->ref_count++;
    dc->pParent=d;
//     int nc=d->nChild;
//     d->pChild=(XMLNode*)myRealloc(d->pChild,(nc+1),memoryIncrease,sizeof(XMLNode));
    d->pChild=(XMLNode*)AddToOrder(&pos,d->nChild,d->pChild,sizeof(XMLNode),eNodeChild);
    d->pChild[pos].d=dc;
    d->nChild++;
    return childNode;
}

//---------------------------------------------------------------------------------------
void XMLNode::DeleteAttribute(int i)
{
    if ((!d)||(i<0)||(i>=d->nAttribute)) return;
    d->nAttribute--;
    PXML_ATTRIB p=d->pAttribute+i;
    free((void*)p->lpszName);
    if (p->lpszValue) free((void*)p->lpszValue);
    if (d->nAttribute) memmove(p,p+1,(d->nAttribute-i)*sizeof(XML_ATTRIB)); else { free(p); d->pAttribute=NULL; }
}

//---------------------------------------------------------------------------------------
void XMLNode::DeleteAttribute(PXML_ATTRIB a){ if (a) DeleteAttribute(a->lpszName); }

//---------------------------------------------------------------------------------------
void XMLNode::DeleteAttribute(const char* lpszName)
{
    int j=0;
    GetAttribute(lpszName,&j);
    if (j) DeleteAttribute(j-1);
}

//---------------------------------------------------------------------------------------
PXML_ATTRIB XMLNode::UpdateAttribute_WOSD(const char* lpszNewValue, const char* lpszNewName,int i)
{
    if (!d) return NULL;
    if (i>=d->nAttribute)
    {
        if (lpszNewName) return AddAttribute_WOSD(lpszNewName,lpszNewValue);
        return NULL;
    }
    PXML_ATTRIB p=d->pAttribute+i;
    if (p->lpszValue&&p->lpszValue!=lpszNewValue) free((void*)p->lpszValue);
    p->lpszValue=lpszNewValue;
    if (lpszNewName&&p->lpszName!=lpszNewName) { free((void*)p->lpszName); p->lpszName=lpszNewName; };
    return p;
}

//---------------------------------------------------------------------------------------
PXML_ATTRIB XMLNode::UpdateAttribute_WOSD(PXML_ATTRIB newAttribute, PXML_ATTRIB oldAttribute)
{
    if (oldAttribute) return UpdateAttribute_WOSD(newAttribute->lpszValue,newAttribute->lpszName,oldAttribute->lpszName);
    return AddAttribute_WOSD(newAttribute->lpszName,newAttribute->lpszValue);
}

//---------------------------------------------------------------------------------------
PXML_ATTRIB XMLNode::UpdateAttribute_WOSD(const char* lpszNewValue, const char* lpszNewName,const char* lpszOldName)
{
    int j=0;
    GetAttribute(lpszOldName,&j);
    if (j) return UpdateAttribute_WOSD(lpszNewValue,lpszNewName,j-1);
    else
    {
        if (lpszNewName) return AddAttribute_WOSD(lpszNewName,lpszNewValue);
        else             return AddAttribute_WOSD(StringDup(lpszOldName),lpszNewValue);
    }
}

//---------------------------------------------------------------------------------------
int XMLNode::IndexText(const char* lpszValue) const
{
    if (!d) return -1;
    int i,l=d->nText;
    if (!lpszValue) { if (l) return 0; return -1; }
    const char* *p=d->pText;
    for (i=0; i<l; i++) if (lpszValue==p[i]) return i; 
    return -1;
}

//---------------------------------------------------------------------------------------
void XMLNode::DeleteText(int i)
{
    if ((!d)||(i<0)||(i>=d->nText)) return;
    d->nText--;
    const char* *p=d->pText+i;
    free((void*)*p);
    if (d->nText) memmove(p,p+1,(d->nText-i)*sizeof(const char*)); else { free(p); d->pText=NULL; }
    RemoveOrderElement(d,eNodeText,i);
}

//---------------------------------------------------------------------------------------
void XMLNode::DeleteText(const char* lpszValue) { DeleteText(IndexText(lpszValue)); }

//---------------------------------------------------------------------------------------
const char* XMLNode::UpdateText_WOSD(const char* lpszNewValue, int i)
{
    if (!d) return NULL;
    if (i>=d->nText) return AddText_WOSD(lpszNewValue);
    const char* *p=d->pText+i;
    if (*p!=lpszNewValue) { free((void*)*p); *p=lpszNewValue; }
    return lpszNewValue;
}

//---------------------------------------------------------------------------------------
const char* XMLNode::UpdateText_WOSD(const char* lpszNewValue, const char* lpszOldValue)
{
    if (!d) return NULL;
    int i=IndexText(lpszOldValue);
    if (i>=0) return UpdateText_WOSD(lpszNewValue,i);
    return AddText_WOSD(lpszNewValue);
}

//---------------------------------------------------------------------------------------
void XMLNode::DeleteClear(int i)
{
    if ((!d)||(i<0)||(i>=d->nClear)) return;
    d->nClear--;
    XML_CLEAR *p=d->pClear+i;
    free((void*)p->lpszValue);
    if (d->nClear) memmove(p,p+1,(d->nText-i)*sizeof(XML_CLEAR)); else { free(p); d->pClear=NULL; }
    RemoveOrderElement(d,eNodeClear,i);
}

//---------------------------------------------------------------------------------------
int XMLNode::IndexClear(const char* lpszValue) const
{
    if (!d) return -1;
    int i,l=d->nClear;
    if (!lpszValue) { if (l) return 0; return -1; }
    XML_CLEAR *p=d->pClear;
    for (i=0; i<l; i++) if (lpszValue==p[i].lpszValue) return i; 
    return -1;
}

//---------------------------------------------------------------------------------------
void XMLNode::DeleteClear(const char* lpszValue) { DeleteClear(IndexClear(lpszValue)); }

//---------------------------------------------------------------------------------------
void XMLNode::DeleteClear(XML_CLEAR *a) { if (a) DeleteClear(a->lpszValue); }

//---------------------------------------------------------------------------------------
XML_CLEAR *XMLNode::UpdateClear_WOSD(const char* lpszNewContent, int i)
{
    if (!d) return NULL;
    if (i>=d->nClear)
    {
        return AddClear_WOSD(g_XMLAllClearTags[0].lpszOpen,lpszNewContent,g_XMLAllClearTags[0].lpszClose);
    }
    XML_CLEAR *p=d->pClear+i;
    if (lpszNewContent!=p->lpszValue) { free((void*)p->lpszValue); p->lpszValue=lpszNewContent; }
    return p;
}

//---------------------------------------------------------------------------------------
XML_CLEAR *XMLNode::UpdateClear_WOSD(const char* lpszNewValue, const char* lpszOldValue)
{
    if (!d) return NULL;
    int i=IndexClear(lpszOldValue);
    if (i>=0) return UpdateClear_WOSD(lpszNewValue,i);
    return AddClear_WOSD(lpszNewValue,g_XMLAllClearTags[0].lpszOpen,g_XMLAllClearTags[0].lpszClose);
}

//---------------------------------------------------------------------------------------
XML_CLEAR *XMLNode::UpdateClear_WOSD(XML_CLEAR *newP,XML_CLEAR *oldP)
{
    if (oldP) return UpdateClear_WOSD(newP->lpszValue,oldP->lpszValue);
    return NULL;
}

//---------------------------------------------------------------------------------------
XMLNode& XMLNode::operator=( const XMLNode& A )
{
    // shallow copy
    if (this != &A)
    {
        DeleteNodeContent();
        d=A.d;
        if (d) (d->ref_count) ++ ;
    }
    return *this;
}

//---------------------------------------------------------------------------------------
XMLNode::XMLNode(const XMLNode &A)
{
    // shallow copy
    d=A.d;
    if (d) (d->ref_count)++ ;
}

//---------------------------------------------------------------------------------------
int XMLNode::nChildNode(const char* name) const
{
    if (!d) return 0;
    int i,j=0,n=d->nChild;
    XMLNode *pc=d->pChild;
    for (i=0; i<n; i++)
    {
        if (_stricmp(pc->d->lpszName, name)==0) j++;
        pc++;
    }
    return j;
}

//---------------------------------------------------------------------------------------
XMLNode XMLNode::GetChildNode(const char* name, int *j) const
{
    if (!d) return emptyXMLNode;
    int i=0,n=d->nChild;
    if (j) i=*j;
    XMLNode *pc=d->pChild+i;
    for (; i<n; i++)
    {
        if (_stricmp(pc->d->lpszName, name)==0)
        {
            if (j) *j=i+1;
            return *pc;
        }
        pc++;
    }
    return emptyXMLNode;
}

//---------------------------------------------------------------------------------------
XMLNode XMLNode::GetChildNode(const char* name, int j) const
{
    if (!d) return emptyXMLNode;
    int i=0;
    while (j-->0) GetChildNode(name,&i);
    return GetChildNode(name,&i);
}

//---------------------------------------------------------------------------------------
int XMLNode::PositionOfText     (int i) const { if (i>=d->nText ) i=d->nText-1;  return FindPosition(d,i,eNodeText ); }

//---------------------------------------------------------------------------------------
int XMLNode::PositionOfClear    (int i) const { if (i>=d->nClear) i=d->nClear-1; return FindPosition(d,i,eNodeClear); }

//---------------------------------------------------------------------------------------
int XMLNode::PositionOfChildNode(int i) const { if (i>=d->nChild) i=d->nChild-1; return FindPosition(d,i,eNodeChild); }

//---------------------------------------------------------------------------------------
int XMLNode::PositionOfText (const char* lpszValue) const { return PositionOfText (IndexText (lpszValue)); }

//---------------------------------------------------------------------------------------
int XMLNode::PositionOfClear(const char* lpszValue) const { return PositionOfClear(IndexClear(lpszValue)); }

//---------------------------------------------------------------------------------------
int XMLNode::PositionOfClear(XML_CLEAR *a) const { if (a) return PositionOfClear(a->lpszValue); return PositionOfClear(); }

//---------------------------------------------------------------------------------------
int XMLNode::PositionOfChildNode(XMLNode x)  const
{
    if ((!d)||(!x.d)) return -1;
    PXML_NODEDATA dd=x.d;
    XMLNode *pc=d->pChild;
    int i=d->nChild;
    while (i--) if (pc[i].d==dd) return FindPosition(d,i,eNodeChild); 
    return -1;
}

//---------------------------------------------------------------------------------------
int XMLNode::PositionOfChildNode(const char* name, int count) const
{
    if (!name) return PositionOfChildNode(count); 
    int j=0; 
    do { GetChildNode(name,&j); if (j<0) return -1; } while (count--);
    return FindPosition(d,j-1,eNodeChild);
}

//---------------------------------------------------------------------------------------
XMLNode XMLNode::GetChildNodeWithAttribute(const char* name,const char* attributeName,const char* attributeValue, int *k) const
{
     int i=0,j;
     if (k) i=*k;
     XMLNode x;
     const char* t;
     do
     {
         x=GetChildNode(name,&i);
         if (!x.IsEmpty())
         {
             if (attributeValue)
             {
                 j=0;
                 do
                 {
                     t=x.GetAttribute(attributeName,&j);
                     if (t&&(_stricmp(attributeValue,t)==0)) { if (k) *k=i+1; return x; }
                 } while (t);
             } else
             {
                 if (x.IsAttributeSet(attributeName)) { if (k) *k=i+1; return x; }
             }
         }
     } while (!x.IsEmpty());
     return emptyXMLNode;
}

//---------------------------------------------------------------------------------------
// Find an attribute on an node.
const char* XMLNode::GetAttribute(const char* lpszAttrib, int *j) const
{
    if (!d) return "";				//Modified By Kenny,2008-03-06
    int i=0,n=d->nAttribute;
    if (j) i=*j;
    PXML_ATTRIB pAttr=d->pAttribute+i;
    for (; i<n; i++)
    {
        if (_stricmp(pAttr->lpszName, lpszAttrib)==0)
        {
            if (j) *j=i+1;
            return pAttr->lpszValue;
        }
        pAttr++;
    }
    return "";						//Modified By Kenny,2008-03-06
}

//---------------------------------------------------------------------------------------
char XMLNode::IsAttributeSet(const char* lpszAttrib) const
{
    if (!d) return 0;
    int i,n=d->nAttribute;
    PXML_ATTRIB pAttr=d->pAttribute;
    for (i=0; i<n; i++)
    {
        if (_stricmp(pAttr->lpszName, lpszAttrib)==0)
        {
            return 1;
        }
        pAttr++;
    }
    return 0;
}

//---------------------------------------------------------------------------------------
const char* XMLNode::GetAttribute(const char* name, int j) const
{
    if (!d) return NULL;
    int i=0;
    while (j-->0) GetAttribute(name,&i);
    return GetAttribute(name,&i);
}

//---------------------------------------------------------------------------------------
XMLNodeContents XMLNode::enumContents(int i) const
{
    XMLNodeContents c;
    if (!d) { c.type=eNodeNULL; return c; }
    if (i<d->nAttribute)
    {
        c.type=eNodeAttribute;
        c.attrib=d->pAttribute[i];
        return c;
    }
    i-=d->nAttribute;
    c.type=(XML_ELEMENTTYPE)(d->pOrder[i]&3);
    i=(d->pOrder[i])>>2;
    switch (c.type)
    {
    case eNodeChild:     c.child = d->pChild[i];      break;
    case eNodeText:      c.text  = d->pText[i];       break;
    case eNodeClear:     c.clear = d->pClear[i];      break;
    default: break;
    }
    return c;
}

//---------------------------------------------------------------------------------------
const char* XMLNode::GetName() const { if (!d) return NULL; return d->lpszName;   }
int XMLNode::nText()       const { if (!d) return 0;    return d->nText;      }
int XMLNode::nChildNode()  const { if (!d) return 0;    return d->nChild;     }
int XMLNode::nAttribute()  const { if (!d) return 0;    return d->nAttribute; }
int XMLNode::nClear()      const { if (!d) return 0;    return d->nClear;     }
int XMLNode::nElement()    const { if (!d) return 0;    return d->nAttribute+d->nChild+d->nText+d->nClear; }
XML_CLEAR XMLNode::GetClear         (int i) const { if ((!d)||(i>=d->nClear    )) return emptyXMLClear;     return d->pClear[i];     }
XML_ATTRIB XMLNode::GetAttribute     (int i) const { if ((!d)||(i>=d->nAttribute)) return emptyXMLAttribute; return d->pAttribute[i]; }
const char* XMLNode::GetAttributeName (int i) const { if ((!d)||(i>=d->nAttribute)) return NULL;              return d->pAttribute[i].lpszName;  }
const char* XMLNode::GetAttributeValue(int i) const { if ((!d)||(i>=d->nAttribute)) return NULL;              return d->pAttribute[i].lpszValue; }
const char* XMLNode::GetText          (int i) const { if ((!d)||(i>=d->nText     )) return NULL;              return d->pText[i];      }
XMLNode XMLNode::GetChildNode     (int i) const { if ((!d)||(i>=d->nChild    )) return emptyXMLNode;      return d->pChild[i];     }
XMLNode XMLNode::GetParentNode    (     ) const { if ((!d)||(!d->pParent     )) return emptyXMLNode;      return XMLNode(d->pParent); }
char XMLNode::IsDeclaration (     ) const { if (!d) return 0;             return d->IsDeclaration; }
bool XMLNode::IsEmpty          (     ) const { return (d==NULL); }

XMLNode       XMLNode::AddChild(const char* lpszName, char IsDeclaration, int pos)
              { return AddChild_WOSD(StringDup(lpszName),IsDeclaration,pos); }
PXML_ATTRIB XMLNode::AddAttribute(const char* lpszName, const char* lpszValue)
              { return AddAttribute_WOSD(StringDup(lpszName),StringDup(lpszValue)); }
const char*       XMLNode::AddText(const char* lpszValue, int pos)
              { return AddText_WOSD(StringDup(lpszValue),pos); }
XML_CLEAR     *XMLNode::AddClear(const char* lpszValue, const char* lpszOpen, const char* lpszClose, int pos)
              { return AddClear_WOSD(StringDup(lpszValue),lpszOpen,lpszClose,pos); }
const char*       XMLNode::UpdateName(const char* lpszName)
              { return UpdateName_WOSD(StringDup(lpszName)); }
PXML_ATTRIB XMLNode::UpdateAttribute(PXML_ATTRIB newAttribute, PXML_ATTRIB oldAttribute)
              { return UpdateAttribute_WOSD(StringDup(newAttribute->lpszValue),StringDup(newAttribute->lpszName),oldAttribute->lpszName); }
PXML_ATTRIB XMLNode::UpdateAttribute(const char* lpszNewValue, const char* lpszNewName,int i)
              { return UpdateAttribute_WOSD(StringDup(lpszNewValue),StringDup(lpszNewName),i); }
PXML_ATTRIB XMLNode::UpdateAttribute(const char* lpszNewValue, const char* lpszNewName,const char* lpszOldName)
              { return UpdateAttribute_WOSD(StringDup(lpszNewValue),StringDup(lpszNewName),lpszOldName); }
const char*       XMLNode::UpdateText(const char* lpszNewValue, int i)
              { return UpdateText_WOSD(StringDup(lpszNewValue),i); }
const char*       XMLNode::UpdateText(const char* lpszNewValue, const char* lpszOldValue)
              { return UpdateText_WOSD(StringDup(lpszNewValue),lpszOldValue); }
XML_CLEAR     *XMLNode::UpdateClear(const char* lpszNewContent, int i)
              { return UpdateClear_WOSD(StringDup(lpszNewContent),i); }
XML_CLEAR     *XMLNode::UpdateClear(const char* lpszNewValue, const char* lpszOldValue)
              { return UpdateClear_WOSD(StringDup(lpszNewValue),lpszOldValue); }
XML_CLEAR     *XMLNode::UpdateClear(XML_CLEAR *newP,XML_CLEAR *oldP)
              { return UpdateClear_WOSD(StringDup(newP->lpszValue),oldP->lpszValue); }

//---------------------------------------------------------------------------------------
void XMLNode::SetGlobalOptions(char _guessUnicodeChars, char strictUTF8Parsing)
{
    guessUnicodeChars=_guessUnicodeChars;
#ifndef _XMLUNICODE
    if (strictUTF8Parsing) XML_ByteTable=XML_utf8ByteTable; else XML_ByteTable=XML_asciiByteTable;
#endif
}

//---------------------------------------------------------------------------------------
char XMLNode::GuessUTF8ParsingParameterValue(void *buf,int l, char useXMLEncodingAttribute)
{
    if (l<25) return 0;
//    if (myIsTextUnicode(buf,l)) return 0;
    unsigned char *b=(unsigned char*)buf;
    if ((b[0]==0xef)&&(b[1]==0xbb)&&(b[2]==0xbf)) return 1;

    // Match utf-8 model ?
    int i=0;
    while (i<l)
        switch (XML_utf8ByteTable[b[i]])
        {
        case 4: i++; if ((i<l)&&(b[i]& 0xC0)!=0x80) return 0; // 10bbbbbb ?
        case 3: i++; if ((i<l)&&(b[i]& 0xC0)!=0x80) return 0; // 10bbbbbb ?
        case 2: i++; if ((i<l)&&(b[i]& 0xC0)!=0x80) return 0; // 10bbbbbb ?
        case 1: i++; break;
        case 0: i=l;
        }
    if (!useXMLEncodingAttribute) return 1;
    // if encoding is specified and different from utf-8 than it's non-utf8
    // otherwise it's utf-8
    char bb[201];
    l=GetMinValue(l,200);
    memcpy(bb,buf,l); // copy buf into bb to be able to do "bb[l]=0"
    bb[l]=0;
    b=(unsigned char*)strstr(bb,"encoding");
    if (!b) return 1;
    b+=8; while XML_isSPACECHAR(*b) b++; if (*b!='=') return 1;
    b++;  while XML_isSPACECHAR(*b) b++; if ((*b!='\'')&&(*b!='"')) return 1;
    b++;  while XML_isSPACECHAR(*b) b++; if ((_strnicmp((char*)b,"utf-8",5)==0)||
                                             (_strnicmp((char*)b,"utf8",4)==0)) return 1;
    return 0;
}
#undef XML_isSPACECHAR

//////////////////////////////////////////////////////////
//      Here starts the base64 conversion functions.    //
//////////////////////////////////////////////////////////

static const char base64Fillchar = '='; // used to mark partial words at the end

// this lookup table defines the base64 encoding
const char* base64EncodeTable="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Decode Table gives the index of any valid base64 character in the Base64 table]
// 96: '='  -   97: space char   -   98: illegal char   -   99: end of string
const unsigned char base64DecodeTable[] = {
    99,98,98,98,98,98,98,98,98,97,  97,98,98,97,98,98,98,98,98,98,  98,98,98,98,98,98,98,98,98,98,  //00 -29
    98,98,97,98,98,98,98,98,98,98,  98,98,98,62,98,98,98,63,52,53,  54,55,56,57,58,59,60,61,98,98,  //30 -59
    98,96,98,98,98, 0, 1, 2, 3, 4,   5, 6, 7, 8, 9,10,11,12,13,14,  15,16,17,18,19,20,21,22,23,24,  //60 -89
    25,98,98,98,98,98,98,26,27,28,  29,30,31,32,33,34,35,36,37,38,  39,40,41,42,43,44,45,46,47,48,  //90 -119
    49,50,51,98,98,98,98,98,98,98,  98,98,98,98,98,98,98,98,98,98,  98,98,98,98,98,98,98,98,98,98,  //120 -149
    98,98,98,98,98,98,98,98,98,98,  98,98,98,98,98,98,98,98,98,98,  98,98,98,98,98,98,98,98,98,98,  //150 -179
    98,98,98,98,98,98,98,98,98,98,  98,98,98,98,98,98,98,98,98,98,  98,98,98,98,98,98,98,98,98,98,  //180 -209
    98,98,98,98,98,98,98,98,98,98,  98,98,98,98,98,98,98,98,98,98,  98,98,98,98,98,98,98,98,98,98,  //210 -239
    98,98,98,98,98,98,98,98,98,98,  98,98,98,98,98,98                                               //240 -255
};

KXMLParserBase64Tool::~KXMLParserBase64Tool(){ FreeBuffer(); }

//---------------------------------------------------------------------------------------
void KXMLParserBase64Tool::FreeBuffer(){ if (buf) free(buf); buf=NULL; buflen=0; }

//---------------------------------------------------------------------------------------
int KXMLParserBase64Tool::EncodeLength(int inlen, char formatted)
{
    unsigned int i=((inlen-1)/3*4+4+1);
    if (formatted) i+=inlen/54; 
    return i;
}

//---------------------------------------------------------------------------------------
char* KXMLParserBase64Tool::Encode(unsigned char *inbuf, unsigned int inlen, char formatted)
{
    int i=EncodeLength(inlen,formatted),k=17,eLen=inlen/3,j;
    alloc(i*sizeof(char));
    char* curr=(char*)buf;
    for(i=0;i<eLen;i++)
    {
        // Copy next three bytes into lower 24 bits of int, paying attention to sign.
        j=(inbuf[0]<<16)|(inbuf[1]<<8)|inbuf[2]; inbuf+=3;
        // Encode the int into four chars
        *(curr++)=base64EncodeTable[ j>>18      ];
        *(curr++)=base64EncodeTable[(j>>12)&0x3f];
        *(curr++)=base64EncodeTable[(j>> 6)&0x3f];
        *(curr++)=base64EncodeTable[(j    )&0x3f];
        if (formatted) { if (!k) { *(curr++)='\n'; k=18; } k--; }
    }
    eLen=inlen-eLen*3; // 0 - 2.
    if (eLen==1)
    {
        *(curr++)=base64EncodeTable[ inbuf[0]>>2      ];
        *(curr++)=base64EncodeTable[(inbuf[0]<<4)&0x3F];
        *(curr++)=base64Fillchar;
        *(curr++)=base64Fillchar;
    } else if (eLen==2)
    {
        j=(inbuf[0]<<8)|inbuf[1];
        *(curr++)=base64EncodeTable[ j>>10      ];
        *(curr++)=base64EncodeTable[(j>> 4)&0x3f];
        *(curr++)=base64EncodeTable[(j<< 2)&0x3f];
        *(curr++)=base64Fillchar;
    }
    *(curr++)=0;
    return (char*)buf;
}

//---------------------------------------------------------------------------------------
unsigned int KXMLParserBase64Tool::DecodeSize(const char* data,XML_ERR *xe)
{
     if (xe) *xe=eXMLErrorNone;
    int size=0;
    unsigned char c;
    //skip any extra characters (e.g. newlines or spaces)
    while (*data)
    {
        c=base64DecodeTable[(unsigned char)(*data)];
        if (c<97) size++;
        else if (c==98) { if (xe) *xe=eXMLErrorBase64DecodeIllegalCharacter; return 0; }
        data++;
    }
    if (xe&&(size%4!=0)) *xe=eXMLErrorBase64DataSizeIsNotMultipleOf4;
    if (size==0) return 0;
    do { data--; size--; } while(*data==base64Fillchar); size++;
    return (unsigned int)((size*3)/4);
}

//---------------------------------------------------------------------------------------
unsigned char KXMLParserBase64Tool::Decode(const char* data, unsigned char *buf, int len, XML_ERR *xe)
{
    if (xe) *xe=eXMLErrorNone;
    int i=0,p=0;
    unsigned char d,c;
    for(;;)
    {

#define BASE64DECODE_READ_NEXT_CHAR(c)                                           \
        do { c=base64DecodeTable[(unsigned char)data[i++]]; }while (c==97);   \
        if(c==98){ if(xe)*xe=eXMLErrorBase64DecodeIllegalCharacter; return 0; }

        BASE64DECODE_READ_NEXT_CHAR(c)
        if (c==99) { return 2; }
        if (c==96)
        {
            if (p==(int)len) return 2;
            if (xe) *xe=eXMLErrorBase64DecodeTruncatedData;
            return 1;
        }

        BASE64DECODE_READ_NEXT_CHAR(d)
        if ((d==99)||(d==96)) { if (xe) *xe=eXMLErrorBase64DecodeTruncatedData;  return 1; }
        if (p==(int)len) {      if (xe) *xe=eXMLErrorBase64DecodeBufferTooSmall; return 0; }
        buf[p++]=(c<<2)|((d>>4)&0x3);

        BASE64DECODE_READ_NEXT_CHAR(c)
        if (c==99) { if (xe) *xe=eXMLErrorBase64DecodeTruncatedData;  return 1; }
        if (p==(int)len)
        {
            if (c==96) return 2;
            if (xe) *xe=eXMLErrorBase64DecodeBufferTooSmall;
            return 0;
        }
        if (c==96) { if (xe) *xe=eXMLErrorBase64DecodeTruncatedData;  return 1; }
        buf[p++]=((d<<4)&0xf0)|((c>>2)&0xf);

        BASE64DECODE_READ_NEXT_CHAR(d)
        if (d==99 ) { if (xe) *xe=eXMLErrorBase64DecodeTruncatedData;  return 1; }
        if (p==(int)len)
        {
            if (d==96) return 2;
            if (xe) *xe=eXMLErrorBase64DecodeBufferTooSmall;
            return 0;
        }
        if (d==96) { if (xe) *xe=eXMLErrorBase64DecodeTruncatedData;  return 1; }
        buf[p++]=((c<<6)&0xc0)|d;
    }
}
#undef BASE64DECODE_READ_NEXT_CHAR

//---------------------------------------------------------------------------------------
void KXMLParserBase64Tool::alloc(int newsize)
{
    if ((!buf)&&(newsize)) { buf=malloc(newsize); buflen=newsize; return; }
    if (newsize>buflen) { buf=realloc(buf,newsize); buflen=newsize; } 
}

//---------------------------------------------------------------------------------------
unsigned char *KXMLParserBase64Tool::Decode(const char* data, int *outlen, XML_ERR *xe)
{
    if (xe) *xe=eXMLErrorNone;
    unsigned int len=DecodeSize(data,xe);
    if (outlen) *outlen=len;
    if (!len) return NULL;
    alloc(len+1);
    if(!Decode(data,(unsigned char*)buf,len,xe)){ return NULL; }
    return (unsigned char*)buf;
}

