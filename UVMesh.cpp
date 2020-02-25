/*                           UVMesh  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*/

#include "UVMesh.h"

#include "OldAPISupport.h"
#include "Helpers.h"

//--------------------------------------------------- PUBLIC

//--------------------------------------------------- Public Methods
//Load the intensity maps

bool UVMesh::loadDiffuseIntensityMap(string filename){
	diffusedIntensityMap.setNumModelVertices(model->cm.vert.size());
	diffusedIntensityMap.loadMap(filename);
	return diffusedIntensityMap.getMapComplete();
}
bool UVMesh::loadDiffuseIntensityMap(int diffuseSrcLvlNb, int diffuseSrcPtNb){
	diffusedIntensityMap.setNumModelVertices(model->cm.vert.size());
	diffusedIntensityMap.loadMap(generateMapNameFromLvlPts(diffuseSrcLvlNb,diffuseSrcPtNb) + "DMap.txt");
	return diffusedIntensityMap.getMapComplete();
}
bool UVMesh::loadReflectedIntensityMap(string filename){
	reflectedIntensityMap.setNumModelVertices(model->cm.vert.size());
	reflectedIntensityMap.loadMap(filename);
	return reflectedIntensityMap.getMapComplete();
}
bool UVMesh::loadReflectedIntensityMap(int reflectedSrcLvlNb, int reflectedSrcPtNb){
	reflectedIntensityMap.setNumModelVertices(model->cm.vert.size());
	reflectedIntensityMap.loadMap(generateMapNameFromLvlPts(reflectedSrcLvlNb,reflectedSrcPtNb) + "RMap.txt");
	return reflectedIntensityMap.getMapComplete();
}

void UVMesh::reloadBoundingBoxes(){
	deleteBoundingBoxes();
	subBoxes = new map<float,subBoundingBox*>;
	createBoundingBoxes(BBoxSubdivX,BBoxSubdivY,BBoxSubdivZ);
}

