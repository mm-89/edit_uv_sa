/*************************************************************************
                           Zone  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

#include "zone.h"

//----------------------------------------------------------------- PUBLIC

//--------------------------------------------------------- Public Methods
void Zone::addZoneIP(int IP){
	zoneIP+=IP;
	if(subZones){
		vector<Zone*>::iterator subzoneIter;
		for(subzoneIter=subZones->begin();subzoneIter!=subZones->end();++subzoneIter){
			(*subzoneIter)->addZoneIP(IP);
		}
	}
	if(zoneVertices){
		
	}
}

void Zone::setZoneIP(int IP){
	zoneIP=IP;
	if(subZones){
		vector<Zone*>::iterator subzoneIter;
		for(subzoneIter=subZones->begin();subzoneIter!=subZones->end();++subzoneIter){
			(*subzoneIter)->setZoneIP(IP);
		}
	}
}
void Zone::resetZoneIP(){
	zoneIP=1;
	if(subZones){
		vector<Zone*>::iterator subzoneIter;
		for(subzoneIter=subZones->begin();subzoneIter!=subZones->end();++subzoneIter){
			(*subzoneIter)->resetZoneIP();
		}
	}
}

void Zone::addSubZone(Zone *subZone){
	if(!subZones){
		subZones = new vector<Zone*>;
	}
	subZones->push_back(subZone);
}

void Zone::mergeZoneIntensity(EvaluatedIntensity* intensity){
	if(!intensity->intensityList || !zoneEvaluatedIntensity->intensityList || intensity->intensityList->size() != zoneEvaluatedIntensity->intensityList->size()){
		return;
	}
	
	//Merge dates
	if((*intensity->beginDate) < (*zoneEvaluatedIntensity->beginDate)){
		delete zoneEvaluatedIntensity->beginDate;
		zoneEvaluatedIntensity->beginDate = new Date(*intensity->beginDate);
	}
	if((*intensity->endDate) > (*zoneEvaluatedIntensity->endDate)){
		delete zoneEvaluatedIntensity->endDate;
		zoneEvaluatedIntensity->endDate = new Date(*intensity->endDate);
	}

	//Merge Intensities
	IntensityList::iterator intIter;
	IntensityList::iterator zoneIntIter = zoneEvaluatedIntensity->intensityList->begin();
	for(intIter = intensity->intensityList->begin(); intIter != intensity->intensityList->end(); ++intIter){
		(*zoneIntIter) += (*intIter);
		++zoneIntIter;
	}



}

void Zone::multiplyZoneIntensities(float factor){
	//Multiply the zone intensity
	IntensityList::iterator zoneIntIter;
	for(zoneIntIter = zoneEvaluatedIntensity->intensityList->begin(); zoneIntIter != zoneEvaluatedIntensity->intensityList->end(); ++zoneIntIter){
		(*zoneIntIter) *= factor;
	}

	//Multiply the subzones intensites
	if(subZones){
		vector<Zone*>::iterator subZoneIter;
		for(subZoneIter = subZones->begin(); subZoneIter != subZones->end(); ++subZoneIter){
			(*subZoneIter)->multiplyZoneIntensities(factor);
		}
	}
}	

//FACE INTENSITIES -- NOT CURRENTLY USED
/*void Zone::addZoneFace(CFaceO *face){
	if(!zoneFaces){
		zoneFaces = new map<CFaceO*,int>;
	}
	(*zoneFaces)[face] += 1;
}*/

