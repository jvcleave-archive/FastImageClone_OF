
#include "FastLaplaceComper.h"

#define glError() { \
        GLenum err = glGetError(); \
        while (err != GL_NO_ERROR) { \
                fprintf(stderr, "glError: %s caught at %s:%u\n", (char *)gluErrorString(err), __FILE__, __LINE__); \
                err = glGetError(); \
        } \
}

// DEBUG CODE, REMOVE WHEN STUFF IS WORKING
//#include "qdebug.h"
//#include <QTime>
// DEBUG CODE

/*

NOTES ON MESHING, SAMPLING, TRANSFORMS
======================================

There is a possibly sparse source poly in the source image pixel space.
We apply the target transform to it, and possibly mesh the poly.  We
rasterize the poly edges and generate a mesh if

   1) the mesh wasn't generated yet
   2) there is a non-unit scale in the target transform
   3) optionally if there is a non-zero rotation in the transform

We have to remesh in the case of a non-unit scale because we
need to have at least one mesh boundary vertex per pixel.  We
probably don't have to remesh for scale < 1 (so the mesh boundary
is finer than the target image pixel lattice, but the results
are likely somewhat better if we do.  Might be worth doing some
tests, since if you can avoid a remesh, that's a good thing.

When sampling boundary colour diffs, there are two situations
to consider:

1) the target transform scale > 1
2) the target transform scale < 1

In the first case, we have to do sub-pixel sampling into
the source image.  In the second, we have to sample into
a region of the source image -- i.e., do an integral.  Alternatively,
we might want to downsample the entire source image, to make lookups
with a given scale quicker (and simpler to code).




*/

// the size of c should be equal to the number of channels in the image.
// which is only ever gonna be 3.  But what the hey.
bool getPixelBilerp(IplImage *img, const pt_t &p, uchar c[])
{
    if(p.x < 0) return false;
    if(p.x >= img->width-1) return false;
    if(p.y < 0) return false;
    if(p.y >= img->height-1) return false;

    //pt_t p(p_.x, (img->height-1) - p_.y);
    //pt_t p(p_);

    int x_floor = (int)floor(p.x);
    int y_floor = (int)floor(p.y);
    double x_rem = p.x - x_floor;
    double y_rem = p.y - y_floor;

    uchar *c00 = (uchar*)(img->imageData + y_floor*img->widthStep + img->nChannels*x_floor);
    uchar *c10 = (uchar*)(c00+img->nChannels);
    uchar *c01 = (uchar*)(img->imageData + (y_floor+1)*img->widthStep + img->nChannels*x_floor);
    uchar *c11 = (uchar*)(c01+img->nChannels);

    for(int i = 0; i < img->nChannels; i++)
    {
        c[i] = c00[i]*(1-x_rem)*(1-y_rem) +
               c10[i]*x_rem*(1-y_rem) +
               c01[i]*(1-x_rem)*y_rem +
               c11[i]*x_rem*y_rem;
    }

    std::swap(c[0], c[2]);

    return true;
}

bool getPixel(IplImage *img, int x, int y, uchar c[])
{
    if(x < 0) return false;
    if(x > img->width-1) return false;
    if(y < 0) return false;
    if(y > img->height-1) return false;

    uchar *c_ = (uchar*)(img->imageData + y*img->widthStep + img->nChannels*x);
    for(int i = 0; i < img->nChannels; i++)
    {
        c[i] = c_[i];
    }

    return true;
}

struct Kernel
{
    Kernel(int w) : _data(w*w) {}

    std::vector<double> _data;
};

struct GaussianKernel
{
    GaussianKernel()
    {
        //cv::getGaussianKernel()
    }
};

bool getPixelAreaSample(IplImage *img, const pt_t &p, const Kernel &k)
{
    return true;
}

void printLog(GLuint obj)
{
    int infologLength = 0;
    int maxLength;

    if(glIsShader(obj))
        glGetShaderiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
    else
        glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&maxLength);

    char infoLog[maxLength];

    if(glIsShader(obj))
        glGetShaderInfoLog(obj, maxLength, &infologLength, infoLog);
    else
        glGetProgramInfoLog(obj, maxLength, &infologLength, infoLog);

    if (infologLength > 0)
        printf("%s\n",infoLog);
}

