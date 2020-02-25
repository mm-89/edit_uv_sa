/*************************************************************************
                           POI  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

#if ! defined ( POI_H )
#define POI_H

//-------------------------------------------------------- Used Interfaces
#include <vcg/space/point3.h> 
#include "intensities.h"
#include <common/meshmodel.h>
#include <string>
#include <vcg/complex/allocate.h>

//------------------------------------------------------------------------ 
//Represents Points Of Interest in the Mesh. These points are used to get
//precise readings from specific points on the Mesh.
class POI
{
public:
	inline vcg::Point3<float> getPoint(){return point;}
	inline EvaluatedIntensity* getEvaluatedIntensity(){return receivedUV;}
	inline void setEvaluatedIntensity(EvaluatedIntensity *intensity){receivedUV = intensity;}
	inline std::string getPointName(){return pointName;}

	inline void setMeshSensor(CVertexO *sensor){meshSensor = sensor;}
	inline CVertexO* getMeshSensor(){return meshSensor;}

	//This method is used to update the pointers on the sensors if the Mesh Topology is changed
	void updateMeshSensor(vcg::tri::Allocator<CMeshO>::PointerUpdater<CMeshO::VertexPointer> pu);



	//--------------------------------------------------- Operator Overloading


	//---------------------------------------------- Constructors - destructor

	POI(vcg::Point3<float> coordinates, std::string name);

	~POI();

private:
	//Pointers to the sensors on the Mesh
	CVertexO *meshSensor;
	//POI Coordinates
	vcg::Point3<float> point;
	//POI Name
	std::string pointName;
	//POI intensities received (incl. its neighbors)
	EvaluatedIntensity *receivedUV;
};

#endif // POI_H




