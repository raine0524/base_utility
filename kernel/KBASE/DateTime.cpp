#include "stdafx.h"
#include "KBASE/DateTime.h"
#include "KBASE/Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------------------
KFormatDateTime::KFormatDateTime()
:m_nYear(0)
,m_nMonth(0)
,m_nMDay(0)
,m_nWDay(0)
,m_nHour(0)
,m_nMinute(0)
,m_nSecond(0)
{
}

//---------------------------------------------------------------------------------------
KFormatDateTime::~KFormatDateTime()
{
}

//---------------------------------------------------------------------------------------
void KFormatDateTime::GetJDay(unsigned long& jday)
{
	 KYMD2JDay(m_nYear,m_nMonth,m_nMDay,jday);
}

//---------------------------------------------------------------------------------------
// 获得当前时间
void GetNow(KFormatDateTime& datetime)
{
	time_t t;
	t=time(&t);
	struct tm* pTimeStruct = localtime(const_cast<time_t *>(&t));

	if(pTimeStruct)
	{
		 // 99 or 02, not 1999 or 2002, Year 2050 problem...
		int nYear = pTimeStruct->tm_year;
		datetime.SetYear((nYear<50)?(nYear+2000):(nYear+1900));
		datetime.SetMonth(pTimeStruct->tm_mon + 1);
		datetime.SetMDay(pTimeStruct->tm_mday);
		datetime.SetWDay(pTimeStruct->tm_wday);
		datetime.SetHour(pTimeStruct->tm_hour);
		datetime.SetMinute(pTimeStruct->tm_min);
		datetime.SetSecond(pTimeStruct->tm_sec);
	}
}

//---------------------------------------------------------------------------------------
// 根据字符串获得时间
void GetFormatDateTime(const std::string& strDateTime,KFormatDateTime& datetime)
{
	STRING_MAP ddatetime=StrSplit(strDateTime," ");
	std::string sdate	= ddatetime[0];
	std::string stime	= ddatetime[1];

	STRING_MAP ddate=StrSplit(sdate,"-"); 
	std::string year	= ddate[0];
	std::string month	= ddate[1];
	std::string day		= ddate[2];

	STRING_MAP dtime=StrSplit(stime,":"); 
	std::string hour	= dtime[0];
	std::string minute	= dtime[1];
	std::string second	= dtime[2];

	int weekday=0;
	KGetWeekDay(datetime.GetYear(),datetime.GetMonth(),datetime.GetMDay(),weekday);

	datetime.SetYear(STR2UINT(year.c_str()));
	datetime.SetMonth(STR2UINT(month.c_str()));
	datetime.SetMDay(STR2UINT(day.c_str()));
	datetime.SetWDay(weekday);
	datetime.SetHour(STR2UINT(hour.c_str()));
	datetime.SetMinute(STR2UINT(minute.c_str()));
	datetime.SetSecond(STR2UINT(second.c_str()));
}

//---------------------------------------------------------------------------------------
bool KYMD2JDay(int year,int month,int day,unsigned long& jday)
{
	long ya, c;
	if(year <= 1000)
	{
		return false;
	}

	if(month > 2)
	{
		month -= 3;
	}
	else 
	{
		month += 9;
		year--;
	}

	c  = year;
	c /= 100;
	ya = year - 100 * c;

	jday = 1721119 + day + ( 146097 * c ) / 4 + ( 1461 * ya ) / 4 + ( 153 * month + 2 ) / 5;
	return true;
}

//---------------------------------------------------------------------------------------
void KJDay2YMD(unsigned long jday,KFormatDateTime& datetime)
{
   int  Day, Month, Year;
   long x;
   long j = jday - 1721119;

   Year   = ( j * 4 - 1 ) / 146097;
   j      = j * 4 - 146097 * Year - 1;
   x      = j / 4;
   j      = ( x * 4 + 3 ) / 1461;
   Year   = 100 * Year + j;
   x      = ( x * 4 ) + 3 - 1461 * j;
   x      = ( x + 4 ) / 4;
   Month  = ( 5 * x - 3 ) / 153;
   x      = 5 * x - 3 - 153 * Month;
   Day    = ( x + 5 ) / 5;

   if( Month < 10 ) 
   {
  		Month += 3;
   }
   else 
   {
		Month -= 9;
		Year++;
   }

   datetime.SetYear(Year);
   datetime.SetMonth(Month);
   datetime.SetMDay(Day);
}

//---------------------------------------------------------------------------------------
void KGetWeekDay(int year,int month,int day,int& weekday)
{
	unsigned long jday = 0;
	KYMD2JDay(year,month,day,jday);
	weekday = (((jday + 1 ) % 7 ) + 6 ) % 7 + 1;
}

