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

class MyVertex  : public vcg::Vertex< MyUsedTypes, vcg::vertex::Coord3f, vcg::vertex::VFAdj, vcg::vertex::Normal3f, vcg::vertex::Color4b, vcg::vertex::Normal3f, vcg::vertex::BitFlags  >{};
class MyFace    : public vcg::Face < MyUsedTypes, vcg::face::VFAdj, vcg::face::VertexRef, vcg::face::Normal3f, vcg::face::Color4b, vcg::face::BitFlags > {};
class MyEdge    : public vcg::Edge<   MyUsedTypes> {};

class MyMesh    : public vcg::tri::TriMesh< std::vector<MyVertex>, std::vector<MyFace>, std::vector<MyEdge> > {};

using namespace vcg;

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

  tri::io::ImporterOBJ<MyMesh>::Open(m,argv[1], mask);

  tri::RequirePerVertexNormal(m);
  tri::UpdateNormal<MyMesh>::PerVertexNormalized(m);
  printf("Input mesh  vn:%i fn:%i\n",m.VN(),m.FN());
  printf( "Mesh has %i vert and %i faces\n", m.VN(), m.FN() );

  // Find all the vertices that are within a path of at most two edges 
  // using the VF Adiacency

  // Let's update the topology using the VF Adiacency
  tri::UpdateTopology<MyMesh>::VertexFace(m);

  // choose a random vertex
  MyMesh::VertexIterator vi = m.vert.begin();

  // Save the colored mesh
  tri::io::ExporterOBJ<MyMesh>::Save(m, "test.obj", vcg::tri::io::Mask::IOM_FACECOLOR);

}