FastLaplaceComper::FastLaplaceComper()
    : _src_img(NULL),
      _tgt_img(NULL),
      _cd(NULL)
{
    const char *shader_src = "uniform sampler2D source_img;\n"
                       "void main()\n"
                       "{\n"
                       "    gl_FragColor = (2.0*gl_Color-1.0) + texture2D(source_img, gl_TexCoord[0].st);\n"
                       "}";

    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_fragmentShader, 1, &shader_src, NULL);

    glCompileShader(m_fragmentShader);
    printLog(m_fragmentShader);

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, m_fragmentShader);
    glLinkProgram(m_shaderProgram);

    printLog(m_shaderProgram);

    m_srcImgTexLoc = glGetUniformLocation(m_shaderProgram, "source_img");

    glGenTextures(1, &m_srcImgTexId);
    glGenTextures(1, &m_tgtImgTexId);

    glError();

    _debugDrawMesh = false;
    _doInterp = true;
}

void FastLaplaceComper::drawMesh(bool flag)
{
    _debugDrawMesh = flag;
}

void FastLaplaceComper::doInterp(bool flag)
{
    _doInterp = flag;
}

FastLaplaceComper::~FastLaplaceComper()
{
    if(_cd) delete _cd;

    glDeleteTextures(1, &m_srcImgTexId);
    glDeleteTextures(1, &m_tgtImgTexId);
    glDeleteShader(m_fragmentShader);
    glDeleteProgram(m_shaderProgram);
}


bool img_valid(IplImage *img)
{
    if(img->nChannels != 3 ||
       img->depth != IPL_DEPTH_8U)
    {
        printf("Images must be 8-bit, 3 channels.");
        return false;
    }

    return true;
}

void FastLaplaceComper::SetSourceImage(IplImage *img)
{
    if(!img_valid(img)) return;

    _src_img = img;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_srcImgTexId);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height, 0, GL_BGR, GL_UNSIGNED_BYTE, img->imageData);
}

void FastLaplaceComper::SetTargetImage(IplImage *img)
{
    if(!img_valid(img)) return;

    _tgt_img = img;

    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, m_tgtImgTexId);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height, 0, GL_BGR, GL_UNSIGNED_BYTE, img->imageData);
}

int FastLaplaceComper::num_vertices() const
{
    if(!_cd) return 0;
    return _cd->num_vertices();
}

GtsFlcVertex** FastLaplaceComper::vertices()
{
    if(!_cd) return NULL;
    return _cd->vertices();
}

int FastLaplaceComper::num_bdry_vertices() const
{
    if(!_cd) return 0;
    return _cd->num_bdry_vertices();
}

GtsFlcVertex** FastLaplaceComper::bdry_vertices()
{
    if(!_cd) return NULL;
    return _cd->bdry_vertices();
}


void FastLaplaceComper::SetSourcePoly(pt_t *pts, int n)
{
    _sourcePoly.assign(pts, pts+n);
    _generateMesh();
    _computeTextureCoords();
}

void FastLaplaceComper::_generateMesh()
{
    // bake in scaling (i.e., apply before meshing),
    // and optionally bake in rotation
    SimilarityTransform first_xfrm(pt_t(),
                                   m_rotationBaked ? _tgt_xfrm.getRot() : 0,
                                   _tgt_xfrm.getCenter(),
                                   _tgt_xfrm.getScale());

    std::vector<pt_t> xfrmed_source_poly;
    size_t n = _sourcePoly.size();
    for(size_t i = 0; i < n; i++)
    {
        xfrmed_source_poly.push_back(first_xfrm.apply(_sourcePoly[i]));
    }

    std::vector<Pt<gdouble> > source_poly_pts;
    // for each segment of the poly, we want to rasterize it
    for(size_t i = 0; i < n; i++)
    {
        int x0 = cvRound(xfrmed_source_poly[i].x);
        int y0 = cvRound(xfrmed_source_poly[i].y);
        int x1 = cvRound(xfrmed_source_poly[(i+1)%n].x);
        int y1 = cvRound(xfrmed_source_poly[(i+1)%n].y);
        std::vector<Pt<gdouble> > line_pts;
        bresenham(x0, y0, x1, y1, line_pts);

        source_poly_pts.insert(source_poly_pts.end(), line_pts.begin(), line_pts.end());
    }

    source_poly_pts.erase(std::unique(source_poly_pts.begin(), source_poly_pts.end()), source_poly_pts.end());

    source_poly_pts.pop_back();

    if(_cd) delete _cd;

    _cd = new ConstrainedDelaunay(&source_poly_pts[0], (int)source_poly_pts.size(), .84);
}


