#include<vcg/complex/complex.h>
#include<wrap/io_trimesh/import_off.h>
#include <wrap/io_trimesh/import_obj.h>
#include <wrap/io_trimesh/export_obj.h>
#include <wrap/io_trimesh/export_off.h>
#include <complex/algorithms/update/color.h>

class MyFace; class MyVertex; class MyEdge;

struct MyUsedTypes : public vcg::UsedTypes<vcg::Use<MyVertex>   ::AsVertexType,
                                           vcg::Use<MyEdge>     ::AsEdgeType,
                                           vcg::Use<MyFace>     ::AsFaceType>{};

class MyVertex  : public vcg::Vertex< MyUsedTypes, vcg::vertex::Coord3f, vcg::vertex::Normal3f, vcg::vertex::Color4b, vcg::vertex::Normal3f, vcg::vertex::BitFlags  >{};
class MyFace    : public vcg::Face < MyUsedTypes, vcg::face::FFAdj, vcg::face::VertexRef, vcg::face::Normal3f, vcg::face::Color4b, vcg::face::BitFlags > {};
class MyEdge    : public vcg::Edge<   MyUsedTypes> {};

class MyMesh    : public vcg::tri::TriMesh< std::vector<MyVertex>, std::vector<MyFace>, std::vector<MyEdge> > {};

class MyVertex0  : public vcg::Vertex< MyUsedTypes, vcg::vertex::Coord3f, vcg::vertex::BitFlags  >{};
class MyVertex1  : public vcg::Vertex< MyUsedTypes, vcg::vertex::Coord3f, vcg::vertex::Normal3f, vcg::vertex::BitFlags  >{};
class MyVertex2  : public vcg::Vertex< MyUsedTypes, vcg::vertex::Coord3f, vcg::vertex::Color4b, vcg::vertex::CurvatureDirf,
                                                    vcg::vertex::Qualityf, vcg::vertex::Normal3f, vcg::vertex::BitFlags  >{};


int main( int argc, char **argv )
{
  if(argc<2)
  {
    printf("Usage trimesh_base <meshfilename.obj>\n");
    return -1;
  }

  // Define a Mesh obj
  MyMesh m;

  // The mask is used to store informations regarding the mesh
  int mask;

  vcg::tri::io::ImporterOFF<MyMesh>::Open(m,argv[1], mask);

  vcg::tri::RequirePerVertexNormal(m);
  vcg::tri::UpdateNormal<MyMesh>::PerVertexNormalized(m);
  printf("Input mesh  vn:%i fn:%i\n",m.VN(),m.FN());
  printf( "Mesh has %i vert and %i faces\n", m.VN(), m.FN() );

  // Inizializziamo la FF topology
  vcg::tri::UpdateTopology<MyMesh>::FaceFace(m);

  // set faces as not visited
  vcg::tri::UpdateFlags<MyMesh>::FaceClearV(m);

  // Use a stack to visit all the faces
  std::stack<MyFace*> s;
  MyMesh::FaceIterator startFace = m.face.begin();
  
  // Beginning the visit of all connected components
  while(startFace != m.face.end())
  { 
    // In each iterations I compute a different color
    vcg::Color4b ourC = vcg::Color4b(rand() % 255, rand() % 255, rand() % 255, 255);

    // Put the first face of the current connected component in the stack
    s.push(&*(startFace));

    // Visit the current connected component, coloring each face
    while(!s.empty()){
      MyFace* face = s.top();
      s.pop();
      face->C() = ourC;
      face->SetV();
      for(int i=0; i<3; ++i){
        MyFace* next = face->FFp(i);
        if(!next->IsV()){
          s.push(next);
        }
      }
    }
    
    // I search for the next connected component (if available)
    while(startFace != m.face.end() && startFace->IsV() )
    {
      startFace++;
    }
  }

  // Save the colored mesh
  vcg::tri::io::ExporterOBJ<MyMesh>::Save(m, "test.obj", vcg::tri::io::Mask::IOM_FACECOLOR);

}