void UVMesh::createBoundingBoxes(int Xsubdiv, int Ysubdiv, int Zsubdiv){

	//Store the number of SubDivs
	BBoxSubdivX = Xsubdiv;
	BBoxSubdivY = Ysubdiv;
	BBoxSubdivZ = Zsubdiv;

	//Get the bounding box
	vcg::Box3f boundingBox = model->cm.trBB();


	//Store the min and max of the bouding box for each of the dimension
	float xMin = boundingBox.min[0];
	float xMax = boundingBox.max[0];
	float yMin = boundingBox.min[1];
	float yMax = boundingBox.max[1];
	float zMin = boundingBox.min[2];
	float zMax = boundingBox.max[2];
	
	/* Now, we subdivise the bounding box given the three attributes Xsubdiv, Ysubdiv, Zsubdiv
	 * and we want to know in which sub-box each face is. 
	 Firstly we ll see in which box each
	 * point is. But we ll cheat to keep the results : we ll temporarily put them in the normal
	 * coordinates. */

	//Get the size of each of the sides of the boxes
	float xDiv = (xMax-xMin) / Xsubdiv;
	float yDiv = (yMax-yMin) / Ysubdiv;
	float zDiv = (zMax-zMin) / Zsubdiv;

	//Find out the subbox each Vertex belongs in.
	// there is only a particular case, which is when one of the coordinates is
	// equal to the maximum, the result of the division will be ?subdiv.
	// in this case, we set the result as ?subdiv-1 to have the right number of boxes.
	vector<CVertexO>::iterator vertIter = model->cm.vert.begin();	
	for (vertIter=model->cm.vert.begin(); vertIter!=model->cm.vert.end(); vertIter++)
	{
		unsigned int Xcomp = min( floor((vertIter->P()[0]-xMin)/xDiv), (float)(Xsubdiv-1) );
		unsigned int Ycomp = min( floor((vertIter->P()[1]-yMin)/yDiv), (float)(Ysubdiv-1) );
		unsigned int Zcomp = min( floor((vertIter->P()[2]-zMin)/zDiv), (float)(Zsubdiv-1) );
		vertIter->N()[0] = Xcomp + Ycomp*Xsubdiv + Zcomp*Xsubdiv*Ysubdiv;

		//Create the new box and insert it in the boxes map.
		//Using a map prevents any duplicates and all boxes inserted with the
		//same coordinates will simply be inserted once.
		subBoundingBox* tempSBB = new subBoundingBox;
		tempSBB->boxFaces = new vector<CFaceO*>;
		tempSBB->boxFaces->empty();
		tempSBB->containedFaces = new vector<CFaceO*>;
		tempSBB->containedFaces->empty();
		subBoxes->insert( pair<float,subBoundingBox*>(vertIter->N()[0],tempSBB) );
	}
	
	//Create the existing bounding boxes
	map<float,subBoundingBox*>::iterator boxIter;
	for(boxIter=subBoxes->begin(); boxIter!=subBoxes->end(); boxIter++)
	{
// --------------------------------------------------------------- AMELIORABLE - ATTENTION LES YEUX : CODE TRES MOCHE
		//Get the position of the box in "boxes coordinates"
		unsigned int Zcomp = boxIter->first/(Xsubdiv*Ysubdiv);
		unsigned int Ycomp = ((unsigned int)boxIter->first%(Xsubdiv*Ysubdiv)) / Xsubdiv;
		unsigned int Xcomp = ((unsigned int)boxIter->first%(Xsubdiv*Ysubdiv)) % Xsubdiv;

		//Create the vertices
		CVertexO* vertex1 = new CVertexO;
		vertex1->P()[0] = xMin + xDiv*Xcomp;
		vertex1->P()[1] = yMin + yDiv*Ycomp;
		vertex1->P()[2] = zMin + zDiv*Zcomp;

		CVertexO* vertex2 = new CVertexO;
		vertex2->P()[0] = xMin + xDiv*(Xcomp+1);
		vertex2->P()[1] = yMin + yDiv*Ycomp;
		vertex2->P()[2] = zMin + zDiv*Zcomp;

		CVertexO* vertex3 = new CVertexO;
		vertex3->P()[0] = xMin + xDiv*Xcomp;
		vertex3->P()[1] = yMin + yDiv*(Ycomp+1);
		vertex3->P()[2] = zMin + zDiv*Zcomp;

		CVertexO* vertex4 = new CVertexO;
		vertex4->P()[0] = xMin + xDiv*Xcomp;
		vertex4->P()[1] = yMin + yDiv*Ycomp;
		vertex4->P()[2] = zMin + zDiv*(Zcomp+1);

		CVertexO* vertex5 = new CVertexO;
		vertex5->P()[0] = xMin + xDiv*(Xcomp+1);
		vertex5->P()[1] = yMin + yDiv*(Ycomp+1);
		vertex5->P()[2] = zMin + zDiv*Zcomp;

		CVertexO* vertex6 = new CVertexO;
		vertex6->P()[0] = xMin + xDiv*(Xcomp+1);
		vertex6->P()[1] = yMin + yDiv*Ycomp;
		vertex6->P()[2] = zMin + zDiv*(Zcomp+1);

		CVertexO* vertex7 = new CVertexO;
		vertex7->P()[0] = xMin + xDiv*Xcomp;
		vertex7->P()[1] = yMin + yDiv*(Ycomp+1);
		vertex7->P()[2] = zMin + zDiv*(Zcomp+1);

		CVertexO* vertex8 = new CVertexO;
		vertex8->P()[0] = xMin + xDiv*(Xcomp+1);
		vertex8->P()[1] = yMin + yDiv*(Ycomp+1);
		vertex8->P()[2] = zMin + zDiv*(Zcomp+1);

		//Create the faces
		//Compute the faces normal as they are used later on
		CFaceO* face1 = new CFaceO;
		face1->V(0) = vertex1;
		face1->V(1) = vertex2;
		face1->V(2) = vertex6;
		OldApi::ComputeNormal(*face1);
		boxIter->second->boxFaces->push_back(face1);

		CFaceO* face2 = new CFaceO;
		face2->V(0) = vertex1;
		face2->V(1) = vertex6;
		face2->V(2) = vertex4;
		OldApi::ComputeNormal(*face2);
		boxIter->second->boxFaces->push_back(face2);

		CFaceO* face3 = new CFaceO;
		face3->V(0) = vertex3;
		face3->V(1) = vertex8;
		face3->V(2) = vertex5;
		OldApi::ComputeNormal(*face3);
		boxIter->second->boxFaces->push_back(face3);

		CFaceO* face4 = new CFaceO;
		face4->V(0) = vertex3;
		face4->V(1) = vertex7;
		face4->V(2) = vertex8;
		OldApi::ComputeNormal(*face4);
		boxIter->second->boxFaces->push_back(face4);

		CFaceO* face5 = new CFaceO;
		face5->V(0) = vertex1;
		face5->V(1) = vertex5;
		face5->V(2) = vertex2;
		OldApi::ComputeNormal(*face5);
		boxIter->second->boxFaces->push_back(face5);

		CFaceO* face6 = new CFaceO;
		face6->V(0) = vertex1;
		face6->V(1) = vertex3;
		face6->V(2) = vertex5;
		OldApi::ComputeNormal(*face6);
		boxIter->second->boxFaces->push_back(face6);

		CFaceO* face7 = new CFaceO;
		face7->V(0) = vertex4;
		face7->V(1) = vertex6;
		face7->V(2) = vertex8;
		OldApi::ComputeNormal(*face7);
		boxIter->second->boxFaces->push_back(face7);

		CFaceO* face8 = new CFaceO;
		face8->V(0) = vertex4;
		face8->V(1) = vertex8;
		face8->V(2) = vertex7;
		OldApi::ComputeNormal(*face8);
		boxIter->second->boxFaces->push_back(face8);

		CFaceO* face9 = new CFaceO;
		face9->V(0) = vertex2;
		face9->V(1) = vertex5;
		face9->V(2) = vertex8;
		OldApi::ComputeNormal(*face9);
		boxIter->second->boxFaces->push_back(face9);

		CFaceO* face10 = new CFaceO;
		face10->V(0) = vertex2;
		face10->V(1) = vertex8;
		face10->V(2) = vertex6;
		OldApi::ComputeNormal(*face10);
		boxIter->second->boxFaces->push_back(face10);

		CFaceO* face11 = new CFaceO;
		face11->V(0) = vertex1;
		face11->V(1) = vertex7;
		face11->V(2) = vertex3;
		OldApi::ComputeNormal(*face11);
		boxIter->second->boxFaces->push_back(face11);

		CFaceO* face12 = new CFaceO;
		face12->V(0) = vertex1;
		face12->V(1) = vertex4;
		face12->V(2) = vertex7;
		OldApi::ComputeNormal(*face12);
		boxIter->second->boxFaces->push_back(face12);
	
	}

	//Assign the faces to the bounding boxes
	vector<CFaceO>::iterator faceIter;
	for(faceIter=model->cm.face.begin(); faceIter!=model->cm.face.end(); faceIter++)
	{
		// all the points are in a same box
		if ( (faceIter->V(0)->N()[0] == faceIter->V(1)->N()[0]) &&
				(faceIter->V(0)->N()[0] == faceIter->V(2)->N()[0]) )
		{
			boxIter = subBoxes->find(faceIter->V(0)->N()[0]);
			boxIter->second->containedFaces->push_back( &(*faceIter) );
		}
		// only 2 points are in a same box (3 cases)
		else if (faceIter->V(0)->N()[0] == faceIter->V(1)->N()[0])
		{
			boxIter = subBoxes->find(faceIter->V(0)->N()[0]);
			boxIter->second->containedFaces->push_back( &(*faceIter) );
			boxIter = subBoxes->find(faceIter->V(2)->N()[0]);
			boxIter->second->containedFaces->push_back( &(*faceIter) );
		}
		else if (faceIter->V(0)->N()[0] == faceIter->V(2)->N()[0])
		{
			boxIter = subBoxes->find(faceIter->V(0)->N()[0]);
			boxIter->second->containedFaces->push_back( &(*faceIter) );
			boxIter = subBoxes->find(faceIter->V(1)->N()[0]);
			boxIter->second->containedFaces->push_back( &(*faceIter) );
		}
		else if (faceIter->V(1)->N()[0] == faceIter->V(2)->N()[0])
		{
			boxIter = subBoxes->find(faceIter->V(1)->N()[0]);
			boxIter->second->containedFaces->push_back( &(*faceIter) );
			boxIter = subBoxes->find(faceIter->V(0)->N()[0]);
				boxIter->second->containedFaces->push_back( &(*faceIter) );
		}
		// if the points are in different boxes
		else
		{
			boxIter = subBoxes->find(faceIter->V(0)->N()[0]);
			boxIter->second->containedFaces->push_back( &(*faceIter) );
			boxIter = subBoxes->find(faceIter->V(1)->N()[0]);
			boxIter->second->containedFaces->push_back( &(*faceIter) );
			boxIter = subBoxes->find(faceIter->V(2)->N()[0]);
			boxIter->second->containedFaces->push_back( &(*faceIter) );
		}
	}
	
	//Re-calculate all the Mesh normals (Vertices and Faces)
	vcg::tri::UpdateNormal<CMeshO>::PerVertexPerFace(model->cm);


}