// scaling is tricky.  Have to apply scale, then rasterize.
// So scale is baked in to the mesh, but rot
// remesh if scale is different;
void FastLaplaceComper::SetTransform(double rot, pt_t center, double scale, pt_t translation, bool bake_rot)
{
    m_rotationBaked = bake_rot;
    bool remesh = false;
    if(_tgt_xfrm.getScale() != scale)
    {
        remesh = true;
    }
    if(_tgt_xfrm.getRot() != rot && m_rotationBaked)
    {
        remesh = true;
    }

    _tgt_xfrm = SimilarityTransform(translation, rot, center, scale);

    if(remesh) _generateMesh();

    _computeTextureCoords();
    _getBdryColourDifferences();
    ComputeInterpolant();
}

// scale always baked, rotation optionally
SimilarityTransform FastLaplaceComper::_bakedPartOfTgtXfrm()
{
    if(m_rotationBaked)
    {
        return SimilarityTransform(pt_t(), _tgt_xfrm.getRot(), _tgt_xfrm.getCenter(), _tgt_xfrm.getScale());
    }
    else
    {
        return SimilarityTransform(pt_t(), 0, pt_t(), _tgt_xfrm.getScale());
    }
}

// scale always baked, rotation optionally
SimilarityTransform FastLaplaceComper::_unbakedPartOfTgtXfrm()
{
    if(m_rotationBaked)
    {
        return SimilarityTransform(_tgt_xfrm.getTranslation(), 0, pt_t(), 1);
    }
    else
    {
        return SimilarityTransform(_tgt_xfrm.getTranslation(), _tgt_xfrm.getRot(), _tgt_xfrm.getCenter(), 1);
    }
}


void FastLaplaceComper::SetSrcTransform(double rot, pt_t center, double scale, pt_t translation)
{
    _src_xfrm = SimilarityTransform(translation, rot, center, scale);
    _computeTextureCoords();
}


void FastLaplaceComper::ComputeInterpolant(int num_iters)
{


    // crap jacobi iteration solver
    int nv = num_vertices();
    GtsFlcVertex** verts = vertices();
    int idx = 0;
    for(int i = 0; i < num_iters; i++)
    {
#pragma omp parallel for
        for(int j = 0; j < nv; j++)
        {
            GtsFlcVertex *v = verts[j];
            if(v->is_bdry) continue;
            gdouble *c_cur = (gdouble*)&v->c[idx];
            c_cur[0] = c_cur[1] = c_cur[2] = 0;
            GSList *i_ = v->v.segments;
            while(i_)
            {
                GtsSegment *s = (GtsSegment*)i_->data;
                GtsFlcVertex *v_other = (GtsFlcVertex*)(s->v1 == (GtsVertex*)v ? s->v2 : s->v1);
                gdouble *c_last = (gdouble*)&v_other->c[(idx+1)%2];
                for(int k = 0; k < 3; k++)
                {
                    c_cur[k] += ((GtsFlcEdge*)s)->w*(c_last[k])/v->w;
                }
                i_ = i_->next;
            }
        }
        idx = (idx+1)%2;
    }
}

void FastLaplaceComper::Render()
{


  /*  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	
    glOrtho(0, 1, 0, 1, -1, 1);
    // TODO: width and height settable
    glViewport(0, 0, _tgt_img->width, _tgt_img->height);
	cout << "_tgt_img->width: " << _tgt_img->width << "_tgt_img->height: " << _tgt_img->height << endl;
    glEnable(GL_TEXTURE_2D);
    // render target image as full-screen quad
    glBindTexture(GL_TEXTURE_2D, m_tgtImgTexId);

    glBegin(GL_QUADS);
        glTexCoord2d(0, 0);
        glVertex2d(0, 0);
        glTexCoord2d(0, 1);
        glVertex2d(0, 1);
        glTexCoord2d(1, 1);
        glVertex2d(1, 1);
        glTexCoord2d(1, 0);
        glVertex2d(1, 0);
    glEnd();

    // now render mesh
    if(!_cd)
    {
        return;
    }else {
		
		cout << "has CD" << endl;
	}
	

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	
    pt_t trans = _tgt_xfrm.getTranslation();
    glTranslated(trans.x, trans.y, 0);
    if(!m_rotationBaked)
    {
        double rot = _tgt_xfrm.getRot();
        SimilarityTransform R(pt_t(), rot, pt_t(), 1);
        pt_t c = _tgt_xfrm.getCenter();
        pt_t Rc = R.apply(c);
        glTranslated(c.x - Rc.x, c.y - Rc.y, 0);
        glRotated(rot, 0, 0, 1);
    }
	
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, _tgt_img->width, 0, _tgt_img->height, -1, 1);

    glBindTexture(GL_TEXTURE_2D, m_srcImgTexId);
    glUseProgram(m_shaderProgram);
    glUniform1i(m_srcImgTexLoc, 0);
    GtsFlcVertex **vs = _cd->vertices_for_rendering();
    int v_idx = 0;
    glColor3d(.5,.5,.5);
    glBegin(GL_TRIANGLES);
        for(int i = 0; i < _cd->num_faces(); i++)
        {
            for(int j = 0; j < 3; j++, v_idx++)
            {
                GtsFlcVertex *v = vs[v_idx];
                glTexCoord2d(v->U, v->V);
                if(_doInterp)
                {
                    glColor3d((v->c[0].r+1)/2., (v->c[0].g+1)/2., (v->c[0].b+1)/2.);
                }
                GtsPoint &p =  v->v.p;
                glVertex3d(p.x, p.y, p.z);
            }
        }
    glEnd();
    glUseProgram(0);*/
	_debugDrawMesh = true;
    if(_debugDrawMesh)
    {
        glColor3d(1, 1, 1);
        glDisable(GL_TEXTURE_2D);
        GtsFlcEdge **edges = _cd->edges();
		cout << "_cd->num_edges(): " << _cd->num_edges() << endl;
        glBegin(GL_LINES);
            for(int i = 0; i < _cd->num_edges(); i++)
            {
                GtsFlcEdge *e = edges[i];
                GtsPoint &p1 =  e->e.segment.v1->p;
                glVertex3d(p1.x, p1.y, p1.z);
                GtsPoint &p2 =  e->e.segment.v2->p;
                glVertex3d(p2.x, p2.y, p2.z);
            }
        glEnd();
        glEnable(GL_TEXTURE_2D);
    }

    //qDebug("Time to render: %lfs.", time.elapsed()/1000.);

}

