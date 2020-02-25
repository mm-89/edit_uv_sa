/*************************************************************************
                           IOZones  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

#if ! defined ( IOZONES_H )
#define IOZONES_H

//-------------------------------------------------------- Used Interfaces

#include <iostream>
#include <vector>
#include "strConv.h"
#include "zone.h"
#include <fstream>
#include <qdom.h>
#include <QTreeWidgetItem>
using namespace std;

//------------------------------------------------------------------------ 

//The IOZones Class takes care of all the IO needs for the zones
class IOZones
{
public:
	// Map for output in multiple files without reopening the file handle
	typedef std::map<std::string, QSharedPointer<std::ofstream>> OutputStreamMap;

	//Parses a zone XML file
	static vector<Zone*>* parseXMLFile(const char* filename);

	//Saves a given QTreeWidget into an XML file
	static void saveZonesFile(QTreeWidget* zonesTree, string filename);

	//Exports the given zones intensities in a CSV file
	static void exportIntensitiesCSV(vector<Zone*>* zones, Intensity *flatSurfaceIntensities, string filename, OutputStreamMap &outputStreams);

private:
	//------------------------------------------------------ Protected Methods

	//Parses a zone
	static Zone* parseZone(QDomElement element);

	//Parses a composed zone
	static Zone* parseComposedZone(QDomElement element);

	//Create a zone DOM element from the item and according to the doc
	static QDomElement createZoneElement(QTreeWidgetItem* item, QDomDocument doc);

	//export a zone to CSV
	static bool exportIntensitiesCSV(Zone* zone, string filenameWithoutExt, OutputStreamMap &outputStreams);

	//Hold the flatsurface exposition values
	static float flatTotal;
	static float flatDiffuse;
	static float flatDirect;
	static float flatReflected;
	
	//Holds the prefix to add between each child and parent zone
	//for the export output
	static std::string subZonePrefix;
	
	//Constant to set the 4th color channel
	static const int color4 = 255;
};

#endif // IOZONES_H




