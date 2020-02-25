/*************************************************************************
                           IOZones  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

#include "IOZones.h"

//Initialize static flatsurface exposition vars
float IOZones::flatTotal = -1.0;
float IOZones::flatDiffuse = -1.0;
float IOZones::flatDirect = -1.0;
float IOZones::flatReflected = -1.0;

//Set the prefix for subzones
std::string IOZones::subZonePrefix = "   ";


vector<Zone*>* IOZones::parseXMLFile(const char* filename){

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
	//Initialize vector to hold zones
	vector<Zone*>* zones = new vector<Zone*>;
	QDomElement root=doc.documentElement();
	// We traverse the protections
	QDomElement child=root.firstChildElement();
	while(!child.isNull())
	{
		if (child.tagName() == "Zone")
		{
		  zones->push_back(parseZone(child));
		}
		else if(child.tagName() == "ComposedZone"){
			zones->push_back(parseComposedZone(child));
		}
		child = child.nextSiblingElement();
	}

	//return the zones
	return zones;

}

void IOZones::saveZonesFile(QTreeWidget* zonesTree, string filename){

	//Create root XML element
	QDomDocument doc("zones");
	QDomElement rootNode = doc.createElement("Zones");
	doc.appendChild(rootNode);

	//Loop over all top-level zones and add them
	for(int i = 0; i < zonesTree->topLevelItemCount(); i++){
		QDomElement zoneNode = createZoneElement(zonesTree->topLevelItem(i),doc);
		rootNode.appendChild(zoneNode);
	}

	//Write the XML file
	std::ofstream file(filename.c_str());
	if (file.is_open())
	{
		//Print the header
		file << doc.toString().toStdString();
		
	}
	else{
		//MessageBox(NULL, L"Failed to open file.", L"Error", 0x10000); //MODIFY
		}

}

void IOZones::exportIntensitiesCSV(vector<Zone *> *zones, Intensity *flatSurfaceIntensities, string filename, IOZones::OutputStreamMap &outputStreams)
{
	//Load flat surface intensities
	if(flatSurfaceIntensities){
		flatDiffuse = flatSurfaceIntensities->diffused;
		flatDirect  = flatSurfaceIntensities->direct;
		flatReflected = flatSurfaceIntensities->reflected;
		flatTotal = flatDiffuse + flatDirect + flatReflected;
	}

	if(filename.substr(filename.size() - 4) == ".csv")
		filename.resize(filename.size() - 4);

	for(auto zoneIter = zones->begin(); zoneIter != zones->end(); zoneIter++)
		exportIntensitiesCSV(*zoneIter, filename, outputStreams);
}

QDomElement IOZones::createZoneElement(QTreeWidgetItem* item, QDomDocument doc){
	QDomElement zoneNode;

	//If the zone is a composed zone, add the subzones recursively
	if(item->childCount() > 0){
		zoneNode = doc.createElement("ComposedZone");
		zoneNode.setAttribute("name",item->text(0));
		for(int i = 0; i < item->childCount(); i++){
			QDomElement childZoneNode = createZoneElement(item->child(i),doc);
			zoneNode.appendChild(childZoneNode);
		}
	}
	//If the zone is final, then set the colors
	else{
		zoneNode = doc.createElement("Zone");
		zoneNode.setAttribute("name",item->text(0));
		zoneNode.setAttribute("red",item->text(2));
		zoneNode.setAttribute("green",item->text(3));
		zoneNode.setAttribute("blue",item->text(4));
	}
	
	//Set the active state
	if(item->checkState(0) == Qt::Checked){
		zoneNode.setAttribute("active","1");
	}
	else{
		zoneNode.setAttribute("active","0");
	}

	return zoneNode;
}

bool IOZones::exportIntensitiesCSV(Zone *zone, string filenameWithoutExt, IOZones::OutputStreamMap &outputStreams)
{
	filenameWithoutExt.append("_" + zone->getName());
	const std::string filename(filenameWithoutExt + ".csv");

	// get file handle
	auto *ptr = &outputStreams[filename];
	bool isNewFile = !ptr->data();
	if(isNewFile)
	{
		ptr->reset(new std::ofstream(filename));
		if(!ptr->data()->is_open())
		{
		//	MessageBox(NULL, L"Failed to open zones file.", L"Error", 0x10000);//Modify
			return false;
		}
	}
	std::ofstream &file = *ptr->data();

	// print header
	if(isNewFile)
		file << "Start Time,End Time,Area[cm2],IP,Total Intensity Received [J/m2],Diffuse Intensity Received [J/m2],Direct Intensity Received [J/m2],Reflected Intensity Received [J/m2]"
				//",Total Intensity Received [%ambiant],Diffuse Intensity Received [%ambiant],Direct Intensity Received [%ambiant],Reflected Intensity Received [%ambiant]"
				"\n";

	// print zone intensities
	EvaluatedIntensity *evalIntensity = zone->getEvaluatedIntensity();
	if(evalIntensity){
		char buffer [500];
		//strftime (buffer,500,"%x %X",&(evalIntensity->beginDate->getDate()));//MODIFY
		string startDate(buffer);
		//strftime (buffer,500,"%x %X",&(evalIntensity->endDate->getDate()));//MODIFY
		string endDate(buffer);

		for(auto intensity = evalIntensity->intensityList->begin(); intensity != evalIntensity->intensityList->end(); intensity++){
			float diffuseIntensity = intensity->diffused;
			float directIntensity = intensity->direct;
			float reflectedIntensity = intensity->reflected;
			float totalIntensity = diffuseIntensity + directIntensity + reflectedIntensity;
			file << startDate << "," << endDate << "," << zone->getSurface()*100.0 << "," << zone->getIP() << ",";
			sprintf (buffer, "%E,%E,%E,%E,", totalIntensity, diffuseIntensity, directIntensity, reflectedIntensity);
			file << string(buffer);
			//if(flatTotal > 0){
			//	sprintf (buffer, "%E,%E,%E,%E\n", (totalIntensity/flatTotal)*100, (diffuseIntensity/flatDiffuse)*100, (directIntensity/flatDirect)*100, (reflectedIntensity/flatReflected)*100);
			//	file << string(buffer);
			//}
			//else{
			//	file << "NA,NA,NA,NA\n";
			//}
			file << "\n";
		}
	}


	// process subzones
	if(auto subZones = zone->getSubZones())
	{
		for(auto zoneIter = subZones->begin(); zoneIter != subZones->end(); zoneIter++)
			if(!exportIntensitiesCSV(*zoneIter, filenameWithoutExt, outputStreams))
				return false;
	}
	return true;
}

Zone* IOZones::parseComposedZone(QDomElement element){

	//Parse whether the zone is active
	bool ok;
	int activeVal;
	bool active = false;
	activeVal = element.attribute("active").toInt(&ok);
	if(ok && activeVal){
		active = true;
	}

	//Parse the zone name
	string name = element.attribute("name","unknown").toStdString();

	//Create the Composed zone
	Zone *composedZone = new Zone(name, active);

	//Parse the subzones
	QDomElement child = element.firstChildElement();
	while(!child.isNull())
	{
		if (child.tagName() == "Zone")
		{
			composedZone->addSubZone(parseZone(child));
		}
		else if(child.tagName() == "ComposedZone"){
			composedZone->addSubZone(parseComposedZone(child));
		}
		child = child.nextSiblingElement();
	}
	return composedZone;	
}

Zone* IOZones::parseZone(QDomElement element){

	//Parse the zone name
	string name = element.attribute("name","unknown").toStdString();

	//Parse the zone colors
	int red, green, blue;
	bool ok;
	bool colorsOK = true;
	if(element.hasAttribute("red")){
		red = element.attribute("red").toInt(&ok);
		if(!ok){
			red = 0;
			colorsOK = false;
		}
	}
	else{
		red = 0;
		colorsOK = false;
	}

	if(element.hasAttribute("green")){
		green = element.attribute("green").toInt(&ok);
		if(!ok){
			green = 0;
			colorsOK = false;
		}
	}
	else{
		green = 0;
		colorsOK = false;
	}

	if(element.hasAttribute("blue")){
		blue = element.attribute("blue").toInt(&ok);
		if(!ok){
			blue = 0;
			colorsOK = false;
		}
	}
	else{
		blue = 0;
		colorsOK = false;
	}

	//Parse whether the zone is active
	//If there was any problem parsing the colors
	//The zone is set inactive to avoid side-effects
	bool active = false;
	if(colorsOK){
		int activeVal;
		if(element.hasAttribute("active")){
			activeVal = element.attribute("active").toInt(&ok);
			if(ok && activeVal){
				active = true;
			}
		}
	}
   
   //Return the new zone
   return new Zone(name,active, new vcg::Color4b(red,green,blue,color4));
	
}