void FastLaplaceComper::Clear()
{
    if(_cd)
    {
        delete _cd;
        _cd = NULL;
    }

    _src_xfrm = SimilarityTransform();
    _tgt_xfrm = SimilarityTransform();
}

pt_t gts2pt(GtsFlcVertex *v)
{
    return pt_t(v->v.p.x, v->v.p.y, v->v.p.z);
}

void FastLaplaceComper::_computeTextureCoords()
{
    SimilarityTransform net_xfrm = _bakedPartOfTgtXfrm().inverted()*_src_xfrm;

    GtsFlcVertex** vs = _cd->vertices();
    int num_verts = _cd->num_vertices();
    for(int i = 0; i < num_verts; i++)
    {
        GtsFlcVertex *v = vs[i];
        pt_t pt = gts2pt(v);
        pt_t pt_xfrmed = net_xfrm.apply(pt);
        v->U = pt_xfrmed.x/_src_img->width;
        v->V = pt_xfrmed.y/_src_img->height;
    }
}


// Some reminders:
//
//    - meshing is always w.r.t. the target image
//    - if net scale < 1, then when getting bdry differences,
//      you have to sample an area of the src image
//

// a bit tricky here:
//
//    - get the mesh bdry vertex location;
//    - if rot not baked in, apply rot (scale already baked in)
//    - apply translation
//    - get the tgt image pixel value
//    - apply inverse _net_ transform on each pt
//    - get the src image interpolated value
void FastLaplaceComper::_getBdryColourDifferences()
{
    SimilarityTransform net_xfrm_src = _bakedPartOfTgtXfrm().inverted()*_src_xfrm;
    SimilarityTransform net_xfrm_tgt = _unbakedPartOfTgtXfrm();

    if(net_xfrm_src.getScale() <= 1)
    {
        for(int i = 0; i < _cd->num_bdry_vertices(); i++)
        {
            uchar src_col[3], tgt_col[3];
            pt_t bdry_v = gts2pt(_cd->bdry_vertices()[i]);
            pt_t pt_dest_src = net_xfrm_src.apply(bdry_v);
            getPixelBilerp(_src_img, pt_dest_src, src_col);
            pt_t pt_dest_tgt = net_xfrm_tgt.apply(bdry_v);
            getPixelBilerp(_tgt_img, pt_dest_tgt, tgt_col);

            double col_diff[] = { (double)tgt_col[0]-(double)src_col[0],
                                  (double)tgt_col[1]-(double)src_col[1],
                                  (double)tgt_col[2]-(double)src_col[2] };

            col_diff[0] /= 256.;
            col_diff[1] /= 256.;
            col_diff[2] /= 256.;

            _cd->setBdryCondition((int)i, col_diff);
        }
    }
    else
    {
        printf("void FastLaplaceComper::_getBdryColourDifferences() -- scale > 1 not implemented yet!!!\n");
    }
}