void UVMesh::evaluateTotalEvaluatedIntensity(bool summary)
{
	auto &total = summary ? summaryTotalEvalIntensity : totalEvalIntensity;
	auto &list = summary ? summaryEvalIntensityList : evalIntensityList;

	if(!total || list->empty())
		return;
	if(!total->intensityList)
	{
		EvaluatedIntensity *evalIntensity = list->front();
		total->beginDate = new Date(*evalIntensity->beginDate);
		total->endDate = new Date(*evalIntensity->endDate);
		total->intensityList = new IntensityList;
		total->intensityList->resize(evalIntensity->intensityList->size());
	}

	for(auto evalIt = list->begin(); evalIt != list->end(); evalIt++)
	{
		const EvaluatedIntensity &evalIntensity = **evalIt;
		if(*(total->beginDate) > *(evalIntensity.beginDate))
			*(total->beginDate) = *(evalIntensity.beginDate);
		if(*(total->endDate) < *(evalIntensity.endDate))
			*(total->endDate) = *(evalIntensity.endDate);

		const IntensityList *intensityList = evalIntensity.intensityList;
		int i = 0;
		for(auto intIt = intensityList->begin(); intIt != intensityList->end(); intIt++, i++)
			(*total->intensityList)[i] += *intIt;
	}
}

void UVMesh::clearEvaluatedIntensities()
{
	Helpers::clearEvaluatedIntensities(&evalIntensityList);
	if(zones)
	{
		for(auto zoneIter = zones->begin(); zoneIter!=zones->end(); zoneIter++)
			(*zoneIter)->clearEvaluatedIntensities();
	}
}

void UVMesh::setPOIs(const char *filename){
	
	//Get the POIs from the XML file
	posturePOIs = IOPoints::parseXMLFile(filename);
	
	//Insert the POIs into the Mesh
	vector<POI>::iterator point_iter;
	vector<POI>::iterator point_iter2;
	vector<CFaceO>::iterator faceIter; 
	CFaceO *POIFace;
	
	//Loop for each of the pts
	for(point_iter=posturePOIs->begin(); point_iter!=posturePOIs->end();point_iter++){
		
		POIFace = findPOIFace(*point_iter);
			
		//If the face containing the sensor was found,
		//Add the sensor to the face
		if(POIFace){
			
			//Store the vertices composing the face
			CVertexO* vertex0 = POIFace->V(0);
			CVertexO* vertex1 = POIFace->V(1);
			CVertexO* vertex2 = POIFace->V(2);
		
			//Delete the face
			vcg::tri::Allocator<CMeshO>::DeleteFace(model->cm,(*POIFace));

			//Insert the sensor into the Mesh
			vcg::tri::Allocator<CMeshO>::PointerUpdater<CMeshO::VertexPointer> pu;
			vcg::tri::Allocator<CMeshO>::AddVertices(model->cm,1,pu);

			//Update the vertex pointers if needed
			if(pu.NeedUpdate()){
				pu.Update(vertex0);
				pu.Update(vertex1);
				pu.Update(vertex2);
				for(point_iter2=posturePOIs->begin(); point_iter2!=posturePOIs->end();point_iter2++){
					point_iter2->updateMeshSensor(pu);
				}
			}

			//Set the coordinates and color of the sensor
			CVertexO *sensor = &(model->cm.vert.back());
			sensor->P() = (*point_iter).getPoint();
			sensor->C() = vertex0->C();

			//Create the new Faces					
			vcg::tri::Allocator<CMeshO>::AddFaces(model->cm,3);			
			
			//Fill the coordinates
			faceIter=model->cm.face.end();
			--faceIter;
			faceIter->V(0) = vertex0;
			faceIter->V(1) = vertex1;
			faceIter->V(2) = sensor;			
			--faceIter;
			faceIter->V(0) = vertex1;
			faceIter->V(1) = vertex2;
			faceIter->V(2) = sensor;
			--faceIter;
			faceIter->V(0) = vertex2;
			faceIter->V(1) = vertex0;
			faceIter->V(2) = sensor;
		
			point_iter->setMeshSensor(sensor);			
		}		
		
	}
	//Permanently remove all deleted faces
	vcg::tri::Allocator<CMeshO>::CompactVertexVector(model->cm);
	vcg::tri::Allocator<CMeshO>::CompactFaceVector(model->cm);
	
	//update the normals
	vcg::tri::UpdateNormal<CMeshO>::PerVertexPerFace(model->cm);
	
	//Reload the bounding boxes since they contain pointers to faces and
	//they need to incorporate the new faces
	reloadBoundingBoxes();

	//Re-evaluate the intensity maps
	diffusedIntensityMap.setNumModelVertices(model->cm.vert.size()); 
	diffusedIntensityMap.evaluateMapComplete();
	reflectedIntensityMap.setNumModelVertices(model->cm.vert.size());
	reflectedIntensityMap.evaluateMapComplete();

}


