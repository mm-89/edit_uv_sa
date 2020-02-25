/*************************************************************************
                           PlaneSurface  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//---------- Class <PlaneSurface> (fichier planeSurface.cpp) ----------

#include "planeSurface.h"

#include "OldAPISupport.h"
#include "Helpers.h"

//----------------------------------------------------------------- PUBLIC

//--------------------------------------------------------- Public Methods
void PlaneSurface::push_backEvaluatedIntensity(EvaluatedIntensity* evaluatedIntensity, float directIntensity, float diffusedIntensity, float reflectedIntensity)
{
	UVMesh::push_backEvaluatedIntensity(evaluatedIntensity);
	uvDifferences->push_back(getUVDiff(directIntensity, diffusedIntensity, reflectedIntensity));
}

void PlaneSurface::clearEvaluatedIntensities()
{
	Helpers::clearEvaluatedIntensities(&evalIntensityList);
	uvDifferences->clear();
}

void PlaneSurface::mergeIntensities(PlaneSurface *toMerge){
	//First merge the intensities
	evalIntensityList->insert(evalIntensityList->begin(), toMerge->getEvaluatedIntensityList()->begin(), toMerge->getEvaluatedIntensityList()->end());

	//Then merge the differences
	uvDifferences->insert(uvDifferences->begin(), toMerge->getUVDifferences()->begin(), toMerge->getUVDifferences()->end());


	//Clear the other list to make sure data is not accessible any more from there
	toMerge->getEvaluatedIntensityList()->clear();
	toMerge->getUVDifferences()->clear();
}

UVDiffStats PlaneSurface::getDifferencesStats(){
	UVDiffStats diffStats;
	
	//Get the means
	float diffusedMean = 0;
	float reflectedMean = 0;
	float directMean = 0;
	float diffusedMin = 1.0;
	float reflectedMin = 1.0;
	float directMin = 1.0;
	float diffusedMax = 0;
	float reflectedMax = 0;
	float directMax = 0;
	
	vector<UVDiff>::iterator uvDiff;
	for(uvDiff = uvDifferences->begin(); uvDiff != uvDifferences->end(); uvDiff++){
		diffusedMean += uvDiff->diffuseDiff;
		if(uvDiff->diffuseDiff > diffusedMax){
			diffusedMax = uvDiff->diffuseDiff;
		}
		if(uvDiff->diffuseDiff < diffusedMin){
			diffusedMin = uvDiff->diffuseDiff;
		}
		reflectedMean += uvDiff->reflectedDiff;
		if(uvDiff->reflectedDiff > reflectedMax){
			reflectedMax = uvDiff->reflectedDiff;
		}
		if(uvDiff->reflectedDiff < reflectedMin){
			reflectedMin = uvDiff->reflectedDiff;
		}
		directMean += uvDiff->directDiff;
		if(uvDiff->directDiff > directMax){
			directMax = uvDiff->directDiff;
		}
		if(uvDiff->directDiff < directMin){
			directMin = uvDiff->directDiff;
		}
		
	}
	diffStats.meanDiffused = diffusedMean/uvDifferences->size();
	diffStats.meanReflected = reflectedMean/uvDifferences->size();
	diffStats.meanDirect = directMean/uvDifferences->size();

	return diffStats;
	
}


Intensity PlaneSurface::getTotalIntensity(bool summary){
	Intensity total;
	total.diffused = 0.0;
	total.direct = 0.0;
	total.reflected = 0.0;
	auto ibegin = summary ? summaryEvalIntensityList->begin() : evalIntensityList->begin();
	auto iend = summary ? summaryEvalIntensityList->end() : evalIntensityList->end();
	for(auto intensities = ibegin; intensities != iend; intensities++){
		auto intensity = (*intensities)->intensityList->begin();
		total.diffused += intensity->diffused;
		total.direct += intensity->direct;
		intensity++;
		total.reflected += intensity->reflected;
	}
	return total;
}

bool PlaneSurface::exportIntensitiesCSV(string filename, IOZones::OutputStreamMap &outputStreams){
	
	string name = filename;
	vector<EvaluatedIntensity*>::iterator evalIntIter;
	EvaluatedIntensity evalIntensity;
	
	//If no file name is provided, generate a meaningful one
	if(filename.empty()){
		name = path+generateOutputFilename((evalIntensityList->front())->beginDate, (evalIntensityList->back())->endDate)+"_planeSurface_result.csv";
	}	

	// get file handle
	auto *ptr = &outputStreams[name];
	bool isNewFile = !ptr->data();
	if(isNewFile)
	{
		ptr->reset(new std::ofstream(filename));
		if(!ptr->data()->is_open())
		{
		//	MessageBox(NULL, L"Failed to open control points file.", L"Error", 0x10000);//MODIFY
			return false;
		}
	}
	std::ofstream &file = *ptr->data();

	if(isNewFile)
	{
		//Print the header
		file << "Start Time,End Time,Total Intensity Measured [J/m2],Diffuse Intensity Measured [J/m2],Direct Intensity Measured [J/m2],Reflected Intensity Measured [J/m2],Total Intensity Calculated [J/m2],Diffuse Intensity Calculated [J/m2],Direct Intensity Calculated [J/m2],Reflected Intensity Calculated [J/m2],Total Intensity Difference [%],Diffuse Intensity Difference [%],Direct Intensity Difference [%],Reflected Intensity Difference [%]\n";
	}
		
	//Loop through each of the intensities and  print them
	char buffer [300];
	float totalIntensity = 0.0;
	float diffuseIntensity = 0.0;
	float directIntensity = 0.0;
	float reflectedIntensity = 0.0;
	IntensityList::iterator intensity;
	vector<UVDiff>::iterator difference = uvDifferences->begin();

	for (evalIntIter=evalIntensityList->begin(); evalIntIter!=evalIntensityList->end(); evalIntIter++)
	{
		//Print the time stamp
		evalIntensity = **evalIntIter;
		//strftime (buffer,300,"%x %X",&(evalIntensity.beginDate->getDate()));//MODIFY
		string startDate(buffer);
		//strftime (buffer,300,"%x %X",&(evalIntensity.endDate->getDate()));//MODIFY
		string endDate(buffer);
		file << startDate << "," << endDate << ",";

		//Print the measured intensities
		diffuseIntensity = difference->measuredIntensity.diffused;
		directIntensity = difference->measuredIntensity.direct;
		reflectedIntensity = difference->measuredIntensity.reflected;
		totalIntensity = diffuseIntensity + directIntensity + reflectedIntensity;
		sprintf (buffer, "%E,%E,%E,%E,", totalIntensity, diffuseIntensity, directIntensity, reflectedIntensity);
		file << string(buffer);

		//Print the calculated intensities
		intensity = evalIntensity.intensityList->begin();
		diffuseIntensity = intensity->diffused;
		directIntensity = intensity->direct;
		intensity++;
		reflectedIntensity = intensity->reflected;
		totalIntensity = diffuseIntensity + directIntensity + reflectedIntensity;
		sprintf (buffer, "%E,%E,%E,%E,", totalIntensity, diffuseIntensity, directIntensity, reflectedIntensity);
		file << string(buffer);

		//Print the differences
		diffuseIntensity = difference->diffuseDiff;
		directIntensity = difference->directDiff;
		reflectedIntensity = difference->reflectedDiff;
		totalIntensity = diffuseIntensity + directIntensity + reflectedIntensity;
		sprintf (buffer, "%E,%E,%E,%E\n", totalIntensity*100, diffuseIntensity*100, directIntensity*100, reflectedIntensity*100);
		file << string(buffer);

		difference++;
	}
	return true;
}

//--------------------------------------------------- Operator Overloading


//---------------------------------------------- Constructors - destructor

PlaneSurface::PlaneSurface(vcg::Point3<float> aCenter, float aSurfaceHeight, MeshDocument *doc){
	

	//Create the vector to hold the intensity differences
	uvDifferences = new vector<UVDiff>;
	
	//Create the model
	model = OldApi::CreateMeshModel(doc);
		
	//Initialize the model and create 3 Vertices and 2 Faces
	model->cm.Clear();
	vcg::tri::Allocator<CMeshO>::AddVertices(model->cm,6);
	vcg::tri::Allocator<CMeshO>::AddFaces(model->cm,8);
		
	//Create the Vertices
	CMeshO::VertexPointer ivp[6];
	vector<CVertexO>::iterator vi=model->cm.vert.begin();

	//Vertex 1 - Upper vertex
	ivp[0]=&*vi;
	(*vi).P()= vcg::Point3<float>(aCenter[0], aCenter[1] + aSurfaceHeight/50.0, aCenter[2] );
	++vi;
	
	//Vertex 2 - lower vertex
	ivp[1]=&*vi;
	(*vi).P()= vcg::Point3<float>(aCenter[0], aCenter[1] - aSurfaceHeight/50.0, aCenter[2] );
	++vi;
	
	//The 4 remaining vertices are the 4 corners of the square flat surface
	//Vertex 3
	ivp[2]=&*vi;
	(*vi).P()= vcg::Point3<float>(aCenter[0] + 0.5*aSurfaceHeight, aCenter[1], aCenter[2]+0.5*aSurfaceHeight);
	++vi;
	
	//Vertex 4
	ivp[3]=&*vi;
	(*vi).P()= vcg::Point3<float>(aCenter[0] + 0.5*aSurfaceHeight, aCenter[1], aCenter[2] - 0.5*aSurfaceHeight);
	++vi;
	
	//Vertex 5
	ivp[4]=&*vi;
	(*vi).P()= vcg::Point3<float>(aCenter[0] - 0.5*aSurfaceHeight, aCenter[1] , aCenter[2]- 0.5*aSurfaceHeight);
	++vi;
	
	//Vertex 6
	ivp[5]=&*vi;
	(*vi).P()= vcg::Point3<float>(aCenter[0] - 0.5*aSurfaceHeight, aCenter[1] , aCenter[2]+ 0.5*aSurfaceHeight);

	
	//Create the surface faces
	//The faces form 2 pyramids with square basis.
	//the pyramids share the same base and their summit point 
	//in opposite direction following the vertical axis
	vector<CFaceO>::iterator fi=model->cm.face.begin();
	
	//Face 1
	(*fi).V(0)=ivp[0];  
	(*fi).V(1)=ivp[2]; 
	(*fi).V(2)=ivp[3]; 
	++fi;	
	
	//Face 2
	(*fi).V(0)=ivp[0];  
	(*fi).V(1)=ivp[3]; 
	(*fi).V(2)=ivp[4]; 
	++fi;
	
	//Face 3
	(*fi).V(0)=ivp[0];  
	(*fi).V(1)=ivp[4]; 
	(*fi).V(2)=ivp[5]; 
	++fi;
	
	//Face 4
	(*fi).V(0)=ivp[0];  
	(*fi).V(1)=ivp[5]; 
	(*fi).V(2)=ivp[2]; 
	++fi;
	
	//Face 5
	(*fi).V(0)=ivp[1];  
	(*fi).V(1)=ivp[2]; 
	(*fi).V(2)=ivp[5]; 
	++fi;
	
	//Face 6
	(*fi).V(0)=ivp[1];  
	(*fi).V(1)=ivp[5]; 
	(*fi).V(2)=ivp[4]; 
	++fi;
	
	//Face 7
	(*fi).V(0)=ivp[1];  
	(*fi).V(1)=ivp[4]; 
	(*fi).V(2)=ivp[3]; 
	++fi;
	
	//Face 8
	(*fi).V(0)=ivp[1];  
	(*fi).V(1)=ivp[3]; 
	(*fi).V(2)=ivp[2];
	
	//Compute the Mesh bounding box
	vcg::tri::UpdateBounding<CMeshO>::Box(model->cm);

	//Create the bounding boxes
	createBoundingBoxes(1,1,1);

	//Set the number of vertices for the intensity maps
	diffusedIntensityMap.setNumModelVertices(model->cm.vert.size());
	reflectedIntensityMap.setNumModelVertices(model->cm.vert.size());

}

//Destructor
PlaneSurface::~PlaneSurface ( )
{
	delete uvDifferences;
}


//---------------------------------------------------------------- PRIVATE


//------------------------------------------------------ Protected Methods

UVDiff PlaneSurface::getUVDiff(float directIntensity, float diffusedIntensity, float reflectedIntensity){

	//To store the differences
	UVDiff uvDifference;
	
	//Get the last evaluatedIntensities
	EvaluatedIntensity* lastEvalIntensity = evalIntensityList->back();
	
	//Check the 2 first vertices of the evaluated intensities since they correspond
	//to our points of interest:
	//The first is a point isolated at the top of the plane surface
	//The second poitn is isolated at the bottom of the plane surface
	IntensityList::iterator intensityIter = lastEvalIntensity->intensityList->begin();
	
	if(diffusedIntensity == (*intensityIter).diffused){
		uvDifference.diffuseDiff = 0.0;
	}
	else{
		uvDifference.diffuseDiff = abs(1-((*intensityIter).diffused/(diffusedIntensity)));
	}
	
	if(directIntensity == (*intensityIter).direct){
		uvDifference.directDiff = 0.0;
	}
	else{
		uvDifference.directDiff = abs(1-((*intensityIter).direct/directIntensity));
	}
	intensityIter++;
	
	if(reflectedIntensity == (*intensityIter).reflected){
		uvDifference.reflectedDiff = 0.0;
	}
	else{
		uvDifference.reflectedDiff = abs(1-((*intensityIter).reflected/reflectedIntensity));	
	}
	
	Intensity measuredIntensity;
	measuredIntensity.direct = directIntensity;
	measuredIntensity.diffused = diffusedIntensity;
	measuredIntensity.reflected = reflectedIntensity;
	uvDifference.measuredIntensity = measuredIntensity;	
	
	return uvDifference;
	
}


bool PlaneSurface::compareDiffuseUVDifferences(const UVDiff &uvdiff1, const UVDiff &uvdiff2){
	return uvdiff1.diffuseDiff < uvdiff2.diffuseDiff;
}
bool PlaneSurface::compareReflectedUVDifferences(const UVDiff &uvdiff1, const UVDiff &uvdiff2){
	return uvdiff1.reflectedDiff < uvdiff2.reflectedDiff;
}

bool PlaneSurface::compareDirectUVDifferences(const UVDiff &uvdiff1, const UVDiff &uvdiff2){
	return uvdiff1.directDiff < uvdiff2.directDiff;
}
