/* ========================================================================= *
 *                                                                           *
 *                               OpenMesh                                    *
 *           Copyright (c) 2001-2015, RWTH-Aachen University                 *
 *           Department of Computer Graphics and Multimedia                  *
 *                          All rights reserved.                             *
 *                            www.openmesh.org                               *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * This file is part of OpenMesh.                                            *
 *---------------------------------------------------------------------------*
 *                                                                           *
 * Redistribution and use in source and binary forms, with or without        *
 * modification, are permitted provided that the following conditions        *
 * are met:                                                                  *
 *                                                                           *
 * 1. Redistributions of source code must retain the above copyright notice, *
 *    this list of conditions and the following disclaimer.                  *
 *                                                                           *
 * 2. Redistributions in binary form must reproduce the above copyright      *
 *    notice, this list of conditions and the following disclaimer in the    *
 *    documentation and/or other materials provided with the distribution.   *
 *                                                                           *
 * 3. Neither the name of the copyright holder nor the names of its          *
 *    contributors may be used to endorse or promote products derived from   *
 *    this software without specific prior written permission.               *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED *
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A           *
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,  *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,       *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR        *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              *
 *                                                                           *
 * ========================================================================= */

#ifndef OPENMESH_SRC_OPENMESH_CORE_GEOMETRY_VECTOR11T_HH_
#define OPENMESH_SRC_OPENMESH_CORE_GEOMETRY_VECTOR11T_HH_

#include <array>
#include <utility>
#include <algorithm>
#include <type_traits>
#include <cmath>
#include <ostream>
#include <istream>
#include <cassert>
#include <cstdlib>

// This header is not needed by this file but expected by others including
// this file.
#include <OpenMesh/Core/System/config.h>


/*
 * Helpers for VectorT
 */
namespace {

template<typename ... Ts>
struct are_convertible_to;

template<typename To, typename From, typename ... Froms>
struct are_convertible_to<To, From, Froms...> {
    static constexpr bool value = std::is_convertible<From, To>::value
            && are_convertible_to<To, Froms...>::value;
};

template<typename To, typename From>
struct are_convertible_to<To, From> : public std::is_convertible<From, To> {
};
}

namespace OpenMesh {

template<typename Scalar, int DIM>
class VectorT {
    private:
        std::array<Scalar, DIM> values_;

    public:

        //---------------------------------------------------------------- class info

        /// the type of the scalar used in this template
        typedef Scalar value_type;

        /// type of this vector
        typedef VectorT<Scalar, DIM> vector_type;

        /// returns dimension of the vector (deprecated)
        static inline int dim() {
            return DIM;
        }

        /// returns dimension of the vector
        static inline size_t size() {
            return DIM;
        }

        static const size_t size_ = DIM;

        //-------------------------------------------------------------- constructors

        /// default constructor creates uninitialized values.
        constexpr VectorT() {}

        /**
         * Creates a vector with all components set to v.
         */
        explicit VectorT(const Scalar &v) {
            vectorize(v);
        }

        template<typename ... T,
            typename = typename std::enable_if<sizeof...(T) == DIM>::type,
            typename = typename std::enable_if<
                are_convertible_to<Scalar, T...>::value>::type>
        constexpr VectorT(T... vs) : values_ { {static_cast<Scalar>(vs)...} }
        {}

        /**
         * Only for 4-component vectors with division operator on their
         * Scalar: Dehomogenization.
         */
        template<int D = DIM,
            typename = typename std::enable_if<D == DIM>::type,
            typename = decltype(values_[0]/values_[3])>
        typename std::enable_if<D==4, VectorT>::type
        homogenized() const {
            return VectorT(
                    values_[0]/values_[3],
                    values_[1]/values_[3],
                    values_[2]/values_[3],
                    1);
        }

        /// construct from a value array (explicit)
        explicit inline VectorT(const Scalar _values[DIM]) {
            std::copy(_values, _values + DIM, values_.begin());
        }

        /// copy & cast constructor (explicit)
        template<typename otherScalarType,
            typename = typename std::enable_if<
                std::is_convertible<otherScalarType, Scalar>::value>>
        explicit inline VectorT(const VectorT<otherScalarType, DIM>& _rhs) {
            operator=(_rhs);
        }

