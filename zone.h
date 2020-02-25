/*************************************************************************
                           UVMesh  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

#if ! defined ( ZONE_H )
#define ZONE_H

//-------------------------------------------------------- Used Interfaces

#include <common/meshmodel.h>
#include "intensities.h"
#include <vcg/space/color4.h> 
#include <vector>
#include <map>
using std::map;
using std::vector;

typedef map<CVertexO*,Intensity*> VertexIntensities;

//The Zone Class holds all he information about an anatomical zone on the model
class Zone
{
public:
	void addSubZone(Zone *subZone);

	//FACE INTENSITIES -- NOT CURRENTLY USED
	//void addZoneFace(CFaceO *face);

	//evaluates the intensity received by the zone.
	void evaluateIntensity(VertexIntensities *vertexIntensities, const Date &beginDate, const Date &endDate);

	void clearEvaluatedIntensities();

	//Get the zones subzones
	inline vector<Zone*>* getSubZones(){ return subZones;}

	//Get the zone color
	inline vcg::Color4b* getColor(){return zoneColor;}

	//Link the zone to the given vertices
	inline void setZoneVertices(vector<CVertexO*> *vertices){zoneVertices = vertices;}

	//Gets the zone surface
	inline float getSurface(){return zoneSurface;}

	//Get the zone received UV intensity
	inline EvaluatedIntensity* getEvaluatedIntensity(){return zoneEvaluatedIntensity;}

	//Get the zone name
	inline std::string getName(){return zoneName;}

	//Get the zone Protection Index (IP)
	inline int getIP(){return zoneIP;}

	//Check whether the zone is active (used in simulation)
	inline bool isZoneActive(){return zoneActive;}

	//Get the zone associated vertices
	inline vector<CVertexO*>* getZoneVertices(){return zoneVertices;}

	//Add an Protection Index to the zone
	void addZoneIP(int IP);
	//Set an Protection Index to the zone
	void setZoneIP(int IP);
	//Reset the zone Protection Index to 1
	void resetZoneIP();

	//Merges the given intensity with the current zone intensity
	//Note that the intensity lists need to be of the same length
	void mergeZoneIntensity(EvaluatedIntensity* intensity);

	//Multiplies the current zone intensities by the given factor
	//Also multiplies its subzones intensities
	//(Used for average calculations)
	void multiplyZoneIntensities(float factor);

	//--------------------------------------------------- Operator Overloading


	//---------------------------------------------- Constructors - destructor
	Zone(std::string name, bool active, vcg::Color4b *color = 0);

	Zone();

	~Zone();

	//---------------------------------------------------------------- PRIVATE

private:
	//------------------------------------------------------ Protected Methods


	//--------------------------------------------------- Protected Attributes

	//Defines whether a zone is active or not
	bool zoneActive;
	//Zone RGB Values
	vcg::Color4b *zoneColor;
	//Zone Name
	std::string zoneName;
	//Pointer to subZones if any
	vector<Zone*> *subZones;
	//Vertices assigned to the zone
	vector<CVertexO*> *zoneVertices;
	//Stores the surface of the zone
	float zoneSurface;
	//Stores the zone exposition
	EvaluatedIntensity *zoneEvaluatedIntensity;
	//Stores the IP to be applied on the zone
	int zoneIP;
	//Faces assigned to the zone and their respective
	//number of vertices in the zone
	//FACE INTENSITIES -- NOT CURRENTLY USED
	//map<CFaceO*,int> *zoneFaces;

	
};

#endif // ZONE_H




