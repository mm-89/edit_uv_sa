/*************************************************************************
                           POI  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/


#include "POI.h"

//----------------------------------------------------------------- PUBLIC

//--------------------------------------------------------- Public Methods

void POI::updateMeshSensor(vcg::tri::Allocator<CMeshO>::PointerUpdater<CMeshO::VertexPointer> pu){
	if(meshSensor){
		pu.Update(meshSensor);
	}
}

//--------------------------------------------------- Operator Overloading

//---------------------------------------------- Constructors - destructor
//Create a new date with the current local Date/Time
POI::POI(vcg::Point3<float> coordinates, std::string name):point(coordinates),pointName(name){ 
	receivedUV = 0;
	meshSensor = 0;
}

POI::~POI(){
	if (receivedUV)	delete receivedUV;
}


//---------------------------------------------------------------- PRIVATE

//------------------------------------------------------ Protected Methods