        //--------------------------------------------------------------------- casts

        /// cast from vector with a different scalar type
        template<typename OtherScalar,
            typename = typename std::enable_if<
                std::is_convertible<OtherScalar, Scalar>::value>>
        vector_type& operator=(const VectorT<OtherScalar, DIM>& _rhs) {
            std::transform(_rhs.data(), _rhs.data() + DIM,
                    data(), [](OtherScalar rhs) {
                        return static_cast<Scalar>(std::move(rhs));
                    });
            return *this;
        }

        /// access to Scalar array
        Scalar* data() { return values_.data(); }

        /// access to const Scalar array
        const Scalar *data() const { return values_.data(); }

        //----------------------------------------------------------- element access

        /// get i'th element read-write
        Scalar& operator[](size_t _i) {
            assert(_i < DIM);
            return values_[_i];
        }

        /// get i'th element read-only
        const Scalar& operator[](size_t _i) const {
            assert(_i < DIM);
            return values_[_i];
        }

        //---------------------------------------------------------------- comparsion

        /// component-wise comparison
        bool operator==(const vector_type& _rhs) const {
            return std::equal(_rhs.values_.cbegin(), _rhs.values_.cend(), values_.cbegin());
        }

        /// component-wise comparison
        bool operator!=(const vector_type& _rhs) const {
            return !std::equal(_rhs.values_.cbegin(), _rhs.values_.cend(), values_.cbegin());
        }

        //---------------------------------------------------------- scalar operators

        /// component-wise self-multiplication with scalar
        template<typename OtherScalar>
        typename std::enable_if<std::is_convertible<
                decltype(std::declval<Scalar>() * std::declval<OtherScalar>()),
                Scalar>::value,
            VectorT<Scalar, DIM>&>::type
        operator*=(const OtherScalar& _s) {
            std::transform(values_.begin(), values_.end(), values_.begin(),
                    [&_s](const Scalar & c) { return c * _s; });
            return *this;
        }

        /** component-wise self-division by scalar
         \attention v *= (1/_s) is much faster than this  */
        template<typename OtherScalar>
        typename std::enable_if<std::is_convertible<
                decltype(std::declval<Scalar>() / std::declval<OtherScalar>()),
                Scalar>::value,
            VectorT<Scalar, DIM>&>::type
        operator/=(const OtherScalar& _s) {
            std::transform(values_.begin(), values_.end(), values_.begin(),
                    [&_s](const Scalar & c) { return c / _s; });
            return *this;
        }

        /// component-wise multiplication with scalar
        //template<typename OtherScalar,
        //    typename ResultScalar = decltype(std::declval<Scalar>() *
        //                                     std::declval<OtherScalar>())>
        //VectorT<ResultScalar, DIM> operator*(const OtherScalar& _s) const {
        //    return vector_type(*this) *= _s;
        //}
        template<typename OtherScalar>
        typename std::enable_if<std::is_convertible<
                decltype(std::declval<Scalar>() * std::declval<OtherScalar>()),
                Scalar>::value,
            VectorT<Scalar, DIM>>::type
        operator*(const OtherScalar& _s) const {
            return vector_type(*this) *= _s;
        }

        /// component-wise division by with scalar
        template<typename OtherScalar>
        typename std::enable_if<std::is_convertible<
                decltype(std::declval<Scalar>() / std::declval<OtherScalar>()),
                Scalar>::value,
            VectorT<Scalar, DIM>>::type
        operator/(const OtherScalar& _s) const {
            return vector_type(*this) /= _s;
        }

        //---------------------------------------------------------- vector operators

        /// component-wise self-multiplication
        template<typename OtherScalar,
            typename = decltype(std::declval<Scalar>() *
                                std::declval<OtherScalar>())>
        vector_type& operator*=(const VectorT<OtherScalar, DIM>& _rhs) {
            std::transform(data(), data() + DIM,
                    _rhs.data(), data(),
                    [](const Scalar &l, const OtherScalar &r) { return l * r; });
            return *this;
        }

