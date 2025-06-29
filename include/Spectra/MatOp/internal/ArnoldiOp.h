// Copyright (C) 2018-2025 Yixuan Qiu <yixuan.qiu@cos.name>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SPECTRA_ARNOLDI_OP_H
#define SPECTRA_ARNOLDI_OP_H

#include <Eigen/Core>
#include <cmath>    // std::sqrt
#include <complex>  // std::real

namespace Spectra {

///
/// \ingroup Internals
/// @{
///

///
/// \defgroup Operators Operators
///
/// Different types of operators.
///

///
/// \ingroup Operators
///
/// Operators used in the Arnoldi factorization.
///
template <typename OpType, typename BOpType>
class ArnoldiOp
{
public:
    using Scalar = typename OpType::Scalar;

private:
    // The real part type of the matrix element
    using RealScalar = typename Eigen::NumTraits<Scalar>::Real;
    using Index = Eigen::Index;
    using Vector = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>;

    const OpType& m_op;
    const BOpType& m_Bop;
    mutable Vector m_cache;

public:
    ArnoldiOp(const OpType& op, const BOpType& Bop) :
        m_op(op), m_Bop(Bop), m_cache(op.rows())
    {}

    // Move constructor
    ArnoldiOp(ArnoldiOp&& other) :
        m_op(other.m_op), m_Bop(other.m_Bop)
    {
        // We emulate the move constructor for Vector using Vector::swap()
        m_cache.swap(other.m_cache);
    }

    inline Index rows() const { return m_op.rows(); }

    // In generalized eigenvalue problem Ax=lambda*Bx, define the inner product to be <x, y> = (x^H)By.
    // For regular eigenvalue problems, it is the usual inner product <x, y> = (x^H)y

    // Compute <x, y> = (x^H)By
    // x and y are two vectors
    template <typename Arg1, typename Arg2>
    Scalar inner_product(const Arg1& x, const Arg2& y) const
    {
        m_Bop.perform_op(y.data(), m_cache.data());
        return x.dot(m_cache);
    }

    // Compute res = <X, y> = (X^H)By
    // X is a matrix, y is a vector, res is a vector
    template <typename Arg1, typename Arg2>
    void adjoint_product(const Arg1& x, const Arg2& y, Eigen::Ref<Vector> res) const
    {
        m_Bop.perform_op(y.data(), m_cache.data());
        res.noalias() = x.adjoint() * m_cache;
    }

    // B-norm of a vector, ||x||_B = sqrt((x^H)Bx)
    template <typename Arg>
    RealScalar norm(const Arg& x) const
    {
        using std::sqrt;
        using std::real;
        return sqrt(real(inner_product<Arg, Arg>(x, x)));
    }

    // The "A" operator to generate the Krylov subspace
    inline void perform_op(const Scalar* x_in, Scalar* y_out) const
    {
        m_op.perform_op(x_in, y_out);
    }
};

///
/// \ingroup Operators
///
/// Placeholder for the B-operator when \f$B = I\f$.
///
class IdentityBOp
{};

///
/// \ingroup Operators
///
/// Partial specialization for the case \f$B = I\f$.
///
template <typename OpType>
class ArnoldiOp<OpType, IdentityBOp>
{
public:
    using Scalar = typename OpType::Scalar;

private:
    // The real part type of the matrix element
    using RealScalar = typename Eigen::NumTraits<Scalar>::Real;
    using Index = Eigen::Index;
    using Vector = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>;

    const OpType& m_op;

public:
    ArnoldiOp(const OpType& op, const IdentityBOp& /*Bop*/) :
        m_op(op)
    {}

    inline Index rows() const { return m_op.rows(); }

    // Compute <x, y> = (x^H)y
    // x and y are two vectors
    template <typename Arg1, typename Arg2>
    Scalar inner_product(const Arg1& x, const Arg2& y) const
    {
        return x.dot(y);
    }

    // Compute res = <X, y> = (X^H)y
    // X is a matrix, y is a vector, res is a vector
    template <typename Arg1, typename Arg2>
    void adjoint_product(const Arg1& x, const Arg2& y, Eigen::Ref<Vector> res) const
    {
        res.noalias() = x.adjoint() * y;
    }

    // B-norm of a vector. For regular eigenvalue problems it is simply the L2 norm
    template <typename Arg>
    RealScalar norm(const Arg& x) const
    {
        return x.norm();
    }

    // The "A" operator to generate the Krylov subspace
    inline void perform_op(const Scalar* x_in, Scalar* y_out) const
    {
        m_op.perform_op(x_in, y_out);
    }
};

///
/// @}
///

}  // namespace Spectra

#endif  // SPECTRA_ARNOLDI_OP_H
