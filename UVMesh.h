/*************************************************************************
                           UVMesh  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

#if ! defined ( UVMESH_H )
#define UVMESH_H

//-------------------------------------------------------- Used Interfaces

//For now include Meshmodel. Maybe later include only VCG required interfaces.
//#include "hemisphericalSrc.h"
//#include "ptSrc.h"
#include "date.h"
#include "intensityMap.h"
#include <common/meshmodel.h>
#include "IOFile.h"
#include "strConv.h"
//#include "comdef.h" //ONLY for Windows
#include "intensities.h"
#include "IOPoints.h"
#include "IOZones.h"
#include "protections.h"
#include <vcg/complex/algorithms/update/normal.h>
#include <vcg/complex/algorithms/update/bounding.h>
#include <vcg/complex/allocate.h>
#include <vcg/space/point3.h> 
#include <time.h>
#include <vector>
#include <map>
#include <string>

//-------------------------------------------------------------- Constants 

//------------------------------------------------------------------ Types 

//Struct holding a bounding box
typedef struct
{
	vector<CFaceO*>* boxFaces;
	vector<CFaceO*>* containedFaces;
} subBoundingBox;

//------------------------------------------------------------------------ 

//------------------------------------------------------------------------ 
//The UVMesh contains the actual Mesh used for a SimUVEx simulation and
//provides the interface and necessary methods to access and use the
//Mesh.
//Note that some of the functionnalities are only available if the Mesh
//fulfills certain requirements. Please have a look at the different
//Method description for information about assumptions prior to using
//them.
class UVMesh
{
public:

	//Accessors
	inline IntensityMap& getDiffusedIntensityMap(){return diffusedIntensityMap;}

	inline IntensityMap& getReflectedIntensityMap(){return reflectedIntensityMap;}

	inline void push_backEvaluatedIntensity(EvaluatedIntensity* evaluatedIntensity)
	{
		evalIntensityList->push_back(evaluatedIntensity);
		if(summaryEvalIntensityList->empty())
			summaryEvalIntensityList->push_back(new EvaluatedIntensity(*evaluatedIntensity));
		else
			*summaryEvalIntensityList->back() += *evaluatedIntensity;
	}

	void evaluateTotalEvaluatedIntensity(bool summary);

	void clearEvaluatedIntensities();

	inline vector<EvaluatedIntensity*>* getEvaluatedIntensityList(){return evalIntensityList;}

	inline vcg::Box3f getMeshBoundingBox(){return model->cm.trBB();}

	inline map<float,subBoundingBox*>* getSubBoxes(){return subBoxes;}

	inline MeshModel* getModel(){return model;}

	inline vector<CFaceO>* getModelFaces(){return &(model->cm.face);}

	inline vector<CVertexO>* getModelVertices(){return &(model->cm.vert);}

	inline const char* getModelFilename(){return fullname;}

	inline const char* getModelName(){return name;}

	inline int getNumPOIs(){return posturePOIs->size();}

	inline vector<Zone*>* getZones(){return zones;}

	inline bool hasPOIs() const { return posturePOIs != 0; }

	inline bool hasZones() const { return zones != 0; }

	//Initialization methods
	//Note: Maybe all these should be combined in an init method.

	//Inserts POIs in the Mesh from a valid XML PP file
	void setPOIs(const char *filename);

	//Evaluates intensities for all given POIs
	void evaluatePOI(float maxDistance, bool useBaseVertices);

	inline vector<POI>* getPOIs(){return posturePOIs;}

	//Setup the anatomical zones based on XML file
	//IMPORTANT: THIS SHOULD BE DONE AFTER INSERTING THE POI (If POI + zones are needed)
	//Important2: This can only work if the virtual manequin contains proper color coding
	void setZones(const char *filename);

	//Evaluates the irradiation of the different zones based on the values calculated
	//for this posture
	void evaluateZones();

	//Sets protection factors based on protections loaded in Protections lib
	//The boolean reset defines whether previous protections applied should be removed
	//IMPORTANT NOTE: If the protections rely on a protections library, the library should be loaded first
	//using Protections::loadProtectionsLib
	void setProtections(bool reset = true);

	//Removes all protections from the zones
	void resetProtections();

	//Recreate the bounding boxes
	void reloadBoundingBoxes();

	//Creates the bounding boxes based on the number of subdivision specified
	void createBoundingBoxes(int Xsubdiv, int Ysubdiv, int Zsubdiv);

	//Load the intensity maps
	bool loadDiffuseIntensityMap(string filename);
	bool loadDiffuseIntensityMap(int diffuseSrcLvlNb, int diffuseSrcPtNb);
	bool loadReflectedIntensityMap(string filename);
	bool loadReflectedIntensityMap(int reflectedSrcLvlNb, int reflectedSrcPtNb);

	//Sets the colors to the mesh with the evaluated intensities
	//maxValue specifies the maxValue to be used in the scale
	//if setColorsAboveMaxWhite is set, then any value above the max value is set to white,
	//Otherwise it will be set to bright red (highest color of the general scale)
	void setColors(float blueFromValue, float greenFromValue, float redFromValue, float redToValue, int sources = 0x00000007);

	//Version of setColors that colors based on zones intensities rather than vertices intensities
	void setColorsFromZones(float blueFromValue, float greenFromValue, float redFromValue, float redToValue, int sources = 0x00000007);

	//Static version of setColors, letting the user pass the desired Mesh to color (The mesh and intensities need to correspond!)
	//Can be used to color a copy of the Mesh if the original needs to be kept untouched
	static void setColors(MeshModel *m, vector<EvaluatedIntensity*>* intensities, float blueFromValue, float greenFromValue, float redFromValue, float redToValue, int sources = 0x00000007);

	//Saves the Mesh to file with the colors set.
	//Note that currently it will only work properly for "separate = false".
	//i.e., it doesn't support multiple time slices.
	//Also, it should be seen whether the colors should be assigned before it is run or not
	void saveMeshToFile(string filename = "");

	//Exports the received intensities as a CSV file
	void exportIntensitiesCSV(Intensity *flatSurfaceIntensities=0,string filename = "");

	//Exports the POI received intensities as a CSV file
	void exportPOIIntensitiesCSV(Intensity *flatSurfaceIntensities=0,string filename = "Points_result.csv");

	//Exports the Zones received intensities as a CSV file
	void exportZonesIntensitiesCSV(Intensity *flatSurfaceIntensities, string filename, IOZones::OutputStreamMap &outputStreams);

	//Multiplies each intensity by the given factor
	//(Used for averages mainly)
	void multiplyIntensities(float factor);

	//Merges the given zones intensities into the current zones intensities
	//to the same values to keep the Mesh integrity intact
	void mergeZonesIntensities(vector<Zone*>* zonesAdd);

	//Multiplies each of the zones received intensities by a constant
	//to the same values to keep the Mesh integrity intact
	//(Used for averages)
	void multiplyZonesIntensities(float factor);

	//Finds a zone by its name and returns a pointer to it
	Zone* findZoneByName(string name, vector<Zone*>* searchZones = 0);


	//--------------------------------------------------- Operator Overloading


	//---------------------------------------------- Constructors - destructor

	UVMesh(MeshDocument *doc, const char* fileName);
	UVMesh(MeshModel* m, const char* filename);

	~UVMesh();

protected:
	//Protected Constructor for derived classes that don't need to load the
	//Mesh from a file
	//Note that they should then create the model and
	//Calculate the model bounding box using vcg::tri::UpdateBounding<CMeshO>::Box(model->cm);
	//themselves
	UVMesh();

	//------------------------------------------------------ Protected Methods

	//Sets the colors from the given zones
	void setColorsFromZones(vector<Zone*>* zonesToColor, float blueFromValue, float greenFromValue, float redFromValue, float redToValue, int sources = 0x00000007);

	//Set one vertex color
	static void setVertexColor(CVertexO *vert, const Intensity &intensity, float blueFromValue, float greenFromValue, float redFromValue, float redToValue, int sources = 0x00000007);

	//Creates a default name for the intensity maps based on the number of levels and points
	string generateMapNameFromLvlPts(int srcLvlNb, int srcPtNb);

	//Set the face intensities based on their vertices
	//FACE INTENSITIES -- NOT CURRENTLY USED
	//void UVMesh::setFaceIntensities();

	//tag vertices around given vertice for a given POI
	void tagVerticesForPOI(vcg::Point3<float> &pointCoords, CVertexO *vertex, float maxDistance);

	//Finds the face where the POI lies. Takes an optional parameter defining the acceptable
	//margin of error (%)
	CFaceO* findPOIFace(POI point, float allowedDiff = 0.01);

	//Sets all the vertices for a given zone based on the color
	void setVerticesForZone(Zone *zone);

	//Delete all the bounding boxes
	void deleteBoundingBoxes();

	//Load a new Mesh from a file and optionally clears out previous results.
	//Note that the preferred method is still to load the Mesh at construction time.
	//Set to private for now since the Mesh really shouldn't be reloaded
	void initModel(const char* fileName, bool clearPreviousResults = true);

	//Gets a meaningful filename based on dates and resolution
	string generateOutputFilename(Date *beginDate=0, Date *endDate=0);

	//--------------------------------------------------- Protected Attributes

	//Intensity Maps
	IntensityMap diffusedIntensityMap;
	IntensityMap reflectedIntensityMap;

	//Evaluated intensities
	vector<EvaluatedIntensity*>* evalIntensityList;
	EvaluatedIntensity* totalEvalIntensity;

	//Total evaluated intensities
	vector<EvaluatedIntensity*>* summaryEvalIntensityList;
	EvaluatedIntensity* summaryTotalEvalIntensity;

	//FACE INTENSITIES -- NOT CURRENTLY USED
	//map<CFaceO*,evaluatedIntensity*>* faceIntensities;

	//The Mesh
	MeshModel* model;

	//The Bounding Boxes
	// The sub-bounding boxes container
	map<float,subBoundingBox*>* subBoxes;
	//The number of Subdivs
	int BBoxSubdivX;
	int BBoxSubdivY;
	int BBoxSubdivZ;

	// Information on the file being read
	char* path;
	char* name;
	char* extension;
	const char* fullname;

	//Points of Interest
	vector<POI>* posturePOIs;

	//Anatomical zones
	vector<Zone*>* zones;
};

#endif // UVMESH_H

