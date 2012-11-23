/**
 * \file dcs/perfeval/qn/operation/traffic_rates.hpp
 *
 * \brief Compute the visit ratios of a Queueing Network..
 *
 * Copyright (C) 2009-2010  Distributed Computing System (DCS) Group, Computer
 * Science Department - University of Piemonte Orientale, Alessandria (Italy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_PERFEVAL_QN_OPERATION_TRAFFIC_RATES_HPP
#define DCS_PERFEVAL_QN_OPERATION_TRAFFIC_RATES_HPP


#include <dcs/assert.hpp>
//#include <dcs/math/la/container/identity_matrix.hpp>
//#include <dcs/math/la/operation/lu.hpp>
//#include <dcs/math/la/operation/matrix_basic_operations.hpp>
//#include <dcs/math/la/operation/num_rows.hpp>
//#include <dcs/math/la/operation/subrange.hpp>
//#include <dcs/math/la/operation/sum.hpp>
//#include <dcs/math/la/operation/vector_basic_operations.hpp>
//#include <dcs/math/la/traits/matrix.hpp>
//#include <dcs/math/la/traits/promote.hpp>
//#include <dcs/math/la/traits/vector.hpp>
//#include <boost/numeric/ublas/detail/temporary.hpp>
#include <boost/numeric/ublas/expression_types.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublasx/operation/lu.hpp>
#include <boost/numeric/ublasx/operation/num_rows.hpp>
#include <boost/numeric/ublasx/operation/lsq.hpp>
#include <boost/numeric/ublasx/operation/lu.hpp>
#include <boost/numeric/ublasx/operation/sum.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <boost/numeric/ublas/traits.hpp>
#include <boost/multi_array.hpp>
#include <stdexcept>


namespace dcs { namespace perfeval { namespace qn {

namespace detail {

template <typename ME, typename VE>
struct open_traffic_rates_traits
{
	typedef typename ::boost::numeric::ublas::promote_traits<
				typename ::boost::numeric::ublas::matrix_traits<ME>::value_type,
				typename ::boost::numeric::ublas::vector_traits<VE>::value_type
			>::promote_type value_type;
	typedef ::boost::numeric::ublas::vector<value_type> result_type;
};


template <typename ME>
struct closed_traffic_rates_traits
{
	typedef typename ::boost::numeric::ublas::matrix_traits<ME>::value_type value_type;
	typedef ::boost::numeric::ublas::vector<value_type> result_type;
};

} // Namespace detail


/**
 * \brief Compute the single-class traffic rates for an open queueing network.
 *
 * Traffic rates are computed according to the <em>First-order Traffic
 * Equations</em> [1,2] for which:
 * \f{align}{
 *   \lambda_j &= \lambda_0 p_{0j} + \sum_{i=1}^K lambda_i p_{ij}, \quad j=1,\ldots,K
 * \f}
 * where:
 * - \f$p_{ij}\f$ is the <em>routing probability</em>, that is the fraction of
 *   jobs proceeding next to station \f$j\f$ on completing a service request at
 *   station \f$j\f$;
 * - \f$p_{0j}\f$ is the probability that an exogeneous arrival arrives at
 *   station \f$j\f$.
 *  -\f$\lambda_0\f$ is the exogenous arrival rate.
 * .
 * The product \f$\lambda_0 p_{0j}\f$ can be seen as the arrival rate at station
 * \f$j\f$, that is the fraction of external arrivals to the system which
 * proceed first to station \f$j\f$.
 * By denoting with \f$\gamma_i=\lambda_0 p_{0i}\f$, in matrix form we have:
 * \f[
 *  \mathbf{\lambda}(\mathbf{I}-\mathbf{P})=\mathbf{\gamma}
 * \f]
 *
 * References:
 * - [1] P. Denning et al.
 *       "The Operational Analysis of Queueing Network Models".
 *       Computing Surveys 10(3):225-261, 1978
 * - [2] G. Bolch et al.
 *       "Queueing Networks and Markov Chains".
 *       John Wiley & Sons, Inc. 1998.
 * .
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename MatrixExprT, typename VectorExprT>
typename detail::open_traffic_rates_traits<
		MatrixExprT,
		VectorExprT
	>::result_type traffic_rates(::boost::numeric::ublas::matrix_expression<MatrixExprT> const& P,
								 ::boost::numeric::ublas::vector_expression<VectorExprT> const& lambda0)
{
	typedef typename ::boost::numeric::ublas::promote_traits<
					typename ::boost::numeric::ublas::matrix_traits<MatrixExprT>::size_type,
					typename ::boost::numeric::ublas::vector_traits<VectorExprT>::size_type
				>::promote_type size_type;
	typedef typename detail::open_traffic_rates_traits<MatrixExprT,VectorExprT>::value_type value_type;
	typedef ::boost::numeric::ublas::matrix<value_type> matrix_type;
	typedef typename detail::open_traffic_rates_traits<MatrixExprT,VectorExprT>::result_type vector_type;

	size_type n = ::boost::numeric::ublas::num_rows(P);

	// precondition: n == num_columns(P)
	DCS_ASSERT(
		n == ::boost::numeric::ublasx::num_columns(P),
		throw ::std::invalid_argument("[dcs::perfeval::queue::qnet::traffic_rates] Probability transitions matrix must be a square matrix.")
	);

	// precondition: n == size(lambda0)
	DCS_ASSERT(
		n == ::boost::numeric::ublasx::size(lambda0),
		throw ::std::invalid_argument("[dcs::perfeval::queue::qnet::traffic_rates] External arrival rates vector and probability transitions matrix are of non-conformant size.")
	);

	// precondition: all(lambda0 > 0)
	DCS_ASSERT(
		::boost::numeric::ublasx::all(
			lambda0,
			::std::bind2nd(::std::greater_equal<value_type>(), value_type(0))
		),
		throw ::std::invalid_argument("[dcs::perfeval::queue::qnet::traffic_rates] External arrival rates must be non-negative values.")
	);

	// precondition: all(P >= 0) && all(sum_rows(P) <= 1)
	DCS_ASSERT(
		::boost::numeric::ublasx::all(
			P,
			::std::bind2nd(::std::greater_equal<value_type>(), value_type(0))
		)
		&&
		::boost::numeric::ublasx::all(
			::boost::numeric::ublasx::sum<2>(P),
//			::std::bind2nd(::std::less_equal<value_type>(), value_type(1+1.0e-5))
			::std::bind2nd(::std::less_equal<value_type>(), value_type(1))
		),
		throw ::std::invalid_argument("[dcs::perfeval::queue::qnet::traffic_rates] Probability transitions matrix is not a stochastic matrix.")
	);

	matrix_type A;
	vector_type b;

	// A = [ I_n - P ]
	A = ::boost::numeric::ublas::identity_matrix<value_type>(n) - P;

	// b = \lambda_0
	b = lambda0;

	// Find \lambda s.t. \lambda A=b.
	// This is achieved by solving the system (A'\b')'
	A = ::boost::numeric::ublas::trans(A);
	size_type singular;
	singular = ::boost::numeric::ublasx::lu_solve_inplace(A, b);
	if (singular)
	{
		b = vector_type();
	}

	return b;
}


/**
 * \brief Compute the single-class traffic rates for a closed queueing network.
 *
 * Traffic rates are computed according to the <em>Traffic Rate Equations</em>
 * [1] for which:
 * \f{align}{
 *   \lambda_j &= \sum_{i=1}^K \lambda_i p_{ij}, \quad j=1,\ldots,K
 * \f}
 * where:
 * - \f$p_{ij}\f$ is the <em>routing probability</em>, that is the fraction of
 *   jobs proceeding next to station \f$j\f$ on completing a service request at
 *   station \f$j\f$.
 * .
 * In matrix form we have:
 * \f[
 *  \mathbf{\lambda}(\mathbf{I}-\mathbf{Q})=\mathbf{0}
 * \f]
 * Since there are only \f$(K-1)\f$ independent equations for the visit ratios
 * in closed networks, the \f$\lambda_j\f$ can only be determined up to a
 * multiplicative constant.
 * Usually one assume that \f$\lambda_1=1\f$, although other possibilities are
 * used as well.
 * In this case, the solution is found in the least-square sense.
 * Note, if we set \f$\lambda_1=1\f$, then \f$\lambda_i\f$ (\f$i=1\f$) can be
 * interpreted as the mean number of visits by a customer to node \f$i\ne1\f$
 * relative to the number of visits to node \f$1\f$.
 * Therefore, the \f$\lambda_i\f$ is sometimes called the <em>relative
 * visitation rate</em> of node \f$i\f$.
 *
 * References:
 * - [1] G. Bolch et al.
 *       "Queueing Networks and Markov Chains".
 *       John Wiley & Sons, Inc. 1998.
 * .
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename MatrixExprT>
typename detail::closed_traffic_rates_traits<MatrixExprT>::result_type traffic_rates(::boost::numeric::ublas::matrix_expression<MatrixExprT> const& P)
{
	typedef typename ::boost::numeric::ublas::matrix_traits<MatrixExprT>::value_type value_type;
	typedef typename ::boost::numeric::ublas::matrix_traits<MatrixExprT>::size_type size_type;
	typedef ::boost::numeric::ublas::matrix<value_type> matrix_type;
	typedef typename detail::closed_traffic_rates_traits<MatrixExprT>::result_type vector_type;

	size_type n = ::boost::numeric::ublasx::num_rows(P);

	DCS_ASSERT(
		n == ::boost::numeric::ublasx::num_columns(P),
		throw ::std::invalid_argument("[dcs::perfeval::queue::qnet::traffic_rates] Probability transitions matrix must be a square matrix.")
	);

	DCS_ASSERT(
		::boost::numeric::ublasx::all(
			P,
			::std::bind2nd(::std::greater_equal<value_type>(), value_type(0))
		)
		&&
		::boost::numeric::ublasx::all(
			::boost::numeric::ublasx::sum<2>(P),
//			::std::bind2nd(::std::less_equal<value_type>(), value_type(1+1e-5))
			::std::bind2nd(::std::less_equal<value_type>(), value_type(1))
		),
		throw ::std::invalid_argument("[dcs::perfeval::queue::qnet::traffic_rates] Probability transitions matrix is not a stochastic matrix.")
	);

	matrix_type A(n, n+1, 0);
	vector_type b(n+1, 0);

	// A = [(P-I_n) [1; 0; ... 0]] (this will impose \lambda_1=1)
	::boost::numeric::ublas::subrange(A, 0, n, 0, n) = P - ::boost::numeric::ublas::identity_matrix<value_type>(n);
	A(0, n) = value_type(1);

	// b = [ 0 ... 0 1 ] (row vector)
	b(n) = value_type(1);

	vector_type lambda(n);

	// Find \lambda s.t. \lambda A = b
	// Since the system is underdetermined, \lambda is the least-squares solution,
	// that is the solution that minimizes \|A'\lambda'-b'\|
	A = ::boost::numeric::ublas::trans(A);
	//b = ::boost::numeric::ublas::trans(b); // useless
	lambda = ::boost::numeric::ublasx::llsq(A, b);
	//lambda = ::boost::numeric::ublas::trans(lambda); // useless

	return lambda;
}

}}} // Namespace dcs::perfeval::qn


#endif // DCS_PERFEVAL_QN_OPERATION_TRAFFIC_RATES_HPP