void UVMesh::evaluatePOI(float maxDistance, bool useBaseVertices){
	
	vector<POI>::iterator point_iter;
	vector<CFaceO>::iterator faceIter; 
	vector<CVertexO>::iterator vi;
	
	//Update the Mesh Data Mask to contain the topology information
	//This information is then used internally to follow faces around vertices
	model->clearDataMask(MeshModel::MM_VERTFACETOPO);
	model->updateDataMask(MeshModel::MM_VERTFACETOPO);

	//Prepare the datetime information since it will be the same for all POIs
	Date *beginDate = 0;
	Date *endDate = 0;
	if (evalIntensityList && evalIntensityList->size()>0){
		beginDate = new Date(*(evalIntensityList->front()->beginDate));
		endDate = new Date(*(evalIntensityList->front()->endDate));
	}
	
	//Loop for each of the pts
	for(point_iter=posturePOIs->begin(); point_iter!=posturePOIs->end();point_iter++){
		
		//Tag all the vertices of interest for this sensor
		//i.e. all vertices around the POI in the given radius
		
		//If the POI already has a pointer to the mesh, use it and tag around it
		if(point_iter->getMeshSensor()){
			vcg::Point3<float> myFloat;	
//			tagVerticesForPOI(point_iter->getPoint(), point_iter->getMeshSensor(), maxDistance);
			tagVerticesForPOI(myFloat, point_iter->getMeshSensor(), maxDistance);
		}
		//Otherwise find its containing face and tag from there
		else{
			CFaceO* POIFace = findPOIFace(*point_iter);
			if(POIFace){
				//Tag the points
				vcg::Point3<float> P = (*point_iter).getPoint();
				int i;
				for(i=0;i<3;i++){
					if(useBaseVertices || ((POIFace->V(i)->P()-P).Norm()<=maxDistance)){
						tagVerticesForPOI(P, POIFace->V(i), maxDistance);	
					}
				}
			}
		}		
				
		//Run through the Mesh and save the intensities for all tagged vertices
		//Note that at the moment it is no possible to directly get intensities 
		//when tagging since the evaluated intensities and the vertices are not linked				
		EvaluatedIntensity *POIintensity = new EvaluatedIntensity;
		POIintensity->beginDate = beginDate;
		POIintensity->endDate = endDate;
		POIintensity->intensityList = new IntensityList;
		IntensityList::iterator iic = (*evalIntensityList->begin())->intensityList->begin();
		for(vi = model->cm.vert.begin(); vi!=model->cm.vert.end(); vi++){			
			if (vi->C()[3] == 100){
				Intensity vertexIntensity;
				vertexIntensity.direct = (*iic).direct;
				vertexIntensity.diffused = (*iic).diffused;
				vertexIntensity.reflected = (*iic).reflected;
				POIintensity->intensityList->push_back(vertexIntensity);
				vi->C()[3] = 255;	
			}
			
			iic++;
		}
		(*point_iter).setEvaluatedIntensity(POIintensity);		
		
	}
}

void UVMesh::setZones(const char *filename){
	//Parse the zones from XML
	zones = IOZones::parseXMLFile(filename);
	
	//Match the zones and the vertices
	vector<Zone*>::iterator zoneIter;
	for(zoneIter = zones->begin(); zoneIter!=zones->end();zoneIter++){
		setVerticesForZone(*zoneIter);
	}
}

void UVMesh::evaluateZones(){
	
	//Set the area and intensities for the faces
	//FACE INTENSITIES -- NOT CURRENTLY USED
	//setFaceIntensities();
	
	//Enable and reload the necessary Mesh Topology elements
	model->clearDataMask(MeshModel::MM_VERTFACETOPO);
	model->updateDataMask(MeshModel::MM_VERTFACETOPO);
	
	//Create a map of the intensities based on vertex address
	VertexIntensities vertexIntensities;
	vector<CVertexO>::iterator vertIter;
	IntensityList::iterator intensIter = evalIntensityList->back()->intensityList->begin();
	
	for(vertIter=model->cm.vert.begin(); vertIter != model->cm.vert.end(); vertIter++){
		vertexIntensities[&(*vertIter)] = &(*intensIter);
		intensIter++;
	}
	
	//Evaluate all zones intensities
	vector<Zone*>::iterator zoneIter;
	Date beginDate = *(evalIntensityList->back()->beginDate);
	Date endDate = *(evalIntensityList->back()->endDate);
	for(zoneIter=zones->begin();zoneIter!=zones->end();zoneIter++){
		(*zoneIter)->evaluateIntensity(&vertexIntensities, beginDate, endDate);
	}
	
		
	
}

void UVMesh::setProtections(bool reset){
	if(zones){
		if(reset){
			resetProtections();
		}
		
		Protections::applyProtections(zones);
	}
}


void UVMesh::resetProtections(){
	
	if(zones){
		vector<Zone*>::iterator zoneIter;
		for(zoneIter=zones->begin();zoneIter!=zones->end();++zoneIter){
			(*zoneIter)->resetZoneIP();
		}
	}
}

void UVMesh::setColors(float blueFromValue, float greenFromValue, float redFromValue, float redToValue, int sources)
{
	UVMesh::setColors(model, summaryEvalIntensityList, blueFromValue, greenFromValue, redFromValue, redToValue, sources);
}