        /// component-wise self-division
        template<typename OtherScalar,
            typename = decltype(std::declval<Scalar>() /
                                std::declval<OtherScalar>())>
        vector_type& operator/=(const VectorT<OtherScalar, DIM>& _rhs) {
            std::transform(data(), data() + DIM,
                    _rhs.data(), data(),
                    [](const Scalar &l, const OtherScalar &r) { return l / r; });
            return *this;
        }

        /// vector difference from this
        template<typename OtherScalar,
            typename = decltype(std::declval<Scalar>() -
                                std::declval<OtherScalar>())>
        vector_type& operator-=(const VectorT<OtherScalar, DIM>& _rhs) {
            std::transform(data(), data() + DIM,
                    _rhs.data(), data(),
                    [](const Scalar &l, const OtherScalar &r) { return l - r; });
            return *this;
        }

        /// vector self-addition
        template<typename OtherScalar,
            typename = decltype(std::declval<Scalar>() +
                                std::declval<OtherScalar>())>
        vector_type& operator+=(const VectorT<OtherScalar, DIM>& _rhs) {
            std::transform(data(), data() + DIM,
                    _rhs.data(), data(),
                    [](const Scalar &l, const OtherScalar &r) { return l + r; });
            return *this;
        }

        /// component-wise vector multiplication
        template<typename OtherScalar,
            typename = decltype(std::declval<vector_type>() *=
                            std::declval<const VectorT<OtherScalar, DIM>&>())>
        vector_type operator*(const VectorT<OtherScalar, DIM>& _rhs) const {
            return vector_type(*this) *= _rhs;
        }

        /// component-wise vector division
        template<typename OtherScalar,
            typename = decltype(std::declval<vector_type>() /=
                            std::declval<const VectorT<OtherScalar, DIM>&>())>
        vector_type operator/(const VectorT<OtherScalar, DIM>& _rhs) const {
            return vector_type(*this) /= _rhs;
        }

        /// component-wise vector addition
        template<typename OtherScalar,
            typename = decltype(std::declval<vector_type>() +=
                            std::declval<const VectorT<OtherScalar, DIM>&>())>
        vector_type operator+(const VectorT<OtherScalar, DIM>& _rhs) const {
            return vector_type(*this) += _rhs;
        }

        /// component-wise vector difference
        template<typename OtherScalar,
            typename = decltype(std::declval<vector_type>() -=
                            std::declval<const VectorT<OtherScalar, DIM>&>())>
        vector_type operator-(const VectorT<OtherScalar, DIM>& _rhs) const {
            return vector_type(*this) -= _rhs;
        }

        /// unary minus
        vector_type operator-(void) const {
            vector_type v;
            std::transform(v.values_.begin(), v.values_.end(), v.values_.begin(),
                    [](const Scalar &s) { return -s; });
            return v;
        }

        /// cross product: only defined for Vec3* as specialization
        /// \see OpenMesh::cross
        template<typename OtherScalar,
            typename ResultScalar = decltype(
                std::declval<Scalar>() * std::declval<const OtherScalar &>() -
                std::declval<Scalar>() * std::declval<const OtherScalar &>())>
        typename std::enable_if<DIM == 3, VectorT<ResultScalar, DIM>>::type
        operator% (const VectorT<OtherScalar, DIM> &_rhs) const {
            return VectorT<ResultScalar, DIM>(
                values_[1] * _rhs.values_[2] - values_[2] * _rhs.values_[1],
                values_[2] * _rhs.values_[0] - values_[0] * _rhs.values_[2],
                values_[0] * _rhs.values_[1] - values_[1] * _rhs.values_[0]);
        }

        /// compute scalar product
        /// \see OpenMesh::dot
        template<typename OtherScalar,
            typename ResultScalar = decltype(
                std::declval<Scalar>() * std::declval<const OtherScalar &>())>
        typename std::enable_if<DIM == 3, ResultScalar>::type
        operator|(const VectorT<OtherScalar, DIM>& _rhs) const {
            ResultScalar p = values_[0] * _rhs[0];
            for (int i = 1; i < DIM; ++i) {
                p += values_[i] * _rhs[i];
            }
            return p;
        }

        //------------------------------------------------------------ euclidean norm

        /// \name Euclidean norm calculations
        //@{

