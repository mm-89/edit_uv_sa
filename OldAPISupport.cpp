#include "OldAPISupport.h"

#include <common/meshmodel.h>

namespace vcg
{
    namespace face
    {
        /// Returns the normal to the plane passing through p0,p1,p2
        template<class TriangleType>
        Point3<typename TriangleType::ScalarType> Normal(const TriangleType &t)
        {
            return (( t.cP(1) - t.cP(0)) ^ (t.cP(2) - t.cP(0)));
        }

        /// Like the above, it returns the normal to the plane passing through p0,p1,p2, but normalized.
        template<class TriangleType>
        typename TriangleType::CoordType NormalizedNormal(const TriangleType &t)
        {
            return (( t.cP(1) - t.cP(0)) ^ (t.cP(2) - t.cP(0))).Normalize();
        }
    }
}

MeshModel *OldApi::CreateMeshModel(MeshDocument *doc)
{
    return new MeshModel(doc, QString(), QString());
}


MeshModel *OldApi::AddMeshModel(MeshDocument *doc, const char *str)
{
//    RenderMode rm;
//    rm.colorMode = vcg::GLW::CMPerVert;
//    return doc->addNewMesh(QString(), str, false, rm);
      MeshModel *am;
      return am;
}


void OldApi::ComputeNormal(CFaceO &face)
{
    face.N().Import(vcg::face::Normal(face));
}

void OldApi::ComputeNormalizedNormal(CFaceO &face)
{
    face.N().Import(vcg::face::NormalizedNormal(face));
}
