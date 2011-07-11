#ifndef CONSTRAINEDDELAUNAY_H
#define CONSTRAINEDDELAUNAY_H


#include <gts.h>
#include <vector>
#include <algorithm>
#include <math.h>
#include <iostream>
#include "Pt.h"

/////////////////////////////////
// START GtsFlcVertex DEFINITIONS
/////////////////////////////////
struct Colour
{
    Colour() : r(0), g(0), b(0) {}
    gdouble r, g, b;
};

struct _GtsFlcVertex {
     GtsVertex v;

     gdouble w;
     Colour c[2];
     gdouble U, V;
     bool is_bdry;
};

typedef struct _GtsFlcVertex        GtsFlcVertex;
typedef struct _GtsFlcVertexClass   GtsFlcVertexClass;

#define GTS_IS_FLC_VERTEX(obj)   (gts_object_is_from_class (obj,\
                                                            gts_flc_vertex_class ()))
#define GTS_FLC_VERTEX(obj)             GTS_OBJECT_CAST (obj,\
                                                         GtsFlcVertex,\
                                                         gts_flc_vertex_class ())
#define GTS_FLC_VERTEX_CLASS(klass)     GTS_OBJECT_CLASS_CAST (klass,\
                                                               GtsFlcVertexClass,\
                                                               gts_flc_vertex_class ())


struct _GtsFlcVertexClass {
  GtsVertexClass parent_class;
};

GtsFlcVertexClass * gts_flc_vertex_class(void);
GtsFlcVertex* gts_flc_vertex_new(GtsFlcVertexClass *klass, gdouble x, gdouble y, gdouble z);
void flc_vertex_clone (GtsObject * clone_obj, GtsObject * object);
void flc_vertex_destroy (GtsObject * object);
void flc_vertex_class_init (GtsVertexClass * klass);
void flc_vertex_init (GtsFlcVertex * vertex);
GtsFlcVertexClass * gts_flc_vertex_class (void);

///////////////////////////////
// END GtsFlcVertex DEFINITIONS
///////////////////////////////

/////////////////////////////////
// START GtsFlcEdge DEFINITIONS
/////////////////////////////////

struct _GtsFlcEdge
{
    GtsEdge e;
    gdouble w;
 };

typedef struct _GtsFlcEdge          GtsFlcEdge;
typedef struct _GtsFlcEdgeClass     GtsFlcEdgeClass;

#define GTS_IS_FLC_EDGE(obj)   (gts_object_is_from_class (obj,\
                                                          gts_flc_edge_class ()))
#define GTS_FLC_EDGE(obj)             GTS_OBJECT_CAST (obj,\
                                                       GtsFlcEdge,\
                                                       gts_flc_edge_class ())
#define GTS_FLC_EDGE_CLASS(klass)     GTS_OBJECT_CLASS_CAST (klass,\
                                                             GtsFlcEdgeClass,\
                                                             gts_flc_edge_class ())


struct _GtsFlcEdgeClass {
  GtsEdgeClass parent_class;
};

GtsFlcEdgeClass * gts_flc_edge_class(void);
GtsFlcEdge* gts_flc_edge_new(GtsFlcEdgeClass *klass, GtsVertex * v1, GtsVertex * v2);
void flc_edge_clone (GtsObject * clone_obj, GtsObject * object);
void flc_edge_destroy(GtsObject * object);
void flc_edge_class_init (GtsEdgeClass * klass);
void flc_edge_init (GtsFlcEdge * edge);
GtsFlcEdgeClass * gts_flc_edge_class (void);

///////////////////////////////
// END GtsFlcEdge DEFINITIONS
///////////////////////////////