void UVMesh::setColors(MeshModel *m, vector<EvaluatedIntensity*>* intensities, float blueFromValue, float greenFromValue, float redFromValue, float redToValue, int sources)
{

	vector<EvaluatedIntensity*>::iterator evalIntIter;
	EvaluatedIntensity evalIntensity;
	
	for (evalIntIter=intensities->begin(); evalIntIter!=intensities->end(); ++evalIntIter)
	{
		evalIntensity = **evalIntIter;
		
		if (evalIntensity.intensityList->size() == m->cm.vert.size())
		{
			IntensityList::iterator intensityIter = evalIntensity.intensityList->begin();
			vector<CVertexO>::iterator vertIter;
			
			for (vertIter=m->cm.vert.begin(); vertIter!=m->cm.vert.end(); vertIter++)
			{
				setVertexColor(&(*vertIter), *intensityIter, blueFromValue, greenFromValue, redFromValue, redToValue, sources);
				intensityIter++;
			}
		}
		else
		{
			//MessageBox(NULL, L"Data Integrity error. Please reload the file and re-evalute the colors.", L"Notice", 0x10000);//MODIFY
		}
	}
}


void UVMesh::setColorsFromZones(float blueFromValue, float greenFromValue, float redFromValue, float redToValue, int sources){
	setColorsFromZones(zones, blueFromValue, greenFromValue, redFromValue, redToValue, sources);
}


void UVMesh::saveMeshToFile(string aFilename)
{
	vector<EvaluatedIntensity*>::iterator evalIntIter;
	EvaluatedIntensity evalIntensity;
	
	string pathTemp = path;;
	if(!aFilename.empty()){
		pathTemp = aFilename;
	}
	
	if(evalIntensityList->size() < 1){
		cout << "Saved to : " << pathTemp+"_result.ply" << endl;
		IOFile::saveToFile((pathTemp+"_result.ply").c_str(), model, 0x0005);
	}
	
	for (evalIntIter=evalIntensityList->begin(); evalIntIter!=evalIntensityList->end(); evalIntIter++)
	{	
		evalIntensity = **evalIntIter;

		string name = generateOutputFilename(evalIntensity.beginDate, evalIntensity.endDate);

		cout << "Saved to : " << pathTemp+name+"_result.ply" << endl;
		IOFile::saveToFile((pathTemp+name+"_result.ply").c_str(), model, 0x0005);
	}
}


void UVMesh::exportIntensitiesCSV(Intensity *flatSurfaceIntensities, string filename){
	
	string name = filename;
	
	//If no file name is provided, generate a meaningful one
	if(filename.empty()){
		name = path+generateOutputFilename((evalIntensityList->front())->beginDate, (evalIntensityList->back())->endDate)+"_result.csv";
	}
	
	float flatTotal = -1.0;
	float flatDiffuse = -1.0;
	float flatDirect = -1.0;
	float flatReflected = -1.0;
	
	if(flatSurfaceIntensities){
		flatDiffuse = flatSurfaceIntensities->diffused;
		flatDirect  = flatSurfaceIntensities->direct;
		flatReflected = flatSurfaceIntensities->reflected;
		flatTotal = flatDiffuse + flatDirect + flatReflected;
	}
	

	ofstream file(name.c_str());
	if(!file.is_open())
	{
		cout << "Unable to open file";
		return;
	}

	//Print the header
	file << "Vertex Num,Start Time,End Time,Total Intensity Received [J/m2],Diffuse Intensity Received [J/m2],Direct Intensity Received [J/m2],Reflected Intensity Received [J/m2],Total Intensity Received [%ambiant],Diffuse Intensity Received [%ambiant],Direct Intensity Received [%ambiant],Reflected Intensity Received [%ambiant]\n";

	//Loop through each of the intensities and  print them
	int vertexNum = 0;
	char buffer [500];
	float totalIntensity = 0.0;
	float diffuseIntensity = 0.0;
	float directIntensity = 0.0;
	float reflectedIntensity = 0.0;

	//for(auto evalIntIter=evalIntensityList->begin(); evalIntIter!=evalIntensityList->end(); evalIntIter++)
	{
		//auto evalIntensity = **evalIntIter;
		auto &evalIntensity = *totalEvalIntensity;

		//strftime (buffer,500,"%x %X",&(evalIntensity.beginDate->getDate()));//MODIFY
		string startDate(buffer);
		//strftime (buffer,500,"%x %X",&(evalIntensity.endDate->getDate()));//MODIFY
		string endDate(buffer);

		for(auto intensity = evalIntensity.intensityList->begin(); intensity != evalIntensity.intensityList->end(); intensity++){
			diffuseIntensity = intensity->diffused;
			directIntensity = intensity->direct;
			reflectedIntensity = intensity->reflected;
			totalIntensity = diffuseIntensity + directIntensity + reflectedIntensity;
			sprintf (buffer, "%i", vertexNum);
			file << string(buffer) << "," << startDate << "," << endDate << ",";
			sprintf (buffer, "%E,%E,%E,%E,", totalIntensity, diffuseIntensity, directIntensity, reflectedIntensity);
			file << string(buffer);
			sprintf (buffer, "%E,%E,%E,%E\n", (totalIntensity/flatTotal)*100, (diffuseIntensity/flatDiffuse)*100, (directIntensity/flatDirect)*100, (reflectedIntensity/flatReflected)*100);
			file << string(buffer);
			++vertexNum;
		}
	}

	file.close();
}

//Exports the POI received intensities as a CSV file
void UVMesh::exportPOIIntensitiesCSV(Intensity *flatSurfaceIntensities,string filename){
	IOPoints::exportIntensitiesCSV(posturePOIs,flatSurfaceIntensities,filename);
}

//Exports the Zones received intensities as a CSV file
void UVMesh::exportZonesIntensitiesCSV(Intensity *flatSurfaceIntensities,string filename,IOZones::OutputStreamMap &outputStreams){
	IOZones::exportIntensitiesCSV(zones,flatSurfaceIntensities, filename, outputStreams);
}

void UVMesh::multiplyIntensities(float factor){
	
	vector<EvaluatedIntensity*>::iterator intListIter;
	for(intListIter = evalIntensityList->begin(); intListIter != evalIntensityList->end(); ++intListIter){
		IntensityList::iterator intIter;
		for(intIter = (*intListIter)->intensityList->begin(); intIter != (*intListIter)->intensityList->end(); ++intIter){
			intIter->direct = intIter->direct * factor;
			intIter->diffused = intIter->diffused * factor;
			intIter->reflected = intIter->reflected * factor;
		}
	}
}

