#ifndef FASTLAPLACECOMPER_H
#define FASTLAPLACECOMPER_H

#include <GL/glew.h>
#include <cv.h>
#include <cv.hpp>
#include <vector>

#include "ConstrainedDelaunay.h"
#include "SimilarityTransform.h"



class FastLaplaceComper
{
public:

    FastLaplaceComper();
    ~FastLaplaceComper();

    void SetSourceImage(IplImage *img);
    void SetTargetImage(IplImage *img);

    int num_vertices() const;
    GtsFlcVertex** vertices();

    int num_bdry_vertices() const;
    GtsFlcVertex** bdry_vertices();

    // possibly sparse set of points defining the source poly; internally
    // rasterized to lattice pts.
    void SetSourcePoly(pt_t *pts, int n);

    void SetTransform(double rot, pt_t center, double scale, pt_t translation, bool bake_rotation=false);

    void SetSrcTransform(double rot, pt_t center, double scale, pt_t translation);

    void ComputeInterpolant(int num_iters=40);

    void Render();

    void Clear();

    ConstrainedDelaunay* Mesh() { return _cd; }

    // debug methods
    void drawMesh(bool flag);
    void doInterp(bool flag);

protected:

    void _getBdryColourDifferences();
    void _generateMesh();
    void _computeTextureCoords();
    SimilarityTransform _bakedPartOfTgtXfrm();
    SimilarityTransform _unbakedPartOfTgtXfrm();

    std::vector<pt_t> _sourcePoly; //

    static GtsFlcVertex* other_vertex(GtsFlcVertex*v, GtsFlcEdge*e);

    IplImage *_src_img, *_tgt_img;

    ConstrainedDelaunay *_cd;

    bool m_rotationBaked;

    GLint m_fragmentShader, m_shaderProgram;

    GLuint m_srcImgTexId, m_tgtImgTexId;
    GLint m_srcImgTexLoc;

    SimilarityTransform _tgt_xfrm, _src_xfrm;

    bool _debugDrawMesh;
    bool _doInterp;
};


#endif // FASTLAPLACECOMPER_H
