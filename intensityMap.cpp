/*************************************************************************
                           IntensityMap  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//---------- Class <IntensityMap> (fichier intensityMap.cpp) ----------

#include "intensityMap.h"

//----------------------------------------------------------------- PUBLIC

//--------------------------------------------------------- Public Methods

void IntensityMap::clear(){
	intensities->clear();
	mapComplete = false;
}

//Sets the number of Vertices of the model (important to know if map is complete)
void IntensityMap::setNumModelVertices(int numModelVertices)
{
	numModelVert = numModelVertices;
	evaluateMapComplete();
}

//Re-Evaluate if the map is complete. Should be used if unsure whether the evaluation
//is up-to-date.
bool IntensityMap::evaluateMapComplete()
{
	mapComplete = false;
	if (numModelVert>0 && intensities->size() == numModelVert)
	{
		mapComplete = true;
	}
	return mapComplete;
}

//Loads the map corresponding to the given path. Checks whether the map is complete
void IntensityMap::loadMap(string path)
{
	mapPath = path;
	if(!mapPath.empty()){
		delete intensities;
		intensities = IOFile::loadIntensityMap(mapPath.c_str());
	}
	evaluateMapComplete();
}

//Saves the current map.
//If the map was complete before saving, you need to set overwrite
//to true to actually save the map.
void IntensityMap::saveMap(bool overwrite)
{
	if ((overwrite || !mapComplete) && evaluateMapComplete() && !mapPath.empty())
	{
		IOFile::saveIntensityMap(mapPath.c_str(), intensities);
	}
}

//--------------------------------------------------- Operator Overloading


//---------------------------------------------- Constructors - destructor

//Constructor with optional args
IntensityMap::IntensityMap(int numModelVertices, string path){
	numModelVert = numModelVertices;
	loadMap(path);
	evaluateMapComplete();	
	intensities = new vector<float>;
}

//Destructor
IntensityMap::~IntensityMap ( )
{
	delete intensities;
}


//---------------------------------------------------------------- PRIVATE


//------------------------------------------------------ Protected Methods