template <class Scalar>
void bresenham(int x0, int y0, int x1, int y1, std::vector<Pt<Scalar> > &pts)
{
    int x0_orig = x0, y0_orig = y0;
    bool steep = (abs(y1 - y0) > abs(x1 - x0));
    if(steep)
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    if(x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int deltax = x1 - x0;
    int deltay = abs(y1 - y0);
    int error = deltax / 2;
    int ystep;
    int y = y0;
    ystep = y0 < y1 ? 1 : -1;
    for(int x=x0; x<=x1; x++)
    {
        steep ? pts.push_back(Pt<Scalar>((Scalar)y,(Scalar)x)) : pts.push_back(Pt<Scalar>((Scalar)x, (Scalar)y));
        error = error - deltay;
        if(error < 0)
        {
            y = y + ystep;
            error = error + deltax;
        }
    }
    if(pts[0].x != (Scalar)x0_orig || pts[0].y != (Scalar)y0_orig) // HACK.
    {
        std::reverse(pts.begin(), pts.end());
    }
}




struct ConstrainedDelaunay
{
    // successive pts are treated as defining constraint edges
    ConstrainedDelaunay(pt_t *pts, int n, gdouble triangle_quality=.82, bool verbose=false, bool check_delaunay=false)
    {
        GtsFifo *edges = gts_fifo_new();
        GPtrArray *vertices = g_ptr_array_new();
        g_ptr_array_set_size (vertices, n);

        for(int i = 0; i < n; i++)
        {
            GtsFlcVertex *v = gts_flc_vertex_new(gts_flc_vertex_class(), pts[i].x, pts[i].y, 0);
            v->is_bdry = true;
            g_ptr_array_index(vertices, i) = v;
            m_bdry_vertices.push_back(v);
        }

        for(int i = 0; i < n; i++)
        {
            gts_fifo_push(edges,
                          gts_edge_new(GTS_EDGE_CLASS(gts_constraint_class ()),
                                       (GtsVertex*)g_ptr_array_index(vertices, i),
                                       (GtsVertex*)g_ptr_array_index(vertices, (i+1)%n)));
        }

        // create triangle enclosing all the vertices
        GtsFlcVertex *v1, *v2, *v3;
        GtsTriangle *t;
        {
            GSList * list = NULL;
            for(unsigned int i = 0; i < vertices->len; i++)
            {
                list = g_slist_prepend(list, g_ptr_array_index(vertices, i));
            }
            t = gts_triangle_enclosing(gts_triangle_class(), list, 100.);
            g_slist_free(list);
        }
        gts_triangle_vertices(t, (GtsVertex**)&v1, (GtsVertex**)&v2, (GtsVertex**)&v3);

        // create surface with one face: the enclosing triangle
        m_surface = gts_surface_new(gts_surface_class(),
                                    gts_face_class(),
                                    (GtsEdgeClass*)gts_flc_edge_class(),
                                    (GtsVertexClass*)gts_flc_vertex_class());

        gts_surface_add_face(m_surface, gts_face_new(gts_face_class(), t->e1, t->e2, t->e3));

        // add vertices
        for (unsigned int i = 0; i < vertices->len; i++)
        {
            GtsVertex *v1 = (GtsVertex*)g_ptr_array_index(vertices, i);
            GtsVertex *v = (GtsVertex*)gts_delaunay_add_vertex(m_surface, v1, NULL);

            g_assert (v != v1);
            if (v != NULL)
            {
                gts_vertex_replace(v1, v);
            }
        }

        g_ptr_array_free(vertices, TRUE);

        // add remaining constraints
        gts_fifo_foreach(edges, (GtsFunc)add_constraint, m_surface);

        // destroy enclosing triangle
        gts_allow_floating_vertices = TRUE;
        gts_object_destroy(GTS_OBJECT(v1));
        gts_object_destroy(GTS_OBJECT(v2));
        gts_object_destroy(GTS_OBJECT(v3));
        gts_allow_floating_vertices = FALSE;

        gts_delaunay_remove_hull(m_surface);

        int steiner_max = -1; // can add unlimited steiner pts
        guint encroached_number = gts_delaunay_conform(m_surface,
                                                       steiner_max,
                                                       (GtsEncroachFunc)gts_vertex_encroaches_edge,
                                                       NULL);

        if (encroached_number == 0)
        {
            guint unrefined_number;
            gpointer data[2];

            data[0] = &triangle_quality;
            unrefined_number = gts_delaunay_refine(m_surface,
                                                   steiner_max,
                                                   (GtsEncroachFunc)gts_vertex_encroaches_edge,
                                                   NULL,
                                                   (GtsKeyFunc)triangle_cost,
                                                   data);
        }

        if(verbose)
        {
            gts_surface_print_stats(m_surface, stderr);
        }

        if(check_delaunay && gts_delaunay_check(m_surface))
        {
            fprintf (stderr, "delaunay: triangulation is not Delaunay\n");
        }

        m_vertices.clear();
        gts_surface_foreach_vertex(m_surface, store_vertices, this);

        m_edges.clear();
        gts_surface_foreach_edge(m_surface, store_edges, this);

        m_faces.clear();
        m_vertices_for_rendering.clear();
        gts_surface_foreach_face(m_surface, store_faces, this);

        compute_LB_weights();
		
		std::cout << "ConstrainedDelaunay created" << std::endl;
    }

    int num_vertices() const { return (int)m_vertices.size(); }
    GtsFlcVertex** vertices() { return &m_vertices[0]; }

    int num_bdry_vertices() const { return (int)m_bdry_vertices.size(); }
    GtsFlcVertex** bdry_vertices() { return &m_bdry_vertices[0]; }

    int num_edges() const { return (int)m_edges.size(); }
    GtsFlcEdge** edges() { return &m_edges[0]; }

    int num_faces() const { return (int)m_faces.size(); }
    GtsFace** faces() { return &m_faces[0]; }

    GtsFlcVertex** vertices_for_rendering() { return &m_vertices_for_rendering[0]; }

    gdouble areaOfBoundaryPoly()
    {
        size_t i,j;
        double area = 0;

        for (i = 0; i < m_bdry_vertices.size();i++)
        {
            j = (i + 1) % m_bdry_vertices.size();
            area += m_bdry_vertices[i]->v.p.x * m_bdry_vertices[j]->v.p.y;
            area -= m_bdry_vertices[i]->v.p.y * m_bdry_vertices[j]->v.p.x;
        }

        area /= 2;
        return(area < 0 ? -area : area);
    }


    bool setBdryCondition(int vert_idx, gdouble col[3])
    {
        if((size_t)vert_idx > m_bdry_vertices.size()) return false;
        GtsFlcVertex *v = m_bdry_vertices[vert_idx];
        v->c[0].r = v->c[1].r = col[0];
        v->c[0].g = v->c[1].g = col[1];
        v->c[0].b = v->c[1].b = col[2];
        return true;
    }

    static gint store_vertices(gpointer item, gpointer data)
    {
        GtsFlcVertex* v= (GtsFlcVertex*)item;
        ConstrainedDelaunay *cd = (ConstrainedDelaunay*)data;
        cd->m_vertices.push_back(v);

        return 0;
    }

    static gint store_edges(gpointer item, gpointer data)
    {
        ConstrainedDelaunay *cd = (ConstrainedDelaunay*)data;
        cd->m_edges.push_back((GtsFlcEdge*)item);
        return 0;
    }

    static gint store_faces(gpointer item, gpointer data)
    {
        ConstrainedDelaunay *cd = (ConstrainedDelaunay*)data;
        cd->m_faces.push_back((GtsFace*)item);

        GtsTriangle *t = (GtsTriangle*)item;
        GtsVertex *v1, *v2, *v3;
        gts_triangle_vertices(t, &v1, &v2, &v3);

        cd->m_vertices_for_rendering.push_back((GtsFlcVertex*)v1);
        cd->m_vertices_for_rendering.push_back((GtsFlcVertex*)v2);
        cd->m_vertices_for_rendering.push_back((GtsFlcVertex*)v3);
        return 0;
    }


    ~ConstrainedDelaunay()
    {
        gts_object_destroy((GtsObject*)m_surface);
    }


    static void add_constraint(GtsConstraint *c, GtsSurface *s)
    {
        g_assert(gts_delaunay_add_constraint(s, c) == NULL);
    }


    static gdouble triangle_cost(GtsTriangle *t, gpointer *data)
    {
        gdouble *min_quality = (gdouble*)data[0];
        gdouble quality = gts_triangle_quality(t);

        if(quality < *min_quality)
        {
            return quality;
        }

        return 0.;
    }

    // LB = Laplace-Beltrami
    void compute_LB_weights()
    {
        for(size_t i = 0; i < m_edges.size(); i++)
        {
            GtsEdge *edge = (GtsEdge*)m_edges[i];

            GtsFlcEdge *e = (GtsFlcEdge*)edge;

            gdouble w[] = { 0, 0 };
            GtsFace *f1, *f2;
            if(!gts_edge_manifold_faces(edge, m_surface, &f1, &f2))
            {
                continue;
            }
            GtsFace *faces[] = { f1, f2 };
            for(int j = 0; j < 2; j++)
            {
                GtsFace *f = faces[j];
                if(!f)
                {
                    continue;
                }

                GtsVertex * v = gts_triangle_vertex_opposite(GTS_TRIANGLE(f), edge);
                GtsSegment *s = (GtsSegment*)edge;
                gdouble x1 = s->v1->p.x - v->p.x;
                gdouble y1 = s->v1->p.y - v->p.y;
                gdouble z1 = s->v1->p.z - v->p.z;
                gdouble x2 = s->v2->p.x - v->p.x;
                gdouble y2 = s->v2->p.y - v->p.y;
                gdouble z2 = s->v2->p.z - v->p.z;

                gdouble dot = x1*x2 + y1*y2 + z1*z2;

                gdouble cr_x = y1*z2-z1*y2;
                gdouble cr_y = z1*x2-x1*z2;
                gdouble cr_z = x1*y2-y1*x2;

                gdouble cr_len = sqrt(cr_x*cr_x + cr_y*cr_y + cr_z*cr_z);

                w[j] = dot/cr_len;
            }
            e->w = -.5*(w[0] + w[1]);
        }

        // find sum of edge weights for each vertex
        for(size_t i = 0; i < m_vertices.size(); i++)
        {
            GtsFlcVertex *v = m_vertices[i];
            gdouble w_sum = 0;
            GSList *j = v->v.segments;
            while(j)
            {
                GtsFlcEdge *e = (GtsFlcEdge *)j->data;
                w_sum += e->w;
                j = j->next;
            }
            v->w = w_sum;
        }
    }

    GtsSurface *m_surface;
    std::vector<GtsFlcVertex*> m_vertices;
    std::vector<GtsFlcVertex*> m_bdry_vertices;
    std::vector<GtsFlcEdge*> m_edges;
    std::vector<GtsFace*> m_faces;
    std::vector<GtsFlcVertex*> m_vertices_for_rendering;
};

#endif // CONSTRAINEDDELAUNAY_H
