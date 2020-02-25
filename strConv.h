/*************************************************************************
                           strConv  -  description
                             -------------------
	authors		     : CAO Alexandre
    copyright        : Institut universitaire romand de Sante au Travail
					   University of Geneva (UniGe)
*************************************************************************/

//---------- Class Interface of <strConv> (fichier strConv.h) ----------
#if ! defined ( STRCONV_H )
#define STRCONV_H

//-------------------------------------------------------- Used Interfaces
#include <sstream>
#include <string>

//-------------------------------------------------------------- Constants 

//------------------------------------------------------------------ Types 

//------------------------------------------------------------------------ 
// Aim of the class <strConv>
// This interface is used to convert a lot of base type to string
// and vice versa.
//
//------------------------------------------------------------------------ 

class strConv
{
//----------------------------------------------------------------- PUBLIC

public:

template<typename T>
static std::string to_string( const T & Value )
{
	// use a output stream to create the string
    std::ostringstream oss;
	oss.flush();
	// write the value in the stream
    oss << Value;
    return oss.str();
}

template<typename T>
static bool from_string( const std::string & Str, T & Dest )
{
	// create a stream from the given string
    std::istringstream iss( Str );
    // try to convert to Dest
//    return iss >> Dest != 0;
	return true;
}


//---------------------------------------------------------------- PRIVATE

protected:
};

//------------------------------ Other Depending Definitions of <strConv>

#endif // STRCONV_H

