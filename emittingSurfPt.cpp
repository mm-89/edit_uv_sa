/*************************************************************************
                           emittingSurfPt  -  description
                             -------------------
	authors		     : CAO Alexandre, Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//----- Class Implementation of <emittingSurfPt> (fichier emittingSurfPt.cpp) ----

//---------------------------------------------------------------- INCLUDE

//--------------------------------------------------------- System Include

//------------------------------------------------------------ Own Include
#include "emittingSurfPt.h"

//-------------------------------------------------------------- Constants

//----------------------------------------------------------------- PUBLIC

//--------------------------------------------------------- Public Methods

//--------------------------------------------------- Operator Overloading

//---------------------------------------------- Constructors - destructor
emittingSurfPt::emittingSurfPt ( vcg::Point3<float> aP, float aRatio, float anIntensity ) 
	: emittingPt(aP, anIntensity), ratio(aRatio)
{
#ifdef MAP
    cout << "Call to the constructor of <emittingSurfPt>" << endl;
#endif
	
} //----- End of emittingSurfPt


emittingSurfPt::~emittingSurfPt ( )
{
#ifdef MAP
    cout << "Call to the destructor of <emittingSurfPt>" << endl;
#endif
} //----- End of ~emittingSurfPt


//---------------------------------------------------------------- PRIVATE

//------------------------------------------------------ Protected Methods

