#include "SpreadSheet.h"

#include <algorithm>

#include "IOCSV.h"

using namespace std;

SpreadSheet::SpreadSheet()
{
	type = ST_Count;
	for(int i = 0; i < ST_Count; i++)
	{
		SpreadSheetTypes thisType = (SpreadSheetTypes)i;
		if(!isSupported(thisType)) continue;

		if(type == ST_Count)
	{		type = thisType;

		// Add children
		children[thisType] = new IOCSV();
		}
	}
}

SpreadSheet::~SpreadSheet()
{
	for(auto it = children.begin(); it != children.end(); it++)
		delete it->second;
}

int SpreadSheet::test()
{
	return current()->test();
}

#include <QDebug>
int SpreadSheet::init(const char *fileName)
{
	std::string ext = fileName;
	ext = ext.substr(ext.length() - 3, 3);
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	ext = "*." + ext;

	for(auto it = children.begin(); it != children.end(); it++)
		if(it->second->extensions().contains(ext.c_str()))
		{
			type = it->first;
			break;
		}

	return current()->init(fileName);
}

int SpreadSheet::open(const char *fileName)
{
	return current()->open(fileName);
}

string SpreadSheet::read(int nRow, int nColumn)
{
	return current()->read(nRow, nColumn);
}

Date SpreadSheet::readTimeStamp(int nRow)
{
	return current()->readTimeStamp(nRow);
}

int SpreadSheet::searchDateRow(const Date &searchedDate)
{
	return current()->searchDateRow(searchedDate);
}

Date *SpreadSheet::getFirstDate()
{
	return current()->getFirstDate();
}

Date *SpreadSheet::getLastDate()
{
	return current()->getLastDate();
}

int SpreadSheet::getTimeStep()
{
	return current()->getTimeStep();
}

QStringList SpreadSheet::extensions() const
{
	QStringList res;
	for(auto it = children.begin(); it != children.end(); it++)
	{
		IOSpreadSheet *child = it->second;
		QStringList subRes = child->extensions();
		for(int j = 0; j < subRes.size(); j++)
			res << subRes[j];
	}
	return res;
}

void SpreadSheet::readTimeData()
{
	current()->readTimeData();
}

int SpreadSheet::searchExcelRow(int searchedNumber, int startRow, int column)
{
	return current()->searchExcelRow(searchedNumber, startRow, column);
}

IOSpreadSheet *SpreadSheet::current()
{
	return children[type];
}

bool SpreadSheet::isSupported(SpreadSheet::SpreadSheetTypes type) const
{
	switch(type)
	{
	case ST_Excel:
		return false;
	case ST_CSV:
		return true;
	default:
		return false;
	}
}
