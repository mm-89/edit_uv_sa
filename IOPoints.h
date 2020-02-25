/*************************************************************************
                           IOPoints  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

#if ! defined ( IOPOINTS_H )
#define IOPOINTS_H

//-------------------------------------------------------- Used Interfaces
#include <iostream>
#include <vector>
#include "strConv.h"
#include "POI.h"
#include <fstream>
#include <qdom.h>
#include <QtGui> //MODIFICATED from qtgui
using namespace std;

//------------------------------------------------------------------------ 
//IOPoints takes care of the IO operations for POIs.
class IOPoints
{
//----------------------------------------------------------------- PUBLIC

public:
//--------------------------------------------------------- Public Methods
static vector<POI>* parseXMLFile(const char* filename);

//Exports the received intensities as a CSV file
static void exportIntensitiesCSV(vector<POI>* points, Intensity *flatSurfaceIntensities=0,string filename = "Points_result.csv");

//--------------------------------------------------- Operator Overloading



//---------------------------------------------- Constructors - destructor

IOPoints();

virtual ~IOPoints ( );
//---------------------------------------------------------------- PRIVATE

//------------------------------------------------------ Protected Methods

};

#endif // IOPOINTS_H




