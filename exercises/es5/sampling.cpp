// Today we do some sampling examples
#include <vcg/complex/complex.h>
#include <wrap/io_trimesh/import_off.h>
#include <wrap/io_trimesh/import_obj.h>
#include <wrap/io_trimesh/export_obj.h>
#include <wrap/io_trimesh/export_ply.h>
#include <complex/algorithms/clean.h>
#include <complex/algorithms/update/color.h>
#include <complex/algorithms/point_sampling.h> 

class MyFace; class MyVertex; class MyEdge;

struct MyUsedTypes : public vcg::UsedTypes<vcg::Use<MyVertex>   ::AsVertexType,
                                           vcg::Use<MyEdge>     ::AsEdgeType,
                                           vcg::Use<MyFace>     ::AsFaceType>{};

class MyVertex  : public vcg::Vertex< MyUsedTypes, vcg::vertex::Coord3f, vcg::vertex::Normal3f, vcg::vertex::Color4b, vcg::vertex::Normal3f, vcg::vertex::BitFlags  >{};
class MyFace    : public vcg::Face < MyUsedTypes, vcg::face::FFAdj, vcg::face::VertexRef, vcg::face::Normal3f, vcg::face::Color4b, vcg::face::BitFlags, vcg::edge::EEAdj > {};
class MyEdge    : public vcg::Edge<   MyUsedTypes, vcg::edge::VertexRef, vcg::edge::BitFlags> {};

class MyMesh    : public vcg::tri::TriMesh< std::vector<MyVertex>, std::vector<MyFace>, std::vector<MyEdge> > {};

using namespace vcg;

int main(int argc, char* argv[]){
    if(argc<2){
        printf("Usage trimesh_base <meshfilename.obj>\n");
        return -1;
    }

    // Setting parameters exercise
    MyMesh src, edgeMesh;
    int mask;

    // Importing src Mesh
    tri::io::ImporterOFF<MyMesh>::Open(src,argv[1], mask);
    tri::RequirePerVertexNormal(src);
    tri::UpdateNormal<MyMesh>::PerVertexNormalized(src);
    tri::UpdateNormal<MyMesh>::PerFaceNormalized(src);
    printf("Input mesh  vn:%i fn:%i\n",src.VN(),src.FN());

    // Setting adjacency FF
    tri::UpdateTopology<MyMesh>::FaceFace(src);

    tri::UpdateColor<MyMesh>::PerVertexConstant(src, vcg::Color4b::White);
    tri::UpdateQuality<MyMesh>::VertexConstant(src, 0);

    // we search fo all the edges where the difference
    // between the normals of the two incidents faces
    // is greater than a given threshold
    float thr_deg = 30;
    float thr_cos = cosf(math::ToRad(thr_deg));

    for(MyMesh::FaceIterator fi = src.face.begin() ; fi != src.face.end(); ++fi){
        MyMesh::FaceType &f = *fi;
        for(int i = 0; i < 3 ; i++){
            float dot = f.N() * f.FFp(i)->N();
            if(dot < thr_cos && f.FFp(i) > &f){
                f.V(i)->C() = vcg::Color4b::Red;
                f.V1(i)->C() = vcg::Color4b::Red;

                f.V0(i)->C() = vcg::Color4b::Red;
                f.V0(i)->C() = vcg::Color4b::Red;

                tri::Allocator<MyMesh>::AddEdge(edgeMesh, f.P0(i), f.P1(i));
            }
        }
    }

    for(MyMesh::VertexIterator vi = src.vert.begin(); vi != src.vert.end(); ++vi){
        MyMesh::VertexType &v = *vi;
        if(v.Q() >= 3.0f)
            v.C() = Color4b::Blue;
    }

    tri::Clean<MyMesh>::RemoveDuplicateVertex(edgeMesh);
    tri::Allocator<MyMesh>::CompactEveryVector(edgeMesh);
    vcg::tri::io::ExporterPLY<MyMesh>::Save(src, "vertcolor.ply", vcg::tri::io::Mask::IOM_VERTCOLOR);
    vcg::tri::io::ExporterPLY<MyMesh>::Save(edgeMesh, "edgeMesh.ply", vcg::tri::io::Mask::IOM_EDGEINDEX);

    // Montecarlo sampling o fthe edge mesh

    tri::TrivialSampler<MyMesh> ts;
    float radius = src.bbox.Diag() * 0.01f;
    tri::Clean<MyMesh>::SelectNonManifoldVertexOnEdgeMesh(edgeMesh);
    for(MyMesh::EdgeIterator ei = edgeMesh.edge.begin(); ei != edgeMesh.edge.end(); ++ei){
        MyMesh::EdgeType &e = *ei;
        if(e.V(0) -> IsS() || e.V(1) -> IsS())
            tri::Allocator<MyMesh>::DeleteEdge(edgeMesh, e);
        

    }
    
    for(MyMesh::VertexIterator vi = edgeMesh.vert.begin(); vi != edgeMesh.vert.end(); ++vi){
        if((*vi).IsS())
            tri::Allocator<MyMesh>::DeleteVertex()

    }

    tri::SurfaceSampling<MyMesh, tri::TrivialSampler <MyMesh>>::EdgeMeshUniform(edgeMesh, ts, radius);

    MyMesh samples;
    for(auto it = ts.SampleVec().begin(); it != ts.SampleVec().end(); ++it){
        tri::Allocator<MyMesh>::AddVertex(samples, *it);
    }


    vcg::tri::io::ExporterPLY<MyMesh>::Save(samples, "SamplesPointsOnEdges.ply");

    // Todo Build a Poisson surface sampling function that sample accurately both the feature vertex
    // and the  feature edges of a given mesh.
    // Things to do:
    // 1) Implement Uniform Edge Sampling strategy that works correctly with respected to non 1-manifold edges
    // 2) Implement Poisson sruface sampling strategy that can take in input a set of pre samples that must be removed befor the other ones

    // to implement 1 we write a SplitNonManifoldVertex
    // function that split the non 1-manifold vertex in an edge mesh

    // utilizzo le funzioni che già mi determinano gli edge non manifold
    return 0;
}