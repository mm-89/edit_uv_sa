/*************************************************************************
                           IOPoints  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//---------- Class <Date> (fichier IOPoints.cpp) ----------

#include "IOPoints.h"

//----------------------------------------------------------------- PUBLIC

//--------------------------------------------------------- Public Methods

vector<POI>* IOPoints::parseXMLFile(const char* filename){
	
	//Prepare containers for the values to parse
	vector<POI> *POIs = new vector<POI>;
	int active;
	float x,y,z;
	std::string name; 
	bool ok;
	bool coordsOK;

	QDomDocument doc;
	//********************************
	// Read the DOM tree form file
	//********************************
	QFile f(filename);
	f.open(QIODevice::ReadOnly);
	doc.setContent(&f);
	f.close();
	//********************************
	// Parse the DOM tree
	//********************************
	QDomElement root=doc.documentElement();
	// We traverse the protections
	QDomElement child=root.firstChildElement();
	while(!child.isNull())
	{
		if (child.tagName() == "point")
		{
			//Check if the POI is active
			active = child.attribute("active").toInt(&ok);
			if(ok && active){

				//Parse the coordinates
				coordsOK = true;

				x = child.attribute("x").toFloat(&ok);
				if(!ok){
					coordsOK = false;
				}

				y = child.attribute("y").toFloat(&ok);
				if(!ok){
					coordsOK = false;
				}

				z = child.attribute("z").toFloat(&ok);
				if(!ok){
					coordsOK = false;
				}
				
				if(coordsOK){
					//Parse the name
					name = child.attribute("name","unknown").toStdString();

					POIs->push_back(POI(vcg::Point3<float>(x,y,z),name));
				}
			}

		}
		child = child.nextSiblingElement();
	}

   //Return the points
   return POIs;
	
}

void IOPoints::exportIntensitiesCSV(vector<POI>* points, Intensity *flatSurfaceIntensities,string filename){
	string name = filename;
	vector<POI>::iterator pointsIter;
	EvaluatedIntensity *evalIntensity;
	
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
	if (file.is_open())
	{
		//Print the header
		file << "POI Name,Vertex Num,Start Time,End Time,Total Intensity Received [J/m2],Diffuse Intensity Received [J/m2],Direct Intensity Received [J/m2],Reflected Intensity Received [J/m2],Total Intensity Received [%ambiant],Diffuse Intensity Received [%ambiant],Direct Intensity Received [%ambiant],Reflected Intensity Received [%ambiant]\n";
		
		//Loop through each of the intensities and  print them
		char buffer [500];
		float totalIntensity = 0.0;
		float diffuseIntensity = 0.0;
		float directIntensity = 0.0;
		float reflectedIntensity = 0.0;
		IntensityList::iterator intensity;
					
		for (pointsIter=points->begin(); pointsIter!=points->end(); pointsIter++)
		{	
			evalIntensity = (*pointsIter).getEvaluatedIntensity();
			if(evalIntensity){
				//strftime (buffer,500,"%x %X",&(evalIntensity->beginDate->getDate()));//MODIFY
				string startDate(buffer);
				//strftime (buffer,500,"%x %X",&(evalIntensity->endDate->getDate()));//MODIFY
				string endDate(buffer);	
				string pointName = (*pointsIter).getPointName();
				int vertexNum = 1;
				
				for(intensity = evalIntensity->intensityList->begin(); intensity != evalIntensity->intensityList->end(); intensity++){
					
					diffuseIntensity = intensity->diffused;
					directIntensity = intensity->direct;
					reflectedIntensity = intensity->reflected;
					totalIntensity = diffuseIntensity + directIntensity + reflectedIntensity;
					file << pointName;
					sprintf (buffer, ",%i", vertexNum);
					file << string(buffer) << "," << startDate << "," << endDate << ",";
					sprintf (buffer, "%E,%E,%E,%E,", totalIntensity, diffuseIntensity, directIntensity, reflectedIntensity);
					file << string(buffer);
					sprintf (buffer, "%E,%E,%E,%E\n", (totalIntensity/flatTotal)*100, (diffuseIntensity/flatDiffuse)*100, (directIntensity/flatDirect)*100, (reflectedIntensity/flatReflected)*100);
					file << string(buffer);
					++vertexNum;
				
				}
			}
		}
		
		file.close();
	}
	else cout << "Unable to open file";
}



//---------------------------------------------------------------- PRIVATE

//------------------------------------------------------ Protected Methods