void Zone::evaluateIntensity(VertexIntensities *vertexIntensities, const Date &beginDate, const Date &endDate){
	//Create Intensity container and intiate its values
	Intensity intensity;
	intensity.direct = 0;
	intensity.diffused = 0;
	intensity.reflected = 0;
	//First evaluate the subzones -if any - recursively and get their values back
	if(subZones){
		vector<Zone*>::iterator zoneIter;
		float vertSurface;
		for(zoneIter = subZones->begin(); zoneIter!=subZones->end(); zoneIter++){
			(*zoneIter)->evaluateIntensity(vertexIntensities,beginDate,endDate);
			vertSurface = (*zoneIter)->getSurface();			
			//Note that the intensities need to be multiplied by the vertex surface in order to ponder them.
			//The full zone intensity will be divided by the full zone surface later
			intensity.direct += vertSurface*((*zoneIter)->getEvaluatedIntensity()->intensityList->back().direct);
			intensity.diffused += vertSurface*((*zoneIter)->getEvaluatedIntensity()->intensityList->back().diffused);
			intensity.reflected += vertSurface*((*zoneIter)->getEvaluatedIntensity()->intensityList->back().reflected);
			zoneSurface += 	vertSurface;
		}
	}
	//If the zone contains vertices, evaluate the intensity received
	//by the zone (depends on vertex intensity and zone surface)
	if(zoneVertices){
		vector<CVertexO*>::iterator vertIter;
		for(vertIter=zoneVertices->begin();vertIter!=zoneVertices->end();vertIter++){
			//Get the area of all the faces around hte vertex
			vcg::face::VFIterator<CFaceO> vfi(*vertIter);
			float vertSurface = 0;
			for(;!vfi.End();++vfi)
			{
				vertSurface += vcg::DoubleArea<CFaceO>(*(vfi.F())) / 2.0;
			}			
			//Add the vertex intensity and area to the zone intensity and area
			//Note that each vertex is multiplied by its surface for ponderation
			//TODO: THIS SHOULD BE DONE INDEPENDANTLY OF THE ZONE EVALUATION!!!
			(*vertexIntensities)[(*vertIter)]->setIP(zoneIP);
			//END OF IMPORTANT TODO
			intensity.direct += (vertSurface*((*vertexIntensities)[(*vertIter)]->direct));
			intensity.diffused += (vertSurface*((*vertexIntensities)[(*vertIter)]->diffused));
			intensity.reflected += (vertSurface*((*vertexIntensities)[(*vertIter)]->reflected));
			zoneSurface += vertSurface;
		}
	}
	//After all the subzones and vertices intensities and the zone
	//area has been calculated, save all the data
	if(zoneSurface == 0)
	{
		intensity.direct = 0;
		intensity.diffused = 0;
		intensity.reflected = 0;
	}
	else
	{
		intensity.direct = intensity.direct / zoneSurface;
		intensity.diffused = intensity.diffused / zoneSurface;
		intensity.reflected = intensity.reflected / zoneSurface;
	}
	zoneEvaluatedIntensity->beginDate = new Date(beginDate);
	zoneEvaluatedIntensity->endDate = new Date(endDate);
	zoneEvaluatedIntensity->intensityList = new vector<Intensity>;
	zoneEvaluatedIntensity->intensityList->push_back(intensity);
}

void Zone::clearEvaluatedIntensities()
{
	zoneSurface = 0;
	if(zoneEvaluatedIntensity)
	{
		if(zoneEvaluatedIntensity->intensityList)
		{
			zoneEvaluatedIntensity->intensityList->clear();
			delete zoneEvaluatedIntensity->intensityList;
			zoneEvaluatedIntensity->intensityList = 0;
		}
		if(zoneEvaluatedIntensity->beginDate)
		{
			delete zoneEvaluatedIntensity->beginDate;
			zoneEvaluatedIntensity->beginDate = 0;
		}
		if(zoneEvaluatedIntensity->endDate)
		{
			delete zoneEvaluatedIntensity->endDate;
			zoneEvaluatedIntensity->endDate = 0;
		}
	}


	if(subZones){
		for(auto subzoneIter=subZones->begin();subzoneIter!=subZones->end();++subzoneIter)
			(*subzoneIter)->clearEvaluatedIntensities();
	}
}


//--------------------------------------------------- Operator Overloading

//---------------------------------------------- Constructors - destructor

Zone::Zone(std::string name, bool active, vcg::Color4b *color):zoneName(name),zoneActive(active), zoneColor(color){ 
	subZones = 0;
	zoneVertices = 0;
	zoneSurface = 0;
	zoneEvaluatedIntensity = new EvaluatedIntensity;
	zoneIP = 1;
	//FACE INTENSITIES -- NOT CURRENTLY USED
	//zoneFaces = 0;
}


Zone::~Zone(){
	if(zoneEvaluatedIntensity)
	{
		if(zoneEvaluatedIntensity->intensityList)
		{
			zoneEvaluatedIntensity->intensityList->clear();
			delete zoneEvaluatedIntensity->intensityList;
		}
		delete zoneEvaluatedIntensity->beginDate;
		delete zoneEvaluatedIntensity->endDate;
		delete zoneEvaluatedIntensity;
	}

	if (zoneColor)	delete zoneColor;
	if (subZones){
		while(!subZones->empty()){
		 delete subZones->back();
		 subZones->pop_back();
		 }
		delete subZones;
	}	
	if(zoneVertices){
		delete zoneVertices;
	}
	//FACE INTENSITIES -- NOT CURRENTLY USED
	/*if (zoneFaces){
		zoneFaces->clear();
		delete zoneFaces;
	}*/
}


//---------------------------------------------------------------- PRIVATE

//------------------------------------------------------ Protected Methods
