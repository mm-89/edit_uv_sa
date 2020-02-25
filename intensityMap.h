/*************************************************************************
                           IntensityMap  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//---------- Class Interface of <IntensityMap> (fichier intensityMap.h) ----------
#if ! defined ( INTENSITYMAP_H )
#define INTENSITYMAP_H

//-------------------------------------------------------- Used Interfaces

#include "IOFile.h"
#include <vector>
#include <string>
using namespace std;


//IntensityMap stores a map of how much relative intensity every vertex of a given
//model receives under a given hemsipherical source.
//Since this information is constant for a given model/source, it is only calculated
//once per simulation and can be written to a file for further reuse.
class IntensityMap
{
	//----------------------------------------------------------------- PUBLIC

public:
	//--------------------------------------------------------- Public Methods

	//Accessors
	inline string getMapPath(){ return mapPath;}

	//Caution: This method only outputs a cached result. If you're unsure whether the state
	//could have changed, use evaluateMapComplete() instead.
	inline bool getMapComplete(){return mapComplete;}

	//Accessor for the intensities stored in the map
	inline vector<float>* getIntensities(){return intensities;}

	//Clears all the intensities previously stored
	void clear();

	//Sets the number of Vertices of the model (important to know if map is complete)
	void setNumModelVertices(int numModelVertices);

	//Methods

	//Saves the current map.
	//If the map was complete before saving, you need to set overwrite
	//to true to actually save the map.
	void saveMap(bool overwrite = false);

	//Loads the map corresponding to the given path. Checks whether the map is complete
	void loadMap(string path);

	//Re-Evaluate if the map is complete. Should be used if unsure whether the evaluation
	//is up-to-date. Otherwise, use getMapComplete(), which does not re-evaluate and only
	//outputs the cached result
	bool evaluateMapComplete();


	//--------------------------------------------------- Operator Overloading


	//---------------------------------------------- Constructors - destructor

	IntensityMap(int numModelVertices = 0, string path = "");

	~IntensityMap();

private:
	//Stores the intensities
	vector<float>* intensities;
	//Stores the path to the map file
	string mapPath;
	//Caches whether the map is complete
	bool mapComplete;
	//Stores the number of vertices the model contains
	int numModelVert;
};

#endif // INTENSITYMAP_H

