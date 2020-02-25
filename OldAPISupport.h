/*************************************************************************
This file is created for compability of plugin with older API
Some function that was supported in that API have analogues here
*************************************************************************/

#ifndef OLDAPISUPPORT_H
#define OLDAPISUPPORT_H

class CFaceO;
class MeshDocument;
class MeshModel;
class OldApi
{
public:
    // MeshModel constructor without parameters
    static MeshModel *CreateMeshModel(MeshDocument *doc);
    // Creates MeshModel and add to MeshDocument
    static MeshModel *AddMeshModel(MeshDocument *doc, const char *str);
    // Computes of normal for face
    static void ComputeNormal(CFaceO &face);
    // Computes of normalized normal for face
    static void ComputeNormalizedNormal(CFaceO &face);
    // SetRGB color
    template<class Color>
    static void SetRGB(Color &c, int r, int g, int b)
    {
        c[0] = r;
        c[1] = g;
        c[2] = b;
    }
};

#endif // OLDAPISUPPORT_H
