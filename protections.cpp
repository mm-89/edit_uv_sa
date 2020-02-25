/*************************************************************************
                           Protections  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

#include "protections.h"

//Initialize static maps to 0
map<string,int>* Protections::materials = 0;
map<string,vector<string>*>* Protections::clothes = 0;
vector<Protection>* Protections::protections = 0;

void Protections::loadProtectionsLib(const char* filename, bool replace){

	//Delete existing protections if any
	if(replace){
		resetProtectionsLibrary();
	}

	//Initialize the maps
	if(!materials){
		materials = new map<string,int>;
	}
	if(!clothes){
		clothes = new map<string,vector<string>*>;
	}

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
		if (child.tagName() == "Clothes")
		{
		  parseClothes(child);
		}
		else if(child.tagName() == "Materials"){
			parseMaterials(child);
		}
		child = child.nextSiblingElement();
	}
}

void Protections::loadProtections(const char* filename, bool replace){

	//Delete existing protections if any
	if(replace){
		resetProtections();
	}
	protections = new vector<Protection>;

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
	QDomElement child=root.firstChild().toElement();
	while(!child.isNull())
	{
	if (child.tagName() == "Protection")
	{
	  // We traverse clothing and zones
	  QDomElement grandChild=child.firstChild().toElement();
	  while(!grandChild.isNull())
	  {
		if (grandChild.tagName() == "Clothing" || grandChild.tagName() == "Zone")
		{
			Protection p;
			p.name = grandChild.attribute("name").toStdString();
			if(grandChild.tagName() == "Clothing"){
				p.zoneType = Protections::CLOTHINGTYPE;
			}
			else{
				p.zoneType = Protections::ZONETYPE;
			}
			if(child.hasAttribute("material")){
				p.material = child.attribute("material").toStdString();
				if(materials && materials->find(p.material)!=materials->end()){
					p.IP = (*materials)[p.material];
				}
				else{
					p.IP = 1;
				}
			}
			else{
				p.IP = child.attribute("IP").toInt();
			}
			if(child.attribute("active") == "1"){
				p.active = true;
			}
			else{
				p.active = false;
			}
			
			protections->push_back(p);
		}
		grandChild = grandChild.nextSibling().toElement();
	   }
	}
	child = child.nextSibling().toElement();
	}
}

void Protections::applyProtections(vector<Zone*>* zones){

	//Check that the zones exist and are not empty
	if(!zones || zones->size() < 1){
		return;
	}

	//Check that the protections list exists
	if(!protections){
		return;
	}
	
	//Apply all protections
	for(vector<Protection>::iterator proIter = protections->begin(); proIter != protections->end(); proIter++){
		//Check that the protection is active and that the IP is > 1
		if(proIter->active && proIter->IP > 1){
			//If it's a clothing, check that the clothing is loaded from a protection lib
			//and apply the IP to all the zones covered
			if(proIter->zoneType == Protections::CLOTHINGTYPE){
				vector<string>* protectedZones = (*clothes)[string(proIter->name)];
				if(protectedZones){
					vector<string>::iterator zoneIter;
					for(zoneIter = protectedZones->begin(); zoneIter != protectedZones->end(); ++zoneIter){
						applyProtection(zones, *zoneIter, proIter->IP);
					}
				}
			}
			//If it's a simple zone, just apply the IP
			else{
				applyProtection(zones,proIter->name, proIter->IP);
			}


		}
	}			

}

void Protections::resetProtectionsLibrary(){
	if(materials){
		delete materials;
		materials = 0;
	}
	if(clothes){
		for(map<string,vector<string>*>::iterator i = clothes->begin(); i != clothes->end(); i++){
			delete i->second;
		}
		clothes->clear();
		delete clothes;
		clothes = 0;
	}
	//Update the protections (reset all materials to IP=1)
	if(protections){
		vector<Protection>::iterator protIter;
		for(protIter = protections->begin(); protIter != protections->end(); protIter++){
			if(!protIter->material.empty()){
				protIter->IP = 1;
			}
		}
	}
}

void Protections::saveProtectionsLibraryAsXML(QTreeWidget* clothesTree, QTreeWidget* materialsTree, string filename){
	//Create root XML element
	QDomDocument doc("Protections");
	QDomElement rootNode = doc.createElement("Protections");
	doc.appendChild(rootNode);

	//Create the Clothes part
	QDomElement clothesNode = doc.createElement("Clothes");
	rootNode.appendChild(clothesNode);

	//Loop over all the clothes and add them
	for(int i = 0; i < clothesTree->topLevelItemCount(); i++){
		QDomElement clothingNode = createClothingElement(clothesTree->topLevelItem(i),doc);
		clothesNode.appendChild(clothingNode);
	}

	//Create the Materials part
	QDomElement materialsNode = doc.createElement("Materials");
	rootNode.appendChild(materialsNode);

	//Loop over all the clothes and add them
	for(int i = 0; i < materialsTree->topLevelItemCount(); i++){
		QDomElement materialNode = doc.createElement("Material");
		materialNode.setAttribute("name",materialsTree->topLevelItem(i)->text(0));
		materialNode.setAttribute("IP", materialsTree->topLevelItem(i)->text(1));
		materialsNode.appendChild(materialNode);
	}

	//Write the XML file
	std::ofstream file(filename.c_str());
	if (file.is_open())
	{
		file << doc.toString().toStdString();
	}
	else{
	//	MessageBox(NULL, L"Failed to open file.", L"Error", 0x10000);//MODIFY
		}

}

void Protections::resetProtections(){
	if(protections){
		delete protections;
		protections = 0;
	}
}

void Protections::saveProtectionsAsXML(QTreeWidget* protections, string filename){
	
	//Create root XML element
	QDomDocument doc("Protections");
	QDomElement rootNode = doc.createElement("Protections");
	doc.appendChild(rootNode);

	//Loop over all the clothes and add them
	QTreeWidgetItem *currentProtection;
	for(int i = 0; i < protections->topLevelItemCount(); i++){
		currentProtection = protections->topLevelItem(i);
		//Create the main protection node
		QDomElement protectionNode = doc.createElement("Protection");
		if(currentProtection->text(1).size() > 0){
			protectionNode.setAttribute("material",currentProtection->text(1));
		}
		else{
			protectionNode.setAttribute("IP",currentProtection->text(2));
		}
		if(currentProtection->checkState(0) == Qt::Checked){
			protectionNode.setAttribute("active","1");
		}
		else{
			protectionNode.setAttribute("active","0");
		}
		//Create the typed subnode
		QDomElement subNode = doc.createElement(currentProtection->text(3));
		subNode.setAttribute("name",currentProtection->text(0));

		//Append the nodes
		protectionNode.appendChild(subNode);
		rootNode.appendChild(protectionNode);
	}

	//Write the XML file
	std::ofstream file(filename.c_str());
	if (file.is_open())
	{
		file << doc.toString().toStdString();
	}
	else{
		//MessageBox(NULL, L"Failed to open file.", L"Error", 0x10000);//MODIFY
		}
}


QDomElement Protections::createClothingElement(QTreeWidgetItem* item, QDomDocument doc){
	QDomElement clothingNode = doc.createElement("Clothing");
	clothingNode.setAttribute("name",item->text(0));

	for(int i=0; i<item->childCount();i++){
		QDomElement zoneNode = doc.createElement("Zone");
		zoneNode.setAttribute("name",item->child(i)->text(0));
		clothingNode.appendChild(zoneNode);
	}

	return clothingNode;
}

void Protections::parseClothes(QDomElement clothesElement){

	//Parse and store each of the clothes
	QDomElement clothing = clothesElement.firstChildElement();
	while(!clothing.isNull()){
		if(clothing.tagName() == "Clothing" && clothing.hasAttribute("name")){
			//Parse the name of the zones protected by the clothing
			vector<string> *zones = new vector<string>;
			QDomElement zone = clothing.firstChildElement();
			while(!zone.isNull()){
				if(zone.tagName() == "Zone" && zone.hasAttribute("name")){
					zones->push_back(zone.attribute("name").toStdString());
				}
				zone = zone.nextSiblingElement();
			}			
			//Add the clothing to the clothes lib
			(*clothes)[clothing.attribute("name").toStdString()] = zones;

		}
		clothing = clothing.nextSiblingElement();
	}
}

void Protections::parseMaterials(QDomElement materialsElement){

	//Parse and store each of the materials
	QDomElement material = materialsElement.firstChildElement();
	while(!material.isNull()){
		if(material.tagName() == "Material" && material.hasAttribute("name")){
			//Check that the IP is a valid number
			bool ok;
			int IP = material.attribute("IP").toInt(&ok);
			if(ok && IP > 0){
				//Add the material to the clothes lib
				string name = material.attribute("name").toStdString();
				(*materials)[name] = IP;
				
				//Update the protections where the material is present
				if(protections){
					vector<Protection>::iterator protIter;
					for(protIter = protections->begin(); protIter != protections->end(); protIter++){
						if(protIter->material == name){
							protIter->IP = IP;
						}
					}
				}
			}
		}
		material = material.nextSiblingElement();
	}
	
}


bool Protections::applyProtection(vector<Zone*>* zones, string zonename, int IP){

	vector<Zone*>::iterator zoneIter;
	//Loop over all zones
	for(zoneIter = zones->begin(); zoneIter!=zones->end();++zoneIter){
		//If the zone found is the right one, apply the IP factor
		if((*zoneIter)->getName() == zonename){
			(*zoneIter)->setZoneIP(IP);
			return true;
		}
		//Otherwise, if the zone contains subzones, loop over the subzones
		else if((*zoneIter)->getSubZones()){
			if(applyProtection((*zoneIter)->getSubZones(), zonename, IP)){
				return true;
			}
		}
	}
	return false;
}

