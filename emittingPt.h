/*************************************************************************
                           emittingPt  -  description
                             -------------------
	authors		     : CAO Alexandre, Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//---------- Class Interface of <emittingPt> (fichier emittingPt.h) ----------
#if ! defined ( EMITTINGPT_H )
#define EMITTINGPT_H

//-------------------------------------------------------- Used Interfaces

#include <vcg/space/point3.h> 

//-------------------------------------------------------------- Constants 

//------------------------------------------------------------------ Types 

//------------------------------------------------------------------------ 
// Aim of the class <emittingPt>
// This object is used to represent a point that emits a given intensity.
// (in our case, UV rays)
//
//------------------------------------------------------------------------ 

class emittingPt
{
public:
	// point coordinates
	inline virtual vcg::Point3<float> getP() const
	{
		return P;
	}
	inline virtual void setCoordinates( float x, float y, float z )
	{	
		P = vcg::Point3<float>(x,y,z);
	}
	inline virtual void setCoordinates( vcg::Point3<float> aP )
	{	
		P = aP;
	}

// intensity of the emitting point
	float virtual getIntensity() const
	{
		return intensity;
	}
	void virtual setIntensity( float anIntensity )
	{
		intensity = anIntensity;
	}

//--------------------------------------------------- Operator Overloading

//---------------------------------------------- Constructors - destructor
    emittingPt ( float aX, float aY, float aZ, float anIntensity = 0 );
    emittingPt (vcg::Point3<float> aP, float anIntensity = 0 );

    virtual ~emittingPt ( );

protected:
	//point coordinates
	vcg::Point3<float> P;
	// intensity of the emitting point
	float intensity;
};

#endif // EMITTINGPT_H

