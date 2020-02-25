/*************************************************************************
                           PlaneSurface  -  description
                             -------------------
	authors		     : Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

#if ! defined ( PLANESURFACE_H )
#define PLANESURFACE_H

//-------------------------------------------------------- Used Interfaces

#include "UVMesh.h"
#include <vcg/complex/allocate.h>
#include <math.h>

//-------------------------------------------------------------- Constants 

//------------------------------------------------------------------ Types 

//Used to store the UV differences at every step
typedef struct
{
	float diffuseDiff;
	float reflectedDiff;
	float directDiff;
	Intensity measuredIntensity;
} UVDiff;

//Used to stre the statistics about UV differences
typedef struct
{
	float minDiffused;
	float maxDiffused;
	float meanDiffused;
	//float medianDiffused;
	float minReflected;
	float maxReflected;
	float meanReflected;
	//float medianReflected;
	float minDirect;
	float maxDirect;
	float meanDirect;
	//float medianDirect;
} UVDiffStats;


//------------------------------------------------------------------------ 
//The PlaneSurface models a pseudo-plane surface and is used to check
//simulated values on a plane surface against measured data.
//It is pseudo-plane in the sense that in fact it is composed of 6 points
// forming a regular diamond-shape polygon. The height of the points of the diamond
//is very small and since all faces are regular, the normal at the points
//is the same as if the surface was plane.
//It was done this way in order to test simple occlusion and separate
//the diffuse/direct and refelected UV components.
//------------------------------------------------------------------------ 

class PlaneSurface : public UVMesh
{
public:
    //Inserts a new evaluated intensity along with the measured intensities
    void push_backEvaluatedIntensity(EvaluatedIntensity* EvaluatedIntensity, float directIntensity, float diffusedIntensity, float reflectedIntensity);

    void clearEvaluatedIntensities();

    //Merges 2 plane surface intensities together
    //Note that at the moment, the merged planeSurface is cleared of all its data
    void mergeIntensities(PlaneSurface *toMerge);

    vector<UVDiff>* getUVDifferences(){return uvDifferences;}

    //Returns stats
    UVDiffStats getDifferencesStats();

    //Return the total intensity of the UV received over the full period of time in [J/m2]
    Intensity getTotalIntensity(bool summary);

    //Exports the received intensities as a CSV file
    bool exportIntensitiesCSV(string filename, IOZones::OutputStreamMap &outputStreams);

    //--------------------------------------------------- Operator Overloading


    //---------------------------------------------- Constructors - destructor

    PlaneSurface(vcg::Point3<float> aCenter, float aSurfaceHeight, MeshDocument *doc);

    ~PlaneSurface();

private:
    //------------------------------------------------------ Protected Methods

    //Checks the last intensities received by the surface vertices against
    //the intensities of the sources
    UVDiff getUVDiff(float directIntensity, float diffusedIntensity, float reflectedIntensity);

    //Defines comparison methods to sort the intensities
    static bool compareDiffuseUVDifferences(const UVDiff &uvdiff1, const UVDiff &uvdiff2);
    static bool compareReflectedUVDifferences(const UVDiff &uvdiff1, const UVDiff &uvdiff2);
    static bool compareDirectUVDifferences(const UVDiff &uvdiff1, const UVDiff &uvdiff2);

    //--------------------------------------------------- Protected Attributes

    vector<UVDiff>* uvDifferences;
};

#endif // PLANESURFACE_H