void UVMesh::mergeZonesIntensities(vector<Zone*>* zonesAdd){
	//Loop over all the zones to add
	vector<Zone*>::iterator zonesAddIter;
	vector<Zone*>::iterator subZonesAddIter;
	Zone* targetZone;
	for(zonesAddIter = zonesAdd->begin(); zonesAddIter != zonesAdd->end(); ++zonesAddIter){
		//Add the zone intensity
		if((*zonesAddIter)->getEvaluatedIntensity()->intensityList){
			targetZone = findZoneByName((*zonesAddIter)->getName());
			if(targetZone){
				targetZone->mergeZoneIntensity((*zonesAddIter)->getEvaluatedIntensity());
			}
		}
		
		//If the zone has subzones, add the subzones intensities
		vector<Zone*>* subZones = (*zonesAddIter)->getSubZones();
		if(subZones){
			for(subZonesAddIter = subZones->begin(); subZonesAddIter != subZones->end(); ++subZonesAddIter){
				targetZone = findZoneByName((*subZonesAddIter)->getName());
				if(targetZone){
					targetZone->mergeZoneIntensity((*subZonesAddIter)->getEvaluatedIntensity());
				}

			}
		}
	}

}

void UVMesh::multiplyZonesIntensities(float factor){
	vector<Zone*>::iterator zoneIter;
	for(zoneIter = zones->begin(); zoneIter != zones->end(); ++zoneIter){
		(*zoneIter)->multiplyZoneIntensities(factor);
	}
}

Zone* UVMesh::findZoneByName(string name, vector<Zone*>* searchZones){
	if(!searchZones){
		searchZones = zones;
	}
	vector<Zone*>::iterator zoneIter;
	vector<Zone*>::iterator subZonesIter;
	//Look for the searched zone
	for(zoneIter = searchZones->begin(); zoneIter != searchZones->end(); ++zoneIter){
		if((*zoneIter)->getName() == name){
			return (*zoneIter);
		}
		//Look in subzones
		vector<Zone*>* subZones = (*zoneIter)->getSubZones();
		if(subZones){
			for(subZonesIter = subZones->begin(); subZonesIter != subZones->end(); ++subZonesIter){
				Zone* subzone = findZoneByName(name, subZones);
				if(subzone){
					return subzone;
				}
			}
		}
	}
	return 0;
}


//--------------------------------------------------- Operator Overloading


//---------------------------------------------- Constructors - destructor

//Constructor
UVMesh::UVMesh(MeshModel* m, const char* filename){
	//Intensity container
	evalIntensityList = new vector<EvaluatedIntensity*>;
	totalEvalIntensity = new EvaluatedIntensity;
	summaryEvalIntensityList = new vector<EvaluatedIntensity*>;
	summaryTotalEvalIntensity = new EvaluatedIntensity;

	//Face intensities containes
	//FACE INTENSITIES -- NOT CURRENTLY USED
	//faceIntensities = 0;
	
	//Bounding boxes container
	subBoxes = new map<float,subBoundingBox*>;
	//Dummy values for BBoxes
	BBoxSubdivX = 1;
	BBoxSubdivY = 1;
	BBoxSubdivZ = 1;
	
	//load the model
	model = m;
	//init the model
	initModel(filename, false);
	
	//Calculate the model bounding box
	vcg::tri::UpdateBounding<CMeshO>::Box(model->cm);
	
	//Set optional components pointers to null as default
	posturePOIs = 0;
	zones = 0;

	
}

UVMesh::UVMesh(MeshDocument *doc, const char* fileName){
	//Intensity container
	evalIntensityList = new vector<EvaluatedIntensity*>;
	totalEvalIntensity = new EvaluatedIntensity;
	summaryEvalIntensityList = new vector<EvaluatedIntensity*>;
	summaryTotalEvalIntensity = new EvaluatedIntensity;

	//Face intensities containes
	//FACE INTENSITIES -- NOT CURRENTLY USED
	//faceIntensities = 0;
	
	//Bounding boxes container
	subBoxes = new map<float,subBoundingBox*>;
	//Dummy values for BBoxes
	BBoxSubdivX = 1;
	BBoxSubdivY = 1;
	BBoxSubdivZ = 1;
	
	fullname = fileName;

	//Load the model from the file
	model = IOFile::loadFromFile(fileName, doc);
	//init the model
	initModel(fileName, false);
	
	//Calculate the model bounding box
	vcg::tri::UpdateBounding<CMeshO>::Box(model->cm);
	
	//Set optional components pointers to null as default
	posturePOIs = 0;
	zones = 0;

	
}

//Protected Constructor (for derived classes that don't load a model from a file)
UVMesh::UVMesh(){
	//Intensity container
	evalIntensityList = new vector<EvaluatedIntensity*>;
	totalEvalIntensity = new EvaluatedIntensity;
	summaryEvalIntensityList = new vector<EvaluatedIntensity*>;
	summaryTotalEvalIntensity = new EvaluatedIntensity;

	//Face intensities containes
	//FACE INTENSITIES -- NOT CURRENTLY USED
	//faceIntensities = 0;
	
	//Bounding boxes container
	subBoxes = new map<float,subBoundingBox*>;
	
	path = 0;
	extension = 0;
	name = 0;
	fullname = 0;
	
	//Set optional components pointers to null as default
	posturePOIs = 0;
	zones = 0;
	
}

