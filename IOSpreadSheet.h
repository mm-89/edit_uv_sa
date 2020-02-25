#ifndef IOSPREADSHEET_H
#define IOSPREADSHEET_H

#include <string>
#include "date.h"

#include <QStringList>

using namespace std;

class IOSpreadSheet
{
public:
	enum Columns
	{
		//Year Column index. Column Format: YYYY
		YEAR_COL = 1,
		//Month Column index. Column Format: MM
		MONTH_COL = 2,
		//Day Column index. Column Format: DD
		DAY_COL = 3,
		//Hour Column index. Column Format: HH
		HOUR_COL = 4,
		//Minutes Column index. Column Format: MM
		MIN_COL = 5,
		//Zenith Column index. Column Format: Degrees
		ZENITH_COL = 6,
		//Azimut Column index. Column Format: Degrees
		AZIMUT_COL = 7,
		//Global UV Column index. Column Format: Float
		GLOBALUV_COL = 8,
		//Diffuse UV Column index. Column Format: Float
		DIFFUSEUV_COL = 9,
		//Direct UV Column index. Column Format: Float
		DIRECTUV_COL = 10,
		//Reflected UV Column index. Column Format: Float
		REFLECTEDUV_COL = 11,
		//Total column count
		TOTAL_COLUMN_COUNT = REFLECTEDUV_COL
	};

	virtual ~IOSpreadSheet() {}

	virtual int test() = 0;

	virtual int init(const char* fileName) = 0;

	virtual int open(const char* fileName) = 0;

	virtual string read(int nRow, int nColumn) = 0;

	virtual Date readTimeStamp(int nRow) = 0;

	virtual int searchDateRow(const Date &searchedDate) = 0;

	virtual Date* getFirstDate() = 0;

	virtual Date* getLastDate() = 0;

	virtual int getTimeStep() = 0;

	virtual void readTimeData() = 0;

	virtual int searchExcelRow(int searchedNumber, int startRow, int column) = 0;

	virtual QStringList extensions() const = 0;
};

#endif // IOSPREADSHEET_H
