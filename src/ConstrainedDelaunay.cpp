#include "ConstrainedDelaunay.h"

GtsFlcVertex* gts_flc_vertex_new(GtsFlcVertexClass *klass, gdouble x, gdouble y, gdouble z)
{
    GtsFlcVertex *v = GTS_FLC_VERTEX (gts_object_new (GTS_OBJECT_CLASS (klass)));
    v->v.p.x = x;
    v->v.p.y = y;
    v->v.p.z = z;
    v->w = 0;
    v->U = v->V = 0;
    v->c[0] = v->c[1] = Colour();
    return v;
}

void flc_vertex_clone (GtsObject * clone_obj, GtsObject * object)
{
    (*GTS_OBJECT_CLASS (gts_flc_vertex_class ())->parent_class->clone) (clone_obj, object);

    GtsFlcVertex *orig = GTS_FLC_VERTEX(object);
    GtsFlcVertex *clone = GTS_FLC_VERTEX(clone_obj);
    clone->is_bdry = orig->is_bdry;
    clone->c[0] = orig->c[0];
    clone->c[1] = orig->c[1];
    clone->w = orig->w;
    clone->U = orig->U;
    clone->V = orig->V;
}

void flc_vertex_destroy (GtsObject * object)
{
    (* GTS_OBJECT_CLASS (gts_flc_vertex_class ())->parent_class->destroy) (object);
}

void flc_vertex_class_init (GtsVertexClass * klass)
{
  klass->intersection_attributes = NULL;
  GTS_OBJECT_CLASS (klass)->clone = flc_vertex_clone;
  GTS_OBJECT_CLASS (klass)->destroy = flc_vertex_destroy;
}

void flc_vertex_init (GtsFlcVertex * vertex)
{
  vertex->is_bdry = false;
}


GtsFlcVertexClass * gts_flc_vertex_class (void)
{
  static GtsFlcVertexClass * klass = NULL;

  if (klass == NULL) {
    GtsObjectClassInfo flc_vertex_info = {
      "GtsFlcVertex",
      sizeof (GtsFlcVertex),
      sizeof (GtsFlcVertexClass),
      (GtsObjectClassInitFunc) flc_vertex_class_init,
      (GtsObjectInitFunc) flc_vertex_init,
      (GtsArgSetFunc) NULL,
      (GtsArgGetFunc) NULL
    };
    klass = (GtsFlcVertexClass*)gts_object_class_new (GTS_OBJECT_CLASS (gts_vertex_class ()), &flc_vertex_info);
  }

  return klass;
}


GtsFlcEdge* gts_flc_edge_new(GtsFlcEdgeClass *klass, GtsVertex * v1, GtsVertex * v2)
{
    GtsFlcEdge *e = (GtsFlcEdge*)GTS_FLC_EDGE(gts_edge_new(GTS_EDGE_CLASS (klass), v1, v2));
    e->w = 1;
    return e;
}

void flc_edge_clone (GtsObject * clone_obj, GtsObject * object)
{
    (*GTS_OBJECT_CLASS (gts_flc_edge_class ())->parent_class->clone) (clone_obj, object);

    GtsFlcEdge *orig = GTS_FLC_EDGE(object);
    GtsFlcEdge *clone = GTS_FLC_EDGE(clone_obj);
    clone->w = orig->w;
}

void flc_edge_destroy(GtsObject * object)
{
    (* GTS_OBJECT_CLASS (gts_flc_edge_class ())->parent_class->destroy) (object);
}

void flc_edge_class_init (GtsEdgeClass * klass)
{
  GTS_OBJECT_CLASS (klass)->clone = flc_edge_clone;
  GTS_OBJECT_CLASS (klass)->destroy = flc_edge_destroy;
}

void flc_edge_init (GtsFlcEdge * edge)
{
  edge->w = 0;
}

GtsFlcEdgeClass * gts_flc_edge_class (void)
{
  static GtsFlcEdgeClass * klass = NULL;

  if (klass == NULL) {
    GtsObjectClassInfo flc_edge_info = {
      "GtsFlcEdge",
      sizeof (GtsFlcEdge),
      sizeof (GtsFlcEdgeClass),
      (GtsObjectClassInitFunc) flc_edge_class_init,
      (GtsObjectInitFunc) flc_edge_init,
      (GtsArgSetFunc) NULL,
      (GtsArgGetFunc) NULL
    };
    klass = (GtsFlcEdgeClass*)gts_object_class_new (GTS_OBJECT_CLASS (gts_edge_class ()), &flc_edge_info);
  }

  return klass;
}
