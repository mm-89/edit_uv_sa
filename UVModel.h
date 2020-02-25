/*************************************************************************
                           UVModel  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//---------- Class Interface of <UVModel> (fichier UVModel.h) ----------
#if ! defined ( UVMODEL_H )
#define UVMODEL_H

//-------------------------------------------------------- Used Interfaces
#include "hemisphericalSrc.h"
#include "ptSrc.h"
#include "date.h"
#include "intensityMap.h"
#include "UVMesh.h"
#include "IOSpreadSheet.h"
#include "planeSurface.h"
#include <vcg/space/point3.h> 
#include <time.h>
#include <vector>
#include <map>
#include <vcg/simplex/face/pos.h>
#include <vcg/simplex/face/jumping_pos.h>
#include <QProgressDialog>

//-------------------------------------------------------------- Constants 

//------------------------------------------------------------------ Types

//------------------------------------------------------------------------ 
// Aim of the class <UVModel>
// This class holds the SimuUVEx model and provides the necessary 
//functionnalities to run the simulation and retrieve its results.
//
// First, the attributes will be explicited :
// - diffuseSrc is the object representing the diffuse UV source.
// - reflectedSrc is the object representing the reflected UV source, aka albedo.
// - directSrc is the object representing the direct UV source, ie the sun.
// - posture is the object which contains the 3D mesh.
// - evalIntensityList is a vector that stores the simulated intensities
// - path, name and extension are corresponding to the file of the 3D model.
// - Xsubdiv, Ysubdiv and Zsubdiv are the number of the division of the
//   surrouding box with respect to each axis.
//
// Then there is the "proper" way to use these methods. Of course one may
// and maybe should do as one wants as long it works ;) :
// - Construct UVModel with proper params
// - then use "initModel", 
// - "evaluateUV..." with proper parameters,
//
//------------------------------------------------------------------------ 

class UVModel
{
//----------------------------------------------------------------- PUBLIC

public:
//--------------------------------------------------------- Public Methods

	//This method initiates the model with all the parameters previously set.
	void initModel();

	//Note that a compatible data file should have been loaded in IOSpreadSheet prior
	//to calling one of the following functions:
	/* There is a coherence contract :
	 * The Excel data file MUST have columns as following in order to
	 * read the right data at the right place.
	 * Year / Month / Day / Hour / Min / Zenith / Azimut / Gloabal UV / Diffuse UV / Direct UV / Reflected UV
	 * YYYY /  MM   /  DD /  HH  / mm  / in deg / in deg / ( the remaining are all float in W/m2)
	 * Furthermore, the rows MUST be sorted chronologically and not even one time step can be forgotten
	 * else we cannot say what may happen. */
	//Evaluates UVs for a fixed position from beginDate ot endDate
	//sources represents sources to be simulated (as defined in intensities.h)
	//If progress is passed, it will be upated as the simulation goes on for each vertex evaluated at each time.
	void evaluateUVBetweenFixed(const Date &beginDate, const Date &endDate, float angle, int sources = 0x00000007);
	//Evaluates UVs as an average of the given position starting from startAngle to endAngle, from beginDate ot endDate
	//sources represents sources to be simulated (as defined in intensities.h)
	//If progress is passed, it will be upated as the simulation goes on for each vertex evaluated at each time.
	void evaluateUVBetweenAvg(const Date &beginDate, const Date &endDate, float startAngle, float endAngle, float angleStep, int sources = 0x00000007);
	//Evaluates UVs by rotating the position of angleStep each timeStep and starting from the startAngle, from beginDate ot endDate
	//sources represents sources to be simulated (as defined in intensities.h)
	//If progress is passed, it will be upated as the simulation goes on for each vertex evaluated at each time.
	void evaluateUVBetweenSeq(const Date &beginDate, const Date &endDate, float startAngle, float angleStep, int timeStep, int sources = 0x00000007);

	void clearEvaluatedIntensities();

	//Insert POIs into the Mesh from an XML file
	void setPOIs(const char *filename);
	
	//Evaluate the POIs based on a list of POIs and a radius
	void evaluatePOI(float maxDistance, bool useBaseVertices = true);
	
	bool hasPOIs() const { return posture->hasPOIs(); }

	//Creates Mesh Zones from an XML file
	void setZones(const char *filename);
	
	//Evaluate the POIs based on a list of POIs and a radius
	void evaluateZones();
	
	bool hasZones() const { return posture->hasZones(); }

	//Add Protections to the mesh
	void setProtections(bool reset = true);

	//Removes all protections from the zones
	void resetProtections();

	//Merges the results from the given mesh into the mesh
	//Important note1: The precision of the model irradiation will be reduced to zones (Vertices values unchanged)
	//Important note2: The integrity of the merged model is not guaranteed any more
	void mergeResults(UVModel *modelToMerge);

