/*************************************************************************
                           hemisphericalSrc  -  description
                             -------------------
	authors		     : Laurent Francioli, CAO Alexandre
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//Class Implementation of <hemisphericalSrc> (fichier hemisphericalSrc.cpp)

//---------------------------------------------------------------- INCLUDE

//--------------------------------------------------------- System Include

//------------------------------------------------------------ Own Include
#include "hemisphericalSrc.h"

#include "OldAPISupport.h"
//-------------------------------------------------------------- Constants

//----------------------------------------------------------------- PUBLIC

//--------------------------------------------------------- Public Methods

void hemisphericalSrc::loadEmittingPts()
{
	// surface of the whole hemisphere
	float totalSurf = 2*M_PI*pow(radius,2);
	// height of this surface from the origin
	float height = radius; // - ( summitSurf/(2*M_PI*radius) );
	//Here, step is a little misleading since it actually refers to a 1/2level height
	float step = height / (2*lvlNb);
	float angularStep = 2*M_PI /ptNb;
	float lvlHeight = step; // height-step;
	float total_surf = 0;
	float r2,h1,h2,d1,d2,surface,ratio,circleRadius, alpha;
	float additional_ratio = 0;
	for (int i = 1; i <= lvlNb; i++)
	{
		//Get the surface represented by this level and its ratio of the total hemisphere
		r2 = radius*radius;
		h1 = (lvlHeight-step)*(lvlHeight-step);
		h2 = min((lvlHeight+step)*(lvlHeight+step), r2);
		d1 = sqrt(r2 - h1);
		d2 = sqrt(r2-h2);
		
		surface = 2*M_PI*sqrt(step*step*4+(d1-d2)*(d1-d2))*(d2+((d1-d2)/2));
		total_surf += surface;
		
		
		//Moderate the ratio depending on the level height
		//If on the lower levels, attenuate the intensity
		alpha = asin(lvlHeight/radius)/M_PI*180;
		if(attenuation_angle > 0 && alpha < attenuation_angle){
			additional_ratio += ratio * (1-alpha/attenuation_angle);
			ratio = ratio * alpha/attenuation_angle;
			
		}
		
		//Divide the ratio over the points in the level
		ratio = (surface / totalSurf) / ptNb;
		
		//Calculate the radius of the level and the angular step to place each of the 
		//points of the level
		circleRadius = sqrt(pow(radius,2) - pow(lvlHeight,2));

		//Create all the points for this level
		for (int j = 0; j < ptNb; j++)
		{
			//Create the cartesian coordinates in the hemisphere referential
			//(center of the hemisphere as origin)
			vcg::Point3<float> coords;
			coords[0] = circleRadius*sin(angularStep*j);
			coords[1] = lvlHeight;
			coords[2] = circleRadius*cos(angularStep*j);

			//Change the origin to the global origin and add the emitting point
			emitPtList->push_back(emittingSurfPt(coords+center, ratio));
			
			
		}

		lvlHeight += 2*step;
	}
	
	//Place the last point => point at the summit of the hemisphere
	ratio = (totalSurf-total_surf) / totalSurf;
	emitPtList->push_back(emittingSurfPt(vcg::Point3<float>(center[0],center[1]+radius,center[2]),ratio));
}

void hemisphericalSrc::setPtIntensity(float anIntensity)
{
	//set the total intensity
	totalIntensity = anIntensity;

	vector<emittingSurfPt>::iterator ptIter;
	for(ptIter=emitPtList->begin(); ptIter!=emitPtList->end(); ptIter++)
	{
		ptIter->setIntensity(2*anIntensity);
	}
}

void hemisphericalSrc::saveHemiSourceAsMesh(string filename, MeshDocument *doc){

	//Create the model
	MeshModel *model = OldApi::CreateMeshModel(doc);
		
	//Initialize the model and create 3 Vertices and 2 Faces
	model->cm.Clear();	

	vcg::tri::Allocator<CMeshO>::AddVertices(model->cm,emitPtList->size()+1);
	//vcg::tri::Allocator<CMeshO>::AddFaces(model->cm,8);
		
	//Create the Vertices
	CMeshO::VertexPointer *ivp = new CMeshO::VertexPointer[emitPtList->size()+1];
	vector<CVertexO>::iterator vi=model->cm.vert.begin();

	//Add Center of the hemisphere
	ivp[0]=&*vi;
	(*vi).P()= center;
	
	//Add all the point sources
	int emitPtCount = 1;
	vector<emittingSurfPt>::iterator emitPts; 
	for(emitPts = emitPtList->begin(); emitPts != emitPtList->end(); emitPts++){
		++vi;
		ivp[emitPtCount]=&*vi;
		(*vi).P()= emitPts->getP();
		++emitPtCount;
	}
	
	//Save the mesh
	IOFile::saveToFile(filename.c_str(), model, 0x0005);
	
	//delete the mesh
	delete model;

}


//--------------------------------------------------- Operator Overloading

//---------------------------------------------- Constructors - destructor
hemisphericalSrc::hemisphericalSrc ( )
{

	//Set default dummy values for members
	center = vcg::Point3<float>(0,0,0);
	radius = 1;
	radiusFactor = 1;
	
	//Set default values for the number of levels
	//and poitns per level
	ptNb = 20;
	lvlNb = 20;

	//By default, hemispherical source is isotropic
	attenuation_angle = 0;

	//Create empty vector for the emittingSurfPts
	emitPtList = new vector<emittingSurfPt>;

} //----- End of hemisphericalSrc


hemisphericalSrc::~hemisphericalSrc ( )
{
	// Deletion of emitPtList
	vector<emittingSurfPt>::iterator iter;
	for (iter = emitPtList->begin(); iter != emitPtList->end(); )
	{
		iter = emitPtList->erase(iter);
	}
	delete emitPtList;

} //----- End of ~hemisphericalSrc


//---------------------------------------------------------------- PRIVATE

//------------------------------------------------------ Protected Methods

