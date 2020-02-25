#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include <map>

#include "IOSpreadSheet.h"

using namespace std;

class SpreadSheet : public IOSpreadSheet
{
public:
	enum SpreadSheetTypes
	{
		ST_Excel,
		ST_CSV,
		ST_Count
	};

	SpreadSheet();
	virtual ~SpreadSheet();

	int test();

	int init(const char* fileName);

	int open(const char* fileName);

	string read(int nRow, int nColumn);

	Date readTimeStamp(int nRow);

	int searchDateRow(const Date &searchedDate);

	Date* getFirstDate();

	Date* getLastDate();

	int getTimeStep();

	QStringList extensions() const;

private:
	void readTimeData();

	int searchExcelRow(int searchedNumber, int startRow, int column);

	IOSpreadSheet *current();

	bool isSupported(SpreadSheetTypes type) const;

	map<SpreadSheetTypes, IOSpreadSheet *> children;
	SpreadSheetTypes type;
};

#endif // SPREADSHEET_H
