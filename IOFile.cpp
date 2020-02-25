/*************************************************************************
                           IOFile  -  description
                             -------------------
	authors		     : CAO Alexandre, Laurent Francioli
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//-------Class Implementation of <IOFile> (fichier IOFile.cpp) -------

//---------------------------------------------------------------- INCLUDE

//--------------------------------------------------------- System Include

//------------------------------------------------------------ Own Include
#include "IOFile.h"
#include <wrap/io_trimesh/import.h>
#include <wrap/io_trimesh/export.h>
#include <wrap/io_trimesh/import_ply.h>
#include <wrap/io_trimesh/import_stl.h>
#include <wrap/io_trimesh/import_obj.h>
#include <wrap/io_trimesh/import_off.h>
#include <wrap/io_trimesh/import_ptx.h>
#include <wrap/io_trimesh/import_vmi.h>

#include <wrap/io_trimesh/export_ply.h>
#include <wrap/io_trimesh/export_stl.h>
#include <wrap/io_trimesh/export_obj.h>
#include <wrap/io_trimesh/export_vrml.h>
#include <wrap/io_trimesh/export_dxf.h>
#include <wrap/io_trimesh/export_vmi.h>
#include <wrap/io_trimesh/export.h>

#include<iostream>
#include<fstream>

#include"strConv.h"

#include <string>
#include "OldAPISupport.h"

//-------------------------------------------------------------- Constants

//----------------------------------------------------------------- PUBLIC

//--------------------------------------------------------- Public Methods
MeshModel* IOFile::loadFromFile(const char* fileName, MeshDocument *doc)
{
	MeshModel *res = OldApi::CreateMeshModel(doc);
	int mask;
//	vcg::tri::io::ImporterPLY<CMeshO>::LoadMask(fileName,mask);
//	res->Enable(mask);
//	vcg::tri::io::ImporterPLY<CMeshO>::Open(res->cm, fileName, mask);
	return res;
}

void IOFile::saveToFile(const char* fileName, MeshModel* mesh, int mask )
{
//	vcg::tri::io::Exporter<CMeshO>::Save(mesh->cm, fileName, mask);
}

vector<float>* IOFile::loadIntensityMap(const char* fileName)
{
	vector<float>* result = new vector<float>;
	float value;

	std::ifstream file(fileName);
	if ( file.is_open() )
	{
		while ( file.good() )
		{
			std::string buffer;
			getline( file, buffer );

			if ( buffer.length()!=0 )
			{
				strConv::from_string(buffer, value);
				result->push_back(value);
			}
		}
	}
	else
	{
		std::cout << "Error. Cannot open map file or file doesn t exist." << std::endl << std::endl;
	}

	file.close();

	return result;
}

void IOFile::saveIntensityMap(const char* fileName, vector<float>* intensityMap)
{
	std::ofstream file(fileName, std::fstream::out|std::fstream::trunc);

	if ( file.is_open() )
	{
		vector<float>::iterator iter;
		for(iter=intensityMap->begin(); iter!=intensityMap->end(); iter++)
		{
			std::string buffer = strConv::to_string( *iter );
			file << buffer << std::endl;
		}
	}
	else
	{
		std::cout << "Error. Cannot open map file." << std::endl << std::endl;
	}

	file.close();
}

//--------------------------------------------------- Operator Overloading

//---------------------------------------------- Constructors - destructor
// NEVER USED !!!
IOFile::IOFile ( )
{
#ifdef MAP
    cout << "Call to the constructor of <IOFile>" << endl;
#endif
} //----- End of IOFile


IOFile::~IOFile ( )
{
#ifdef MAP
    cout << "Call to the destructor of <IOFile>" << endl;
#endif
} //----- End of ~IOFile


//---------------------------------------------------------------- PRIVATE

//------------------------------------------------------ Protected Methods

