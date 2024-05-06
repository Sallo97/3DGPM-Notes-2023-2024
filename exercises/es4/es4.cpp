#include<vcg/complex/complex.h>
#include<wrap/io_trimesh/import_off.h>
#include <wrap/io_trimesh/import_obj.h>
#include <wrap/io_trimesh/export_obj.h>
#include <complex/algorithms/update/color.h>

class MyFace; class MyVertex; class MyEdge;

struct MyUsedTypes : public vcg::UsedTypes<vcg::Use<MyVertex>   ::AsVertexType,
                                           vcg::Use<MyEdge>     ::AsEdgeType,
                                           vcg::Use<MyFace>     ::AsFaceType>{};

class MyVertex  : public vcg::Vertex< MyUsedTypes, vcg::vertex::Coord3f, vcg::vertex::Normal3f, vcg::vertex::Color4b, vcg::vertex::Normal3f, vcg::vertex::BitFlags  >{};
class MyFace    : public vcg::Face < MyUsedTypes, vcg::face::FFAdj, vcg::face::VertexRef, vcg::face::Normal3f, vcg::face::Color4b, vcg::face::BitFlags > {};
class MyEdge    : public vcg::Edge<   MyUsedTypes> {};

class MyMesh    : public vcg::tri::TriMesh< std::vector<MyVertex>, std::vector<MyFace>, std::vector<MyEdge> > {};

using namespace vcg;

// Given a face and an edge index, add a quad to the pathMesh
// the quad is placed in the middle of the edge
void AddQuadPerEdge(MyMesh &pathMesh, MyMesh::FacePointer fp, int i=0, float w1 = 0.5)
{
    float w2 =  ( 1.0f - w1 ) / 2.0f;

    Point3f V0 = fp->V0(i)->P()*w1 + fp->V1(i)->P()*w2 + fp->V2(i)->P()*w2;
    Point3f V1 = fp->V0(i)->P()*w2 + fp->V1(i)->P()*w1 + fp->V2(i)->P()*w2;
    
    MyMesh::FacePointer fp2 = fp->FFp(i);
    int j = fp->FFi(i);
    Point3f V2 = fp2->V0(j)->P()*w1 + fp2->V1(j)->P()*w2 + fp2->V2(j)->P()*w2;
    Point3f V3 = fp2->V0(j)->P()*w2 + fp2->V1(j)->P()*w1 + fp2->V2(j)->P()*w2;
    
    tri::Allocator<MyMesh>::AddFace(pathMesh, V0, V2, V1);
    tri::Allocator<MyMesh>::AddFace(pathMesh, V0, V3, V2);    
}

int main( int argc, char **argv ){
    if(argc<2)
    {
        printf("Usage trimesh_base <meshfilename.obj>\n");
        return -1;
    }
    
    // Define a Mesh obj
    MyMesh m, pathMesh, wallMesh, remMesh;

    float w1 = 0.7f;


    // The mask is used to store informations regarding the mesh
    int mask;
    tri::io::ImporterOFF<MyMesh>::Open(m,argv[1], mask);
    tri::RequirePerVertexNormal(m);
    tri::UpdateNormal<MyMesh>::PerVertexNormalized(m);
    printf("Input mesh  vn:%i fn:%i\n",m.VN(),m.FN());

    // Find all the vertices that are within a path of at most two edges 
    // using the FF Adiacency
  
    // Update the topology
    vcg::tri::UpdateTopology<MyMesh>::FaceFace(m);

    // mark all faces as not visited
    tri::UpdateFlags<MyMesh>::FaceClearV(m);

    // sourceH is a Handle that allow to retrieve a typed attribute associated to each face. 
    // To use it just use the handle as a vector indexed with the object, 
    // e.g. to retrieve the attribute associated to a face f you can do
    // sourceH[f] (e.g. with the object)
    // sourceH[&f] (eg. with the pointer to the object)
    // sourceH[i] (e.g. with the index of the face in the vector of the faces m.face )
    auto sourceH = tri::Allocator<MyMesh>::AddPerFaceAttribute<MyMesh::FacePointer>(m,"source");

    // stack of face pointers
    std::vector<MyMesh::FacePointer> stack;

    // push into the stack the first face
    stack.push_back(&m.face[0]);
    m.face[0].SetV();



    // visit the mesh by faces
    while(!stack.empty())
    {
        //random visit
        if(stack.size()>2){
        std::swap(stack[rand()%stack.size()], stack.back());
        }

        MyMesh::FacePointer fp = stack.back();
        stack.pop_back();
        assert(fp->IsV());
            
        for(int i=0;i<3;++i){
            if(!fp->FFp(i)->IsV()){
                fp->FFp(i)->SetV();
                stack.push_back(fp->FFp(i));
                AddQuadPerEdge(pathMesh, fp, i, w1);
                sourceH[fp->FFp(i)] = fp;
            }
            else{
                if(sourceH[fp] != fp->FFp(i))
                    AddQuadPerEdge(wallMesh, fp, i, w1);
            }
        }
    }
    

    // Save the mesh
    tri::io::ExporterOBJ<MyMesh>::Save(pathMesh, "pathMesh_1.obj", vcg::tri::io::Mask::IOM_FACECOLOR);

    float w2 =  ( 1.0f - w1 ) / 2.0f;
    // Now add to pathMesh the missing triangles
    for(size_t i = 0; i<m.face.size(); ++i){
        MyMesh::FacePointer fp = &m.face[i];

        Point3f V0 = fp->V(0)->P() * w1 + w2 * fp->V(1)->P() + fp->V(2)->P() * w2;
        Point3f V1 = fp->V(0)->P() * w2 + w1 * fp->V(1)->P() + fp->V(2)->P() * w2;
        Point3f V2 = fp->V(0)->P() * w2 + w2 * fp->V(1)->P() + fp->V(2)->P() * w1;

        tri::Allocator<MyMesh>::AddFace(pathMesh, V0, V1, V2);

    }

    // TODO Add the missing romboidal piece for each vertex of each triangle
    

    // TODO 2 (Optional) Move the wall faces up (use the face Normals fp->N() 
    // or the vertex Normals vp->N() but remember to initalize them)


    // Remove duplicate vertice that we have created in both meshes
    tri::Clean<MyMesh>::RemoveDuplicateVertex(pathMesh);
    tri::Clean<MyMesh>::RemoveDuplicateVertex(wallMesh);
    tri::Clean<MyMesh>::RemoveDuplicateVertex(remMesh);

    // Save the new meshes
    tri::io::ExporterOBJ<MyMesh>::Save(wallMesh, "wallMesh.obj", vcg::tri::io::Mask::IOM_FACECOLOR);
    tri::io::ExporterOBJ<MyMesh>::Save(pathMesh, "pathMesh_2.obj", vcg::tri::io::Mask::IOM_FACECOLOR);
    tri::io::ExporterOBJ<MyMesh>::Save(pathMesh, "remMesh.obj", vcg::tri::io::Mask::IOM_FACECOLOR);


}