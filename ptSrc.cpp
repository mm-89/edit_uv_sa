/*************************************************************************
                           ptSrc  -  description
                             -------------------
	authors		     : CAO Alexandre, Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//------Class Implementation of <ptSrc> (fichier ptSrc.cpp) ------

//---------------------------------------------------------------- INCLUDE

//--------------------------------------------------------- System Include

//------------------------------------------------------------ Own Include
#include "ptSrc.h"

#include "OldAPISupport.h"
//-------------------------------------------------------------- Constants

//----------------------------------------------------------------- PUBLIC

//--------------------------------------------------------- Public Methods
void ptSrc::setSunPosIntensity( float radius, float azimut, float zenith, float intensity )
{	
	
	//Transform the spherical coordinates into cartesian coordinates
	//And set the coordinates with the source Reference point as origin
	vcg::Point3<float> res;
	float correctedAzimuth = 360-azimut+angleToNorth;
	 correctedAzimuth = angleToNorth-azimut;
	float cosineCorrection = cos(zenith*M_PI/180);
	res[0] = radius*sin(zenith*M_PI/180)*sin(correctedAzimuth*M_PI/180);
	res[1] = radius*cosineCorrection;
	res[2] = radius*sin(zenith*M_PI/180)*cos(correctedAzimuth*M_PI/180);
	this->setCartSunPos(res);
	
	//Set the measured intensity as the sun source intensity
	//And set the corrected intensity as the flat surface intensity
	sun->setIntensity(intensity);
	flatIntensity = intensity * cosineCorrection;
	
} //------ End of setSunPosIntensity


void ptSrc::savePtSrcAsMesh(IOSpreadSheet *spreadsheet, string filename, MeshDocument *doc, const char* datafilePath, Date beginDate, Date endDate, float radius){

	//Read the spreadsheet data file and save all the necessary information as point sources
	//Last check that the dates are correct
	if (!beginDate.isValid() || !endDate.isValid() || beginDate>endDate)
	{
		//MessageBox(NULL, L"Please check the errors and re-launch the operation.", L"Notice", 0x10000);//MODIFY
		return;
	}
	
	//Open the Excel data file
	if (spreadsheet->init(datafilePath)!=0 || spreadsheet->open(datafilePath)!=0){
		//MessageBox(NULL, L"Error. Failed to open Excel File.", L"Notice", 0x10000);//MODIFY
		return;
	}

	//Find the indices of the first and last rows
	//corresponding to the input dates and times
	int firstRow = spreadsheet->searchDateRow(beginDate);
	int lastRow = spreadsheet->searchDateRow(endDate);
	
	//Read the data from the file and save each of the
	//points as an emittingPoint
	float zenith;
	float azimut;
	float directIntensity;		
	float maxIntensity = 0;
	vector<emittingPt> *emittingPoints = new vector<emittingPt>;
	for(int i=firstRow; i<=lastRow; i++)
	{
		strConv::from_string(spreadsheet->read(i, IOSpreadSheet::ZENITH_COL), zenith);
		strConv::from_string(spreadsheet->read(i, IOSpreadSheet::AZIMUT_COL), azimut);
		strConv::from_string(spreadsheet->read(i, IOSpreadSheet::DIRECTUV_COL), directIntensity);
		directIntensity = max(directIntensity,0.0f);
		//Save the max intensity for colouring
		if(directIntensity > maxIntensity){
			maxIntensity = directIntensity;
		}
		setSunPosIntensity(radius, azimut, zenith, directIntensity*60);
		emittingPoints->push_back(emittingPt(sun->getP(),sun->getIntensity()));
	}

	//Create the model
	MeshModel *model = OldApi::CreateMeshModel(doc);
	model->cm.Clear();
		
	//Create the Vertices
	vcg::tri::Allocator<CMeshO>::AddVertices(model->cm,emittingPoints->size()+1);
	CMeshO::VertexPointer *ivp = new CMeshO::VertexPointer[emittingPoints->size()+1];
	vector<CVertexO>::iterator vi=model->cm.vert.begin();

	//Add Reference Point
	ivp[0]=&*vi;
	(*vi).P()= ref;
	OldApi::SetRGB((*vi).C(),255,255,255);
	
	//Add all other vertices (one vertex per trajectory point)
	int emitPtCount = 1;
	vector<emittingPt>::iterator emitPts; 
	for(emitPts = emittingPoints->begin(); emitPts != emittingPoints->end(); emitPts++){
		++vi;
		ivp[emitPtCount]=&*vi;
		(*vi).P()= emitPts->getP()+ref;
		OldApi::SetRGB((*vi).C(), emitPts->getIntensity()/maxIntensity*255,0,0);
		++emitPtCount;
	}
	
	//Save the mesh now
	IOFile::saveToFile(filename.c_str(), model, 0x0005);
	
	//delete the mesh and the trajectory points
	delete model;	
	delete emittingPoints;

}

//--------------------------------------------------- Operator Overloading

//---------------------------------------------- Constructors - destructor
ptSrc::ptSrc ( )
{	
	//Set default dummy values to the class members
	flatIntensity =  0.0;
	ref = vcg::Point3<float>(0,0,0);
	sun = new emittingPt(0, 0, 0);
	
	//Set the default angle to North to 180 deg
	//This is quite natural since then when loading the
	//model, it is oriented with North pointing "up"
	angleToNorth = 180;

} //----- End of ptSrc


ptSrc::~ptSrc ( )
{
	delete sun;
} //----- End of ~ptSrc


//---------------------------------------------------------------- PRIVATE

//------------------------------------------------------ Protected Methods