//Destructor
UVMesh::~UVMesh( )
{	
	deleteBoundingBoxes();

	if(extension) delete[] extension;
	if(name) delete[] name;
	if (path) delete[] path;
	Helpers::clearEvaluatedIntensities(&evalIntensityList);
	delete evalIntensityList;
	if(totalEvalIntensity)
	{
		if(totalEvalIntensity->intensityList)
		{
			totalEvalIntensity->intensityList->clear();
			delete totalEvalIntensity->intensityList;
		}
		if(totalEvalIntensity->beginDate)
			delete totalEvalIntensity->beginDate;
		if(totalEvalIntensity->endDate)
			delete totalEvalIntensity->endDate;
		delete totalEvalIntensity;
	}
	Helpers::clearEvaluatedIntensities(&summaryEvalIntensityList);
	delete summaryEvalIntensityList;
	if(summaryTotalEvalIntensity)
	{
		if(summaryTotalEvalIntensity->intensityList)
		{
			summaryTotalEvalIntensity->intensityList->clear();
			delete summaryTotalEvalIntensity->intensityList;
		}
		if(summaryTotalEvalIntensity->beginDate)
			delete summaryTotalEvalIntensity->beginDate;
		if(summaryTotalEvalIntensity->endDate)
			delete summaryTotalEvalIntensity->endDate;
		delete summaryTotalEvalIntensity;
	}

	//delete model;
	
	//delete optional components when present
	if(posturePOIs){
		delete posturePOIs;
	}
	
	Helpers::deleteZones(&zones);
	
	//FACE INTENSITIES -- NOT CURRENTLY USED
	/*if(faceIntensities){
		for(map<CFaceO*,evaluatedIntensity*>::iterator faceIntensIter = faceIntensities->begin(); faceIntensIter!=faceIntensities->end();faceIntensIter++){
			delete faceIntensIter->second;
		}
		faceIntensities->clear();
		delete faceIntensities;
	}*/
	
}


//---------------------------------------------------------------- PRIVATE


//------------------------------------------------------ Protected Methods

string UVMesh::generateMapNameFromLvlPts(int srcLvlNb, int srcPtNb){

	string filename = path;
	filename += name;
	char temp[20];
	sprintf(temp, "_%ix%i_", srcLvlNb, srcPtNb);
	filename += temp;
	return filename;

}

void UVMesh::setColorsFromZones(vector<Zone*>* zonesToColor, float blueFromValue, float greenFromValue, float redFromValue, float redToValue, int sources){
	
	vector<Zone*>::iterator zonesIter;
	vector<Zone*>* subZones;
	vector<Zone*>::iterator subZonesIter;
	for(zonesIter = zonesToColor->begin(); zonesIter != zonesToColor->end(); ++zonesIter){
		subZones = (*zonesIter)->getSubZones();
		if(subZones){
			setColorsFromZones(subZones,blueFromValue, greenFromValue, redFromValue, redToValue, sources);
		}
		else{
			vector<CVertexO*>::iterator vertIter;
			for(vertIter = (*zonesIter)->getZoneVertices()->begin(); vertIter != (*zonesIter)->getZoneVertices()->end(); ++vertIter){
				setVertexColor(*vertIter, (*zonesIter)->getEvaluatedIntensity()->intensityList->front(), blueFromValue, greenFromValue, redFromValue, redToValue, sources);
			}
		}
	}
}

void UVMesh::setVertexColor(CVertexO *vert, const Intensity &intensities, float blueFromValue, float greenFromValue, float redFromValue, float redToValue, int sources){
	OldApi::SetRGB(vert->C(),0,0,0);
	vert->C()[3] = 255;

	//Get the vertex intensity based on the sources to take into account
	float intensity = 0;
	if(sources & IntensitySources::DIRECT){
		intensity += intensities.direct;
	}
	if(sources & IntensitySources::DIFFUSED){
		intensity += intensities.diffused;
	}
	if(sources & IntensitySources::REFLECTED){
		intensity += intensities.reflected;
	}

	if(intensity > redToValue){
		vert->C()[0] = 255;
		vert->C()[1] = 255;
		vert->C()[2] = 255;
	}

	else{
		int color = intensity*255/redToValue;

		if(intensity > redFromValue){
			vert->C()[0] = color;
		}
		else if(intensity > greenFromValue){
			vert->C()[1] = color;
		}
		else if(intensity > blueFromValue){
			vert->C()[2] = color;
		}
	}
}

//FACE INTENSITIES -- NOT CURRENTLY USED
/*void UVMesh::setFaceIntensities(){
	
	//Create container
	faceIntensities = new map<CFaceO*,evaluatedIntensity*>;
	
	//Create a map of the intensities based on vertex address
	VertexIntensities vertexIntensities;
	vector<CVertexO>::iterator vertIter;
	IntensityList::iterator intensIter = evalIntensityList->back()->intensityList->begin();
	
	for(vertIter=model->cm.vert.begin(); vertIter != model->cm.vert.end(); vertIter++){
		vertexIntensities[&(*vertIter)] = &(*intensIter);
		intensIter++;
	}
	
	//Iterate through the faces and set their intensities
	vector<CFaceO>::iterator faceIter;
	int i;
	Date* beginDate=evalIntensityList->back()->beginDate;
	Date* endDate=evalIntensityList->back()->endDate;
	for(faceIter=model->cm.face.begin();faceIter!=model->cm.face.end();faceIter++){
		evaluatedIntensity *faceIntensity = new evaluatedIntensity;
		faceIntensity->area = vcg::DoubleArea<CFaceO>((*faceIter)) / 2.0;
		faceIntensity->beginDate = beginDate;
		faceIntensity->endDate = endDate;
		faceIntensity->intensityList = new IntensityList;
		for(int i=0;i<3;i++){
			Intensity *vertexIntensity = vertexIntensities[(faceIter->V(i))];
			faceIntensity->intensityList->push_back(*vertexIntensity);
		}
		(*faceIntensities)[&(*faceIter)] = faceIntensity;
		
	}
	
}*/

void UVMesh::tagVerticesForPOI(vcg::Point3<float> &pointCoords, CVertexO *vertex, float maxDistance){

	float distance;
	
	//Tag the vertex
	vertex->C()[3] = 100;	

	//Loop over all faces around that vertex
	vcg::face::VFIterator<CFaceO> vfi(vertex);
	for(;!vfi.End();++vfi)
	{
 		CFaceO* f = vfi.F();
 		for(int j=0;j<3;j++){
 			if(f->V(j)->C()[3] != 100){
				distance = (pointCoords-f->V(j)->P()).Norm();
		
				if(distance <= maxDistance){

				//tag the vertex recursively
				tagVerticesForPOI(pointCoords, f->V(j), maxDistance);
				}
			}
		}
	}
}

