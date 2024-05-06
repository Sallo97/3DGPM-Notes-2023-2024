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


void AddQuadPerEdge(MyMesh &pathMesh, MyMesh::FacePointer fp, int i=0, float w1=0.5){
    
    float w2 = (1.0f - w1) / 2.0f;

    Point3f V0 = fp->V0(i)->P() * w1  +   fp->V1(i)->P() * w2 + fp->V2(i)->P() * w2;
    Point3f V1 = fp->V0(i)->P() * w2  +   fp->V1(i)->P() * w1 + fp->V2(i)->P() * w2;
    Point3f V2 = fp->V0(i)->P() * w2  +   fp->V1(i)->P() * w2 + fp->V2(i)->P() * w1;
    tri::Allocator<MyMesh>::AddFace(pathMesh, V0, V1, V2);

    MyMesh::FacePointer fp2 = fp->FFp(i);
    int j = fp->FFi(i);
    Point3f V3 = fp2->V0(j)->P() * w1 +   fp2->V1(j)->P() * w2 + fp2->V2(j)->P() * w2;
    Point3f V4 = fp2->V0(j)->P() * w2 +   fp2->V1(j)->P() * w1 + fp2->V2(j)->P() * w2;
    Point3f V5 = fp2->V0(j)->P() * w2 +   fp2->V1(j)->P() * w2 + fp2->V2(j)->P() * w1;
    tri::Allocator<MyMesh>::AddFace(pathMesh, V3, V4, V5);

    tri::Allocator<MyMesh>::AddFace(pathMesh, V0, V3, V1);
    tri::Allocator<MyMesh>::AddFace(pathMesh, V0, V4, V3);
}

int main(int argc, char* argv[]){
    if(argc<2){
        printf("Usage trimesh_base <meshfilename.obj>\n");
        return -1;
    }

    // Setting parameters exercise
    MyMesh src, quads, quads2, quads3; 
    float w1 = 0.5f;
    float w2 = (1.0f - w1) / 2.0;
    int mask;

    // Importing src Mesh
    tri::io::ImporterOFF<MyMesh>::Open(src,argv[1], mask);
    tri::RequirePerVertexNormal(src);
    tri::UpdateNormal<MyMesh>::PerVertexNormalized(src);
    printf("Input mesh  vn:%i fn:%i\n",src.VN(),src.FN());

    // Updating the topology and setting all the faces as not visited
    tri::UpdateTopology<MyMesh>::FaceFace(src);
    tri::UpdateFlags<MyMesh>::FaceClearV(src);
    
    // Defining the stack of face pointers
    std::vector<MyMesh::FacePointer> stack;
    stack.push_back(&src.face[0]);
    src.face[0].SetV();

    // sourceH is a Handle that allow to retrieve a typed attribute associated to each face. 
    // To use it just use the handle as a vector indexed with the object, 
    // e.g. to retrieve the attribute associated to a face f you can do
    // sourceH[f] (e.g. with the object)
    // sourceH[&f] (eg. with the pointer to the object)
    // sourceH[i] (e.g. with the index of the face in the vector of the faces m.face )
    auto sourceH = tri::Allocator<MyMesh>::AddPerFaceAttribute<MyMesh::FacePointer>(src,"source");

    // Visit the Mesh by faces
    while(!stack.empty()){

        // Random Visit
        //if(stack.size() > 2){
        //    std::swap(stack[rand()%stack.size()], stack.back());
        //}

        MyMesh::FacePointer fp = stack.back();
        stack.pop_back();
        assert(fp->IsV());

        for(int i = 0; i<3; ++i){
            if(!fp->FFp(i)->IsV()){
                fp->FFp(i)->SetV();
                stack.push_back(fp->FFp(i));
                AddQuadPerEdge(quads, fp, i, w1);
                sourceH[fp->FFp(i)] = fp;
            }
            else{
                if(sourceH[fp] != fp->FFp(i))
                    AddQuadPerEdge(quads2, fp, i, w1);
            }
        }
    }

    // Saving the new mesh
    tri::Clean<MyMesh>::RemoveDuplicateVertex(quads);
    tri::Clean<MyMesh>::RemoveDuplicateVertex(quads2);
    tri::io::ExporterOBJ<MyMesh>::Save(quads, "quads.obj", vcg::tri::io::Mask::IOM_FACECOLOR);
    tri::io::ExporterOBJ<MyMesh>::Save(quads2, "quads2.obj", vcg::tri::io::Mask::IOM_FACECOLOR);

}