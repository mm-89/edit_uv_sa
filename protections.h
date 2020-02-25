/*************************************************************************
                           Protection  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

#if ! defined ( PROTECTIONS_H )
#define PROTECTIONS_H

//-------------------------------------------------------- Used Interfaces

#include <iostream>
#include <vector>
#include "strConv.h"
#include "zone.h"
#include <fstream>
#include <map>
#include <qdom.h>
#include <QTreeWidgetItem>
#include <qfile.h>

using std::map;
using std::vector;
using std::string;
using std::pair;

//------------------------------------------------------------------------ 
//Struct to store a Protection
struct Protection
{
		string material;
		int IP;
		string name;
		int zoneType;
		bool active;
};

//The IOProtections Class handles the protection parts of the model.
class Protections
{
public:
	//Loads a protections definition library
	static void loadProtectionsLib(const char* filename, bool replace = false);

	//Loads a protections file
	static void loadProtections(const char* filename, bool replace = true);

	//Applies the protections previously loaded
	static void applyProtections(vector<Zone*>* zones);

	//Returns the Materials
	static map<string,int>* getMaterials(){return materials;}

	//Returns the clothes
	static map<string,vector<string>*>* getClothes(){return clothes;}

	//Returns the protections
	static vector<Protection>* getProtections(){return protections;}

	//Completely resets the protections library
	static void resetProtectionsLibrary();

	//Completely resets the protections
	static void resetProtections();

	//Saves the given protections library as XML
	static void saveProtectionsLibraryAsXML(QTreeWidget* clothesTree, QTreeWidget* materialsTree, string filename);

	//Saves the given protections as XML
	static void saveProtectionsAsXML(QTreeWidget* protections, string filename);

private:
	//------------------------------------------------------ Protected Methods

	//Parse all clothes in the protections library
	static void parseClothes(QDomElement clothesElement);
	//Parses all materials in the protections library
	static void parseMaterials(QDomElement materialsElement);
	//Applies a given protection to a given zone
	//Returns whether the zone was found
	static bool applyProtection(vector<Zone*>* zones, string zonename, int IP);
	//Creates a QDomElement for exporting XML
	static QDomElement createClothingElement(QTreeWidgetItem* item, QDomDocument doc);


	//--------------------------------------------------- Protected Attributes


	//Hold the protections library
	static map<string,int>* materials;
	static map<string,vector<string>*>* clothes;
	static vector<Protection>* protections;

public:
	//Constants for protection types
	static const int ZONETYPE = 0;
	static const int CLOTHINGTYPE = 1;
};

#endif // PROTECTIONS_H




