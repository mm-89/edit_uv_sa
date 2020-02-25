#include "IOCSV.h"

#include "strConv.h"

#include <QFile>
#include <QTextStream>

using namespace std;

const int totalColumnCount = IOSpreadSheet::TOTAL_COLUMN_COUNT;

struct Record
{
	Record()
	{
	}

	Record(const Record &r)
	{
		for(int i = 0; i < totalColumnCount; i++)
			values[i] = r[i];
	}

	string &operator[](size_t i)
	{
		return values[i];
	}

	const string &operator[](size_t i) const
	{
		return values[i];
	}

	string values[totalColumnCount];
};

///////////////////////////////////////////////////////////////////////////////

struct IOCSV::Data
{
	Data()
	{
		timeStep = 0;
		cachedRow = 1;
		cachedDate = Date(0, 0, 0, 0, 0);
	}

	vector<Record> records;

	Date firstDate;
	Date lastDate;
	int timeStep;

	// cache
	int cachedRow;
	Date cachedDate;
};

///////////////////////////////////////////////////////////////////////////////

IOCSV::IOCSV()
{
	data = new Data();
}

IOCSV::~IOCSV()
{
	if(data)
		delete data;
}

int IOCSV::test()
{
	return 0;
}

int IOCSV::init(const char *)
{
	return 0;
}

int IOCSV::open(const char *fileName)
{
	data->records.clear();

	QFile file(fileName);
	if(!file.open(QFile::ReadOnly))
	{
		return -1;
	}

	QTextStream textStream(&file);
	QStringList str;
	textStream.readLine(); // skip header
	while(!textStream.atEnd())
	{
//		str = textStream.readLine().split(',');
//		Record r;
//		for(int i = 0; i < totalColumnCount && i < str.size(); i++)
//			r[i] = str[i].toStdString();
//		data->records.push_back(r);
	}

	file.close();

	readTimeData();

	return 0;
}

string IOCSV::read(int nRow, int nColumn)
{
	if(nRow >= (int)data->records.size())
		return "";
	return data->records[nRow - 1][nColumn - 1];
}

Date IOCSV::readTimeStamp(int nRow)
{
	int year, month, day, hour, min;

	strConv::from_string(read(nRow, YEAR_COL), year);
	strConv::from_string(read(nRow, MONTH_COL), month);
	strConv::from_string(read(nRow, DAY_COL), day);
	strConv::from_string(read(nRow, HOUR_COL), hour);
	strConv::from_string(read(nRow, MIN_COL), min);

	return Date(year, month, day, hour, min);
}

int IOCSV::searchDateRow(const Date &searchedDate)
{
	int searchedRow = 1;
	if(data->cachedDate <= searchedDate)
		searchedRow = data->cachedRow;
	if((searchedRow = searchExcelRow(searchedDate.getYear(), searchedRow, YEAR_COL)) > -1)
	{
		if((searchedRow = searchExcelRow(searchedDate.getMonth(), searchedRow, MONTH_COL)) > -1)
		{
			if((searchedRow = searchExcelRow(searchedDate.getDay(), searchedRow, DAY_COL)) > -1)
			{
				if((searchedRow = searchExcelRow(searchedDate.getHour(), searchedRow, HOUR_COL)) > -1)
				{
					searchedRow = searchExcelRow(searchedDate.getMinute(), searchedRow, MIN_COL);
				}
			}
		}
	}
	data->cachedRow = searchedRow;
	data->cachedDate = searchedDate;
	return searchedRow;
}

Date *IOCSV::getFirstDate()
{
	return &data->firstDate;
}

Date *IOCSV::getLastDate()
{
	return &data->lastDate;
}

int IOCSV::getTimeStep()
{
	return data->timeStep;
}

QStringList IOCSV::extensions() const
{
	return QStringList() << "*.csv";
}

void IOCSV::readTimeData()
{
	int emptyCount = 0;
	string buffer;
	int firstRow = -1;
	int lastRow = -1;
	int currentRow = 1;
	int currentValue;
	while(emptyCount<10)
	{
		if((buffer = read(currentRow, YEAR_COL)).length()==0)
		{
			emptyCount++;
		}
		else if(strConv::from_string(buffer, currentValue) && currentValue > 1990)
		{
			if(firstRow == -1){
				firstRow = currentRow;
			}
			lastRow = currentRow;
		}
		currentRow++;
	}

	//Create the dates if found
	if(firstRow > 0 && lastRow > 0){
		int year,month,day,hour,min;
		buffer = read(firstRow, YEAR_COL);
		strConv::from_string(buffer, year);
		buffer = read(firstRow, MONTH_COL);
		strConv::from_string(buffer, month);
		buffer = read(firstRow, DAY_COL);
		strConv::from_string(buffer, day);
		buffer = read(firstRow, HOUR_COL);
		strConv::from_string(buffer, hour);
		buffer = read(firstRow, MIN_COL);
		strConv::from_string(buffer, min);
		data->firstDate = Date(year,month,day,hour,min);

		buffer = read(lastRow, YEAR_COL);
		strConv::from_string(buffer, year);
		buffer = read(lastRow, MONTH_COL);
		strConv::from_string(buffer, month);
		buffer = read(lastRow, DAY_COL);
		strConv::from_string(buffer, day);
		buffer = read(lastRow, HOUR_COL);
		strConv::from_string(buffer, hour);
		buffer = read(lastRow, MIN_COL);
		strConv::from_string(buffer, min);
		data->lastDate = Date(year,month,day,hour,min);
	}

	data->timeStep = (data->lastDate-data->firstDate)/(lastRow-firstRow);
}

int IOCSV::searchExcelRow(int searchedNumber, int startRow, int column)
{
	int emptyCount = 0;
	string buffer;
	int currentValue;
	int currentRow = startRow;
	while(emptyCount < 10)
	{
		if((buffer = read(currentRow, column)).length()==0)
		{
			emptyCount++;
		}
		else if(strConv::from_string(buffer, currentValue) && currentValue == searchedNumber)
		{
			return currentRow;
		}
		currentRow++;
	}
	return -1;
}