        /// compute squared euclidean norm
        decltype(values_[0] * values_[0]) sqrnorm() const {
            typedef decltype(values_[0] * values_[0]) RESULT;
            return std::accumulate(values_.cbegin() + 1, values_.cend(),
                    values_[0] * values_[0],
                    [](const RESULT &l, const Scalar &r) { return l + r * r; });
        }

        /// compute euclidean norm
        decltype(std::sqrt(std::declval<vector_type>().sqrnorm())) norm() const {
            return std::sqrt(sqrnorm());
        }

        decltype(std::declval<vector_type>().norm()) length() const {
            return norm();
        }

        /** normalize vector, return normalized vector
         */
        vector_type& normalize() {
            *this /= norm();
            return *this;
        }

        /** return normalized vector
         */
        decltype(std::declval<vector_type>() / std::declval<vector_type>().norm())
        normalized() const {
            return *this / norm();
        }

        /** normalize vector, return normalized vector and avoids div by zero
         */
        vector_type& normalize_cond() {
            Scalar n = norm();
            if (n != (Scalar)0.0)
            {
                *this /= n;
            }
            return *this;
        }

        //@}

        //------------------------------------------------------------ euclidean norm

        /// \name Non-Euclidean norm calculations
        //@{

        /// compute L1 (Manhattan) norm
        Scalar l1_norm() const {
            return std::accumulate(
                    values_.cbegin() + 1, values_.end(), values_[0]);
        }

        /// compute l8_norm
        Scalar l8_norm() const {
            return max_abs();
        }

        //@}

        //------------------------------------------------------------ max, min, mean

        /// \name Minimum maximum and mean
        //@{

        /// return the maximal component
        Scalar max() const {
            return *std::max_element(values_.cbegin(), values_.cend());
        }

        /// return the maximal absolute component
        Scalar max_abs() const {
            return std::abs(
                *std::max_element(values_.cbegin(), values_.cend(),
                    [](const Scalar &a, const Scalar &b) {
                return std::abs(a) < std::abs(b);
            }));
        }

        /// return the minimal component
        Scalar min() const {
            return *std::min_element(values_.cbegin(), values_.cend());
        }

        /// return the minimal absolute component
        Scalar min_abs() const {
            return std::abs(
                *std::min_element(values_.cbegin(), values_.cend(),
                    [](const Scalar &a, const Scalar &b) {
                return std::abs(a) < std::abs(b);
            }));
        }

        /// return arithmetic mean
        Scalar mean() const {
            return l1_norm()/DIM;
        }

        /// return absolute arithmetic mean
        Scalar mean_abs() const {
            return std::accumulate(values_.cbegin() + 1, values_.end(),
                    std::abs(values_[0]),
                    [](const Scalar &l, const Scalar &r) {
                        return l + std::abs(r);
                    }) / DIM;
        }

        /// minimize values: same as *this = min(*this, _rhs), but faster
        vector_type& minimize(const vector_type& _rhs) {
            std::transform(values_.cbegin(), values_.cend(),
                    _rhs.values_.cbegin(),
                    values_.begin(),
                    [](const Scalar &l, const Scalar &r) {
                        return std::min(l, r);
                    });
            return *this;
        }

        /// minimize values and signalize coordinate minimization
        bool minimized(const vector_type& _rhs) {
            bool result = false;
            std::transform(values_.cbegin(), values_.cend(),
                    _rhs.values_.cbegin(),
                    values_.begin(),
                    [&result](const Scalar &l, const Scalar &r) {
                        if (l < r) {
                            return l;
                        } else {
                            result = true;
                            return r;
                        }
                    });
            return result;
        }

        /// maximize values: same as *this = max(*this, _rhs), but faster
        vector_type& maximize(const vector_type& _rhs) {
            std::transform(values_.cbegin(), values_.cend(),
                    _rhs.values_.cbegin(),
                    values_.begin(),
                    [](const Scalar &l, const Scalar &r) {
                        return std::max(l, r);
                    });
            return *this;
        }

