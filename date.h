/*************************************************************************
                           Date  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//---------- Class Interface of <date> (fichier date.h) ----------
#if ! defined ( DATE_H )
#define DATE_H

//-------------------------------------------------------- Used Interfaces
#include <time.h>

//-------------------------------------------------------------- Constants 

//------------------------------------------------------------------ Types 

//------------------------------------------------------------------------ 

//------------------------------------------------------------------------ 

//This class has been implemented to provide an easy way to represent dates
//Note that now that SimUVEx uses Qt, this could/shoudl be replace with
//QDates
class Date
{
//----------------------------------------------------------------- PUBLIC

public:
//--------------------------------------------------------- Public Methods

//Accessors
//Note that if an invalid date is entered in the first place,
//the accessors might return data outside common boundaries
//A date can be checked for validity using isValid()
inline tm getDate() const {return tm_date;}

//Returns the date in current format
inline int getYear() const {return 1900+tm_date.tm_year;}

//Returns the month as in current format (1..12)
inline int getMonth() const {return tm_date.tm_mon+1;}

//Returns the month as in current format (1..31)
inline int getDay() const {return tm_date.tm_mday;}

//Returns the month as in current format (0..23)
inline int getHour() const {return tm_date.tm_hour;}

//Returns the month as in current format (0..59)
inline int getMinute() const {return tm_date.tm_min;}

//Check that the date is valid: month/day/hour/min in acceptable range
bool isValid() const;

//--------------------------------------------------- Operator Overloading

bool operator<(const Date &date2) const;
bool operator<=(const Date &date2) const;
bool operator>(const Date &date2) const;
bool operator>=(const Date &date2) const;
Date operator=(const Date &date2);
int  operator-(const Date &date2) const;
Date operator+(const int &seconds) const;
Date& operator+=(const int &seconds);



//---------------------------------------------- Constructors - destructor

Date();

Date ( int year, int month, int day, int hour, int minute );


//---------------------------------------------------------------- PRIVATE

private:
//------------------------------------------------------ Protected Methods

//--------------------------------------------------- Protected Attributes

	//Representation of the date as time_t
	time_t tt_date;
	//Representation of hthe date as tm struct
	tm tm_date;


};

#endif // DATE_H

