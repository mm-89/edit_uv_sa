/*************************************************************************
                           IOFile  -  description
                             -------------------
	authors		     : CAO Alexandre, Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//---------- Class Interface of <IOFile> (fichier IOFile.h) ----------
#if ! defined ( IOFILE_H )
#define IOFILE_H

//-------------------------------------------------------- Used Interfaces


	#include <common/meshmodel.h>
#include <common/interfaces.h>
#include <wrap/io_trimesh/io_ply.h>

using std::vector;
//-------------------------------------------------------------- Constants 

//------------------------------------------------------------------ Types 

//------------------------------------------------------------------------ 
// Aim of the class <IOFile>
// This class is used to do IO operation with a 3D file.
// Since it uses VCG/Meshlab sources, the available format are the same as
// Meshlab, which is open source.
// It also does IO operation on text files used as map for the hemispherical
// sources.
//
//------------------------------------------------------------------------ 

class IOFile
{
//----------------------------------------------------------------- PUBLIC

public:
//--------------------------------------------------------- Public Methods
	static MeshModel* loadFromFile(const char* fileName, MeshDocument *doc);

	static void saveToFile(const char* fileName, MeshModel* mesh, int mask = 0);

	static vector<float>* loadIntensityMap(const char* fileName);

	static void saveIntensityMap(const char* fileName, vector<float>* intensityMap);

//--------------------------------------------------- Operator Overloading

//---------------------------------------------- Constructors - destructor
    IOFile ( );

    virtual ~IOFile ( );

//---------------------------------------------------------------- PRIVATE

protected:
//------------------------------------------------------ Protected Methods

//--------------------------------------------------- Protected Attributes

};

//------------------------------ Other Depending Definitions of <IOFile>

#endif // IOFILE_H