        /// maximize values and signalize coordinate maximization
        bool maximized(const vector_type& _rhs) {
            bool result = false;
            std::transform(values_.cbegin(), values_.cend(),
                    _rhs.values_.cbegin(),
                    values_.begin(),
                    [&result](const Scalar &l, const Scalar &r) {
                        if (l > r) {
                            return l;
                        } else {
                            result = true;
                            return r;
                        }
                    });
            return result;
        }

        /// component-wise min
        inline vector_type min(const vector_type& _rhs) const {
            return vector_type(*this).minimize(_rhs);
        }

        /// component-wise max
        inline vector_type max(const vector_type& _rhs) const {
            return vector_type(*this).maximize(_rhs);
        }

        //@}

        //------------------------------------------------------------ misc functions

        /// component-wise apply function object with Scalar operator()(Scalar).
        template<typename Functor>
        inline vector_type apply(const Functor& _func) const {
            vector_type result;
            std::transform(result.values_.begin(), result.values_.end(),
                    result.values_.begin(), _func);
            return result;
        }

        /// store the same value in each component (e.g. to clear all entries)
        vector_type& vectorize(const Scalar& _s) {
            std::fill(values_.begin(), values_.end(), _s);
            return *this;
        }

        /// store the same value in each component
        static vector_type vectorized(const Scalar& _s) {
            return vector_type().vectorize(_s);
        }

        /// lexicographical comparison
        bool operator<(const vector_type& _rhs) const {
            return std::lexicographical_compare(
                    values_.begin(), values_.end(),
                    _rhs.values_.begin(), _rhs.values_.end());
        }

