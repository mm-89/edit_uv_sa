#ifndef IOCSV_H
#define IOCSV_H

#include "IOSpreadSheet.h"

class IOCSV : public IOSpreadSheet
{
public:
	IOCSV();
	virtual ~IOCSV();

	int test();

	int init(const char* fileName);

	int open(const char *fileName);

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

	struct Data;
	Data *data;
};

#endif // IOCSV_H
