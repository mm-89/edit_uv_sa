/*************************************************************************
                           hemisphericalSrc  -  description
                             -------------------
	authors		     : Laurent Francioli, CAO Alexandre
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//-- Class Interface of <hemisphericalSrc> (fichier hemisphericalSrc.h) --
#if ! defined ( HEMISPHERICAL_SRC_H )
#define HEMISPHERICAL_SRC_H

//-------------------------------------------------------- Used Interfaces
#include <vector>
#include <math.h>
#include "emittingSurfPt.h"
#include <common/meshmodel.h>
#include "IOFile.h"
using namespace std;

//-------------------------------------------------------------- Constants

//------------------------------------------------------------------ Types 

//------------------------------------------------------------------------ 
// Aim of the class <hemisphericalSrc>
// This object is used to represent the diffuse and reflected UV sources.
// The attributes are used as following :
// - radius is the radius of the hemisphere.
// - lvlNb is the number of level in which the hemisphere will be divided.
// - ptNb is the number of point on each level.
// - emitPtList is a vector which contains the points representing the
//   hemisphere.
//
// NOTE : The sign of the radius indicates the direction of the vertical
//        directing vector of the base.
//------------------------------------------------------------------------ 

class hemisphericalSrc
{
//----------------------------------------------------------------- PUBLIC

public:
//--------------------------------------------------------- Public Methods
	
	void loadEmittingPts();
	/* calculate the position of the emitting points which simplify the
	 * representation of the hemisphere */

	void setPtIntensity(float anIntensity);
	/* calculate the intensity for every points of emitPtList. */
	
	//Saves the hemispherical source as a mesh containing only vertices
	//It could be later enhanced to show colours depending on emitingPts intensities
	//Or faces.
	void saveHemiSourceAsMesh(string filename, MeshDocument *doc);

//-------------------------------------------------- Public Inline Methods
	// Center coordinates
	inline vcg::Point3<float> getCenter(){return center;}
	inline void setCenterCoordinates(vcg::Point3<float> coords)
	{
		center = coords;
	}

	// Get the radius hemisphere
	inline float getRadius() const
	{
		return radius;
	}
	//Sets the model size and calculate radius
	//Radius = modelSize * radiusFactor
	inline void calculateRadius(float modelSize)
	{
		radius = modelSize * radiusFactor;
	}

	//Radius Factor of the hemisphere
	inline float getRadiusFactor(){return radiusFactor;}
	inline void setRadiusFactor(float factor){radiusFactor=factor;}
	
	//total intensity
	inline float getTotalIntensity(){return totalIntensity;}

	// Point Parameters
	inline int getPtNb() const
	{
		return ptNb;
	}
	inline void setPtNb (int aPtNb)
	{
		ptNb = aPtNb;
	}
	inline int getLvlNb() const
	{
		return lvlNb;
	}
	inline void setLvlNb (int aLvlNb)
	{
		lvlNb = aLvlNb;
	}

	//Attenuation angle for anisotropic simulation
	inline void setAttenuationAngle(float angle){attenuation_angle = angle;}

// List of points simplifying the hemisphere
	inline vector<emittingSurfPt>::iterator listBegin() const
	{
		return emitPtList->begin();
	}

	inline vector<emittingSurfPt>::iterator listEnd() const
	{
		return emitPtList->end();
	}

    hemisphericalSrc ( );

    virtual ~hemisphericalSrc ( );

protected:
	// center coordinates
	vcg::Point3<float> center;
	// radius of the hemisphere
	float radius;
	//radius multiplication factor for the hemisphere
	float radiusFactor;
	//total intensity
	float totalIntensity;
	// Point Parameters
	int ptNb;
	int lvlNb;
	// List of points simplifying the hemisphere
	vector<emittingSurfPt>* emitPtList;
	//Anisotropic attenuation angle
	float attenuation_angle;

};

#endif // HEMISPHERICAL_SRC_H