    public:
        /// Component wise multiplication from the left
        template<typename OtherScalar>
        friend
        decltype(std::declval<const vector_type &>().operator*(std::declval<OtherScalar>()))
        operator*(const OtherScalar& _s, const vector_type &rhs) {
            return rhs * _s;
        }
};

/// output a vector by printing its space-separated compontens
template<typename Scalar, int DIM>
typename std::enable_if<sizeof(decltype(
        std::declval<std::ostream>() <<
        std::declval<const Scalar &>())) >= 0, std::ostream&>::type
operator<<(std::ostream& os, const VectorT<Scalar, DIM> &_vec) {
    os << _vec[0];
    for (int i = 1; i < DIM; ++i) {
        os << " " << _vec[i];
    }
    return os;
}

/// read the space-separated components of a vector from a stream
template<typename Scalar, int DIM>
typename std::enable_if<sizeof(decltype(
        std::declval<std::ostream>() >>
        std::declval<const Scalar &>())) >= 0, std::istream&>::type
operator>> (std::istream& is, VectorT<Scalar, DIM> &_vec) {
    for (int i = 0; i < DIM; ++i)
        is >> _vec[i];
    return is;
}

/// \relates OpenMesh::VectorT
/// symmetric version of the dot product
template<typename Scalar, int DIM>
Scalar
dot(const VectorT<Scalar, DIM>& _v1, const VectorT<Scalar, DIM>& _v2) {
  return (_v1 | _v2);
}


/// \relates OpenMesh::VectorT
/// symmetric version of the cross product
template<typename LScalar, typename RScalar, int DIM>
decltype(std::declval<const VectorT<LScalar, DIM>&>() %
         std::declval<const VectorT<RScalar, DIM>&>())
cross(const VectorT<LScalar, DIM>& _v1, const VectorT<RScalar, DIM>& _v2) {
  return (_v1 % _v2);
}


//== TYPEDEFS =================================================================

/** 1-byte signed vector */
typedef VectorT<signed char,1> Vec1c;
/** 1-byte unsigned vector */
typedef VectorT<unsigned char,1> Vec1uc;
/** 1-short signed vector */
typedef VectorT<signed short int,1> Vec1s;
/** 1-short unsigned vector */
typedef VectorT<unsigned short int,1> Vec1us;
/** 1-int signed vector */
typedef VectorT<signed int,1> Vec1i;
/** 1-int unsigned vector */
typedef VectorT<unsigned int,1> Vec1ui;
/** 1-float vector */
typedef VectorT<float,1> Vec1f;
/** 1-double vector */
typedef VectorT<double,1> Vec1d;

/** 2-byte signed vector */
typedef VectorT<signed char,2> Vec2c;
/** 2-byte unsigned vector */
typedef VectorT<unsigned char,2> Vec2uc;
/** 2-short signed vector */
typedef VectorT<signed short int,2> Vec2s;
/** 2-short unsigned vector */
typedef VectorT<unsigned short int,2> Vec2us;
/** 2-int signed vector */
typedef VectorT<signed int,2> Vec2i;
/** 2-int unsigned vector */
typedef VectorT<unsigned int,2> Vec2ui;
/** 2-float vector */
typedef VectorT<float,2> Vec2f;
/** 2-double vector */
typedef VectorT<double,2> Vec2d;

/** 3-byte signed vector */
typedef VectorT<signed char,3> Vec3c;
/** 3-byte unsigned vector */
typedef VectorT<unsigned char,3> Vec3uc;
/** 3-short signed vector */
typedef VectorT<signed short int,3> Vec3s;
/** 3-short unsigned vector */
typedef VectorT<unsigned short int,3> Vec3us;
/** 3-int signed vector */
typedef VectorT<signed int,3> Vec3i;
/** 3-int unsigned vector */
typedef VectorT<unsigned int,3> Vec3ui;
/** 3-float vector */
typedef VectorT<float,3> Vec3f;
/** 3-double vector */
typedef VectorT<double,3> Vec3d;
/** 3-bool vector */
typedef VectorT<bool,3> Vec3b;

/** 4-byte signed vector */
typedef VectorT<signed char,4> Vec4c;
/** 4-byte unsigned vector */
typedef VectorT<unsigned char,4> Vec4uc;
/** 4-short signed vector */
typedef VectorT<signed short int,4> Vec4s;
/** 4-short unsigned vector */
typedef VectorT<unsigned short int,4> Vec4us;
/** 4-int signed vector */
typedef VectorT<signed int,4> Vec4i;
/** 4-int unsigned vector */
typedef VectorT<unsigned int,4> Vec4ui;
/** 4-float vector */
typedef VectorT<float,4> Vec4f;
/** 4-double vector */
typedef VectorT<double,4> Vec4d;

/** 5-byte signed vector */
typedef VectorT<signed char, 5> Vec5c;
/** 5-byte unsigned vector */
typedef VectorT<unsigned char, 5> Vec5uc;
/** 5-short signed vector */
typedef VectorT<signed short int, 5> Vec5s;
/** 5-short unsigned vector */
typedef VectorT<unsigned short int, 5> Vec5us;
/** 5-int signed vector */
typedef VectorT<signed int, 5> Vec5i;
/** 5-int unsigned vector */
typedef VectorT<unsigned int, 5> Vec5ui;
/** 5-float vector */
typedef VectorT<float, 5> Vec5f;
/** 5-double vector */
typedef VectorT<double, 5> Vec5d;

/** 6-byte signed vector */
typedef VectorT<signed char,6> Vec6c;
/** 6-byte unsigned vector */
typedef VectorT<unsigned char,6> Vec6uc;
/** 6-short signed vector */
typedef VectorT<signed short int,6> Vec6s;
/** 6-short unsigned vector */
typedef VectorT<unsigned short int,6> Vec6us;
/** 6-int signed vector */
typedef VectorT<signed int,6> Vec6i;
/** 6-int unsigned vector */
typedef VectorT<unsigned int,6> Vec6ui;
/** 6-float vector */
typedef VectorT<float,6> Vec6f;
/** 6-double vector */
typedef VectorT<double,6> Vec6d;

} // namespace OpenMesh

/**
 * Literal operator for inline specification of colors in HTML syntax.
 *
 * Example:
 * \code{.cpp}
 * OpenMesh::Vec4f light_blue = 0x1FCFFFFF_htmlColor;
 * \endcode
 */
constexpr OpenMesh::Vec4f operator"" _htmlColor(unsigned long long raw_color) {
    return OpenMesh::Vec4f(
            ((raw_color >> 24) & 0xFF) / 255.0f,
            ((raw_color >> 16) & 0xFF) / 255.0f,
            ((raw_color >>  8) & 0xFF) / 255.0f,
            ((raw_color >>  0) & 0xFF) / 255.0f);
}

#endif /* OPENMESH_SRC_OPENMESH_CORE_GEOMETRY_VECTOR11T_HH_ */
