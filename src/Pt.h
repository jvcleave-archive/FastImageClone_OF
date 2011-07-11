#ifndef PT_H
#define PT_H

template <class Scalar>
struct Pt
{
    Pt() : x(0), y(0), z(0) {}
    Pt(Scalar x, Scalar y, Scalar z=0) : x(x), y(y), z(z) {}
    Pt(const Pt &other) : x(other.x), y(other.y), z(other.z) {}

    Pt operator+(const Pt &rhs)
    {
        return Pt(x+rhs.x, y+rhs.y, z+rhs.z);
    }

    Pt operator-(const Pt &rhs)
    {
        return Pt(x-rhs.x, y-rhs.y, z-rhs.z);
    }


    friend Pt operator*(Scalar s, const Pt &rhs)
    {
        return Pt(s*rhs.x, s*rhs.y, s*rhs.z);
    }

    Pt& operator+=(const Pt &rhs)
    {
        x +=rhs.x;
        y +=rhs.y;
        z +=rhs.z;
        return *this;
    }


    Pt rotate(Scalar degrees)
    {
        Scalar angle = degrees*(Scalar)(3.141592653589793/180);
        Scalar x_, y_;
        x_ = cos(angle)*x - sin(angle)*y;
        y_ = sin(angle)*x + cos(angle)*y;
        x = x_, y = y_;
    }

    Pt rotated(Scalar degrees)
    {
        Pt ret(*this);
        ret.rotate(degrees);
        return ret;
    }

    Pt cross(const Pt &rhs)
    {
        return Pt(y*rhs.z-z*rhs.y, z*rhs.x-x*rhs.z, x*rhs.y-y*rhs.x);
    }

    Pt normalized()
    {
        Scalar mag = sqrt(x*x + y*y + z*z);
        return Pt(x/mag, y/mag, z/mag);
    }


    Scalar x, y, z;
};



typedef Pt<double> pt_t;

template <class Scalar>
bool operator==(const Pt<Scalar> &lhs, const Pt<Scalar> &rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

#endif // PT_H
