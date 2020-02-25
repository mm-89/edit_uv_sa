/*************************************************************************
                           ptSrc  -  description
                             -------------------
	authors		     : Laurent Francioli, CAO Alexandre
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//--------- Class Interface of <ptSrc> (fichier ptSrc.h) ---------
#if ! defined ( PTSRC_H )
#define PTSRC_H

//-------------------------------------------------------- Used Interfaces
#include "emittingPt.h"
#include "date.h"
#include "IOFile.h"
#include "IOSpreadSheet.h"
#include "strConv.h"

//-------------------------------------------------------------- Constants 

//------------------------------------------------------------------ Types 

//------------------------------------------------------------------------ 
// Aim of the class <ptSrc>
// This object is used to represent the direct UV source, ie the sun.
// The attributes are used as following :
// - ref is used to set the postion of the center of the model.
// - angleToNorth is the angle between the z-axis and the North
// - sun is an emittingPt used to represent the sun.
//------------------------------------------------------------------------ 

class ptSrc
{
public:
	//Sets the sun position and its intensity based on weather data
	void setSunPosIntensity( float radius, float azimut, float zenith, float intensity );
	
	//Saves the trajectory of the sun from the begin date until the end date as a Mesh
	//It also sets colours to the points composing the Mesh from 0 to 255 Red based on the
	//sun intensity 
		void savePtSrcAsMesh(IOSpreadSheet *spreadsheet, string filename, MeshDocument *doc, const char* datafilePath, Date beginDate, Date endDate, float radius);

	// Reference Point
	inline vcg::Point3<float> getRef() const
	{
		return ref;
	}
	inline void setRefPt( vcg::Point3<float> coords )
	{
		ref = coords;
	}

	// The angle between the z-axis and the North
	inline float getAngleToNorth() const
	{
		return angleToNorth;
	}
	inline void setAngleToNorth( float anAngle )
	{
		angleToNorth = anAngle;
	}

	// Sun object
	inline void setCartSunPos( vcg::Point3<float> coordinates )
	{
		sun->setCoordinates(coordinates);
	}
	inline float getSunIntensity() const
	{
		return sun->getIntensity();
	}
	//Returns the measured intensity for a flat surface
	inline float getSunFlatIntensity(){ return flatIntensity;}

	inline emittingPt* getEmittingPt()
	{
		return sun;
	}

    ptSrc ( );

    virtual ~ptSrc ( );

protected:
	//Intensity of the sun measured for a flat surface
	float flatIntensity;

	// position of the reference point
	vcg::Point3<float> ref;
	// The angle between the directing vector i of the base and the north.
	float angleToNorth;
	// object representing the sun
	emittingPt* sun;
};

#endif // PTSRC_H