//-------------------------------------------------- Public Inline Methods

	//Get the mesh
	inline UVMesh* getUVMesh(){return posture;}

	inline void setRayPlaneTolerance(float tolerance){ ray_plane_tolerance = tolerance;}

	inline void setUseDirectSource(bool useSource){ useDirectSource = useSource;}
	inline void setUseDiffuseSource(bool useSource){useDiffuseSource = useSource;}
	inline void setUseReflectedSource(bool useSource){useReflectedSource = useSource;}
	
	//Acccessors for the Number of Levels and Points per Level for the diffuse
	//and reflected sources
	//Diffuse source
	inline void setDiffuseLvlNb(int lvlNb){ diffuseSrc->setLvlNb(lvlNb);}
	inline int getDiffuseLvlNb(){ return diffuseSrc->getLvlNb();}
	inline void setDiffusePtNb(int ptNb){ diffuseSrc->setPtNb (ptNb);}
	inline int getDiffusePtNb(){ return diffuseSrc->getPtNb();}
	inline void setDiffuseRadiusFactor(float factor){diffuseSrc->setRadiusFactor(factor);}
	inline void setDiffuseAttenuationAngle(float angle){diffuseSrc->setAttenuationAngle(angle);}
	inline bool setDiffuseMapPath(string path){return posture->loadDiffuseIntensityMap(path);}

	//Reflected Source
	inline void setReflectedLvlNb(int lvlNb){ reflectedSrc->setLvlNb(lvlNb);}
	inline int getReflectedLvlNb(){ return reflectedSrc->getLvlNb();}
	inline void setReflectedPtNb(int ptNb){ reflectedSrc->setPtNb (ptNb);}
	inline int getReflectedPtNb(){ return reflectedSrc->getPtNb();}
	inline void setReflectedRadiusFactor(float factor){reflectedSrc->setRadiusFactor(factor);}
	inline bool setReflectedMapPath(string path){return posture->loadReflectedIntensityMap(path);}

	//Accessors for the bounding boxes parameters
	//Number of bounding boxes
	inline void setNumBoxes(unsigned int xNumBoxes, unsigned int yNumBoxes, unsigned int zNumBoxes){
		Xsubdiv = xNumBoxes;
		Ysubdiv = yNumBoxes;
		Zsubdiv = zNumBoxes;
	}
	inline unsigned int getNumXBoxes(){return Xsubdiv;}
	inline unsigned int getNumYBoxes(){return Ysubdiv;}
	inline unsigned int getNumZBoxes(){return Zsubdiv;}
	
	//Direct source
	inline void setAngleToNorth(float angle){directSrc->setAngleToNorth(angle);}
	inline void setDirectUseBoxes(bool useBoxes){ useDirectBoxes = useBoxes; }
	inline bool getDirectUseBoxes(){return useDirectBoxes;}

	//Diffuse Source
	inline void setDiffuseUseBoxes(bool useBoxes){useDiffuseBoxes = useBoxes;	}
	inline bool getDiffuseUseBoxes(){return useDiffuseBoxes;}

	//Reflected Source
	inline void setReflectedUseBoxes(bool useBoxes){useReflectedBoxes = useBoxes;}
	inline bool getReflectedUseBoxes(){return useReflectedBoxes;}
	
	
	//Accessors for the plane surface
	inline void setUsePlaneSurface(bool useSurface){usePlaneSurface = useSurface;}
	inline bool getUsePlaneSurface(){ return usePlaneSurface;}
	inline PlaneSurface* getPlaneSurface(){return planeSurface;}

//--------------------------------------------------- Operator Overloading

//---------------------------------------------- Constructors - destructor
//FOR NOW, Force the filename argument. Later, should be changed so that 1 or more
//postures can be loaded.
	UVModel (MeshDocument *doc, const char* fileName, IOSpreadSheet* spreadsheet);

	UVModel (MeshModel* model, const char* filename, IOSpreadSheet* spreadsheet);

    virtual ~UVModel ( );

//---------------------------------------------------------------- PRIVATE
private:

