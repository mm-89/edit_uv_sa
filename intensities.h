/*************************************************************************
                           intensities  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//This file contains Structures and classes relative to the storage
//and use of intensities throughout the model simulation

#if ! defined ( INTENSITIES_H )
#define INTENSITIES_H

#include "date.h"
#include <vector>
using std::vector;

//Definition of sources
struct IntensitySources
{
	enum Type
	{
		DIRECT	      = 0x00000001,
        DIFFUSED      = 0x00000002,
        REFLECTED     = 0x00000004
	};
};

//Class containing a simulated intensity
//Along with methods to perform calculations on it.
class Intensity
{
public:
	double direct;
	double diffused;
	double reflected;

	//Add extra protection indice
	inline void addIP(int extraIP){
		direct = direct*IP;
		diffused = diffused*IP;
		reflected = reflected*IP;
		IP += extraIP;
		direct = direct/IP;
		diffused = diffused/IP;
		reflected = reflected/IP;
	}

	//Set a given protection indice
	inline void setIP(int newIP){
		resetIP();
		IP = newIP;
		direct = direct/IP;
		diffused = diffused/IP;
		reflected = reflected/IP;
	}

	//Removes all protection
	inline void resetIP(){
		direct = direct*IP;
		diffused = diffused*IP;
		reflected = reflected*IP;
		IP = 1;
	}

	inline int getIP(){return IP;}

	//Constructor
	Intensity(){
		IP = 1;
		direct = diffused = reflected = 0.0;
	}

	//operators
	Intensity operator+(const Intensity &container2){
		Intensity result;
		result.direct = this->direct + container2.direct;
		result.diffused = this->diffused + container2.diffused;
		result.reflected = this->reflected + container2.reflected;
		return result;
	}
	Intensity& operator+=(const Intensity &container2){
		this->direct += container2.direct;
		this->diffused += container2.diffused;
		this->reflected += container2.reflected;
		return *this;
	}
	Intensity operator*(double factor){
		Intensity result;
		result.direct = this->direct * factor;
		result.diffused = this->diffused * factor;
		result.reflected = this->reflected * factor;
		return result;
	}
	Intensity& operator*=(double factor){
		this->direct *= factor;
		this->diffused *= factor;
		this->reflected *= factor;
		return *this;
	}
	Intensity operator/(int num){
		Intensity result;
		result.direct = this->direct/num;
		result.diffused = this->diffused/num;
		result.reflected = this->reflected/num;
		return result;
	}

private:
	int IP;
};

// List of intensities from begin date to end date
typedef vector<Intensity> IntensityList;

//Stores all simulated intensities from begin date/time to end date/time
struct EvaluatedIntensity
{
	Date* beginDate;
	Date* endDate;
	//FACE INTENSITIES -- NOT CURRENTLY USED
	//double area;
	IntensityList* intensityList;

	EvaluatedIntensity() :
		beginDate(0),
		endDate(0),
		intensityList(0)
	{
	}

	EvaluatedIntensity(const EvaluatedIntensity &other)
	{
		if(other.beginDate)
			beginDate = new Date(*other.beginDate);
		if(other.endDate)
			endDate = new Date(*other.endDate);
		intensityList = new IntensityList();
		for(const auto &intensity : *other.intensityList)
			intensityList->push_back(intensity);
	}

	EvaluatedIntensity operator+=(const EvaluatedIntensity &other)
	{
		if(other.endDate)
			*endDate = *other.endDate;
		int index = 0;
		for(const auto &intensity : *intensityList)
			(*intensityList)[index++] += intensity;
		return *this;
	}
};

#endif // INTENSITIES_H
