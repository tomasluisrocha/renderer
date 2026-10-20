#pragma once
#include <cmath>
#include <cassert>
#include <iostream>

// Vectors

template <int n>
struct vec
{
    double data[n] = {0};
    double &operator[](const int i)
    {
        assert(i >= 0 && i < n);
        return data[i];
    }
    double operator[](const int i) const
    {
        assert(i >= 0 && i < n);
        return data[i];
    }
};

template <>
struct vec<2>
{
    double x = 0, y = 0;
    double &operator[](const int i)
    {
        assert(i >= 0 && i < 2);
        return i ? y : x;
    }
    double operator[](const int i) const
    {
        assert(i >= 0 && i < 2);
        return i ? y : x;
    }
};

template <>
struct vec<3>
{
    double x = 0, y = 0, z = 0;
    double &operator[](const int i)
    {
        assert(i >= 0 && i < 3);
        return i ? (1 == i ? y : z) : x;
    }
    double operator[](const int i) const
    {
        assert(i >= 0 && i < 3);
        return i ? (1 == i ? y : z) : x;
    }
    vec<2> xy() const { return {x, y}; }
};

template <>
struct vec<4>
{
    double x = 0, y = 0, z = 0, w = 0;
    double &operator[](const int i)
    {
        assert(i >= 0 && i < 4);
        switch (i)
        {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        default:
            return w;
        }
    }
    double operator[](const int i) const
    {
        assert(i >= 0 && i < 4);
        switch (i)
        {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        default:
            return w;
        }
    }
    vec<2> xy() const { return {x, y}; }
    vec<3> xyz() const { return {x, y, z}; }
};

typedef vec<2> vec2;
typedef vec<3> vec3;
typedef vec<4> vec4;

// Operators

template <int n>
std::ostream &operator<<(std::ostream &out, const vec<n> &v)
{
    for (int i = 0; i < n; i++)
        out << v[i] << " ";
    return out;
}

template <int n>
vec<n> operator+(const vec<n> &v1, const vec<n> &v2)
{
    vec<n> result;
    for (int i = 0; i < n; i++)
        result[i] = v1[i] + v2[i];
    return result;
}

template <int n>
vec<n> operator-(const vec<n> &v1, const vec<n> &v2)
{
    vec<n> result;
    for (int i = 0; i < n; i++)
        result[i] = v1[i] - v2[i];
    return result;
}

template <int n>
vec<n> operator-(const vec<n> &v1)
{
    vec<n> result;
    for (int i = 0; i < n; i++)
        result[i] = -v1[i];
    return result;
}

template <int n>
double operator*(const vec<n> &v1, const vec<n> &v2)
{
    double result = 0;
    for (int i = 0; i < n; i++)
        result += v1[i] * v2[i];
    return result;
}

template <int n>
vec<n> operator*(const vec<n> &v, const double scale)
{
    vec<n> result;
    for (int i = 0; i < n; i++)
        result[i] = v[i] * scale;
    return result;
}

template <int n>
vec<n> operator*(const double scale, const vec<n> &v)
{
    vec<n> result;
    for (int i = 0; i < n; i++)
        result[i] = v[i] * scale;
    return result;
}

template <int n>
vec<n> operator/(const vec<n> &v, const double scale)
{
    assert(scale != 0);
    vec<n> result;
    for (int i = 0; i < n; i++)
        result[i] = v[i] / scale;
    return result;
}

// Functions

template <int n>
double norm_squared(const vec<n> &v)
{
    return (v * v);
}

template <int n>
double norm(const vec<n> &v)
{
    return std::sqrt(norm_squared(v));
}

template <int n>
vec<n> unit_v(const vec<n> &v)
{
    double len = norm(v);
    assert(len != 0);
    return v / len;
}

inline vec3 cross(const vec3 &a, const vec3 &b)
{
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

inline vec4 embed(const vec3 &v, const double w = 1.)
{
    return {v.x, v.y, v.z, w};
}

// Matrices

template <int R, int C>
struct mat
{
    vec<C> rows[R] = {{}};

    vec<C> &operator[](const int i)
    {
        assert(i >= 0 && i < R);
        return rows[i];
    }
    const vec<C> &operator[](const int i) const
    {
        assert(i >= 0 && i < R);
        return rows[i];
    }

    // Identity matrix
    static mat<R, C> ident()
    {
        mat<R, C> result;
        for (int i = 0; i < R; i++)
        {
            for (int j = 0; j < C; j++)
            {
                result[i][j] = (i == j ? 1.0 : 0.0);
            }
        }
        return result;
    }
};

// Matrix-Vector multiplication: mat<R,C> * vec<C> = vec<R>
template <int R, int C>
vec<R> operator*(const mat<R, C> &m, const vec<C> &v)
{
    vec<R> result;
    for (int i = 0; i < R; i++)
    {
        double sum = 0;
        for (int j = 0; j < C; j++)
        {
            sum += m[i][j] * v[j];
        }
        result[i] = sum;
    }
    return result;
}

template <int R, int K, int C>
mat<R, C> operator*(const mat<R, K> &a, const mat<K, C> &b)
{
    mat<R, C> result;
    for (int i = 0; i < R; i++)
    {
        for (int j = 0; j < C; j++)
        {
            double sum = 0;
            for (int k = 0; k < K; k++)
            {
                sum += a[i][k] * b[k][j];
            }
            result[i][j] = sum;
        }
    }
    return result;
}

inline double det(const mat<3, 3> &m)
{
    return m[0] * cross(m[1], m[2]);
}

// The columns of M^-1 are cross(row1,row2), cross(row2,row0), cross(row0,row1),
// each divided by det(M), so the rows of (M^-1)^T are those same vectors.
inline mat<3, 3> invert_transpose(const mat<3, 3> &m)
{
    vec3 c0 = cross(m[1], m[2]);
    vec3 c1 = cross(m[2], m[0]);
    vec3 c2 = cross(m[0], m[1]);
    double d = m[0] * c0; // same value as det(m), without recomputing the cross
    assert(d != 0);
    return {{c0 / d, c1 / d, c2 / d}};
}

template <int R, int C>
std::ostream &operator<<(std::ostream &out, const mat<R, C> &m)
{
    for (int i = 0; i < R; i++)
        out << m[i] << "\n";
    return out;
}

typedef mat<3, 3> matrix33;
typedef mat<4, 4> matrix44;