//------------------------------------------------------ Private Methods

	//Generic evaluation method given start and end rows and angle
	//results are stored/appended in evvalIntensity
	//sources represents sources to be simulated (as defined in intensities.h)
	//If progress is passed, it will be upated as the simulation goes on for each vertex evaluated at each time.
	void evaluateUVBetween(int firstRow, int lastRow, float angle, EvaluatedIntensity* evalIntensity, int sources = 0x00000007);

	//Get the first and last row from the weather data file corresponding to the given dates
	pair<int,int> getRowsFromDates(const Date &beginDate, const Date &endDate);

	//Set the data for all UV sources from the rowNB'th row in the Excel data file
	void setSourcesData(int rowNB);
	
	//Evaluate the received UV for a Mesh using the setup sources (time does not matter here since sources must have been setup properly)
	//Results are stored/appended in intensityList
	//sources represents sources to be simulated (as defined in intensities.h)
	//If progress is passed, it will be upated as the simulation goes on for each vertex evaluated at each time.
	void evaluateReceivedUV(UVMesh &uvmesh, EvaluatedIntensity &evalIntensity, int sources = 0x00000007);
	
	//This method returns whether the given emitting source is visible
	//from the vertex (i.e. not occluded by another face)
	//It can be set to run with or without the bounding boxes optimization
	bool vertexVisible(UVMesh &uvmesh, emittingPt &aEmitPt,CVertexO &aObjPt, bool useBoundingBoxesOptimization) const;
	
	//This method calculate the complete intensity received by a vertex (direct/diffuse/reflected)
	//It returns an intensity container filled with the different intensities
Intensity calculateIntensityForVertex(UVMesh &uvmesh, CVertexO &aObjPt, 
vector<float>::iterator &DIMIter, vector<float>::iterator &RIMIter, int sources = 0x00000007) const;
	
	//This method calculates the intensity for the ptSrc component for a given vertex
	double calculatePtSrcIntensity(UVMesh &uvmesh, CVertexO &aObjPt) const;
	
	//This method calculates the intensity for the given Hemispherical source on a given vertex
	//It returns a pair that contains the total intensity and the number of visible emitting points
	//(used for the creation and storage of the maps)
	pair<double,float> calculateHemisphericalSrcIntensity(UVMesh &uvmesh, CVertexO &aObjPt, const hemisphericalSrc &aHemiSource) const;
	//double UVModel::calculateHemisphericalSrcIntensity(CVertexO &aObjPt, const hemisphericalSrc &aHemiSource);

	float isPotentiallyVisible(emittingPt &aEmitPt, CVertexO &aObjPt) const;
	/* This method returns the cosine of the angle between the ray from the emitPt
	 * to the ObjPt and the normal to the ObjPt. */

	bool linePlanIntersect(emittingPt &aEmitPt, CVertexO &aObjPt, CFaceO &aObjFace) const;
	/* This method returns true if the ray between the EmitPt and the ObjPt
	 * intersects the face ObjFace and that the face is between the two points. */



//--------------------------------------------------- Private Attributes

	// Representation of the UV sources
	hemisphericalSrc* diffuseSrc;
	hemisphericalSrc* reflectedSrc;
	ptSrc* directSrc;
	IOSpreadSheet* spreadsheet;
	
	// Representation of the posture
	UVMesh* posture;

	//Whether to simulate - or not- the different sources
	bool useDirectSource;
	bool useDiffuseSource;
	bool useReflectedSource;

	// Nb of subdivisions of each axis to determine the sub-bounding boxes
	unsigned int Xsubdiv;
	unsigned int Ysubdiv;
	unsigned int Zsubdiv;

	//Whether the bounding boxes should be used for the calculation of
	//the different types of emissions
	//Direct emissions - default value = true since it changes for every minute
	bool useDirectBoxes;
	//Important note: In case the pre-calculated diffuse/refleced maps are used,
	//these parameters won't matter
	//Diffuse emissions - Default to false since calculated once and then reused
	bool useDiffuseBoxes;
	//Reflected emissions - Default to false since calculated once and then reused
	bool useReflectedBoxes;
	
	//Constant specifying the tolerance for the ray/plane intersection. 
	//Has to be <1, yet very close to it.
	float ray_plane_tolerance;
	
	//Whether to do the calculation for the plane surface or not
	bool usePlaneSurface;
	
	//Representation of the PlaneSurface if used
	PlaneSurface* planeSurface;

    //Mesh document
    MeshDocument* meshDocument;
};

#endif // UVMODEL_H

