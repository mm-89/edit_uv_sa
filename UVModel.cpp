/*************************************************************************
                           UVMesh  -  description
                             -------------------
	authors		     : Laurent Francioli, CAO Alexandre
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//-------Class Implementation of <UVModel> (fichier UVModel.cpp) -------

//---------------------------------------------------------------- INCLUDE

//--------------------------------------------------------- System Include

//------------------------------------------------------------ Own Include
#include "UVModel.h"
#include "IOFile.h"
#include "strConv.h"


//-------------------------------------------------------------- Constants

//----------------------------------------------------------------- PUBLIC

//--------------------------------------------------------- Public Methods

void UVModel::initModel()
{
	//Get the posture bounding box
	vcg::Box3f boundingBox = posture->getMeshBoundingBox();	
	
	//Calculation of the center of the model on the "ground"
	vcg::Point3<float> center;
	center[0] = (boundingBox.min[0]+boundingBox.max[0])/2;
	center[1] = boundingBox.min[1];
	center[2] = (boundingBox.min[2]+boundingBox.max[2])/2;
	
	//Set the radius to twice the size of the longest side of the posture bounding box
	float modelSize = max( boundingBox.max[0]-boundingBox.min[0], max(boundingBox.max[1]-boundingBox.min[1], boundingBox.max[2]-boundingBox.min[2]) );

	//Set the Reference point for the directionnal direct light source (sun)
	directSrc->setRefPt(center);
	diffuseSrc->calculateRadius(modelSize);
	reflectedSrc->calculateRadius(-modelSize);	
		
	//Load the emitting points based on the set number of levels and points per level
	//These can be set using the accessors
	diffuseSrc->loadEmittingPts();	
	reflectedSrc->loadEmittingPts();
	
	//Create the bounding boxes for the posture
	posture->createBoundingBoxes(Xsubdiv, Ysubdiv, Zsubdiv);

	//Load the Plane Surface used for checking
	if(usePlaneSurface){
            planeSurface = new PlaneSurface(diffuseSrc->getCenter(), diffuseSrc->getRadius()/3.0, meshDocument);
	}
	
}

void UVModel::setPOIs(const char *filename){
	posture->setPOIs(filename);
}

void UVModel::evaluatePOI(float maxDistance, bool useBaseVertices){
	posture->evaluatePOI(maxDistance, useBaseVertices);
}

void UVModel::setZones(const char *filename){
	posture->setZones(filename);
}

void UVModel::evaluateZones(){
	posture->evaluateZones();
}


void UVModel::setProtections(bool reset){
	posture->setProtections(reset);
}


void UVModel::resetProtections(){
	posture->resetProtections();
}

void UVModel::mergeResults(UVModel *modelToMerge){
	//First merge the zone results.
	posture->mergeZonesIntensities(modelToMerge->getUVMesh()->getZones());

	//Merge the flat surface intensities too
	//TODO: Ev. make deep copies of the flat surface in order
	//to preserve the integrity of the merged Mesh
	planeSurface->mergeIntensities(modelToMerge->getPlaneSurface());
}

void UVModel::evaluateUVBetweenAvg(const Date &beginDate, const Date &endDate, float startAngle, float endAngle, float angleStep, int sources){

	//Get the row information
	pair<int,int> rows = getRowsFromDates(beginDate,endDate);
	if(rows.first <0 || rows.second < 0){
		return;
	}

	//Create evaulatedIntensity for the posture
	EvaluatedIntensity* evalTemp = new EvaluatedIntensity;
	evalTemp->beginDate = new Date;
	*(evalTemp->beginDate) = beginDate;
	evalTemp->endDate = new Date;
	*(evalTemp->endDate) = endDate;
	evalTemp->intensityList = new vector<Intensity>;

	//Make sure all angles are minimal
	startAngle = fmod(startAngle, 360.0f);
	endAngle = fmod(endAngle,360.0f);
	angleStep = fmod(angleStep,360.0f);

	//Make sure the step is between 1 deg and 359 deg
	//In case the step is a full turn, use the fixed orientation
	//instead
	if(!angleStep){
		evaluateUVBetweenFixed(beginDate,endDate,startAngle,sources);
		return;
	}

	//If the endAngle == startAngle,
	//user probably wants 1 full turn
	if(endAngle == startAngle){
		endAngle -= angleStep;
	}

	//Get the total rotation angle
	float totalAngle;
	if(angleStep > 0){
		 totalAngle = endAngle-startAngle;
	}
	else{
		totalAngle = startAngle-endAngle;
	}
	if(totalAngle < 0){
		totalAngle = 360 + totalAngle;
	}

	//Evaluate the UVs for each of the angles
	float numSteps = totalAngle / abs(angleStep);
	float currentAngle = startAngle;
	for(int i=0; i<=numSteps; ++i){
		evaluateUVBetween(rows.first, rows.second, currentAngle, evalTemp, sources);
		currentAngle=fmod(currentAngle+angleStep,360.0f);
	}

	//Pushback results
	posture->push_backEvaluatedIntensity(evalTemp);

	//If at least one step was done,
	//Divide all the intensities by the numSteps to get the
	//average value
	if(numSteps){
		posture->multiplyIntensities(1.0f/numSteps);
		planeSurface->multiplyIntensities(1.0f/numSteps);
	}
	posture->evaluateTotalEvaluatedIntensity(false);
}

void UVModel::evaluateUVBetweenSeq(const Date &beginDate, const Date &endDate, float startAngle, float angleStep, int timeStep, int sources){

	//Make sure the timeStep is positive
	if(!timeStep>0){
		return;
	}

	//Make sure that the timestep is at least as big as the values steps
	int rowStep = timeStep/spreadsheet->getTimeStep();
	if(rowStep < 1){
		return;
	}

	//Create evaulatedIntensity for the posture
	EvaluatedIntensity* evalTemp = new EvaluatedIntensity;
	evalTemp->beginDate = new Date;
	*(evalTemp->beginDate) = beginDate;
	evalTemp->endDate = new Date;
	*(evalTemp->endDate) = endDate;
	evalTemp->intensityList = new vector<Intensity>;

	//Get the row information
	pair<int,int> rows = getRowsFromDates(beginDate,endDate);
	if(rows.first <0 || rows.second < 0){
		return;
	}

	//Evaluate UVs while turning the mannequin around
	Date currentStartDate = beginDate;
	Date currentEndDate = beginDate+timeStep;
	float currentAngle = startAngle;
	int currentEndRow;
	for(int currentStartRow = rows.first; currentStartRow <= rows.second; currentStartRow += rowStep){
		currentEndRow = currentStartRow+rowStep-1;
		if(currentEndRow > rows.second){
			currentEndRow = rows.second;
		}
		evaluateUVBetween(currentStartRow, currentEndRow, currentAngle, evalTemp,sources);
		currentAngle=fmod(currentAngle+angleStep,360.0f);
	}

	//Pushback results
	posture->push_backEvaluatedIntensity(evalTemp);	
	posture->evaluateTotalEvaluatedIntensity(false);
}

void UVModel::clearEvaluatedIntensities()
{
	posture->clearEvaluatedIntensities();
	if(planeSurface)
		planeSurface->clearEvaluatedIntensities();
}

void UVModel::evaluateUVBetweenFixed(const Date &beginDate, const Date &endDate, float angle, int sources){
	//Create evaulatedIntensity for the posture
	EvaluatedIntensity* evalTemp = new EvaluatedIntensity;
	evalTemp->beginDate = new Date;
	*(evalTemp->beginDate) = beginDate;
	evalTemp->endDate = new Date;
	*(evalTemp->endDate) = endDate;
	evalTemp->intensityList = new vector<Intensity>;

	//Get the row information
	pair<int,int> rows = getRowsFromDates(beginDate,endDate);
	if(rows.first <0 || rows.second < 0){
		return;
	}

	//Evaluate UV
	evaluateUVBetween(rows.first, rows.second, angle, evalTemp, sources);

	//Pushback results
	posture->push_backEvaluatedIntensity(evalTemp);
	posture->evaluateTotalEvaluatedIntensity(false);
}

pair<int,int> UVModel::getRowsFromDates(const Date &beginDate, const Date &endDate){
	//Last check that the dates are correct
	if (!beginDate.isValid() || !endDate.isValid() || beginDate>endDate)
	{
//		MessageBox(NULL, L"Dates are invalid. Make sure the end date is after the begining date and that both date formats are correct.", L"Notice", 0x10000);//MODIFY
		return make_pair(-1,-1);
	}
	
	//Find the indices of the first and last rows
	//corresponding to the input dates and times
	int firstRow = spreadsheet->searchDateRow(beginDate);
	int lastRow = spreadsheet->searchDateRow(endDate);

	//Check that the data is valid
	if ( (firstRow==-1) || (lastRow==-1) )
	{
		//MessageBox(NULL, L"Error. Row index error. The date may not exist in the data file.", L"Notice", 0x10000);//MODIFY
		return make_pair(-1,-1);
	}
	else if (firstRow > lastRow)
	{
		//MessageBox(NULL, L"Error. The data is not sorted in chronological order", L"Notice", 0x10000);//MODIFY
		return make_pair(-1,-1);
	}
	return make_pair(firstRow,lastRow);
}

void UVModel::evaluateUVBetween(int firstRow, int lastRow, float angle, EvaluatedIntensity* evalIntensity, int sources)
{
	//Set the orientation
	directSrc->setAngleToNorth(angle);

	Date currentTimeStamp = spreadsheet->readTimeStamp(firstRow);
	for(int i=firstRow; i<=lastRow; i++)
	{
		//Set the sources
		setSourcesData(i);
		//Evaluate UVs for the posture
		evaluateReceivedUV(*posture, *evalIntensity, sources);
		
		//If the evaluation of the plane surface was reqiured, perform it
		if(planeSurface){
		//Create evaluatedIntensity for the checking plane surface			
			EvaluatedIntensity* evalPS = new EvaluatedIntensity;
			evalPS->beginDate = new Date(currentTimeStamp);
			currentTimeStamp = spreadsheet->readTimeStamp(i);
			evalPS->endDate = new Date(currentTimeStamp);
			evalPS->intensityList = new vector<Intensity>;
			evaluateReceivedUV(*planeSurface,*evalPS);
			planeSurface->push_backEvaluatedIntensity(evalPS,directSrc->getSunFlatIntensity(), diffuseSrc->getTotalIntensity(), reflectedSrc->getTotalIntensity());
		}				
	}
}

void UVModel::setSourcesData(int rowNb)
{
	// First we read the corresponding data and set the remaining parameters
	// for the three sources.
	float radius = diffuseSrc->getRadius();
	float zenith;
	float azimut;
	float globalIntensity;
	float diffuseIntensity;
	float directIntensity;
	float reflectedIntensity;
	
	//Read the values from the Excel file
	strConv::from_string(spreadsheet->read(rowNb, IOSpreadSheet::ZENITH_COL), zenith);
	strConv::from_string(spreadsheet->read(rowNb, IOSpreadSheet::AZIMUT_COL), azimut);
	strConv::from_string(spreadsheet->read(rowNb, IOSpreadSheet::GLOBALUV_COL), globalIntensity);
	strConv::from_string(spreadsheet->read(rowNb, IOSpreadSheet::DIFFUSEUV_COL), diffuseIntensity);
	strConv::from_string(spreadsheet->read(rowNb, IOSpreadSheet::DIRECTUV_COL), directIntensity);
	strConv::from_string(spreadsheet->read(rowNb, IOSpreadSheet::REFLECTEDUV_COL), reflectedIntensity);

	// Since negative intensities are impossible, we set negative values at 0.
	directIntensity = std::max(directIntensity,0.0f);
	diffuseIntensity = std::max(diffuseIntensity,0.0f);
	reflectedIntensity = std::max(reflectedIntensity,0.0f);

	//Record the intensities in J/M2. Note that they have to be in W/M2 over a constant period of time
	directSrc->setSunPosIntensity(radius, azimut, zenith, directIntensity*spreadsheet->getTimeStep());
	diffuseSrc->setPtIntensity(diffuseIntensity*spreadsheet->getTimeStep());
	reflectedSrc->setPtIntensity(reflectedIntensity*spreadsheet->getTimeStep());

}


void UVModel::evaluateReceivedUV(UVMesh &uvmesh, EvaluatedIntensity &evalIntensity, int sources)
{
	bool isNewList = evalIntensity.intensityList->empty();
//	int progressValue = 0;
//	int progressFactor = 1;

	//If the intensity maps have been loaded and are complete,
	//get an iterator on them
	vector<float>::iterator DIMIter;
	vector<float>::iterator RIMIter;

//	if(progress){
//		progress->setLabelText("Simulation of " + QString::fromStdString(string(uvmesh.getModelName())));
//		progressValue = progress->value();
//	}

	if(sources & IntensitySources::DIFFUSED){
		if (uvmesh.getDiffusedIntensityMap().getMapComplete())
		{
			DIMIter = uvmesh.getDiffusedIntensityMap().getIntensities()->begin();
		}
		else{
			uvmesh.getDiffusedIntensityMap().clear();
//			progressFactor += getDiffuseLvlNb()*getDiffusePtNb();
//			if(progress){
//				progress->setLabelText("Calculation of visibility maps for " + QString::fromStdString(string(uvmesh.getModelName())));
//			}
		}
	}
	if(sources & IntensitySources::REFLECTED){
		if (uvmesh.getReflectedIntensityMap().getMapComplete())
		{
			RIMIter = uvmesh.getReflectedIntensityMap().getIntensities()->begin();
		}
		else{
			uvmesh.getReflectedIntensityMap().clear();
//			progressFactor += getReflectedLvlNb()*getReflectedPtNb();
//			if(progress){
//				progress->setLabelText("Calculation of visibility maps for " + QString::fromStdString(string(uvmesh.getModelName())));
//			}
		}
	}

	//Iterate over all the vertices and calculate the intensity each of them receives
	IntensityList::iterator intensityIter = evalIntensity.intensityList->begin();
	vector<CVertexO> *modelVertices = uvmesh.getModelVertices();
	vector<CVertexO>::iterator vertIter;
	Intensity intensities;
	if(isNewList)
		evalIntensity.intensityList->reserve(modelVertices->size());
	for (vertIter=modelVertices->begin(); vertIter!=modelVertices->end(); vertIter++)
	{
		intensities = UVModel::calculateIntensityForVertex(uvmesh, *vertIter, DIMIter, RIMIter, sources);

		// the intensity values are put in the corresponding vector
		if ( isNewList )
		{
			evalIntensity.intensityList->push_back(intensities);
		}
		else
		{
			intensityIter->direct += intensities.direct;
			intensityIter->diffused += intensities.diffused;
			intensityIter->reflected += intensities.reflected;
			intensityIter++;
		}
		
//		if(progress){
//			if(progress->wasCanceled()){
//				QMessageBox cancelCheck(QMessageBox::Warning, "Cancel Simulation?", "Are you sure you want to cancel the current simulation?",
//										QMessageBox::Yes | QMessageBox::No, progress);
//				if(cancelCheck.exec() == QMessageBox::Yes){
//					throw std::exception("Simulation canceled by user.");
//				}
//				//If the user does not cancel in the end, reset the progressDialog to reset the cancel flag.
//				progress->reset();
//			}
//			else{
//				progressValue = progress->value();
//			}
//			progress->setValue(progressValue+progressFactor);
//		}

	}

	//If the maps don t exist yet, we save them in files for further uses.
	//Note that if no path was set beforehand, they are not saved
	uvmesh.getDiffusedIntensityMap().saveMap();
	uvmesh.getReflectedIntensityMap().saveMap();
}

Intensity UVModel::calculateIntensityForVertex(UVMesh &uvmesh, CVertexO &aObjPt,
vector<float>::iterator &DIMIter, 
vector<float>::iterator &RIMIter, int sources) const
{

	// DIRECT INTENSITY
	double sunIntensity = 0;
	if(sources & IntensitySources::DIRECT){
		sunIntensity = UVModel::calculatePtSrcIntensity(uvmesh, aObjPt);
	}
	//DIFFUSED INTENSITY
	//Here we use a pair, with:
	//first: the intensity received by the vertex
	//second: the number of emittingPts that are in sight of the vertex
	pair<double,float> diffusedIntensity;

	//TODO: Change this to try whether the iterator exists...otherwise maybe some side-effects could
	//be encountered
	if(sources & IntensitySources::DIFFUSED){
		//If the diffuse map doesn't exist, create it
		if ( !uvmesh.getDiffusedIntensityMap().getMapComplete() )
		{			
			//Calculate the diffused intensity
			diffusedIntensity = UVModel::calculateHemisphericalSrcIntensity(uvmesh, aObjPt, *diffuseSrc);
			//Add the intensity to the map
			uvmesh.getDiffusedIntensityMap().getIntensities()->push_back(diffusedIntensity.second);
		}	
		//If the map already exists, simply read the map and update the vertex 
		//intensity accordingly
		else
		{
			diffusedIntensity = std::make_pair((*DIMIter)*(diffuseSrc->listBegin()->getIntensity()),*DIMIter);
			//diffusedIntensity = std::make_pair(0.0,0);
			DIMIter++;
		}
	}
	else{
		diffusedIntensity.first = 0.0;
		diffusedIntensity.second = 0.0f;
	}
	//REFLECTED INTENSITY
	//Here we use a pair, with:
	//first: the intensity received by the vertex
	//second: the number of emittingPts that are in sight of the vertex
	pair<double,float> reflectedIntensity;

	//TODO: Change this to try whether the iterator exists...otherwise maybe some side-effects could
	//be encountered
	if(sources & IntensitySources::REFLECTED){
		//If the reflected map doesn't exist, create it
		if ( !uvmesh.getReflectedIntensityMap().getMapComplete() )
		{			
			//Calculate the reflected intensity
			reflectedIntensity = UVModel::calculateHemisphericalSrcIntensity(uvmesh, aObjPt, *reflectedSrc);
			//Add the intensity to the map
			uvmesh.getReflectedIntensityMap().getIntensities()->push_back(reflectedIntensity.second);			
		}
		//If the map already exists, simply read the map and update the vertex 
		//intensity accordingly
		else
		{
			reflectedIntensity = std::make_pair((*RIMIter)*(reflectedSrc->listBegin()->getIntensity()),*RIMIter);
			//reflectedIntensity.first *= 100;
			//reflectedIntensity = std::make_pair(0.0,0.0f);
			RIMIter++;
		}
	}
	else{
		reflectedIntensity.first = 0.0;
		reflectedIntensity.second = 0.0f;
	}
	//Create and return the intensities as an intensityContainer
	Intensity intensities;
	intensities.direct = sunIntensity;
	intensities.diffused = diffusedIntensity.first;
	intensities.reflected = reflectedIntensity.first;
	return intensities;
	
}

double UVModel::calculatePtSrcIntensity(UVMesh &uvmesh, CVertexO &aObjPt) const
{

	// DIRECT INTENSITY
	// THE SUN IS A DIRECTIONNAL SOURCE ( AND NOT A PONCTUAL ONE )	
	emittingPt tempEmitPt(directSrc->getEmittingPt()->getP() + aObjPt.P(), directSrc->getSunIntensity());

	//Get the cosineFactor first since it is quickly calculated
	//and will return whether the ray from the emitting source
	//arrives on the front side of the vertex
	float cosineFactor = isPotentiallyVisible(tempEmitPt,aObjPt);
	if ( cosineFactor<0 )
	{
		cosineFactor = fabs(cosineFactor);
		//Check whether the direct source is visible for the vertex
		//Use or not the bounding boxes depending on the parameter set
		if (vertexVisible(uvmesh, tempEmitPt, aObjPt, useDirectBoxes))
		{
			//If the direct source si visible for the vertex, add its intensity 
			//to the vertex received intensity
			return  cosineFactor* directSrc->getSunIntensity();
		}
	}
	return 0.0;
}

pair<double,float> UVModel::calculateHemisphericalSrcIntensity(UVMesh &uvmesh, CVertexO &aObjPt, const hemisphericalSrc &aHemiSource) const
//double UVModel::calculateHemisphericalSrcIntensity(CVertexO &aObjPt, const hemisphericalSrc &aHemiSource)
{
	double HemiSrcIntensity = 0.0;
	float ptsRatio = 0.0;
	float cosineFactor;
	vector<emittingSurfPt>::iterator emitPtIter;
	//emittingPt tempEmitPt; <-- Maybe a default constructor should be created but really a stylistic issue...

	//Check which of the points composing the diffuse light source are visible to the vertex	
	for(emitPtIter=aHemiSource.listBegin(); emitPtIter!=aHemiSource.listEnd(); emitPtIter++)
	{
		emittingPt tempEmitPt = *emitPtIter;
		tempEmitPt.setCoordinates(tempEmitPt.getP()+ aObjPt.P());
	
		//Get the cosineFactor first since it is quickly calculated
		//and will return whether the ray from the emitting source
		//arrives on the front side of the vertex
		cosineFactor = isPotentiallyVisible(tempEmitPt, aObjPt);
		if ( cosineFactor<0 ) 
		{
			//Check whether this diffuse source point is visible for the vertex
			//Use or not the bounding boxes depending on the parameter set
			//With bounding boxes optimization
			if (vertexVisible(uvmesh, tempEmitPt, aObjPt, useDiffuseBoxes))
			{
				//If the diffuse light source point is visible to the vertex,
				//update the vertex received intensity and update the diffuse map
				HemiSrcIntensity += fabs(cosineFactor)*emitPtIter->getIntensity()*emitPtIter->getRatio();
				ptsRatio += fabs(cosineFactor)*emitPtIter->getRatio();
			}
		}
		
	}
	//return HemiSrcIntensity;
	return std::make_pair(HemiSrcIntensity,ptsRatio);
}

//--------------------------------------------------- Operator Overloading

//---------------------------------------------- Constructors - destructor
UVModel::UVModel(MeshModel* model, const char* filename, IOSpreadSheet* spreadsheet)
    : meshDocument(model->parent)
	, spreadsheet(spreadsheet)
{
	posture = new UVMesh(model, filename);
		
	//Instanciate variables with default values
	//UV Sources
	diffuseSrc = new hemisphericalSrc();
	reflectedSrc = new hemisphericalSrc();
	directSrc = new ptSrc();

	//By default, simulate all sources
	useDirectSource = true;
	useDiffuseSource = true;
	useReflectedSource = true;

	//Use of bounding boxes
	useDirectBoxes = true;
	useDiffuseBoxes = false;
	useReflectedBoxes = false;
	
	//Number of bounding boxes
	Xsubdiv = 7;
	Ysubdiv = 7;
	Zsubdiv = 7;
	
	//Assign the ray/plane tolerance value
	ray_plane_tolerance=0.99f;
	
	//Plane Surface
	usePlaneSurface = true;
	
	//Representation of the PlaneSurface if used
	planeSurface = 0;
}
UVModel::UVModel (MeshDocument *doc, const char* fileName, IOSpreadSheet* spreadsheet)
	: meshDocument(doc)
	, spreadsheet(spreadsheet)
{
	posture = new UVMesh(doc, fileName);
		
	//Instanciate variables with default values
	//UV Sources
	diffuseSrc = new hemisphericalSrc();
	reflectedSrc = new hemisphericalSrc();
	directSrc = new ptSrc();

	//Use of bounding boxes
	useDirectBoxes = true;
	useDiffuseBoxes = false;
	useReflectedBoxes = false;
	
	//Number of bounding boxes
	Xsubdiv = 7;
	Ysubdiv = 7;
	Zsubdiv = 7;
	
	//Assign the ray/plane tolerance value
	ray_plane_tolerance=0.99f;
	
	//Plane Surface
	usePlaneSurface = true;
	
	//Representation of the PlaneSurface if used
	planeSurface = 0;

	

} //----- End of UVModel

UVModel::~UVModel ( )
{
	delete posture;
	delete diffuseSrc;
	delete reflectedSrc;
	delete directSrc;
	if(planeSurface){
		delete planeSurface;
	}
} //----- End of ~UVModel


//---------------------------------------------------------------- PRIVATE

//------------------------------------------------------ Protected Methods

bool UVModel::vertexVisible(UVMesh &uvmesh, emittingPt &aEmitPt, CVertexO &aObjPt, bool useBoundingBoxesOptimization) const
{

	//With bounding boxes optimization
	if (useBoundingBoxesOptimization)
	{
		map<float,subBoundingBox*>* subBoxes = uvmesh.getSubBoxes();
		map<float,subBoundingBox*>::iterator boxIter = subBoxes->begin();
		while( boxIter!=subBoxes->end())
		{
			vector<CFaceO*>::iterator faceIter1 = boxIter->second->boxFaces->begin();
			while( faceIter1!=boxIter->second->boxFaces->end())
			{
				if ( linePlanIntersect(aEmitPt,aObjPt,**faceIter1) )
				{
					vector<CFaceO*>::iterator faceIter2 = boxIter->second->containedFaces->begin();
					while( faceIter2!=boxIter->second->containedFaces->end())
					{
						if ( linePlanIntersect(aEmitPt,aObjPt,**faceIter2) )
						{
							return false;
						}
						faceIter2++;
					}
				}
				faceIter1++;
			}
			boxIter++;
		}
	}
	// Without optimization
	else
	{
		vector<CFaceO> *modelFaces = uvmesh.getModelFaces();
		vector<CFaceO>::iterator faceIter = modelFaces->begin();
		while( faceIter!=modelFaces->end())
		{
			if ( linePlanIntersect(aEmitPt,aObjPt,*faceIter) )
			{
				return false;
			}
			faceIter++;
		}
	}
	return true;
}


float UVModel::isPotentiallyVisible(emittingPt &aEmitPt, CVertexO &aObjPt) const
{	
	// the ray vector
	vcg::Point3<float> Ray = aObjPt.P()-aEmitPt.getP();
	return (Ray*aObjPt.N())/(Ray.Norm()*aObjPt.N().Norm());
}

bool UVModel::linePlanIntersect(emittingPt &aEmitPt, CVertexO &aObjPt, CFaceO &aObjFace) const
{
	// There may be better way to solve this problem (line/triangle intersection) but
	// this way is the easiest to understand.
	
	// The emitting point coordinates
	//IMPORTANT: Here we use the screen coordinates since the mesh is in these.
	//But it implies we have to use the emitting point screen coordinates.
	//The transform is:
	//Standard(X,Y,Z) -> Screen(Y,Z,X)
	vcg::Point3<float> E = aEmitPt.getP();

	// the ray vector
	vcg::Point3<float> R = aObjPt.P()-E;

	//Vector from the corner of the face to the emitting point
	vcg::Point3<float> AE = E - aObjFace.V(0)->P();

	//Check if the ray and the plane are parallel.
	if (aObjFace.N()*R==0.0)
	{
		// the plan and the ray are parallel.
		// then we must now if the ray is coplanar to the plan,
		// ie A, B, C and E (O is possible too) are coplanar. So we use the triple product.
		if ( (aObjFace.N()*AE)==0 )
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	/* t is the spacial parameter of the ray line of equation :
	 * M(t) = R*t + E
	 * we suppose there is a point P such as the triple product [AB,AC,AP] is equal to 0
	 * which means that P belong the plane ABC
	 * furthermore it belongs to the ray line so, we can deduce the corresponding t.
	 * to find t, after simplification, is to solve the follonwing equation : */	
	float t = -(aObjFace.N()*AE) / (aObjFace.N()*R);

	// We can now find P.
	vcg::Point3<float> P = (R*t)+E;

	// Then we must know if the ray reaches the point first or the face plane first.
	//If the ray reaches the point first, it cannot be occluded by the face.
	//If the face and the point are very close, consider the point above the face.
	//Note that this should be checked and was added since rounding differs from 
	//the previous version of the function
	vcg::Point3<float> EP = P-E;
	float normR = R.Norm();
	float normEP = EP.Norm();
	if ( (normR < normEP) || (fabs(normEP/normR)>ray_plane_tolerance))
	{
		return false;
	}
	
	/* These easy to understand tests say if the point P belongs to the triangle ABC
	 * first, if the projection of the vector AP on the vector AB is
	 * negative or shorter than AB
	 * then the point P cannot be in the triangle.
	 * One can verify it with a drawing.
	 * if the dot product AB.AP is negative, it means that the projection is negative.
	 * if the dot product AB.AP divided by the norm of AB is inferior to
	 * the norm of AB then the projection is shorter than AB.
	 * Then we repeat with AC.AP . */
	vcg::Point3<float> Atest = aObjFace.V(0)->P();
	vcg::Point3<float> AP = P - aObjFace.V(0)->P();
	vcg::Point3<float> AB = aObjFace.V(1)->P()-aObjFace.V(0)->P();
	vcg::Point3<float> AC = aObjFace.V(2)->P()-aObjFace.V(0)->P();
	float normAB = AB.Norm();
	float normAC = AC.Norm();
	float P1 = (AB*AP)/normAB;
	float P2 = (AC*AP)/normAC;

	if ( (P1<0) || (P1>normAB) || (P2<0) || (P2>normAC) )
	{
		return false;
	}

	// if it passes the previous test, it doesn t mean the point P is ont the triangle
	// there are 3 zones where it can verify the test without being on the triangle
	// so we repeat the test with other vectors composed with the triangle points and P
	// here we use B to do the test (first I used A), then endly C (below).
	//Note that here, to avoid computing BA, we use the following properties:
	//* BA = -AB
	//* BP = -PB
	//* BC = -CB
	//* and the dot product property that X*Y = -X*-Y (where * is the dot prod.)
	vcg::Point3<float> PB = aObjFace.V(1)->P() - P;
	vcg::Point3<float> CB = aObjFace.V(1)->P()-aObjFace.V(2)->P();
	float normCB = CB.Norm();
	P1 = (AB*PB) / normAB;
	P2 = (CB*PB) / normCB;

	if ( (P1<0) || (P1>normAB) || (P2<0) || (P2>normCB) )
	{
		return false;
	}

	// with C as reference.
	//Here we have to compute either CA or BC. We chose to compute CA = -AC
	vcg::Point3<float> CP = P - aObjFace.V(2)->P();
	vcg::Point3<float> CA = AC*-1;
	P1 = (CA*CP) / normAC;
	P2 = (CB*CP) / normCB;

	if ( (P1<0) || (P1>normAC) || (P2<0) || (P2>normCB) )
	{
		return false;
	}

	//Finally if the intersection of the ray and the plane happens before it 
	//reaches the point and inside the triangle of the face, then we can
	//return that the point is occluded by the face.
	return true;
	
}
