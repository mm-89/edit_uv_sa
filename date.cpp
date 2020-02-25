/*************************************************************************
                           Date  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//---------- Class <Date> (fichier date.cpp) ----------

#include "date.h"

//----------------------------------------------------------------- PUBLIC

//--------------------------------------------------------- Public Methods

//Check that the date is valid: month/day/hour/min in acceptable range
bool Date::isValid() const
{

	//Check if there was a unresolved conversion issue
	if(tt_date == (time_t)-1){return false;}
	
	//Check the month
	if(tm_date.tm_mon<0 || tm_date.tm_mon>11){return false;}
	
	//Check the hour
	if(tm_date.tm_hour<0 || tm_date.tm_hour>23){return false;}
	
	//Check the minutes
	if(tm_date.tm_min<0 || tm_date.tm_min>59){ return false;}
	
	//Check the day
	if(tm_date.tm_mday>0)
	{
		//If the day is <29, it exists for any month
		if(tm_date.tm_mday<29){return true;}
		//If the day is > 31, it is wrong in any case
		else if(tm_date.tm_mday>31){return false;}
		//If the day is between 29 and 31, it has to be checked
		else
		{
			tm result_date=*(localtime(&tt_date));
			if(result_date.tm_mday != tm_date.tm_mday){return false;}
			return true;
		}
	}
	//The day is <1
	return false;	
}

//--------------------------------------------------- Operator Overloading

bool Date::operator<(const Date &date2) const {
	return (tt_date< date2.tt_date);
}
bool Date::operator<=(const Date &date2) const {
	return (tt_date<= date2.tt_date);
}
bool Date::operator>(const Date &date2) const {
	return (tt_date> date2.tt_date);
}
bool Date::operator>=(const Date &date2) const {
	return (tt_date>= date2.tt_date);
}
Date Date::operator=(const Date &date2) {
	tm_date = date2.tm_date;
	tt_date = date2.tt_date;
	return *this;
}

int Date::operator-(const Date &date2) const {
	return tt_date-date2.tt_date;
}	

Date Date::operator+(const int &seconds) const{
	Date result;
	result.tt_date = this->tt_date + seconds;
	result.tm_date = *(localtime(&result.tt_date));
	result.tm_date.tm_isdst=-1;
	return result;
}

Date& Date::operator+=(const int &seconds){
	this->tt_date += seconds;
	this->tm_date = *(localtime(&tt_date ));
	this->tm_date.tm_isdst=-1;
	return *this;
}


//---------------------------------------------- Constructors - destructor
//Create a new date with the current local Date/Time
Date::Date(){
	time ( &tt_date );
	tm_date = *(localtime(&tt_date ));
	tm_date.tm_isdst=-1;
}

//Create a new date with the specified year/month/day hour:minute
Date::Date ( int year, int month, int day, int hour, int minute )
{
	//Set tm_date
	tm_date.tm_year = year-1900;
	tm_date.tm_mon = month-1;
	tm_date.tm_mday = day;
	tm_date.tm_hour = hour;
	tm_date.tm_min = minute;
	tm_date.tm_sec = 0;
	tm_date.tm_isdst=-1;
	
	//Set tt_date
	tm temp_date = tm_date;
	tt_date = mktime(&temp_date);
}


//---------------------------------------------------------------- PRIVATE

//------------------------------------------------------ Protected Methods



