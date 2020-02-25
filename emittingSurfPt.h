/*************************************************************************
                           emittingSurfPt  -  description
                             -------------------
	authors		     : CAO Alexandre, Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//---------- Class Interface of <emittingSurfPt> (fichier emittingSurfPt.h) ----------
#if ! defined ( EMITTINGSURFPT_H )
#define EMITTINGSURFPT_H

//-------------------------------------------------------- Used Interfaces
#include "emittingPt.h"

//-------------------------------------------------------------- Constants 

//------------------------------------------------------------------ Types 

//------------------------------------------------------------------------ 
// Aim of the class <emittingSurfPt>
// This class is used to modelize a surface through a point. It has in
// addition of the base attributes, the attribut ratio which correspond to
// the ratio of surface the point represents.
//
//------------------------------------------------------------------------ 

class emittingSurfPt : public emittingPt
{
//----------------------------------------------------------------- PUBLIC

public:
//--------------------------------------------------------- Public Methods

//-------------------------------------------------- Public Inline Methods
// ratio of surface represented
	float virtual getRatio()
	{
		return ratio;
	}
	void virtual setRatio( float aRatio )
	{
		ratio = aRatio;
	}

    emittingSurfPt ( vcg::Point3<float> aP, float aRatio, float anIntensity = 0 );

    virtual ~emittingSurfPt ( );

protected:
	// ratio of surface represented
	float ratio;
};

#endif // EMITTINGSURFPT_H

