#include "stdafx.h"
#include "KBASE/INIReader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------------------
INIReader::INIReader(void)
{
}

//---------------------------------------------------------------------------------------
INIReader::~INIReader(void)
{
}

//---------------------------------------------------------------------------------------
INIReader::INIReader(const std::string &path)
{
    LoadFromFile(path);
}

//---------------------------------------------------------------------------------------
INIReader::INIReader(std::istream &stream)
{
    LoadFromStream(stream);
}

//---------------------------------------------------------------------------------------
bool INIReader::LoadFromFile(const std::string &path)
{
    std::ifstream fin(path.c_str());
    if(!fin)
        return false;

    LoadFromStream(fin);

    return true;
}

//---------------------------------------------------------------------------------------
void INIReader::LoadFromText(const std::string &text)
{
	std::istringstream a(text);
	LoadFromStream(a);
}

//---------------------------------------------------------------------------------------
void INIReader::LoadFromStream(std::istream &stream)
{
    while(stream)
    {
        string line;
        getline(stream, line);
        if(line.empty())
            continue;

        istringstream sl(line);

        ws(sl);

        if(sl.peek() == ';')
        {
        }
        else if(sl.peek() == '[')
        {
            sl.ignore();

            section sec;
            getline(sl, sec.first, ']');
            if(!sl.eof())
                _sv.push_back(sec);
        }
        else
        {
            if(!_sv.empty())
            {
                key_val kv;
                getline(sl, kv.first, '=');
                if(!sl.eof())
                {
                    getline(sl, kv.second);
                    _sv.back().second.push_back(kv);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------
bool INIReader::SaveToFile(const std::string &path)
{
    ofstream fout(path.c_str());
    if(!fout)
        return false;

    SaveToStream(fout);

    return true;
}

//---------------------------------------------------------------------------------------
void INIReader::SaveToStream(std::ostream &stream)
{
    for(size_type i = 0; i < _sv.size(); ++i)
    {
        stream<<'['<<_sv[i].first<<']'<<std::endl;

        key_val_citer it;
        key_val_citer ed = _sv[i].second.end();
        for(it = _sv[i].second.begin(); it != ed; ++it)
            stream<<it->first<<'='<<it->second<<std::endl;
    }
}

//---------------------------------------------------------------------------------------
section* INIReader::GetSection(const std::string &sec)
{
    for(size_type i = 0; i < _sv.size(); ++i)
    {
        if(sec == _sv[i].first)
            return &_sv[i];
    }

    return NULL;
}

//---------------------------------------------------------------------------------------
const section* INIReader::GetSection(const std::string &sec) const
{
    for(size_type i = 0; i < _sv.size(); ++i)
    {
        if(sec == _sv[i].first)
            return &_sv[i];
    }

    return NULL;
}

//---------------------------------------------------------------------------------------
bool INIReader::GetValue(const std::string &sec, const std::string &key, std::string &val) const
{
	val = "";
    for(size_type i = 0; i < _sv.size(); ++i)
    {
        if(sec != _sv[i].first)
            continue;

        key_val_citer it;
        key_val_citer ed = _sv[i].second.end();
        for(it = _sv[i].second.begin(); it != ed; ++it)
        {
            if(it->first == key)
            {
                val = it->second;
                return true;
            }
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------
bool INIReader::GetValue(const std::string &sec, const std::string &key, bool &val) const 
{ 
	val = false;

    string str; 
    if(!GetValue(sec, key, str)) 
        return false; 
    
    istringstream ss(str); 
    return !!(ss>>val); 
} 

//---------------------------------------------------------------------------------------
bool INIReader::GetValue(const std::string &sec, const std::string &key, short &val) const 
{ 
	val = 0;

    string str; 
    if(!GetValue(sec, key, str)) 
        return false; 
    
    istringstream ss(str); 
    return !!(ss>>val); 
} 

//---------------------------------------------------------------------------------------
bool INIReader::GetValue(const std::string &sec, const std::string &key, unsigned short &val) const 
{ 
	val = 0;

    string str; 
    if(!GetValue(sec, key, str)) 
        return false; 
    
    istringstream ss(str); 
    return !!(ss>>val); 
} 

//---------------------------------------------------------------------------------------
bool INIReader::GetValue(const std::string &sec, const std::string &key, int &val) const 
{ 
	val = 0;

    string str; 
    if(!GetValue(sec, key, str)) 
        return false; 
    
    istringstream ss(str); 
    return !!(ss>>val); 
} 

//---------------------------------------------------------------------------------------
bool INIReader::GetValue(const std::string &sec, const std::string &key, unsigned int &val) const 
{ 
	val = 0;

    string str; 
    if(!GetValue(sec, key, str)) 
        return false; 
    
    istringstream ss(str); 
    return !!(ss>>val); 
} 

//---------------------------------------------------------------------------------------
bool INIReader::GetValue(const std::string &sec, const std::string &key, long &val) const 
{ 
	val = 0;

	std::string str; 
    if(!GetValue(sec, key, str)) 
        return false; 
    
    std::istringstream ss(str); 
    return !!(ss>>val); 
} 

//---------------------------------------------------------------------------------------
bool INIReader::GetValue(const std::string &sec, const std::string &key, unsigned long &val) const 
{ 
	val = 0;

	std::string str; 
    if(!GetValue(sec, key, str)) 
        return false; 
    
    std::istringstream ss(str); 
    return !!(ss>>val); 
} 

//---------------------------------------------------------------------------------------
bool INIReader::GetValue(const std::string &sec, const std::string &key, float &val) const 
{ 
	val = 0;

    std::string str; 
    if(!GetValue(sec, key, str)) 
        return false; 
    
    std::istringstream ss(str); 
    return !!(ss>>val); 
} 

//---------------------------------------------------------------------------------------
bool INIReader::GetValue(const std::string &sec, const std::string &key, double &val) const 
{ 
	val = 0;

    std::string str; 
    if(!GetValue(sec, key, str)) 
        return false; 
    
    std::istringstream ss(str); 
    return !!(ss>>val); 
} 

//---------------------------------------------------------------------------------------
size_type INIReader::GetSectionSize(void) const
{
	return _sv.size();
}

//---------------------------------------------------------------------------------------
section& INIReader::operator[](size_type pos)
{
	return _sv[pos];
}

//---------------------------------------------------------------------------------------
const section& INIReader::operator[](size_type pos) const
{
	return _sv[pos];
}

//---------------------------------------------------------------------------------------
void INIReader::append_section(const section &sec)
{
	_sv.push_back(sec);
}