CFaceO* UVMesh::findPOIFace(POI point, float allowedDiff)
{
	//Prepare necessary variables to ease the calculation
	vcg::Point3<float> P, A, B, C, PA, PB, PC;
	float normPA, normPB, normPC, angle;
	float targetAngle = 2.0*M_PI;
	float angleDiff = 1.0f;
	P = point.getPoint();
	CFaceO *bestFace;
	vector<CFaceO>::iterator faceIter= model->cm.face.begin();
	
	//Loop over all the Mesh faces 
	//For each face, look whether the sum of the angles 
	//Between the face vertices and the POI = 2PI
	//Always keep the best result
	while(faceIter!=model->cm.face.end())
	{			
		//Check that the face was not deleted
		if(!(*faceIter).IsD())
		{			
			A = (*faceIter).V(0)->P();
			B = (*faceIter).V(1)->P();
			C = (*faceIter).V(2)->P();
			
			PA = A-P;
			PB = B-P;
			PC = C-P;
			
			normPA = PA.Norm();
			normPB = PB.Norm();
			normPC = PC.Norm();
			
			angle = acos((PA*PB)/(normPA*normPB));
			angle += acos((PA*PC)/(normPA*normPC));
			angle += acos((PB*PC)/(normPB*normPC));		
			
			angle = fabs(angle-targetAngle);
			
			//Keep the smallest Diff
			if(angle < angleDiff){
				angleDiff = angle;
				bestFace = &(*faceIter);
			}				
		}
		faceIter++;					
	}
	//If the best face found is less then the
	//allowedDiff difference, return it
	//Otherwise return 0
	if(angleDiff <= (targetAngle*allowedDiff)){
		return bestFace;
	}	
	return 0;
}

void UVMesh::setVerticesForZone(Zone *zone){
	
	//Enable and reload the necessary Mesh Topology elements
	//FACE INTENSITIES -- NOT CURRENTLY USED
	/*	model->clearDataMask(MeshModel::MM_VERTFACETOPO);
		model->updateDataMask(MeshModel::MM_VERTFACETOPO);
	*/
	
	//If the zone contains subzones, make sure to set
	//the vertices for all subzones
	vector<Zone*> *subZones = zone->getSubZones();
	if(subZones){
		vector<Zone*>::iterator subZoneIter;
		for(subZoneIter = subZones->begin(); subZoneIter!=subZones->end();subZoneIter++){
			setVerticesForZone(*subZoneIter);
		}	
	}	
	
	//If the zone contains a color, link it with
	//all vertices of that color and their respective faces
	if(zone->getColor()){
		vcg::Color4b zoneColor = *(zone->getColor());
		vector<CVertexO>::iterator vertIter;
		vector<CVertexO*> *zoneVertices = new vector<CVertexO*>;
		for(vertIter = model->cm.vert.begin(); vertIter!=model->cm.vert.end();vertIter++){
			vcg::Color4b vertColor = vertIter->C();
			if(zoneColor == vertColor){
				//Add the vertice
				zoneVertices->push_back(&(*vertIter));	
				//Add the faces
				//FACE INTENSITIES -- NOT CURRENTLY USED
				/*vcg::face::VFIterator<CFaceO> vfi(&(*vertIter));
				for(;!vfi.End();++vfi)
				{
					zone->addZoneFace(vfi.F());
				}*/
			}
		}
		zone->setZoneVertices(zoneVertices);
	}
	
}

void UVMesh::deleteBoundingBoxes(){
	map<float,subBoundingBox*>::iterator boxIter;
	for(boxIter=subBoxes->begin(); boxIter!=subBoxes->end(); boxIter++)
	{
		vector<CFaceO*>::iterator boxFacesIter;
		for(boxFacesIter=boxIter->second->boxFaces->begin(); boxFacesIter!=boxIter->second->boxFaces->end(); boxFacesIter++)
		{
			delete *boxFacesIter;
		}
		delete boxIter->second->boxFaces;
		delete boxIter->second->containedFaces;
	}
	delete subBoxes;
}

string UVMesh::generateOutputFilename(Date *beginDate, Date *endDate)
{
		char buffer1[30] = "";
		if (beginDate)
		{
			sprintf(buffer1, " %u_%.2u_%.2u %.2uh%.2u", beginDate->getYear(),beginDate->getMonth(),
				beginDate->getDay(), beginDate->getHour(), beginDate->getMinute());
		}

		char buffer2[30] = "";
		if (endDate)
		{
			sprintf(buffer2, " to %u_%.2u_%.2u %.2uh%.2u", endDate->getYear(),endDate->getMonth(),
				endDate->getDay(), endDate->getHour(), endDate->getMinute());
		}
		
		return string(buffer1)+string(buffer2);
}

void UVMesh::initModel(const char* fileName, bool clearPreviousResults)
{
	
	//Retrieve the name, extension and path of the model file model for future use
	char* temp1 = strrchr((char*)fileName, '.');
	if (temp1 != NULL)
	{
		extension = new char[strlen(temp1)+1];
		strncpy(extension, temp1, strlen(temp1)+1);
	}
	else
	{
		extension = "";
	}

	char* temp2 = max(strrchr((char*)fileName, (int)'/'), strrchr((char*)fileName, (int)'\\'));
	if (temp2 != NULL)
	{
		temp2++; // we don t want the slash or backslash character in the name.
		name = new char[strlen(temp2)-strlen(temp1)+1];
		strncpy(name, temp2, strlen(temp2)-strlen(temp1));
		name[strlen(temp2)-strlen(temp1)] = '\0';
	}
	else
	{
		name = "";
	}

	path = new char[strlen(fileName)-strlen(temp2)+1];
	strncpy(path, fileName, strlen(fileName)-strlen(temp2));
	path[strlen(fileName)-strlen(temp2)] = '\0';

	// we clear the vector evalIntensityList if set to
	if (clearPreviousResults)
		Helpers::clearEvaluatedIntensities(&evalIntensityList);
}

