/*************************************************************************
                           emittingPt  -  description
                             -------------------
	authors		     : CAO Alexandre, Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//----- Class Implementation of <emittingPt> (fichier emittingPt.cpp) ----

//---------------------------------------------------------------- INCLUDE

//--------------------------------------------------------- System Include

//------------------------------------------------------------ Own Include
#include "emittingPt.h"

//-------------------------------------------------------------- Constants

//----------------------------------------------------------------- PUBLIC

//--------------------------------------------------------- Public Methods

//--------------------------------------------------- Operator Overloading

//---------------------------------------------- Constructors - destructor
emittingPt::emittingPt ( float aX, float aY, float aZ, float anIntensity )
	: intensity(anIntensity)
{
	P = vcg::Point3<float>(aX,aY,aZ);
} //----- End of emittingPt

emittingPt::emittingPt(vcg::Point3<float> aP, float anIntensity)
	: P(aP), intensity(anIntensity)
{
}


emittingPt::~emittingPt ( )
{
} //----- End of ~emittingPt


//---------------------------------------------------------------- PRIVATE

//------------------------------------------------------ Protected Methods

