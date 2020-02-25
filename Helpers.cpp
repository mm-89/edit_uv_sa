#include "Helpers.h"

void Helpers::deleteZones(std::vector<Zone*> **zonesPtr)
{
	std::vector<Zone*> *zones = *zonesPtr;
	if(zones){
		while(!zones->empty()){
			delete zones->back();
			zones->pop_back();
		}
		delete zones;
		*zonesPtr = 0;
	}
}

void Helpers::clearEvaluatedIntensities(vector<EvaluatedIntensity*> **evalIntensityListPtr)
{
	vector<EvaluatedIntensity*> *evalIntensityList = *evalIntensityListPtr;
	for(auto iter=evalIntensityList->begin(); iter!=evalIntensityList->end(); iter++)
	{
		(*iter)->intensityList->clear();
		delete (*iter)->intensityList;
		delete (*iter)->beginDate;
		delete (*iter)->endDate;
		delete *iter;
	}
	evalIntensityList->clear();
}
