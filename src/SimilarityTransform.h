#ifndef SIMILARITYTRANSFORM_H
#define SIMILARITYTRANSFORM_H

// Rotations in DEGREES
struct SimilarityTransform
{
    SimilarityTransform() : rot(0), scale(1) {}
    SimilarityTransform(pt_t trans, double rot, pt_t center, double scale)
        : rot(rot),
          scale(scale),
          center(center),
          trans(trans)
    {
        gen_mat();
    }

    SimilarityTransform(const SimilarityTransform &other)
    {
        rot = other.rot;
        scale = other.scale;
        center = other.center;
        trans = other.trans;
        m11 = other.m11;
        m12 = other.m12;
        m13 = other.m13;
        m21 = other.m21;
        m22 = other.m22;
        m23 = other.m23;
    }

    SimilarityTransform operator*(const SimilarityTransform &rhs)
    {
        double r = rot + rhs.rot;
        double s = scale*rhs.scale;
        double tx = m11*rhs.m13 + m12*rhs.m23 + m13;
        double ty = m21*rhs.m13 + m22*rhs.m23 + m23;
        return SimilarityTransform(pt_t(tx, ty), r, pt_t(), s);
    }

    void invert()
    {
        rot = -rot;
        scale = 1/scale;
        //pt_t tmp = center;
        //center = trans;
        trans = -1*pt_t(m11*trans.x + m22*trans.y, m21*trans.x + m22*trans.y);
        gen_mat();
    }

    pt_t apply(const pt_t &p)
    {
        return pt_t(m11*p.x + m12*p.y + m13, m21*p.x + m22*p.y + m23, p.z);
    }


    SimilarityTransform inverted()
    {
        SimilarityTransform inverse(*this);
        inverse.invert();
        return inverse;
    }

    double getRot() { return rot; }
    double getScale() { return scale; }
    pt_t   getCenter() { return center; }
    pt_t   getTranslation() { return trans; }

protected:

    void gen_mat()
    {
        double r = rot*3.141592653589793/180;
        m11 = scale * cos(r);
        m12 = -scale * sin(r);
        m21 = -m12;
        m22 = m11;

        m13 = trans.x - (m11*center.x + m12*center.y) + center.x;
        m23 = trans.y - (m21*center.x + m22*center.y) + center.y;
    }

    double rot, scale;
    pt_t center, trans;

    double m11, m12, m13;
    double m21, m22, m23;
};

#endif // SIMILARITYTRANSFORM_H
