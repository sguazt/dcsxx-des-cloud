/**
 * \file dcs/des/cloud/lq_application_controller.hpp
 *
 * \brief Class modeling the application controller component using an LQ
 *  controller.
 *
 * Copyright (C) 2009-2011  Distributed Computing System (DCS) Group, Computer
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

#ifndef DCS_DES_CLOUD_LQ_APPLICATION_CONTROLLER_HPP
#define DCS_DES_CLOUD_LQ_APPLICATION_CONTROLLER_HPP

//@{ Consistency checks for macros

#if !defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION) && defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT)
# error Normalized non-deviated input is not allowed.
#endif // !DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION && DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT

#if !defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION) && defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT)
# error Normalized non-deviated output is not allowed.
#endif // !DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION && DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT) && !(defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION) || defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION))
# error Dynamic equilibrium with non-deviated input/output is not allowed.
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT && !(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION || DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION)

#ifndef DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION
# define DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION 'C'
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION
#if    DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION != 'C' \
    && DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION != 'M' \
    && DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION != 'R' \
    && DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION != 'W'
# error Unknwon value for DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION.
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION

#ifdef DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
# error Code is out-of-date.
#endif // DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA

//@} Consistency checks for macros

#include <algorithm>
#include <boost/numeric/ublas/expression_types.hpp>
#ifdef DCS_DEBUG
#	include <boost/numeric/ublas/io.hpp>
#endif // DCS_DEBUG
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/traits.hpp>
#include <boost/numeric/ublasx/operation/all.hpp>
#include <boost/numeric/ublasx/operation/isfinite.hpp>
#include <boost/numeric/ublasx/operation/num_columns.hpp>
#include <boost/numeric/ublasx/operation/num_rows.hpp>
#include <boost/numeric/ublasx/operation/size.hpp>
#include <cstddef>
#include <dcs/control/analysis/controllability.hpp>
#include <dcs/control/analysis/detectability.hpp>
#include <dcs/control/analysis/observability.hpp>
#include <dcs/control/analysis/stabilizability.hpp>
#include <dcs/control/design/dlqi.hpp>
//#include <dcs/control/design/dlqiy.hpp>
#include <dcs/control/design/dlqr.hpp>
#include <dcs/control/design/dlqry.hpp>
#include <dcs/control/design/fmpc.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/mean_estimator.hpp>
#include <dcs/des/cloud/application_controller_triggers.hpp>
#include <dcs/des/cloud/base_application_controller.hpp>
#include <dcs/des/cloud/detail/system_identification_strategies.hpp>
#include <dcs/des/cloud/detail/matlab/controller_proxies.hpp>
#include <dcs/des/cloud/logging.hpp>
#include <dcs/des/cloud/multi_tier_application.hpp>
#include <dcs/des/cloud/performance_measure_category.hpp>
#include <dcs/des/cloud/physical_machine.hpp>
#include <dcs/des/cloud/physical_resource_category.hpp>
#include <dcs/des/cloud/registry.hpp>
#include <dcs/des/cloud/system_identification_strategy_params.hpp>
#include <dcs/des/cloud/utility.hpp>
#include <dcs/exception.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <dcs/sysid/algorithm/rls.hpp>
#include <exception>
#include <map>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>
#ifdef DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
# include <cstdio> 
//# include <iosfwd>
# include <iostream>
# include <fstream>
#endif // DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA


//TODO:
// - Currently the code in this class assumes the single resource (CPU) case.
//


namespace dcs { namespace des { namespace cloud {

namespace detail { namespace /*<unnamed>*/ {

#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_ALT_SS) && DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_ALT_SS == 'X'

template <
//	typename TraitsT,
	typename SysIdentStrategyT,
	typename AMatrixExprT,
	typename BMatrixExprT,
	typename CMatrixExprT,
	typename DMatrixExprT
>
void make_ss(SysIdentStrategyT const& sys_ident_strategy,
			 ::boost::numeric::ublas::matrix_container<AMatrixExprT>& A,
			 ::boost::numeric::ublas::matrix_container<BMatrixExprT>& B,
			 ::boost::numeric::ublas::matrix_container<CMatrixExprT>& C,
			 ::boost::numeric::ublas::matrix_container<DMatrixExprT>& D)
{
//DCS_DEBUG_TRACE("BEGIN make_ss");//XXX
	namespace ublas = ::boost::numeric::ublas;

	typedef typename ublas::promote_traits<
				typename ublas::promote_traits<
					typename ublas::promote_traits<
						typename ublas::matrix_traits<AMatrixExprT>::value_type,
						typename ublas::matrix_traits<BMatrixExprT>::value_type
					>::promote_type,
					typename ublas::matrix_traits<CMatrixExprT>::value_type
				>::promote_type,
				typename ublas::matrix_traits<DMatrixExprT>::value_type
			>::promote_type value_type;
	typedef ::std::size_t size_type; //FIXME: use type-promotion?

	const size_type rls_n_a(sys_ident_strategy.output_order());
	const size_type rls_n_b(sys_ident_strategy.input_order());
//	const size_type rls_d(sys_ident_strategy.input_delay());
	const size_type rls_n_y(sys_ident_strategy.num_outputs());
	const size_type rls_n_u(sys_ident_strategy.num_inputs());
	const size_type n_x(rls_n_a*rls_n_y+(rls_n_b-1)*rls_n_u);
	const size_type n_u(rls_n_u);
//	const size_type n(::std::max(n_x,n_u));
	const size_type n_y(1);

	// Create the state matrix A
	// A=[ 0        I          0         ...  0    0        I          0         ...  0  ;
	//     0        0          I         ...  0    0        0          I         ...  0  ;
	//     .        .          .         ...  .    .        .          0         ...  .
	//     .        .          .         ...  .    .        .          0         ...  .
	//     .        .          .         ...  .    .        .          0         ...  .
	// 	   0        0          0         ...  I    0        0          0         ...  I  ;
	// 	   B_{n_b}  B_{n_b-1}  B_{n_b-2} ...  B_2 -A_{n_a} -A_{n_a-1} -A_{n_a-2} ... -A_1]
	if (n_x > 0)
	{
		size_type broffs(n_x-rls_n_y); // The bottom row offset
		size_type cboffs0(rls_n_u);
		size_type cboffs1(cboffs0+((rls_n_b > 2) ? (rls_n_b-2)*rls_n_u : 0));
		size_type caoffs0(cboffs1+rls_n_y);
		size_type caoffs1(caoffs0+((rls_n_a > 1) ? (rls_n_a-1)*rls_n_y : 0));

		A().resize(n_x, n_x, false);

		// The upper part of A is set to [0_{k,rls_n_u} I_{k,kb} 0_{k,rls_n_y} I_{k,ka}],
		// where: k=n_x-rls_n_y, kb=(rls_n_b-2)*rls_n_u, ka=(rls_n_a-1)*rls_n_y.
		if (cboffs0 > 0)
		{
			ublas::subrange(A(), 0, broffs, 0, cboffs0) = ublas::zero_matrix<value_type>(broffs,rls_n_u);
		}
		if (cboffs1 > cboffs0)
		{
			ublas::subrange(A(), 0, broffs, cboffs0, cboffs1) = ublas::identity_matrix<value_type>(broffs,cboffs1-cboffs0);
		}
		if (caoffs0 > cboffs1)
		{
			ublas::subrange(A(), 0, broffs, cboffs1, caoffs0) = ublas::zero_matrix<value_type>(broffs,caoffs0-cboffs1);
		}
		if (caoffs1 > caoffs0)
		{
			ublas::subrange(A(), 0, broffs, caoffs0, caoffs1) = ublas::identity_matrix<value_type>(broffs,caoffs1-caoffs0);
		}

		// Fill A with B_2, ..., B_{n_b}
		for (size_type i = 1; i < rls_n_b; ++i)
		{
			// Copy matrix B_i from \hat{\Theta} into A.
			// In A the matrix B_i has to go in (rls_n_b-i)-th position:
			//   A(k:(k+n),((rls_n_b-i-1)*rls_n_u):((rls_n_b-i)*rls_n_u)) <- B_i

			size_type c2((rls_n_b-i)*rls_n_u);
			size_type c1(c2-rls_n_u);

			ublas::subrange(A(), broffs, n_x, c1, c2) = sys_ident_strategy.B(i+1);
		}

		// Fill A with A_1, ..., A_{n_a}
		for (size_type i = 0; i < rls_n_a; ++i)
		{
			// Copy matrix -A_i from \hat{\Theta} into A.
			// In A the matrix A_i has to go in ((rls_n_b-1)*rls_n_u+rls_n_a-i)-th position:
			//   A(k:(k+n),((rls_n_b-1)*rls_n_u+(rls_n_a-i-1)*rls_n_y):((rls_n_b-1)*rls_n_u+(rls_n_a-i)*rls_n_y)) <- -A_i

			size_type c2(cboffs1+(rls_n_a-i)*rls_n_y);
			size_type c1(c2-rls_n_y);

			////ublas::subrange(A(), broffs, n_x, c1, c2) = sys_ident_strategy.A(i+1);
			ublas::subrange(A(), broffs, n_x, c1, c2) = -sys_ident_strategy.A(i+1);
			//ublas::subrange(A(), broffs, n_x, c1, c2) = sys_ident_strategy.A(i+1);
		}
	}
	else
	{
		A().resize(0, 0, false);
	}
//DCS_DEBUG_TRACE("A="<<A);//XXX

	// Create the input matrix B
	// B=[I  ;
	//    0  ;
	//    .	 ;
	//    .	 ;
	//    .	 ;
	//    0  ;
	//    B_1]
	if (n_x > 0)
	{
		size_type broffs(n_x-rls_n_u); // The bottom row offset

		B().resize(n_x, n_u, false);

		// The upper part of B is set to [I_{n_u,n_u} 0_{k,n_u}]
		// where: k=n_x-rls_n_u.
		ublas::subrange(B(), 0, n_u, 0, n_u) = ublas::identity_matrix<value_type>(n_u,n_u);
		ublas::subrange(B(), n_u, broffs, 0, n_u) = ublas::zero_matrix<value_type>(broffs-n_u,n_u);
		// The bottom part of B with B_1
		ublas::subrange(B(), broffs, n_x, 0, n_u) = sys_ident_strategy.B(1);
	}
	else
	{
		B().resize(0, 0, false);
	}
//DCS_DEBUG_TRACE("B="<<B);//XXX

	// Create the output matrix C
	if (n_x > 0)
	{
		size_type rcoffs(n_x-rls_n_y); // The right most column offset

		C().resize(n_y, n_x, false);

		ublas::subrange(C(), 0, n_y, 0, rcoffs) = ublas::zero_matrix<value_type>(n_y,rcoffs);
		ublas::subrange(C(), 0, n_y, rcoffs, n_x) = ublas::scalar_matrix<value_type>(n_y, rls_n_y, 1);
	}
	else
	{
		C().resize(0, 0, false);
	}
//DCS_DEBUG_TRACE("C="<<C);//XXX

	// Create the transmission matrix D
	{
		D().resize(n_y, n_u, false);

		D() = ublas::zero_matrix<value_type>(n_y, n_u);
	}
//DCS_DEBUG_TRACE("D="<<D);//XXX

//DCS_DEBUG_TRACE("END make_ss");//XXX
}

#elif defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_ALT_SS) && DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_ALT_SS == 'C'

/// Convert an ARX structure to a state-space model in the canonical controllable form.
template <
//	typename TraitsT,
	typename SysIdentStrategyT,
	typename AMatrixExprT,
	typename BMatrixExprT,
	typename CMatrixExprT,
	typename DMatrixExprT
>
//void make_ss(rls_ff_mimo_proxy<TraitsT> const& sys_ident_strategy,
void make_ss(SysIdentStrategyT const& sys_ident_strategy,
			 ::boost::numeric::ublas::matrix_container<AMatrixExprT>& A,
			 ::boost::numeric::ublas::matrix_container<BMatrixExprT>& B,
			 ::boost::numeric::ublas::matrix_container<CMatrixExprT>& C,
			 ::boost::numeric::ublas::matrix_container<DMatrixExprT>& D)
{
//DCS_DEBUG_TRACE("BEGIN make_ss");//XXX
	namespace ublas = ::boost::numeric::ublas;

	typedef typename ublas::promote_traits<
				typename ublas::promote_traits<
					typename ublas::promote_traits<
						typename ublas::matrix_traits<AMatrixExprT>::value_type,
						typename ublas::matrix_traits<BMatrixExprT>::value_type
					>::promote_type,
					typename ublas::matrix_traits<CMatrixExprT>::value_type
				>::promote_type,
				typename ublas::matrix_traits<DMatrixExprT>::value_type
			>::promote_type value_type;
	typedef ::std::size_t size_type; //FIXME: use type-promotion?

	const size_type rls_n_a(sys_ident_strategy.output_order());
	const size_type rls_n_b(sys_ident_strategy.input_order());
//	const size_type rls_d(sys_ident_strategy.input_delay());
	const size_type rls_n_y(sys_ident_strategy.num_outputs());
	const size_type rls_n_u(sys_ident_strategy.num_inputs());
	const size_type n_x(rls_n_a*rls_n_y);
	const size_type n_u(rls_n_b*rls_n_u);
//	const size_type n(::std::max(n_x,n_u));
	const size_type n_y(1);

	DCS_ASSERT(
			rls_n_y <= 1 && rls_n_u <= 1,
			DCS_EXCEPTION_THROW(
				::std::runtime_error,
				"Actually, only SISO cases are hanlded"
			)
		);
	DCS_ASSERT(
			rls_n_y == rls_n_u,
			DCS_EXCEPTION_THROW(
				::std::runtime_error,
				"Actually, only the same number of channel are treated"
			)
		);

	// Create the state matrix A
	// A=[ 0        I          0         ...  0  ;
	//     0        0          I         ...  0  ;
	//     .        .          .         ...  .
	//     .        .          .         ...  .
	//     .        .          .         ...  .
	// 	   0        0          0         ...  I  ;
	// 	  -A_{n_a} -A_{n_a-1} -A_{n_a-2} ... -A_1]
	if (n_x > 0)
	{
		size_type broffs(n_x-rls_n_y); // The bottom row offset

		A().resize(n_x, n_x, false);

		// The upper part of A is set to [0_{k,rls_n_y} I_{k,k}],
		// where: k=n_x-rls_n_y.
		ublas::subrange(A(), 0, broffs, 0, rls_n_y) = ublas::zero_matrix<value_type>(broffs,rls_n_y);
		ublas::subrange(A(), 0, broffs, rls_n_y, n_x) = ublas::identity_matrix<value_type>(broffs,broffs);

		if (rls_n_a > 0)
		{
			// Fill A with A_1, ..., A_{n_a}
			for (size_type i = 0; i < rls_n_a; ++i)
			{
				// Copy matrix -A_i from \hat{\Theta} into A.
				// In A the matrix A_i has to go in (rls_n_a-i)-th position:
				//   A(k:(k+n),((rls_n_a-i-1)*rls_n_y):((rls_n_a-i)*rls_n_y)) <- -A_i

				size_type c2((rls_n_a-i)*rls_n_y);
				size_type c1(c2-rls_n_y);

				////ublas::subrange(A(), broffs, n_x, c1, c2) = sys_ident_strategy.A(i+1);
				ublas::subrange(A(), broffs, n_x, c1, c2) = -sys_ident_strategy.A(i+1);
				//ublas::subrange(A(), broffs, n_x, c1, c2) = sys_ident_strategy.A(i+1);
			}
		}
		else
		{
			ublas::subrange(A(), broffs, n_x, 0, n_x) = ublas::zero_matrix<value_type>(rls_n_y,n_x);
		}
	}
	else
	{
		A().resize(0, 0, false);
	}
//DCS_DEBUG_TRACE("A="<<A);//XXX

	// Create the input matrix B
	// B=[0;
	//    .;
	//    .;
	//    .;
	//    0;
	//    I]
	if (n_x > 0 && rls_n_b > 0)
	{
		size_type broffs(n_x-rls_n_u); // The bottom row offset

		B().resize(n_x, n_u, false);

		// The upper part of B is set to 0_{k,n_u}
		// where: k=n_x-rls_n_u.
		ublas::subrange(B(), 0, broffs, 0, n_u) = ublas::zero_matrix<value_type>(broffs, n_u);
		// The lower part of B is set to I_{n_u,n_u}
		ublas::subrange(B(), broffs, n_x, 0, n_u) = ublas::identity_matrix<value_type(n_u, n_u);
	}
	else
	{
		B().resize(0, 0, false);
	}
//DCS_DEBUG_TRACE("B="<<B);//XXX

	// Create the output matrix C
	// C=[M_n ... M_0]
	// where M_i=B_i-B_0*A_i
	// NOTE: in our case B_0=0, so M_i=B_i
	if (n_x > 0)
	{
		C().resize(n_y, n_x, false);

		for (size_type i = 0; i < rls_n_b; ++i)
		{
			size_type c2((rls_n_b-i)*rls_n_u);
			size_type c1(c2-rls_n_u);

			ublas::subrange(C(), 0, n_y, c1, c2) = sys_ident_strategy.B(i+1);
		}
	}
	else
	{
		C().resize(0, 0, false);
	}
//DCS_DEBUG_TRACE("C="<<C);//XXX

	// Create the transmission matrix D
	// D=[B0]
	// NOTE: in our case B_0=0, so D=[0]
	{
		D().resize(n_y, n_u, false);

		D() = ublas::zero_matrix<value_type>(n_y, n_u);
	}
//DCS_DEBUG_TRACE("D="<<D);//XXX

//DCS_DEBUG_TRACE("END make_ss");//XXX
}

#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_ALT_SS

template <
//	typename TraitsT,
	typename SysIdentStrategyT,
	typename AMatrixExprT,
	typename BMatrixExprT,
	typename CMatrixExprT,
	typename DMatrixExprT
>
//void make_ss(rls_ff_mimo_proxy<TraitsT> const& sys_ident_strategy,
void make_ss(SysIdentStrategyT const& sys_ident_strategy,
			 ::boost::numeric::ublas::matrix_container<AMatrixExprT>& A,
			 ::boost::numeric::ublas::matrix_container<BMatrixExprT>& B,
			 ::boost::numeric::ublas::matrix_container<CMatrixExprT>& C,
			 ::boost::numeric::ublas::matrix_container<DMatrixExprT>& D)
{
//DCS_DEBUG_TRACE("BEGIN make_ss");//XXX
	namespace ublas = ::boost::numeric::ublas;

	typedef typename ublas::promote_traits<
				typename ublas::promote_traits<
					typename ublas::promote_traits<
						typename ublas::matrix_traits<AMatrixExprT>::value_type,
						typename ublas::matrix_traits<BMatrixExprT>::value_type
					>::promote_type,
					typename ublas::matrix_traits<CMatrixExprT>::value_type
				>::promote_type,
				typename ublas::matrix_traits<DMatrixExprT>::value_type
			>::promote_type value_type;
	typedef ::std::size_t size_type; //FIXME: use type-promotion?

	const size_type rls_n_a(sys_ident_strategy.output_order());
	const size_type rls_n_b(sys_ident_strategy.input_order());
//	const size_type rls_d(sys_ident_strategy.input_delay());
	const size_type rls_n_y(sys_ident_strategy.num_outputs());
	const size_type rls_n_u(sys_ident_strategy.num_inputs());
	const size_type n_x(rls_n_a*rls_n_y);
	const size_type n_u(rls_n_b*rls_n_u);
//	const size_type n(::std::max(n_x,n_u));
	const size_type n_y(1);

	// Create the state matrix A
	// A=[ 0        I          0         ...  0  ;
	//     0        0          I         ...  0  ;
	//     .        .          .         ...  .
	//     .        .          .         ...  .
	//     .        .          .         ...  .
	// 	   0        0          0         ...  I  ;
	// 	  -A_{n_a} -A_{n_a-1} -A_{n_a-2} ... -A_1]
	if (n_x > 0)
	{
		size_type broffs(n_x-rls_n_y); // The bottom row offset

		A().resize(n_x, n_x, false);

		// The upper part of A is set to [0_{k,rls_n_y} I_{k,k}],
		// where: k=n_x-rls_n_y.
		ublas::subrange(A(), 0, broffs, 0, rls_n_y) = ublas::zero_matrix<value_type>(broffs,rls_n_y);
		ublas::subrange(A(), 0, broffs, rls_n_y, n_x) = ublas::identity_matrix<value_type>(broffs,broffs);

		// Fill A with A_1, ..., A_{n_a}
		for (size_type i = 0; i < rls_n_a; ++i)
		{
			// Copy matrix -A_i from \hat{\Theta} into A.
			// In A the matrix A_i has to go in (rls_n_a-i)-th position:
			//   A(k:(k+n),((rls_n_a-i-1)*rls_n_y):((rls_n_a-i)*rls_n_y)) <- -A_i

			size_type c2((rls_n_a-i)*rls_n_y);
			size_type c1(c2-rls_n_y);

			////ublas::subrange(A(), broffs, n_x, c1, c2) = sys_ident_strategy.A(i+1);
			ublas::subrange(A(), broffs, n_x, c1, c2) = -sys_ident_strategy.A(i+1);
			//ublas::subrange(A(), broffs, n_x, c1, c2) = sys_ident_strategy.A(i+1);
		}
	}
	else
	{
		A().resize(0, 0, false);
	}
//DCS_DEBUG_TRACE("A="<<A);//XXX

	// Create the input matrix B
	// B=[0 ... 0;
	//    .	... .
	//    .	... .
	//    .	... .
	//    0 ... 0;
	//    B_{n_b} ... B_1]
	if (n_x > 0)
	{
		size_type broffs(n_x-rls_n_u); // The bottom row offset

		B().resize(n_x, n_u, false);

		// The upper part of B is set to 0_{k,n_u}
		// where: k=n_x-rls_n_u.
		ublas::subrange(B(), 0, broffs, 0, n_u) = ublas::zero_matrix<value_type>(broffs,n_u);

		// Fill B with B_1, ..., B_{n_b}
		for (size_type i = 0; i < rls_n_b; ++i)
		{
			// Copy matrix B_i from \hat{\Theta} into B.
			// In \hat{\Theta} the matrix B_i stays at:
			//   B_i <- (\hat{\Theta}(((n_a*n_y)+i):n_b:n_u,:))^T
			// but in B the matrix B_i has to go in (n_b-i)-th position:
			//   B(k:(k+n_x),((n_b-i-1)*n_u):((n_a-i)*n_u)) <- B_i

			size_type c2((rls_n_b-i)*rls_n_u);
			size_type c1(c2-rls_n_u);

			ublas::subrange(B(), broffs, n_x, c1, c2) = sys_ident_strategy.B(i+1);
		}
	}
	else
	{
		B().resize(0, 0, false);
	}
//DCS_DEBUG_TRACE("B="<<B);//XXX

	// Create the output matrix C
	if (n_x > 0)
	{
		size_type rcoffs(n_x-rls_n_y); // The right most column offset

		C().resize(n_y, n_x, false);

		ublas::subrange(C(), 0, n_y, 0, rcoffs) = ublas::zero_matrix<value_type>(n_y,rcoffs);
		ublas::subrange(C(), 0, n_y, rcoffs, n_x) = ublas::scalar_matrix<value_type>(n_y, rls_n_y, 1);
	}
	else
	{
		C().resize(0, 0, false);
	}
//DCS_DEBUG_TRACE("C="<<C);//XXX

	// Create the transmission matrix D
	{
		D().resize(n_y, n_u, false);

		D() = ublas::zero_matrix<value_type>(n_y, n_u);
	}
//DCS_DEBUG_TRACE("D="<<D);//XXX

//DCS_DEBUG_TRACE("END make_ss");//XXX
}

#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_ALT_SS


template <typename TraitsT>
class lq_application_controller: public base_application_controller<TraitsT>
{
	private: typedef base_application_controller<TraitsT> base_type;
	private: typedef lq_application_controller<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
//	public: typedef ::dcs::control::dlqry_controller<real_type> controller_type;
//	public: typedef LQControllerT lq_controller_type;
//	public: typedef ::dcs::shared_ptr<lq_controller_type> lq_controller_pointer;
	public: typedef typename base_type::application_pointer application_pointer;
	protected: typedef ::boost::numeric::ublas::matrix<real_type> matrix_type;
	protected: typedef ::boost::numeric::ublas::vector<real_type> vector_type;
	private: typedef ::std::size_t size_type;
	private: typedef registry<traits_type> registry_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
//	private: typedef performance_measure<traits_type> performance_measure_type;
	private: typedef typename base_type::application_type application_type;
	private: typedef typename application_type::simulation_model_type application_simulation_model_type;
	private: typedef typename application_simulation_model_type::user_request_type user_request_type;
	private: typedef typename application_simulation_model_type::virtual_machine_type virtual_machine_type;
	private: typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
	private: typedef typename application_type::application_tier_type application_tier_type;
	private: typedef ::dcs::shared_ptr<application_tier_type> application_tier_pointer;
	private: typedef ::std::vector<performance_measure_category> perf_category_container;
//	private: typedef ::std::vector<real_type> measure_container;
	private: typedef ::std::map<performance_measure_category,real_type> category_measure_container;
	private: typedef ::dcs::des::base_statistic<real_type,uint_type> statistic_type;
	private: typedef ::dcs::des::mean_estimator<real_type,uint_type> statistic_impl_type; //FIXME: statistic category (e.g., mean) is hard-coded
	private: typedef ::dcs::shared_ptr<statistic_type> statistic_pointer;
	private: typedef ::std::map<performance_measure_category,statistic_pointer> category_statistic_container;
	private: typedef ::std::vector<category_statistic_container> category_statistic_container_container;
	private: typedef ::std::map<performance_measure_category,real_type> category_value_container;
	private: typedef ::std::vector<category_value_container> category_value_container_container;
	private: typedef physical_machine<traits_type> physical_machine_type;
	private: typedef base_system_identification_strategy<traits_type> system_identification_strategy_type;
	private: typedef ::dcs::shared_ptr<system_identification_strategy_type> system_identification_strategy_pointer;
	public: typedef base_system_identification_strategy_params<traits_type> system_identification_strategy_params_type;
	public: typedef ::dcs::shared_ptr<system_identification_strategy_params_type> system_identification_strategy_params_pointer;
	public: typedef application_controller_triggers<traits_type> triggers_type;


	protected: static const ::std::string cls_id_;
	private: static const size_type default_input_order_;
	private: static const size_type default_output_order_;
	private: static const size_type default_input_delay_;
	private: static const real_type default_min_share_;
	public: static const real_type default_ewma_smoothing_factor;


	public: lq_application_controller()
	: base_type(),
	  n_a_(default_output_order_),
	  n_b_(default_input_order_),
	  d_(default_input_delay_),
	  n_p_(0),
	  n_s_(0),
	  n_x_(0),
	  n_y_(0),
	  n_u_(0),
	  ewma_smooth_(default_ewma_smoothing_factor),
	  x_offset_(0),
	  u_offset_(0),
	  count_(0),
	  ident_fail_count_(0),
	  ctrl_fail_count_(0),
	  ready_(false)
	{
		init();
	}


	public: lq_application_controller(application_pointer const& ptr_app, real_type ts)
	: base_type(ptr_app, ts),
	  n_a_(default_output_order_),
	  n_b_(default_input_order_),
	  d_(default_input_delay_),
	  n_p_(0),
	  n_s_(0),
	  n_x_(0),
	  n_y_(0),
	  n_u_(0),
	  ewma_smooth_(default_ewma_smoothing_factor),
	  x_offset_(0),
	  u_offset_(0),
	  count_(0),
	  ident_fail_count_(0),
	  ctrl_fail_count_(0),
	  ready_(false)
	{
		init();
	}


	public: //template <typename QMatrixExprT, typename RMatrixExprT>
		lq_application_controller(uint_type n_a,
								  uint_type n_b,
								  uint_type d,
								  application_pointer const& ptr_app,
								  real_type ts,
								  system_identification_strategy_params_pointer const& ptr_ident_strategy_params,
								  triggers_type const& triggers,
								  real_type ewma_smoothing_factor/* = default_ewma_smoothing_factor*/)
	: base_type(ptr_app, ts),
	  n_a_(n_a),
	  n_b_(n_b),
	  d_(d),
	  n_p_(0),
	  n_s_(0),
	  n_x_(0),
	  n_y_(0),
	  n_u_(0),
	  ptr_ident_strategy_params_(ptr_ident_strategy_params),
	  ewma_smooth_(ewma_smoothing_factor),
	  x_offset_(0),
	  u_offset_(0),
	  count_(0),
	  ident_fail_count_(0),
	  ctrl_fail_count_(0),
	  ready_(false),
	  triggers_(triggers)
	{
		init();
	}


	/// Copy constructor.
	private: lq_application_controller(lq_application_controller const& that)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(that);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}


	/// Copy assignment.
	private: lq_application_controller& operator=(lq_application_controller const& rhs)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(rhs);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-assigment not yet implemented." );
	}


	/// Destructor
	public: virtual ~lq_application_controller()
	{
		finit();
	}


	private: void init()
	{
		init_measures();

		connect_to_event_sources();
	}


	private: void finit()
	{
		this->disconnect_from_event_sources();
	}


	private: void connect_to_event_sources()
	{
		typedef ::std::vector<application_tier_pointer> tier_container;
		typedef typename tier_container::const_iterator tier_iterator;

		if (this->application_ptr())
		{
			// Connect to application-level request departure event source
			this->application_ptr()->simulation_model().request_departure_event_source().connect(
					::dcs::functional::bind(
							&self_type::process_request_departure,
							this,
							::dcs::functional::placeholders::_1,
							::dcs::functional::placeholders::_2
						)
				);

//			// Connect to tier-level request departure event sources
//			tier_container tiers = this->application().tiers();
//			tier_iterator tier_end_it(tiers.end());
//			for (tier_iterator it = tiers.begin(); it != tier_end_it; ++it)
//			{
//				application_tier_pointer ptr_tier(*it);
//
//				this->application_ptr()->simulation_model().request_tier_departure_event_source(ptr_tier->id()).connect(
//						::dcs::functional::bind(
//								&self_type::process_request_tier_departure,
//								this,
//								::dcs::functional::placeholders::_1,
//								::dcs::functional::placeholders::_2,
//								ptr_tier->id()
//							)
//					);
//			}
		}
	}


	private: void disconnect_from_event_sources()
	{
		typedef ::std::vector<application_tier_pointer> tier_container;
		typedef typename tier_container::const_iterator tier_iterator;

		if (this->application_ptr())
		{
			// Connect to application-level request departure event source
			this->application_ptr()->simulation_model().request_departure_event_source().disconnect(
					::dcs::functional::bind(
							&self_type::process_request_departure,
							this,
							::dcs::functional::placeholders::_1,
							::dcs::functional::placeholders::_2
						)
				);

//			// Connect to tier-level request departure event sources
//			tier_container tiers = this->application().tiers();
//			tier_iterator tier_end_it(tiers.end());
//			for (tier_iterator it = tiers.begin(); it != tier_end_it; ++it)
//			{
//				application_tier_pointer ptr_tier(*it);
//
//				this->application_ptr()->simulation_model().request_tier_departure_event_source(ptr_tier->id()).disconnect(
//						::dcs::functional::bind(
//								&self_type::process_request_tier_departure,
//								this,
//								::dcs::functional::placeholders::_1,
//								::dcs::functional::placeholders::_2,
//								ptr_tier->id()
//							)
//					);
//			}
		}
	}


	private: void init_measures()
	{
		if (this->application_ptr())
		{
			typedef typename perf_category_container::const_iterator category_iterator;

			uint_type num_tiers(this->application().num_tiers());

			tier_measures_.resize(num_tiers);

			perf_category_container categories(this->application().sla_cost_model().slo_categories());
			category_iterator end_it = categories.end();
			for (category_iterator it = categories.begin(); it != end_it; ++it)
			{
				performance_measure_category category(*it);

				// Initialize app-level measure
				measures_[category] = ::dcs::make_shared<statistic_impl_type>();

				// Initialize tier-level measure
				for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
				{
					tier_measures_[tier_id][category] = ::dcs::make_shared<statistic_impl_type>();
//					ewma_tier_s_[tier_id][category] = real_type/*zero*/();
				}
			}

			n_p_ = n_s_
				 = num_tiers;
#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_ALT_SS) && DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_ALT_SS == 'X'
			n_x_ = n_p_*n_a_+n_s_*(n_b_-1);
			n_u_ = n_s_;
			n_y_ = uint_type(1);
			x_offset_ = (n_x_ > 0) ? (n_x_-n_p_) : 0;
			u_offset_ = 0;
#elif defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_ALT_SS) && DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_ALT_SS == 'C'
			n_x_ = n_p_*n_a_;
			n_u_ = n_s_;
			n_y_ = uint_type(1);
			x_offset_ = (n_x_ > 0) ? (n_x_-n_p_) : 0;
			u_offset_ = 0;
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_ALT_SS
			n_x_ = n_p_*n_a_;
			n_u_ = n_s_*n_b_;
			n_y_ = uint_type(1);
			x_offset_ = (n_x_ > 0) ? (n_x_-n_p_) : 0;
			u_offset_ = (n_u_ > 0) ? (n_u_-n_s_) : 0;
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_ALT_SS
//			x_offset_ = 0;
//			u_offset_ = 0;
		}
	}


#if 0 //[EXP-20120201]
	private: void reset_measures()
	{
		// Instead of making a full reset, keep some history of the
		// past in order to solve these issues:
		// - too few observation in the last control period
		// - not so much representative observations in the last control period.
		// Actually, the history is stored according to a EWMA filter.

//FIXME: try to use this below to reset stats
//		::std::for_each(
//				measures_.begin(),
//				measures_.end(),
//				::dcs::functional::bind(
//						&statistic_type::reset,
//						(::dcs::functional::placeholders::_1)->second
//					)
//			);
		typedef typename category_statistic_container::iterator measure_iterator;

		measure_iterator measure_end_it(measures_.end());
		for (measure_iterator it = measures_.begin(); it != measure_end_it; ++it)
		{
			performance_measure_category category(it->first);
			statistic_pointer ptr_stat(it->second);

			// Apply the EWMA filter to previously observed measurements
			//real_type ewma_old_s(ewma_s_.at(category));
::std::cerr << "APP " << this->application().id() << " - STAT: " << ptr_stat->estimate() << " - OLD EWMA: " << ewma_s_.at(category) << " - Smooth: " << ewma_smooth_ << " - Count: " << count_ << " ==> " << ewma_smooth_*ptr_stat->estimate() + (1-ewma_smooth_)*ewma_s_.at(category) << ::std::endl;//XXX
			if (ptr_stat->num_observations() > 0)
			{
				if (count_ > 1)
				{
					ewma_s_[category] = ewma_smooth_*ptr_stat->estimate() + (1-ewma_smooth_)*ewma_s_.at(category);
				}
				else
				{
					ewma_s_[category] = ptr_stat->estimate();
				}
			}

::std::cerr << "APP " << this->application().id() << " - STAT: " << ptr_stat->estimate() << " - EWMA: " << ewma_s_.at(category) << ::std::endl;//XXX
			// Reset stat and set as the first observation a memory of the past
			ptr_stat->reset();
			(*ptr_stat)(ewma_s_.at(category));
		}

		size_type num_tiers(tier_measures_.size());
		for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			measure_end_it = tier_measures_[tier_id].end();
			for (measure_iterator inner_it = tier_measures_[tier_id].begin(); inner_it != measure_end_it; ++inner_it)
			{
				performance_measure_category category(inner_it->first);
				statistic_pointer ptr_stat(inner_it->second);

				// Apply the EWMA filter to previously observed measurements
				//real_type ewma_old_s(ewma_tier_s_[tier_id].at(category));
				if (ptr_stat->num_observations() > 0)
				{
					if (count_ > 1)
					{
						ewma_tier_s_[tier_id][category] = ewma_smooth_*ptr_stat->estimate() + (1-ewma_smooth_)*ewma_tier_s_[tier_id].at(category);
					}
					else
					{
						ewma_tier_s_[tier_id][category] = ptr_stat->estimate();
					}
				}

				// Reset stat and set as the first observation a memory of the past
				ptr_stat->reset();
				(*ptr_stat)(ewma_tier_s_[tier_id].at(category));
			}
		}
	}
#endif // 0 [EXP-20120201]


	private: void update_measures()
	{
		// Instead of making a full reset, keep some history of the
		// past in order to solve these issues:
		// - too few observation in the last control period
		// - not so much representative observations in the last control period.
		// Actually, the history is stored according to a EWMA filter.

//FIXME: try to use this below to reset stats
//		::std::for_each(
//				measures_.begin(),
//				measures_.end(),
//				::dcs::functional::bind(
//						&statistic_type::reset,
//						(::dcs::functional::placeholders::_1)->second
//					)
//			);
		typedef typename category_statistic_container::iterator measure_iterator;

		measure_iterator measure_end_it(measures_.end());
		for (measure_iterator it = measures_.begin(); it != measure_end_it; ++it)
		{
			performance_measure_category category(it->first);
			statistic_pointer ptr_stat(it->second);

			// Apply the EWMA filter to previously observed measurements
			//real_type ewma_old_s(ewma_s_.at(category));
::std::cerr << "APP " << this->application().id() << " - STAT: " << ptr_stat->estimate() << " - OLD EWMA: " << ewma_s_.at(category) << " - Smooth: " << ewma_smooth_ << " - Count: " << count_ << " ==> " << ewma_smooth_*ptr_stat->estimate() + (1-ewma_smooth_)*ewma_s_.at(category) << ::std::endl;//XXX
			if (ptr_stat->num_observations() > 0)
			{
				if (count_ > 1)
				{
					ewma_s_[category] = ewma_smooth_*ptr_stat->estimate() + (1-ewma_smooth_)*ewma_s_.at(category);
				}
				else
				{
					ewma_s_[category] = ptr_stat->estimate();
				}
			}
			else
			{
				(*ptr_stat)(ewma_s_.at(category));
			}

::std::cerr << "APP " << this->application().id() << " - STAT: " << ptr_stat->estimate() << " - EWMA: " << ewma_s_.at(category) << ::std::endl;//XXX
#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
			eq_out_measure_ = ewma_s_.at(category);
#endif
		}

		size_type num_tiers(tier_measures_.size());
		for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			measure_end_it = tier_measures_[tier_id].end();
			for (measure_iterator inner_it = tier_measures_[tier_id].begin(); inner_it != measure_end_it; ++inner_it)
			{
				performance_measure_category category(inner_it->first);
				statistic_pointer ptr_stat(inner_it->second);

				// Apply the EWMA filter to previously observed measurements
				//real_type ewma_old_s(ewma_tier_s_[tier_id].at(category));
				if (ptr_stat->num_observations() > 0)
				{
					if (count_ > 1)
					{
						ewma_tier_s_[tier_id][category] = ewma_smooth_*ptr_stat->estimate() + (1-ewma_smooth_)*ewma_tier_s_[tier_id].at(category);
					}
					else
					{
						ewma_tier_s_[tier_id][category] = ptr_stat->estimate();
					}
				}
				else
				{
					(*ptr_stat)(ewma_tier_s_[tier_id].at(category));
				}
#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
				tier_eq_out_measures_[tier_id] = ewma_tier_s_.at(tier_id).at(category);
#endif
			}
		}
	}


	private: void reset_measures()
	{
		// Instead of making a full reset, keep some history of the
		// past in order to solve these issues:
		// - too few observation in the last control period
		// - not so much representative observations in the last control period.
		// Actually, the history is stored according to a EWMA filter.

//FIXME: try to use this below to reset stats
//		::std::for_each(
//				measures_.begin(),
//				measures_.end(),
//				::dcs::functional::bind(
//						&statistic_type::reset,
//						(::dcs::functional::placeholders::_1)->second
//					)
//			);
		typedef typename category_statistic_container::iterator measure_iterator;

		measure_iterator measure_end_it(measures_.end());
		for (measure_iterator it = measures_.begin(); it != measure_end_it; ++it)
		{
			performance_measure_category category(it->first);
			statistic_pointer ptr_stat(it->second);

			// Reset stat and set as the first observation a memory of the past
			ptr_stat->reset();
//			(*ptr_stat)(ewma_s_.at(category));
		}

		size_type num_tiers(tier_measures_.size());
		for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			measure_end_it = tier_measures_[tier_id].end();
			for (measure_iterator inner_it = tier_measures_[tier_id].begin(); inner_it != measure_end_it; ++inner_it)
			{
				performance_measure_category category(inner_it->first);
				statistic_pointer ptr_stat(inner_it->second);

				// Reset stat and set as the first observation a memory of the past
				ptr_stat->reset();
//				(*ptr_stat)(ewma_tier_s_[tier_id].at(category));
			}
		}
	}


	private: void full_reset_measures()
	{
		typedef typename category_statistic_container::iterator measure_iterator;
		typedef typename category_statistic_container_container::iterator tier_measure_iterator;

		measure_iterator measure_end_it(measures_.end());
		for (measure_iterator it = measures_.begin(); it != measure_end_it; ++it)
		{
			it->second->reset();
			ewma_s_[it->first] = real_type/*zero*/();
		}

//		tier_measure_iterator tier_measure_end_it(tier_measures_.end());
//		for (tier_measure_iterator outer_it = tier_measures_.begin(); outer_it != tier_measure_end_it; ++outer_it)
		size_type num_tiers(tier_measures_.size());
		if (ewma_tier_s_.size() != num_tiers)
		{
			ewma_tier_s_.resize(num_tiers);
		}
		for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
//			measure_end_it = outer_it->end();
			measure_end_it = tier_measures_[tier_id].end();
//			for (measure_iterator inner_it = outer_it->begin(); inner_it != measure_end_it; ++inner_it)
			for (measure_iterator inner_it = tier_measures_[tier_id].begin(); inner_it != measure_end_it; ++inner_it)
			{
				inner_it->second->reset();
				ewma_tier_s_[tier_id][inner_it->first] = real_type/*zero*/();
			}
		}

		count_ = ident_fail_count_
			   = ctrl_fail_count_
			   = size_type/*zero*/();
		ready_ = false;
		x_ = vector_type(n_x_, ::std::numeric_limits<real_type>::quiet_NaN());
		u_ = vector_type(n_u_, ::std::numeric_limits<real_type>::quiet_NaN());
#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
		num_eq_measures_ = ::std::max(uint_type(5), ::std::max(n_a_, n_b_));
		next_eq_out_measure_ = 0;
		eq_out_measure_ = 0;
		next_tier_eq_out_measures_ = ::std::vector<real_type>(num_eq_measures_,0);
		tier_eq_out_measures_ = ::std::vector<real_type>(num_eq_measures_,0);
		next_tier_eq_in_measures_ = ::std::vector<real_type>(num_eq_measures_,0);
		tier_eq_in_measures_ = ::std::vector<real_type>(num_eq_measures_,0);
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
	}


	//@{ Event Handlers

//	private: void process_request_tier_arrival(des_event_type const& evt, des_engine_context_type& ctx)
//	{
//		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing REQUEST-TIER-ARRIVAL (Clock: " << ctx.simulated_time() << ")");
//
//		typedef typename category_statistic_container_container::iterator tier_measure_iterator;
//
//		// Collect system output measures
//
//		user_request_type req = this->application().simulation_model().request_state(evt);
//
//		tier_measure_iterator end_it = tier_measures_.end();
//		for (measure_iterator it = measures_.begin(); it != end_it; ++it)
//		{
//			performance_measure_category category(it->first);
//
//			switch (category)
//			{
//				case response_time_performance_measure:
//					{
//						real_type rt(req.departure_time()-req.arrival_time());
//						(*measures_.at(category))(rt);
//					}
//					break;
//				default:
//					throw ::std::runtime_error("[dcs::des::cloud::lqr_application_controller::process_request_departure] LQ application controller currently handles only the response-time category.");
//			}
//		}
//		DCS_DEBUG_TRACE("(" << this << ") END Processing REQUEST-TIER-ARRIVAL (Clock: " << ctx.simulated_time() << ")");
//	}


//	private: void process_request_tier_departure(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id)
//	{
//		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing REQUEST-TIER-DEPARTURE (Clock: " << ctx.simulated_time() << ")");
//
//		typedef typename category_statistic_container::iterator measure_iterator;
//
//		// Collect system output measures
//
//		user_request_type req = this->application().simulation_model().request_state(evt);
//
//		// check: double check on tier identifier.
//		DCS_DEBUG_ASSERT( tier_id == req.current_tier() );
//
////TODO: measure should be scaled w.r.t. the reference machine
//		measure_iterator end_it = tier_measures_[tier_id].end();
//		for (measure_iterator it = tier_measures_[tier_id].begin(); it != end_it; ++it)
//		{
//			performance_measure_category category(it->first);
//			statistic_pointer ptr_stat(it->second);
//
//			switch (category)
//			{
//				case response_time_performance_measure:
//					{
//						real_type rt(req.tier_departure_time(tier_id)-req.tier_arrival_time(tier_id));
//						(*ptr_stat)(rt);
//					}
//					break;
//				default:
//					throw ::std::runtime_error("[dcs::des::cloud::lqr_application_controller::process_tier_request_departure] LQ application controller currently handles only the response-time category.");
//			}
//		}
//
//		DCS_DEBUG_TRACE("(" << this << ") END Processing REQUEST-TIER-DEPARTURE (Clock: " << ctx.simulated_time() << ")");
//	}


	private: void process_request_departure(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");

		typedef typename base_type::application_type application_type;
		typedef typename category_statistic_container::iterator measure_iterator;

		application_type const& app = this->application();

		// Collect tiers and system output measures

		user_request_type req = app.simulation_model().request_state(evt);

//		real_type app_rt(0);

		size_type num_tiers(tier_measures_.size());

		for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

			measure_iterator end_it = tier_measures_[tier_id].end();
			for (measure_iterator it = tier_measures_[tier_id].begin(); it != end_it; ++it)
			{
				performance_measure_category stat_category(it->first);
				statistic_pointer ptr_stat(it->second);

				switch (stat_category)
				{
					case response_time_performance_measure:
						{
							//NOTE: the relation between response time and
							//      resource capacity is inversely proportional:
							//      the greater is the capacity, the less is the
							//      response time.

							// Compute the residence time for this tier
							::std::vector<real_type> arr_times(req.tier_arrival_times(tier_id));
							::std::vector<real_type> dep_times(req.tier_departure_times(tier_id));
							size_type nt(arr_times.size());
							if (nt > 0)
							{
								real_type rt(0);
								for (size_type t = 0; t < nt; ++t)
								{
									rt += dep_times[t]-arr_times[t];
								}
//								rt *= scale_factor;
								(*ptr_stat)(rt);
::std::cerr << "APP " << app.id() << " - TIER: " << tier_id << " - OBSERVATION: " << rt << " (Clock: " << ctx.simulated_time() << ")" << ::std::endl;//XXX
//								app_rt += rt;
							}
						}
						break;
					default:
						throw ::std::runtime_error("[dcs::des::cloud::lq_application_controller::process_request_departure] LQ application controller currently handles only the response-time category.");
				}
			}
		}

		//FIXME: measure should be scaled w.r.t. the reference machine but at
		//       application-level we are unable to do this since each tier
		//       might have been run on a machine with very different
		//       characteristics. So instead of computing the performance
		//       measure (e.g., the response time) from the request object
		//       (e.g., from its arrival and departure times), compute it as the
		//       sum of tier performance measures (e.g., residence times).
		//       It should be equivalent!
        measure_iterator end_it = measures_.end();
        for (measure_iterator it = measures_.begin(); it != end_it; ++it)
        {
			performance_measure_category category(it->first);
			statistic_pointer ptr_stat(it->second);

            switch (category)
            {
                case response_time_performance_measure:
					{
						real_type rt(req.departure_time()-req.arrival_time());
						(*ptr_stat)(rt);
::std::cerr << "APP " << app.id() << " - OBSERVATION: " << rt << " (Clock: " << ctx.simulated_time() << ")" << ::std::endl;//XXX
//						(*ptr_stat)(app_rt);
					}
					break;
				default:
					throw ::std::runtime_error("[dcs::des::cloud::lq_application_controller::process_request_departure] LQ application controller currently handles only the response-time category.");
			}
		}

		ready_ = true;

		DCS_DEBUG_TRACE("(" << this << ") END Processing REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");
	}

	//@} Event Handlers


	//@{ Inteface Member Functions

	protected: void do_application(application_pointer const& ptr_app)
	{
		if (this->application_ptr())
		{
			// Disconnect from "old" app event sources

			this->application_ptr()->simulation_model().request_departure_event_source().disconnect(
					::dcs::functional::bind(
							&self_type::process_request_departure,
							this,
							::dcs::functional::placeholders::_1,
							::dcs::functional::placeholders::_2
						)
				);
		}

		// Connect to the event sources of the "new" app
		ptr_app->simulation_model().request_departure_event_source().connect(
				::dcs::functional::bind(
						&self_type::process_request_departure,
						this,
						::dcs::functional::placeholders::_1,
						::dcs::functional::placeholders::_2
					)
			);

		init_measures();
	}


	protected: void do_process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Process SYSTEM-INITIALIZATION event (Clock: " << ctx.simulated_time() << ")");

		// Prepare the data structures for the RLS algorithm 
//#ifdef DCS_DES_CLOUD_USE_MATLAB_APP_RPEM
////		rls_proxy_ = rls_proxy_type(n_a_, n_b_, 2, d_, n_p_, n_s_, rls_ff_);
//		ptr_ident_strategy_params_->noise_order(2);
//#else
////		rls_proxy_ = rls_proxy_type(n_a_, n_b_, d_, n_p_, n_s_, rls_ff_);
//#endif // DCS_DES_CLOUD_USE_MATLAB_APP_RPEM
		ptr_ident_strategy_params_->output_order(n_a_);
		ptr_ident_strategy_params_->input_order(n_b_);
		ptr_ident_strategy_params_->input_delay(d_);
		ptr_ident_strategy_params_->num_outputs(n_p_);
		ptr_ident_strategy_params_->num_inputs(n_s_);
		ptr_ident_strategy_ = make_system_identification_strategy(*ptr_ident_strategy_params_);
		ptr_ident_strategy_->init();

		// Completely reset all measures
		full_reset_measures();

		DCS_DEBUG_TRACE("(" << this << ") END Do Process SYSTEM-INITIALIZATION event (Clock: " << ctx.simulated_time() << ")");
	}


	private: void do_process_control(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		namespace ublas = ::boost::numeric::ublas;
		namespace ublasx = ::boost::numeric::ublasx;

		typedef typename base_type::application_type application_type;
		typedef typename application_type::simulation_model_type application_simulation_model_type;
		typedef typename application_type::performance_model_type application_performance_model_type;
		typedef typename perf_category_container::const_iterator category_iterator;
//		typedef typename category_statistic_container_container::const_iterator tier_iterator;

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Process CONTROL event (Clock: " << ctx.simulated_time() << " - Count: " << count_ << "/" << ident_fail_count_ << "/" << ctrl_fail_count_ << ")");
//if ((count_ % 1000) == 0)//XXX
//{//XXX
::std::cerr << "APP: " << this->application().id() << " - BEGIN Process CONTROL event -- Actual Output: " << measures_.at(response_time_performance_measure)->estimate() << " (Clock: " << ctx.simulated_time() << " - Counts: " << count_ << "/" << ident_fail_count_ << "/" << ctrl_fail_count_ << ")" << ::std::endl;//XXX
//}//XXX

		if (!ready_)
		{
			DCS_DEBUG_TRACE("(" << this << ") END Do Process CONTROL event: controller NOT READY (Clock: " << ctx.simulated_time() << " - Count: " << count_ << ")");
			return;
		}
#if 0
::std::cerr << "BEGIN MANUAL CONTROL" << ::std::endl;//XXX
{
	typedef typename traits_type::physical_machine_identifier_type pm_identifier_type;
	typedef ::std::set<pm_identifier_type> pm_id_container;
	typedef typename pm_id_container::const_iterator pm_id_iterator;
	pm_id_container seen_machs;
	for (size_type tier_id = 0; tier_id < this->application().num_tiers(); ++tier_id)
	{
		virtual_machine_pointer ptr_vm(this->application().simulation_model().tier_virtual_machine(tier_id));

		if (ctx.simulated_time() > 3600)
		{
			if (ptr_vm->id() == 0)
			{
				ptr_vm->wanted_resource_share(cpu_resource_category, 0.75);
			}
			else
			{
				ptr_vm->wanted_resource_share(cpu_resource_category, 0.25);
			}
		}
		else
		{
			if (ptr_vm->id() == 0)
			{
				ptr_vm->wanted_resource_share(cpu_resource_category, 0.25);
			}
			else
			{
				ptr_vm->wanted_resource_share(cpu_resource_category, 0.75);
			}
		}

		physical_machine_type const& pm(ptr_vm->vmm().hosting_machine());
		pm_identifier_type pm_id(pm.id());

		if (!seen_machs.count(pm_id))
		{
			seen_machs.insert(pm_id);
		}
	}
	pm_id_iterator end_it(seen_machs.end());
	for (pm_id_iterator it = seen_machs.begin(); it != end_it; ++it)
	{
		pm_identifier_type pm_id(*it);
		this->application().data_centre().physical_machine_controller(pm_id).control();
	}
}
::std::cerr << "END MANUAL CONTROL" << ::std::endl;//XXX
return;
#endif // 0

		//bool skip(false);

#ifdef DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
		// Dump a row on the RLS data file. Each row contains:
		//  <application-id>,
		//  <clock>,
		//  <performance-category>,<response_time>,<response_time-for-rls>
		//  <resource-category>,<share-tier0>,<share-tier0-for-rls>...,<share-tierN>,<share-tierN-for-rls>,
		//  <performance-category>,<residence_time-tier0>,<residence_time-tier0-for-rls>,...,<residence_time-tierN>,<residence_time-tierN-for-rls>,
		//  <performance-category>,<predicted-residence_time-tier0>,<predicted-residence_time-tier0-for-rls>,...,<predicted-residence_time-tierN>,<predicted-residence_time-tierN-for-rls>,
		//  <resource-category>,<optimal-share-tier0>,<optimal-share-tier0-for-rls>...,<optimal-share-tierN>,<optimal-share-tierN-for-rls>,

		char const* cjid(::getenv("CONDOR_JOB_ID"));
		::std::ostringstream oss;
		oss << "rls_data-" << ::std::string(cjid ? cjid : "unknown") << ".dat";
		::std::ofstream ofs(oss.str().c_str(), ::std::ios_base::app);

		// Dump the value of the simulator clock
		ofs << this->application().id() << "," << ctx.simulated_time();
#endif // DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA

		application_type const& app(this->application());
		application_simulation_model_type const& app_sim_model(app.simulation_model());
		application_performance_model_type const& app_perf_model(app.performance_model());

		size_type num_tiers(tier_measures_.size());
		vector_type s(n_s_,0); // control input (relative error of tier resource shares)
		vector_type p(n_p_,0); // control state (relative error of tier peformance measures)
		vector_type y(n_y_,0); // control output (relative error of app peformance measures)
		bool sla_ok(false);


		++count_;

		update_measures();//EXP-20120130

		// Rotate old with new inputs/outputs:
		//  x(k) = [p(k-n_a+1) ... p(k)]^T
		//       = [x_{n_p:n_x}(k-1) p(k)]^T
		//  u(k) = [s(k-n_b+1) ... s(k)]^T
		//       = [u_{n_s:n_u}(k-1) s(k)]^T
		// Check if a measure rotation is needed (always but the first time)
::std::cerr << "Old x=" << x_ << ::std::endl;//XXX
::std::cerr << "Old u=" << u_ << ::std::endl;//XXX
		if (count_ > 1)
		{
			// throw away old observations from x and make space for new ones.
			//detail::rotate(x_, n_a_, n_p_);
			if (n_x_ > 0)
			{
#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_ALT_SS)
::std::cerr << "HERE.1 -- n_b=" << n_b_ << ::std::endl;//XXX
				if (n_b_ > 1)
				{
::std::cerr << "HERE.2" << ::std::endl;//XXX
					if (n_b_ > 2)
					{
::std::cerr << "HERE.3" << ::std::endl;//XXX
						ublas::subrange(x_, 0, (n_b_-2)*n_s_) = ublas::subrange(x_, n_s_, (n_b_-1)*n_s_);
::std::cerr << "HERE.4" << ::std::endl;//XXX
					}
::std::cerr << "HERE.5" << ::std::endl;//XXX
					ublas::subrange(x_, (n_b_-2)*n_s_, (n_b_-1)*n_s_) = u_;
::std::cerr << "HERE.6" << ::std::endl;//XXX
				}
::std::cerr << "HERE.7" << ::std::endl;//XXX
				ublas::subrange(x_, n_s_*(n_b_-1), n_x_-n_p_) = ublas::subrange(x_, (n_b_-1)*n_s_+n_p_, n_x_);
::std::cerr << "HERE.8" << ::std::endl;//XXX
				ublas::subrange(x_, n_x_-n_p_, n_x_) = ublas::scalar_vector<real_type>(n_p_, ::std::numeric_limits<real_type>::quiet_NaN());
::std::cerr << "HERE.9" << ::std::endl;//XXX
				//ublas::subrange(x_, n_p_, n_x_) = ublas::subrange(x_, 0, n_x_-n_p_);
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_ALT_SS
				//ublas::subrange(x_, 0, (n_a_-1)*n_p_) = ublas::subrange(x_, n_p_, n_x_);
				ublas::subrange(x_, 0, n_x_-n_p_) = ublas::subrange(x_, n_p_, n_x_);
				ublas::subrange(x_, n_x_-n_p_, n_x_) = ublas::scalar_vector<real_type>(n_p_, ::std::numeric_limits<real_type>::quiet_NaN());
				//ublas::subrange(x_, n_p_, n_x_) = ublas::subrange(x_, 0, n_x_-n_p_);
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_ALT_SS
			}
			// throw away old observations from u and make space for new ones.
			//detail::rotate(u_, n_b_, n_s_);
			if (n_u_ > 0)
			{
#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_ALT_SS)
				u_ = ublas::scalar_vector<real_type>(n_s_, ::std::numeric_limits<real_type>::quiet_NaN());
#else
				//ublas::subrange(u_, 0, (n_b_-1)*n_s_) = ublas::subrange(u_, n_s_, n_u_);
				ublas::subrange(u_, 0, n_u_-n_s_) = ublas::subrange(u_, n_s_, n_u_);
				ublas::subrange(u_, n_u_-n_s_, n_u_) = ublas::scalar_vector<real_type>(n_s_, ::std::numeric_limits<real_type>::quiet_NaN());
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_ALT_SS
			}
		}
::std::cerr << "New x=" << x_ << ::std::endl;//XXX
::std::cerr << "New u=" << u_ << ::std::endl;//XXX


		// Collect data for creating control input and state
		//
		// NOTE: the application controller always work w.r.t. the reference
		//       machine.
		//       Anyway, at this time, both reference and actual measures have
		//       already been scaled according to the reference machine.
		//
		// NOTE: instead of using past control inputs computed by the controller
		//       we use the actual value used by VM since it can be different
		//       from the one computed by the controller, due to the
		//       "interference" of other components (like the physical machine
		//       controller).
		//
		// NOTE: one can think that this step can be done only once; however, in
		//       general, reference measures are not constant since they can
		//       change over time.
		//

		// Collect new state/output observations:
		// x_j(k) = (<actual-perf-measure-of-tier-j>-<ref-perf-measure-of-tier-j>)/<ref-perf-measure-of-tier-j>
		category_measure_container ref_measures;
		perf_category_container categories(app.sla_cost_model().slo_categories());
		category_iterator end_it = categories.end();
		for (category_iterator it = categories.begin(); it != end_it; ++it)
		{
			performance_measure_category category(*it);
			statistic_pointer ptr_stat(measures_.at(category));

#ifdef DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
			// Dump performance measure category
			ofs << "," << category;
#endif // DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA

			real_type eq_measure;
			real_type actual_measure;

			// Get the actual application-level output measure
			//ref_measure = app.sla_cost_model().slo_value(category);
			////ref_measure = static_cast<real_type>(.5)*app.sla_cost_model().slo_value(category);//EXP
#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
#if 0 // [EXP-20120201]
			if (count_ > num_eq_measures_)
			{
				if ((count_ % num_eq_measures_) == 0)
				{
//					if (count_ > num_eq_measures_)
//					{
//						eq_out_measure_ = next_eq_out_measure_ / static_cast<real_type>(num_eq_measures_);
//					}
//					else
//					{
//						// The first time take into consideration the additional
//						// measure added when count_ <= 1 representing the
//						// steady-state performance measure (see else branch
//						// below).
//						eq_out_measure_ = next_eq_out_measure_ / static_cast<real_type>(num_eq_measures_+1);
//					}
					eq_out_measure_ = next_eq_out_measure_ / static_cast<real_type>(num_eq_measures_);
					//next_eq_out_measure_ = ref_measure;
					next_eq_out_measure_ = eq_out_measure_;
				}
				else
				{
next_eq_out_measure_ += ewma_s_.at(category);//EXP-20120130
#if 0
					next_eq_out_measure_ += ptr_stat->estimate();
#endif // 0 [EXP-20120130]
				}
			}
			else
			{
eq_out_measure_ = ewma_s_.at(category);//EXP-20120130
#if 0
				//eq_out_measure_ = app_perf_model.application_measure(category);
				if (ptr_stat->num_observations() > 0)
				{
					eq_out_measure_ = ptr_stat->estimate();
				}
				else if (count_ == 1)
				{
					eq_out_measure_ = app_perf_model.application_measure(category);
				}
				// else leave unchanged...
#endif // 0 [EXP-20120130]
			}
#endif // 0 [EXP-20120201]
			eq_measure = eq_out_measure_;
#else
			eq_measure = app_perf_model.application_measure(category);
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//actual_measure = ewma_s_.at(category);//EXP-20120130
			if (ptr_stat->num_observations() > 0)
			{
				actual_measure = ptr_stat->estimate();
			}
			else
			{
				// No observation...
				::std::ostringstream oss;
				oss << "APP: " << app.id() << " - No observed application-level measure";
				::dcs::des::cloud::log_warn(cls_id_, oss.str());
				//FIXME: maybe is better to skip the control action
				// -> Assume perfect behavior
				actual_measure = eq_measure;
				// -> Skip the control action
				//skip = true;
			}
DCS_DEBUG_TRACE("APP " << app.id() << " - CONTROL OBSERVATION: ref: " << app_perf_model.application_measure(category) << " - equilibrium: " << eq_measure << " - actual: " << actual_measure);//XXX
::std::cerr << "APP " << app.id() << " - CONTROL OBSERVATION: ref: " << app_perf_model.application_measure(category) << " - equilibrium: " << eq_measure << " - actual: " << actual_measure << ::std::endl;//XXX

			if (triggers_.actual_value_sla_ko())
			{
				::std::vector<performance_measure_category> cats(1);
				cats[0] = category;
				::std::vector<real_type> meas(1);
				meas[0] = actual_measure;
				if (app.sla_cost_model().satisfied(cats.begin(), cats.end(), meas.begin()))
				{
					sla_ok = true;
				}
			}

#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION)
# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT)
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//			y(0) = actual_measure/app_perf_model.application_measure(category) - 1;
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//			y(0) = actual_measure/eq_measure - 1;
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
			y(0) = actual_measure/eq_measure - 1;
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//			y(0) = actual_measure - app_perf_model.application_measure(category);
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//			y(0) = actual_measure - eq_measure;
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
			y(0) = actual_measure - eq_measure;
# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION
//# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT)
////#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
////			y(0) = actual_measure/app_perf_model.application_measure(category);
////#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
////			y(0) = actual_measure/eq_measure;
////#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_DEVIATION
//			y(0) = actual_measure;
//# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_DEVIATION
			//y(0) = actual_measure - eq_measure;
			y(0) = actual_measure;
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION

#ifdef DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
			// Dump actual application response time
			ofs << "," << actual_measure << "," << y(0);
#endif // DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA

			// Get the actual tier-level output measures
			if (n_x_ > 0)
			{
				for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
				{
					switch (category)
					{
						case response_time_performance_measure:
								{
									ptr_stat = tier_measures_[tier_id].at(category);

#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
#if 0 // [EXP-20120201]
									if (count_ > num_eq_measures_)
									{
										if ((count_ % num_eq_measures_) == 0)
										{
											// We have collected a sufficient number of observations to form a new estimate of the equilibrium point
//											if (count_ > num_eq_measures_)
//											{
//												tier_eq_out_measures_[tier_id] = next_tier_eq_out_measures_[tier_id] / static_cast<real_type>(num_eq_measures_);
//											}
//											else
//											{
//												// The first time take into consideration the additional
//												// measure added when count_ <= 1 representing the
//												// steady-state performance measure (see else branch
//												// below).
//												tier_eq_out_measures_[tier_id] = next_tier_eq_out_measures_[tier_id] / static_cast<real_type>(num_eq_measures_+1);
//											}
											tier_eq_out_measures_[tier_id] = next_tier_eq_out_measures_[tier_id] / static_cast<real_type>(num_eq_measures_);
											//next_tier_eq_out_measures_[tier_id] = ref_measure;
											next_tier_eq_out_measures_[tier_id] = tier_eq_out_measures_[tier_id];
										}
										else
										{
next_tier_eq_out_measures_[tier_id] += ewma_tier_s_.at(tier_id).at(category);//EXP-20120130
#if 0
											next_tier_eq_out_measures_[tier_id] += ptr_stat->estimate();
#endif // 0 [EXP-20120130]
										}
									}
									else
									{
tier_eq_out_measures_[tier_id] = ewma_tier_s_.at(tier_id).at(category);//EXP-20120130
#if 0
										//tier_eq_out_measures_[tier_id] = app_perf_model.tier_measure(tier_id, category);
										if (ptr_stat->num_observations() > 0)
										{
											tier_eq_out_measures_[tier_id] = ptr_stat->estimate();
										}
										else if (count_ == 1)
										{
											tier_eq_out_measures_[tier_id] = app_perf_model.tier_measure(tier_id, category);
										}
										// else leave unchanged...
#endif // 0 [EXP-20120130]
									}
#endif // 0 [EXP-20120201]
									eq_measure = tier_eq_out_measures_[tier_id];
#else
									eq_measure = app_perf_model.tier_measure(tier_id, category);
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//actual_measure = ewma_tier_s_.at(tier_id).at(category);//EXP-20120130
									//ref_measure = static_cast<real_type>(.5)*app_perf_model.tier_measure(tier_id, category);//EXP
									if (ptr_stat->num_observations() > 0)
									{
										actual_measure = ptr_stat->estimate();
									}
									else
									{
										// No observation...
										::std::ostringstream oss;
										oss << "APP: " << app.id() << " - No observed tier-level measure for tier: " << tier_id;
										::dcs::des::cloud::log_warn(cls_id_, oss.str());
										//FIXME: maybe is better to skip the control action
										// -> Assume perfect behavior
										actual_measure = eq_measure;
										// -> Skip the control action
										//skip = true;
									}
DCS_DEBUG_TRACE("APP " << app.id() << " - TIER " << tier_id << " CONTROL OBSERVATION: ref: " << app_perf_model.tier_measure(tier_id, category) << " - equilibrium: " << eq_measure << " - actual: " << actual_measure);//XXX
::std::cerr << "APP " << app.id() << " - TIER " << tier_id << " CONTROL OBSERVATION: ref: " << app_perf_model.tier_measure(tier_id, category) << " - equilibrium: " << eq_measure << " - actual: " << actual_measure << ::std::endl;//XXX
#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION)
# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT)
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//									p(tier_id) = actual_measure/eq_measure - 1;
//									x_(x_offset_+tier_id) = actual_measure/app_perf_model.tier_measure(tier_id, category) - 1;
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//									x_(x_offset_+tier_id) = p(tier_id)
//														  = actual_measure/eq_measure - 1;
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//									p(tier_id) = actual_measure/eq_measure - 1;
//									x_(x_offset_+tier_id) = (actual_measure-app_perf_model.tier_measure(tier_id, category))/eq_measure;
									x_(x_offset_+tier_id) = p(tier_id)
														  = actual_measure/eq_measure - 1;
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//									p(tier_id) = actual_measure - eq_measure;
//									x_(x_offset_+tier_id) = actual_measure - app_perf_model.tier_measure(tier_id, category);
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//									x_(x_offset_+tier_id) = p(tier_id)
//														  = actual_measure - eq_measure;
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//									p(tier_id) = actual_measure - eq_measure;
//									x_(x_offset_+tier_id) = actual_measure - app_perf_model.tier_measure(tier_id, category);
									x_(x_offset_+tier_id) = p(tier_id)
														  = actual_measure - eq_measure;
# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION
//# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT)
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//									p(tier_id) = actual_measure/eq_measure;
//									x_(x_offset_+tier_id) = actual_measure/app_perf_model.tier_measure(tier_id, category);
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//									x_(x_offset_+tier_id) = p(tier_id)
//														  = actual_measure/eq_measure;
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
//									x_(x_offset_+tier_id) = p(tier_id)
//														  = actual_measure;
//# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
//									p(tier_id) = actual_measure;
//									x_(x_offset_+tier_id) = actual_measure - app_perf_model.tier_measure(tier_id, category);
									x_(x_offset_+tier_id) = p(tier_id)
														  = actual_measure;
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION
::std::cerr << "Updated x=" << x_ << ::std::endl;//XXX

#ifdef DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
									// Dump actual tier residence time
									ofs << "," << actual_measure << "," << p(tier_id);
#endif // DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
								}
								break;
						default:
							throw ::std::runtime_error("[dcs::des::cloud::lqr_application_controller::do_process_control] LQ application controller currently handles only the response-time category.");
					}
				}
			}
#ifdef DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
			else
			{
				// Dump actual tier residence time
				for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
				{
					ofs << ",,";
				}
			}
#endif // DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
		}

		// Collect new input observations:
		// u_j(k) = (<actual-resource-share-at-tier-j>-<ref-resource-share-at-tier-j>)/<ref-resource-share-at-tier-j>
		if (n_u_ > 0)
		{
			for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
			{
				virtual_machine_pointer ptr_vm = app_sim_model.tier_virtual_machine(tier_id);
				physical_machine_type const& actual_pm(ptr_vm->vmm().hosting_machine());
				//FIXME: resource category is actually hard-coded to CPU
				physical_resource_category res_category(cpu_resource_category);

				// Get the actual resource share from the VM and scale w.r.t. the reference machine
				real_type actual_share;
				actual_share = ::dcs::des::cloud::scale_resource_share(actual_pm.resource(res_category)->capacity(),
																  //actual_pm.resource(res_category)->utilization_threshold(),
																  app.reference_resource(res_category).capacity(),
																  //app.reference_resource(res_category).utilization_threshold(),
																  ptr_vm->resource_share(res_category));

//				// Get the reference resource share for the tier from the application specs
////				//real_type ref_share(ptr_vm->wanted_resource_share(ptr_vm->vmm().hosting_machine(), category));
////				//real_type ref_share(ptr_vm->wanted_resource_share(res_category));
//				real_type ref_share(ptr_vm->guest_system().resource_share(res_category));
#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION)
				real_type eq_share(0); // equilibrium point

# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
#if 0 // [EXP-20120201]
				if (count_ > num_eq_measures_)
				{
					if ((count_ % num_eq_measures_) == 0)
					{
//						if (count_ > num_eq_measures_)
//						{
//							tier_eq_in_measures_[tier_id] = next_tier_eq_in_measures_[tier_id] / static_cast<real_type>(num_eq_measures_);
//						}
//						else
//						{
//							// The first time take into consideration the additional
//							// measure added when count_ <= 1 representing the
//							// steady-state performance measure (see else branch
//							// below).
//							tier_eq_in_measures_[tier_id] = next_tier_eq_in_measures_[tier_id] / static_cast<real_type>(num_eq_measures_+1);
//						}
						tier_eq_in_measures_[tier_id] = next_tier_eq_in_measures_[tier_id] / static_cast<real_type>(num_eq_measures_);
						//next_tier_eq_in_measures_[tier_id] = ref_share;
						next_tier_eq_in_measures_[tier_id] = tier_eq_in_measures_[tier_id];
					}
					else
					{
						next_tier_eq_in_measures_[tier_id] += actual_share;
					}
				}
				else
				{
					//tier_eq_in_measures_[tier_id] =  ptr_vm->guest_system().resource_share(res_category);
					tier_eq_in_measures_[tier_id] =  actual_share;
				}
#endif // 0 [EXP-20120201]
				if (count_ > 1)
				{
					tier_eq_in_measures_[tier_id] = ewma_smooth_*actual_share+(1-ewma_smooth_)*tier_eq_in_measures_[tier_id];
				}
				else
				{
					tier_eq_in_measures_[tier_id] = actual_share;
				}
				eq_share = tier_eq_in_measures_[tier_id];
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
				eq_share = ptr_vm->guest_system().resource_share(res_category);
# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
				DCS_DEBUG_TRACE("APP " << app.id() << " - TIER " << tier_id << " SHARE: ref: " << ptr_vm->guest_system().resource_share(res_category) << " - equilibrium: " << eq_share << " - actual: " << ptr_vm->resource_share(res_category) << " - actual-scaled: " << actual_share);//XXX
::std::cerr << "APP " << app.id() << " - TIER " << tier_id << " SHARE: ref: " << ptr_vm->guest_system().resource_share(res_category) << " - equilibrium: " << eq_share << " - actual: " << ptr_vm->resource_share(res_category) << " - actual-scaled: " << actual_share << ::std::endl;//XXX
# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT)
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//				s(tier_id) = actual_share/eq_share - 1;
////				u_(u_offset_+tier_id) = actual_share/ptr_vm->guest_system().resource_share(res_category) - 1;
//				u_(u_offset_+tier_id) = (actual_share-ptr_vm->guest_system().resource_share(res_category))/eq_share;
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//				u_(u_offset_+tier_id) = s(tier_id)
//									  = actual_share/eq_share - 1;
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
				u_(u_offset_+tier_id) = s(tier_id)
									  = actual_share/eq_share - 1;
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//				s(tier_id) = actual_share - eq_share;
//				u_(u_offset_+tier_id) = actual_share - ptr_vm->guest_system().resource_share(res_category);
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//				u_(u_offset_+tier_id) = s(tier_id)
//									  = actual_share - eq_share;
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
				u_(u_offset_+tier_id) = s(tier_id)
									  = actual_share - eq_share;
# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION
//# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT)
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//				s(tier_id) = actual_share/eq_share;
////				u_(u_offset_+tier_id) = actual_share/ptr_vm->guest_system().resource_share(res_category);
//				u_(u_offset_+tier_id) = actual_share/eq_share;
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//				u_(u_offset_+tier_id) = s(tier_id)
//									  = actual_share/eq_share;
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
//				u_(u_offset_+tier_id) = s(tier_id)
//									  = actual_share;
//# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
				u_(u_offset_+tier_id) = s(tier_id)
									  = actual_share;
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION

#ifdef DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
				// Dump actual tier resource share
				ofs << "," << res_category << "," << actual_share << "," << s(tier_id);
#endif // DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
			}
		}
#ifdef DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
		else
		{
			// Dump actual tier resource share
			for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
			{
				//FIXME: resource category is actually hard-coded to CPU
				physical_resource_category res_category(cpu_resource_category);

				ofs << "," << res_category << ",,";
			}
		}
#endif // DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA

		if (!triggers_.actual_value_sla_ko() || !sla_ok)
		{
			// Estimate system parameters
			bool ok(true);
			vector_type p_hat;
			try
			{
				p_hat = ptr_ident_strategy_->estimate(p, s);
DCS_DEBUG_TRACE("RLS estimation:");//XXX
DCS_DEBUG_TRACE("p=" << p);//XXX
DCS_DEBUG_TRACE("s=" << s);//XXX
DCS_DEBUG_TRACE("p_hat=" << p_hat);//XXX
DCS_DEBUG_TRACE("Theta_hat=" << ptr_ident_strategy_->Theta_hat());//XXX
DCS_DEBUG_TRACE("P=" << ptr_ident_strategy_->P());//XXX
DCS_DEBUG_TRACE("phi=" << ptr_ident_strategy_->phi());//XXX
::std::cerr << "APP: " << app.id() << " - RLS estimation:" << ::std::endl;//XXX
::std::cerr << "p=" << p << ::std::endl;//XXX
::std::cerr << "s=" << s << ::std::endl;//XXX
::std::cerr << "p_hat=" << p_hat << ::std::endl;//XXX
::std::cerr << "Theta_hat=" << ptr_ident_strategy_->Theta_hat() << ::std::endl;//XXX
::std::cerr << "P=" << ptr_ident_strategy_->P() << ::std::endl;//XXX
::std::cerr << "phi=" << ptr_ident_strategy_->phi() << ::std::endl;//XXX
#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION)
# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT)
#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//::std::cerr << "==> Estimated RLS output =" << ((p_hat(0)+1)*eq_out_measure_) << ::std::endl;//XXX
::std::cerr << "==> Estimated RLS output =" << ((ublas::inner_prod(ptr_ident_strategy_->phi(), ublas::column(ptr_ident_strategy_->Theta_hat(),0))+1)*eq_out_measure_) << ::std::endl;//XXX
#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
::std::cerr << "==> Estimated RLS output =" << ((ublas::inner_prod(ptr_ident_strategy_->phi(), ublas::column(ptr_ident_strategy_->Theta_hat(),0))+1)*app_perf_model.application_measure(response_time_performance_measure)) << ::std::endl;//XXX
#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
::std::cerr << "==> Estimated RLS output =" << (ublas::inner_prod(ptr_ident_strategy_->phi(), ublas::column(ptr_ident_strategy_->Theta_hat(),0))+eq_out_measure_) << ::std::endl;//XXX
#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
::std::cerr << "==> Estimated RLS output =" << (ublas::inner_prod(ptr_ident_strategy_->phi(), ublas::column(ptr_ident_strategy_->Theta_hat(),0))+app_perf_model.application_measure(response_time_performance_measure)) << ::std::endl;//XXX
#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION
::std::cerr << "==> Estimated RLS output =" << ublas::inner_prod(ptr_ident_strategy_->phi(), ublas::column(ptr_ident_strategy_->Theta_hat(),0)) << ::std::endl;//XXX
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION

				if (!ublasx::all(ublasx::isfinite(ptr_ident_strategy_->Theta_hat())))
				{
					::std::ostringstream oss;
					oss << "APP: " << app.id() << " - Unable to estimate system parameters: infinite values in system parameters";
					::dcs::des::cloud::log_warn(cls_id_, oss.str());
					ok = false;
				}
			}
			catch (::std::exception const& e)
			{
				DCS_DEBUG_TRACE( "Caught exception: " << e.what() );

				::std::ostringstream oss;
				oss << "APP: " << app.id() << " - Unable to estimate system parameters: " << e.what();
				::dcs::des::cloud::log_warn(cls_id_, oss.str());

				ok = false;
			}

#ifdef DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
			// Dump predicted residence times
			for (size_type i=0; i < n_p_; ++i)
			{
				//FIXME: resource category is actually hard-coded to CPU
				physical_resource_category res_category(cpu_resource_category);

				ofs << "," << res_category << ",";

# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION)
#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
				real_type eq_measure(tier_eq_out_measures_[i]);
#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
				real_type eq_measure(app_perf_model.tier_measure(i, response_time_performance_measure));
#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT)
//#   if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//				ofs << (tier_eq_out_measures_[i]*(1.0+p_hat(i)));
//#   else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//				ofs << (eq_measure*(1.0+p_hat(i)));
//#   endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
				ofs << (eq_measure*(1+p_hat(i)));
#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
//#   if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//				ofs << tier_eq_out_measures_[i]+p_hat(i);
//#   else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//				ofs << (eq_measure+p_hat(i));
//#   endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
				ofs << (eq_measure+p_hat(i));
#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT)
//#   if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//				ofs << (tier_eq_out_measures_[i]*p_hat(i));
//#   else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//				ofs << (eq_measure*p_hat(i));
//#   endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
//				ofs << p_hat(i);
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
				ofs << p_hat(i);
# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION

				ofs << "," << p_hat(i);
			}
#endif // DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA

			// Check if RLS (and LQR) can be applied.
			// If not, then no control is performed.
//#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//			if (ok && ptr_ident_strategy_->count() > (::std::max(n_a_,n_b_)+num_eq_measures_)) // EXP-20120130
//#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
			if (ok && ptr_ident_strategy_->count() > ::std::max(n_a_,n_b_))
//#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
			{
				// Create the state-space representation of the system model:
				//  x(k+1) = Ax(k)+Bu(k)
				//  y(k) = Cx(k)+Du(k)
				// where
				// x(k) = [p(k-n_a+1); ... p(k)]
				// u(k) = [s(k-n_b+1); ... s(k)]
				// y(k) = \sum_i x_i(k)
				//

				matrix_type A;
				matrix_type B;
				matrix_type C;
				matrix_type D;

				detail::make_ss(*ptr_ident_strategy_, A, B, C, D);
				//this->do_make_ss(*ptr_ident_strategy_, A, B, C, D);

				// Compute the optimal control
DCS_DEBUG_TRACE("APP: " << app.id() << " - Solving LQ with");//XXX
DCS_DEBUG_TRACE("A=" << A);//XXX
DCS_DEBUG_TRACE("B=" << B);//XXX
DCS_DEBUG_TRACE("C=" << C);//XXX
DCS_DEBUG_TRACE("D=" << D);//XXX
DCS_DEBUG_TRACE("y= " << y);//XXX
DCS_DEBUG_TRACE("x= " << x_);//XXX
DCS_DEBUG_TRACE("u= " << u_);//XXX
::std::cerr << "APP: " << app.id() << " - Solving LQ with" << ::std::endl;//XXX
::std::cerr << "A=" << A << ::std::endl;//XXX
::std::cerr << "B=" << B << ::std::endl;//XXX
::std::cerr << "C=" << C << ::std::endl;//XXX
::std::cerr << "D=" << D << ::std::endl;//XXX
::std::cerr << "y= " << y << ::std::endl;//XXX
::std::cerr << "x= " << x_ << ::std::endl;//XXX
::std::cerr << "u= " << u_ << ::std::endl;//XXX
#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION)
# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT)
#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//DCS_DEBUG_TRACE("APP: " << app.id() << " - Estimated SS application response time: " << (app_perf_model.application_measure(response_time_performance_measure)+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,u_))+ublas::prod(D,u_))(0)*eq_out_measure_));//XXX
::std::cerr << "APP: " << app.id() << " - Estimated SS application response time: " << (eq_out_measure_*(1+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,u_))+ublas::prod(D,u_))(0))) << ::std::endl;//XXX
#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//DCS_DEBUG_TRACE("APP: " << app.id() << " - Estimated SS application response time: " << (app_perf_model.application_measure(response_time_performance_measure)*(1+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,u_))+ublas::prod(D,u_))(0))));//XXX
::std::cerr << "APP: " << app.id() << " - Estimated SS application response time: " << (app_perf_model.application_measure(response_time_performance_measure)*(1+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,u_))+ublas::prod(D,u_))(0))) << ::std::endl;//XXX
#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
//DCS_DEBUG_TRACE("APP: " << app.id() << " - Estimated SS application response time: " << (app_perf_model.application_measure(response_time_performance_measure)+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,u_))+ublas::prod(D,u_))(0)));//XXX
//::std::cerr << "APP: " << app.id() << " - Estimated SS application response time: " << (app_perf_model.application_measure(response_time_performance_measure)+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,u_))+ublas::prod(D,u_))(0)) << ::std::endl;//XXX
#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
::std::cerr << "APP: " << app.id() << " - Estimated SS application response time: " << (eq_out_measure_+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,u_))+ublas::prod(D,u_))(0)) << ::std::endl;//XXX
#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
::std::cerr << "APP: " << app.id() << " - Estimated SS application response time: " << (app_perf_model.application_measure(response_time_performance_measure)+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,u_))+ublas::prod(D,u_))(0)) << ::std::endl;//XXX
#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION
//DCS_DEBUG_TRACE("APP: " << app.id() << " - Estimated SS application response time: " << (app_perf_model.application_measure(response_time_performance_measure)+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,u_))+ublas::prod(D,u_))(0)));//XXX
//::std::cerr << "APP: " << app.id() << " - Estimated SS application response time: " << (app_perf_model.application_measure(response_time_performance_measure)+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,u_))+ublas::prod(D,u_))(0)) << ::std::endl;//XXX
::std::cerr << "APP: " << app.id() << " - Estimated SS application response time: " << ((ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,u_))+ublas::prod(D,u_))(0)) << ::std::endl;//XXX
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION
				vector_type opt_u;
				try
				{
					opt_u = this->do_optimal_control(x_, u_, y, A, B, C, D);
				}
				catch (::std::exception const& e)
				{
					DCS_DEBUG_TRACE( "APP: " << app.id() << " - Caught exception: " << e.what() );

					::std::ostringstream oss;
					oss << "APP: " << app.id() << " - Unable to compute control input: " << e.what();
					::dcs::des::cloud::log_warn(cls_id_, oss.str());

					ok = false;
				}
				if (ok)
				{
DCS_DEBUG_TRACE("APP: " << app.id() << " - Solved!");//XXX
DCS_DEBUG_TRACE("APP: " << app.id() << " - Optimal Control u*=> " << opt_u);//XXX
::std:: cerr << "APP: " << app.id() << " - Optimal Control u*=> " << opt_u << ::std::endl;//XXX
#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION)
# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT)
#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
DCS_DEBUG_TRACE("APP: " << app.id() << " - Expected application response time: " << (app_perf_model.application_measure(response_time_performance_measure)+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,opt_u))+ublas::prod(D,opt_u))(0)*eq_out_measure_));//XXX
::std::cerr << "APP: " << app.id() << " - Expected application response time: " << (app_perf_model.application_measure(response_time_performance_measure)+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,opt_u))+ublas::prod(D,opt_u))(0)*eq_out_measure_) << ::std::endl;//XXX
#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
DCS_DEBUG_TRACE("APP: " << app.id() << " - Expected application response time: " << (app_perf_model.application_measure(response_time_performance_measure)*(1+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,opt_u))+ublas::prod(D,opt_u))(0))));//XXX
::std::cerr << "APP: " << app.id() << " - Expected application response time: " << (app_perf_model.application_measure(response_time_performance_measure)*(1+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,opt_u))+ublas::prod(D,opt_u))(0))) << ::std::endl;//XXX
#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
DCS_DEBUG_TRACE("APP: " << app.id() << " - Expected application response time: " << (app_perf_model.application_measure(response_time_performance_measure)+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,opt_u))+ublas::prod(D,opt_u))(0)));//XXX
::std::cerr << "APP: " << app.id() << " - Expected application response time: " << (app_perf_model.application_measure(response_time_performance_measure)+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,opt_u))+ublas::prod(D,opt_u))(0)) << ::std::endl;//XXX
//::std::cerr << "APP: " << app.id() << " - Expected application response time #2: " << (eq_out_measure_+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,opt_u))+ublas::prod(D,opt_u))(0)) << ::std::endl;//XXX
# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION
//# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT)
//DCS_DEBUG_TRACE("APP: " << app.id() << " - Expected application response time: " << (app_perf_model.application_measure(response_time_performance_measure)+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,opt_u))+ublas::prod(D,opt_u))(0)));//XXX
////::std::cerr << "APP: " << app.id() << " Expected application response time: " << (app_perf_model.application_measure(response_time_performance_measure)+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,opt_u))+ublas::prod(D,opt_u))(0)) << ::std::endl;//XXX
//# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
//DCS_DEBUG_TRACE("APP: " << app.id() << " - Expected application response time: " << (ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,opt_u))+ublas::prod(D,opt_u))(0));//XXX
////::std::cerr << "APP: " << app.id() << " - Expected application response time: " << (ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,opt_u))+ublas::prod(D,opt_u))(0) << ::std::endl;//XXX
//# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
//DCS_DEBUG_TRACE("APP: " << app.id() << " - Expected application response time: " << (ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,opt_u))+ublas::prod(D,opt_u))(0));//XXX
////::std::cerr << "APP: " << app.id() << " - Expected application response time: " << (ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,opt_u))+ublas::prod(D,opt_u))(0) << ::std::endl;//XXX
DCS_DEBUG_TRACE("APP: " << app.id() << " - Expected application response time: " << (app_perf_model.application_measure(response_time_performance_measure)+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,opt_u))+ublas::prod(D,opt_u))(0)));//XXX
::std::cerr << "APP: " << app.id() << " - Expected application response time: " << (app_perf_model.application_measure(response_time_performance_measure)+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,opt_u))+ublas::prod(D,opt_u))(0)) << ::std::endl;//XXX
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION
//::std::cerr << "APP: " << app.id() << " - Expected application response time: " << (eq_out_measure_+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,opt_u))+ublas::prod(D,opt_u))(0)) << ::std::endl;//[EXP-20120203]

#ifdef DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
					// Dump predicted tier resource share
					for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
					{
						//FIXME: resource category is actually hard-coded to CPU
						physical_resource_category res_category(cpu_resource_category);
						virtual_machine_pointer ptr_vm(app_sim_model.tier_virtual_machine(tier_id));

						ofs << "," << res_category << ",";

# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION)
#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
						real_type eq_share(tier_eq_in_measures_[tier_id]);
#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
						real_type eq_share(ptr_vm->guest_system().resource_share(cpu_resource_category));
#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT)
//#   if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//						ofs << ptr_vm->guest_system().resource_share(res_category)*(1+opt_u(u_offset_+tier_id));
//#   else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//						ofs << ref_share*(1+opt_u(u_offset_+tier_id));
//#   endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
						ofs << eq_share*(1+opt_u(u_offset_+tier_id));
#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
//#   if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//						ofs << ptr_vm->guest_system().resource_share(res_category)+opt_u(u_offset_+tier_id);
//#   else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//						ofs << (ref_share+opt_u(u_offset_+tier_id));
//#   endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
						ofs << eq_share+opt_u(u_offset_+tier_id);
#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT)
//#   if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//						ofs << ptr_vm->guest_system().resource_share(res_category)*opt_u(u_offset_+tier_id);
//#   else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//						ofs << eq_share*opt_u(u_offset_+tier_id);
//#   endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
//						ofs << opt_u(u_offset_+tier_id);
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
						ofs << opt_u(u_offset_+tier_id);
# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION

						ofs << "," << opt_u(u_offset_+tier_id);
					}
#endif // DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA

DCS_DEBUG_TRACE("Applying optimal control");//XXX
					if (triggers_.predicted_value_sla_ko())
					{
						//vector_type adj_opt_u(opt_u);
						vector_type adj_opt_u(u_);//EXP-20120201

						for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
						{
							physical_resource_category res_category(cpu_resource_category);//FIXME

							virtual_machine_pointer ptr_vm(app_sim_model.tier_virtual_machine(tier_id));
							physical_machine_type const& pm(ptr_vm->vmm().hosting_machine());
#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION)
# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
							real_type eq_share(tier_eq_in_measures_[tier_id]);
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
							real_type eq_share(ptr_vm->guest_system().resource_share(res_category));
# endif //DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT)
							real_type new_share(eq_share*(1+opt_u(u_offset_+tier_id)));
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//							real_type new_share(ptr_vm->guest_system().resource_share(res_category)*(opt_u(u_offset_+tier_id) + 1));
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//							real_type ref_share(ptr_vm->guest_system().resource_share(res_category));
//							real_type new_share(ref_share*(opt_u(u_offset_+tier_id)+1));
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
							real_type new_share(eq_share+opt_u(u_offset_+tier_id));
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//							real_type new_share(ptr_vm->guest_system().resource_share(res_category)+opt_u(u_offset_+tier_id));
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//							real_type ref_share(ptr_vm->guest_system().resource_share(res_category));
//							real_type new_share(ref_share+opt_u(u_offset_+tier_id));
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION
//# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT)
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//							real_type new_share(ptr_vm->guest_system().resource_share(res_category)*opt_u(u_offset_+tier_id));
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//							real_type ref_share(ptr_vm->guest_system().resource_share(res_category));
//							real_type new_share(ref_share*opt_u(u_offset_+tier_id));
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
//							real_type new_share(opt_u(u_offset_+tier_id));
//# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
							real_type new_share(opt_u(u_offset_+tier_id));
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION
DCS_DEBUG_TRACE("APP : " << app.id() << " - Tier " << tier_id << " --> New Unscaled share: " << new_share);//XXX
::std::cerr << "APP: " << app.id() << " - Tier " << tier_id << " --> New Unscaled share: " << new_share << ::std::endl;//XXX
							new_share = ::dcs::des::cloud::scale_resource_share(
											// Reference resource capacity and threshold
											app.reference_resource(res_category).capacity(),
											//app.reference_resource(res_category).utilization_threshold(),
											// Actual resource capacity and threshold
											pm.resource(res_category)->capacity(),
											//pm.resource(res_category)->utilization_threshold(),
											// Unscaled share: reference resource share "+" computed deviation
											new_share
								);

							if (new_share >= 0)
							{
//#ifdef DCS_DEBUG
								if (new_share < default_min_share_)
								{
									::std::ostringstream oss;
									oss << "APP: " << app.id() << " - Optimal share (" << new_share << ") too small; adjusted to " << default_min_share_;
									::dcs::des::cloud::log_warn(cls_id_, oss.str());
								}
								if (new_share > 1)
								{
									::std::ostringstream oss;
									oss << "APP: " << app.id() << " - Optimal share (" << new_share << ") too big; adjusted to 1";
									::dcs::des::cloud::log_warn(cls_id_, oss.str());
								}
//#endif // DCS_DEBUG
								new_share = ::std::min(::std::max(new_share, default_min_share_), static_cast<real_type>(1));
							}
							else
							{
#if DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION == 'W'
//#  ifdef DCS_DEBUG
								::std::ostringstream oss;
								oss << "APP: " << app.id() << " - Optimal share (" << new_share << ") is negative; adjusted to max(" << ptr_vm->wanted_resource_share(res_category) << ", " << default_min_share_ << ")";
								::dcs::des::cloud::log_warn(cls_id_, oss.str());
//  #endif // DCS_DEBUG
								new_share = ::std::max(ptr_vm->wanted_resource_share(res_category), default_min_share_);
#elif DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION == 'R'
//#  ifdef DCS_DEBUG
								::std::ostringstream oss;
								oss << "APP: " << app.id() << " - Optimal share (" << new_share << ") is negative; adjusted to max(" << ptr_vm->guest_system().resource_share(res_category) << ", " << default_min_share_ << ")";
								::dcs::des::cloud::log_warn(cls_id_, oss.str());
//#  endif // DCS_DEBUG
								new_share = ::std::max(ptr_vm->guest_system().resource_share(res_category), default_min_share_);
#elif DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION == 'M'
//#  ifdef DCS_DEBUG
								::std::ostringstream oss;
								oss << "APP: " << app.id() << " - Optimal share (" << new_share << ") is negative; adjusted to " << default_min_share_;
								::dcs::des::cloud::log_warn(cls_id_, oss.str());
//#  endif // DCS_DEBUG
								new_share = default_min_share_;
#elif DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION == 'C'
//# ifdef DCS_DEBUG
								::std::ostringstream oss;
								oss << "APP: " << app.id() << " - Optimal share (" << new_share << ") is negative; adjusted to max(" << ptr_vm->resource_share(res_category) << ", " << default_min_share_ << ")";
								::dcs::des::cloud::log_warn(cls_id_, oss.str());
//# endif // DCS_DEBUG
								new_share = ::std::max(ptr_vm->resource_share(res_category), default_min_share_);
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION
							}

//							DCS_DEBUG_TRACE("APP: " << app.id() << " - Assigning new wanted share: VM: " << ptr_vm->name() << " (" << ptr_vm->id() << ") - Tier: " << tier_id << " - Category: " << res_category << " - Actual Share: " << ptr_vm->resource_share(res_category) << " ==> Share: " << new_share);
//::std::cerr << "APP: " << app.id() << " - Assigning new wanted share: VM: " << ptr_vm->name() << " (" << ptr_vm->id() << ") - Tier: " << tier_id << " - Category: " << res_category << " - Actual Share: " << ptr_vm->resource_share(res_category) << " ==> Share: " << new_share << ::std::endl;//XXX

							new_share = ::dcs::des::cloud::scale_resource_share(
											// Actual resource capacity and threshold
											pm.resource(res_category)->capacity(),
											//pm.resource(res_category)->utilization_threshold(),
											// Reference resource capacity and threshold
											app.reference_resource(res_category).capacity(),
											//app.reference_resource(res_category).utilization_threshold(),
											// Scaled share: reference resource share "+" computed deviation
											new_share
								);
#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION)
# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT)
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//							adj_opt_u(u_offset_+tier_id) = new_share/ptr_vm->guest_system().resource_share(res_category) - 1;
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//							adj_opt_u(u_offset_+tier_id) = new_share/ref_share - 1;
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
							adj_opt_u(u_offset_+tier_id) = new_share/eq_share - 1;
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//							adj_opt_u(u_offset_+tier_id) = new_share - ptr_vm->guest_system().resource_share(res_category);
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//							adj_opt_u(u_offset_+tier_id) = new_share - ref_share;
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
							adj_opt_u(u_offset_+tier_id) = new_share - eq_share;
# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION
//# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT)
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//							adj_opt_u(u_offset_+tier_id) = new_share/ptr_vm->guest_system().resource_share(res_category);
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//							adj_opt_u(u_offset_+tier_id) = new_share/ref_share;
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
//							adj_opt_u(u_offset_+tier_id) = new_share;
//# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
							adj_opt_u(u_offset_+tier_id) = new_share;
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION
						}

						//FIXME: performance metric 'response_time' is hard-coded
//						real_type pred_measure = app.sla_cost_model().slo_value(response_time_performance_measure)
//						//real_type pred_measure = static_cast<real_type>(.5)*app.sla_cost_model().slo_value(response_time_performance_measure)//EXP
//												 + (ublas::prod(C, ublas::prod(A,x_)+ ublas::prod(B,opt_u))+ublas::prod(D,adj_opt_u))(0);
#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION)
# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT)
#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//						real_type pred_measure = (app_perf_model.application_measure(response_time_performance_measure)
//												  + (ublas::prod(C, ublas::prod(A,x_) + ublas::prod(B,adj_opt_u)) + ublas::prod(D,adj_opt_u))(0)*eq_out_measure_);
						real_type pred_measure = eq_out_measure_
												 * (1+(ublas::prod(C, ublas::prod(A,x_) + ublas::prod(B,adj_opt_u)) + ublas::prod(D,adj_opt_u))(0));
						real_type cur_measure = eq_out_measure_*(1+y(0));
#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//						real_type pred_measure = app_perf_model.application_measure(response_time_performance_measure)
//												 * (1+ublas::prod(C, ublas::prod(A,x_) + ublas::prod(B,adj_opt_u)) + ublas::prod(D,adj_opt_u))(0);
						real_type pred_measure = app_perf_model.application_measure(response_time_performance_measure)
												 * (1+(ublas::prod(C, ublas::prod(A,x_) + ublas::prod(B,adj_opt_u)) + ublas::prod(D,adj_opt_u))(0));
						real_type cur_measure = app_perf_model.application_measure(response_time_performance_measure)*(1+y(0));
#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
						real_type pred_measure = eq_out_measure_
												 + (ublas::prod(C, ublas::prod(A,x_) + ublas::prod(B,adj_opt_u)) + ublas::prod(D,adj_opt_u))(0);
						real_type cur_measure = eq_out_measure_+y(0);
#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
						real_type pred_measure = app_perf_model.application_measure(response_time_performance_measure)
												 + (ublas::prod(C, ublas::prod(A,x_) + ublas::prod(B,adj_opt_u)) + ublas::prod(D,adj_opt_u))(0);
						real_type cur_measure = app_perf_model.application_measure(response_time_performance_measure)+y(0);
#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION
//# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT)
//						real_type pred_measure = app_perf_model.application_measure(response_time_performance_measure)
//												 * (ublas::prod(C, ublas::prod(A,x_)+ ublas::prod(B,opt_u))+ublas::prod(D,adj_opt_u))(0);
//# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
//						real_type pred_measure = (ublas::prod(C, ublas::prod(A,x_)+ ublas::prod(B,opt_u))+ublas::prod(D,adj_opt_u))(0);
//# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
//						real_type pred_measure = app_perf_model.application_measure(response_time_performance_measure)
//												 + (ublas::prod(C, ublas::prod(A,x_) + ublas::prod(B,adj_opt_u)) + ublas::prod(D,adj_opt_u))(0);
						real_type pred_measure = (ublas::prod(C, ublas::prod(A,x_) + ublas::prod(B,adj_opt_u)) + ublas::prod(D,adj_opt_u))(0);
						real_type cur_measure = y(0);
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION
::std::cerr << "APP: " << app.id() << " - Adjusted Optimal Control u*=> " << adj_opt_u << ::std::endl;//XXX
::std::cerr << "APP: " << app.id() << " - Expected application response time after adjustment: " << pred_measure << " - Current: " << cur_measure << ::std::endl;//XXX

						::std::vector<performance_measure_category> cats(1);
						cats[0] = response_time_performance_measure;
						::std::vector<real_type> cur_meas(1);
						cur_meas[0] = cur_measure;
						::std::vector<real_type> pred_meas(1);
						pred_meas[0] = pred_measure;
						if ((!app.sla_cost_model().satisfied(cats.begin(), cats.end(), cur_meas.begin()) && cur_measure >= pred_measure) || app.sla_cost_model().satisfied(cats.begin(), cats.end(), pred_meas.begin()))
						{
							for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
							{
								physical_resource_category res_category(cpu_resource_category);//FIXME: category hard-coded

								virtual_machine_pointer ptr_vm(app_sim_model.tier_virtual_machine(tier_id));
								physical_machine_type const& pm(ptr_vm->vmm().hosting_machine());

#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION)
# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
								real_type eq_share(tier_eq_in_measures_[tier_id]);
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
								real_type eq_share(ptr_vm->guest_system().resource_share(res_category));
# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT)
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//								real_type new_share(ptr_vm->guest_system().resource_share(res_category)*(adj_opt_u(u_offset_+tier_id) + 1));
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//								real_type ref_share(ptr_vm->guest_system().resource_share(res_category));
//								real_type new_share(ref_share*(adj_opt_u(u_offset_+tier_id)+1));
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
								real_type new_share(eq_share*(adj_opt_u(u_offset_+tier_id)+1));
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//								real_type new_share(ptr_vm->guest_system().resource_share(res_category)+(adj_opt_u(u_offset_+tier_id)));
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//								real_type ref_share(ptr_vm->guest_system().resource_share(res_category));
//								real_type new_share(ref_share+adj_opt_u(u_offset_+tier_id));
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
								real_type new_share(eq_share+adj_opt_u(u_offset_+tier_id));
# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION
//# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT)
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//								real_type new_share(ptr_vm->guest_system().resource_share(res_category)*(adj_opt_u(u_offset_+tier_id)));
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//								real_type ref_share(ptr_vm->guest_system().resource_share(res_category));
//								real_type new_share(ref_share*adj_opt_u(u_offset_+tier_id));
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//								real_type new_share(adj_opt_u(u_offset_+tier_id));
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//								real_type ref_share(ptr_vm->guest_system().resource_share(res_category));
//								real_type new_share(adj_opt_u(u_offset_+tier_id));
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
								real_type new_share(adj_opt_u(u_offset_+tier_id));
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION
								new_share = ::dcs::des::cloud::scale_resource_share(
												// Reference resource capacity and threshold
												app.reference_resource(res_category).capacity(),
												//app.reference_resource(res_category).utilization_threshold(),
												// Actual resource capacity and threshold
												pm.resource(res_category)->capacity(),
												//pm.resource(res_category)->utilization_threshold(),
												// Old resource share + computed deviation
												new_share
									);

								DCS_DEBUG_TRACE("APP: " << app.id() << " - VM: " << ptr_vm->name() << " (" << ptr_vm->id() << ") - Tier: " << tier_id << ": " << res_category << " - Category: " << res_category << " - Actual Output: " << tier_measures_[tier_id].at(response_time_performance_measure)->estimate() << " (Reference-Point: " << app_perf_model.tier_measure(tier_id, response_time_performance_measure) << ") - Actual Share: " << ptr_vm->resource_share(res_category) << " ==> New Share: " << new_share);
::std::cerr << "APP: " << app.id() << " - VM: " << ptr_vm->name() << " (" << ptr_vm->id() << ") - Tier: " << tier_id << ": " << res_category << " Actual Output: " << tier_measures_[tier_id].at(response_time_performance_measure)->estimate() << " (Reference-Point: " << app_perf_model.tier_measure(tier_id, response_time_performance_measure) << ") - Actual Share: " << ptr_vm->resource_share(res_category) << " ==> New Share: " << new_share << ::std::endl;//XXX
								ptr_vm->wanted_resource_share(res_category, new_share);
							}
						}
						else
						{
							++ctrl_fail_count_;
							ok = false;

 							::std::ostringstream oss;
							oss << "APP: " << app.id() << " - Control not applied: failed to find suitable control inputs";
							::dcs::des::cloud::log_warn(cls_id_, oss.str());
						}
					}
					else
					{
						for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
						{
							physical_resource_category res_category(cpu_resource_category);//FIXME: category hard-coded

							virtual_machine_pointer ptr_vm(app_sim_model.tier_virtual_machine(tier_id));
							physical_machine_type const& pm(ptr_vm->vmm().hosting_machine());
	//						real_type actual_share;
	//						actual_share = ::dcs::des::cloud::scale_resource_share(pm.resource(res_category)->capacity(),
	//																		  pm.resource(res_category)->utilization_threshold(),
	//																		  app.reference_resource(res_category).capacity(),
	//																		  app.reference_resource(res_category).utilization_threshold(),
	//																		  ptr_vm->resource_share(res_category));
	//
	//
	//DCS_DEBUG_TRACE("Tier " << tier_id << " --> Actual share: " << actual_share);//XXX
#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION)
# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
							real_type eq_share(tier_eq_in_measures_[tier_id]);
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
							real_type eq_share(ptr_vm->guest_system().resource_share(res_category));
# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT)
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//							real_type new_share(ptr_vm->guest_system().resource_share(res_category)*(opt_u(u_offset_+tier_id) + 1));
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//							real_type ref_share(ptr_vm->guest_system().resource_share(res_category));
//							real_type new_share(ref_share*(opt_u(u_offset_+tier_id) + 1));
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
							real_type new_share(eq_share*(opt_u(u_offset_+tier_id) + 1));
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//							real_type new_share(ptr_vm->guest_system().resource_share(res_category)+(opt_u(u_offset_+tier_id)));
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//							real_type ref_share(ptr_vm->guest_system().resource_share(res_category));
//							real_type new_share(ref_share+opt_u(u_offset_+tier_id));
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
							real_type new_share(eq_share+opt_u(u_offset_+tier_id));
# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION
//# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT)
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//							real_type new_share(ptr_vm->guest_system().resource_share(res_category)*(opt_u(u_offset_+tier_id)));
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//							real_type ref_share(ptr_vm->guest_system().resource_share(res_category));
//							real_type new_share(ref_share*opt_u(u_offset_+tier_id));
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
//#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
//							real_type new_share(opt_u(u_offset_+tier_id));
//#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//							real_type new_share(opt_u(u_offset_+tier_id));
//#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
//# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
							real_type new_share(opt_u(u_offset_+tier_id));
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION
DCS_DEBUG_TRACE("APP : " << app.id() << " - Tier " << tier_id << " --> New Unscaled share: " << new_share);//XXX
::std::cerr << "APP : " << app.id() << " - Tier " << tier_id << " --> New Unscaled share: " << new_share << ::std::endl;//XXX
//							new_share = ::dcs::des::cloud::scale_resource_share(
//											// Reference resource capacity and threshold
//											app.reference_resource(res_category).capacity(),
//											app.reference_resource(res_category).utilization_threshold(),
//											// Actual resource capacity and threshold
//											pm.resource(res_category)->capacity(),
//											pm.resource(res_category)->utilization_threshold(),
//											// Reference resource share "+" computed deviation
//											new_share
//								);
							new_share = ::dcs::des::cloud::scale_resource_share(
											// Reference resource capacity and threshold
											app.reference_resource(res_category).capacity(),
											// Actual resource capacity and threshold
											pm.resource(res_category)->capacity(),
											// Reference resource share "+" computed deviation
											new_share
								);

							if (new_share >= 0)
							{
//#ifdef DCS_DEBUG
								if (new_share < default_min_share_)
								{
									::std::ostringstream oss;
									oss << "APP: " << app.id() << " - Optimal share (" << new_share << ") too small; adjusted to " << default_min_share_;
									::dcs::des::cloud::log_warn(cls_id_, oss.str());
								}
								if (new_share > 1)
								{
									::std::ostringstream oss;
									oss << "APP: " << app.id() << " - Optimal share (" << new_share << ") too big; adjusted to 1";
									::dcs::des::cloud::log_warn(cls_id_, oss.str());
								}
//#endif // DCS_DEBUG
								new_share = ::std::min(::std::max(new_share, default_min_share_), static_cast<real_type>(1));
							}
							else
							{
#if DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION == 'W'
//#  ifdef DCS_DEBUG
								::std::ostringstream oss;
								oss << "APP: " << app.id() << " - Optimal share (" << new_share << ") is negative; adjusted to max(" << ptr_vm->wanted_resource_share(res_category) << ", " << default_min_share_ << ")";
								::dcs::des::cloud::log_warn(cls_id_, oss.str());
//#  endif // DCS_DEBUG
								new_share = ::std::max(ptr_vm->wanted_resource_share(res_category), default_min_share_);
#elif DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION == 'R'
//#  ifdef DCS_DEBUG
								::std::ostringstream oss;
								oss << "APP: " << app.id() << " - Optimal share (" << new_share << ") is negative; adjusted to max(" << ptr_vm->guest_system().resource_share(res_category) << ", " << default_min_share_ << ")";
								::dcs::des::cloud::log_warn(cls_id_, oss.str());
//#  endif // DCS_DEBUG
								new_share = ::std::max(ptr_vm->guest_system().resource_share(res_category), default_min_share_);
#elif DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION == 'M'
//#  ifdef DCS_DEBUG
								::std::ostringstream oss;
								oss << "APP: " << app.id() << " - Optimal share (" << new_share << ") is negative; adjusted to " << default_min_share_;
								::dcs::des::cloud::log_warn(cls_id_, oss.str());
//#  endif // DCS_DEBUG
								new_share = default_min_share_;
#elif DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION == 'C'
//# ifdef DCS_DEBUG
								::std::ostringstream oss;
								oss << "APP: " << app.id() << " - Optimal share (" << new_share << ") is negative; adjusted to max(" << ptr_vm->resource_share(res_category) << ", " << default_min_share_ << ")";
								::dcs::des::cloud::log_warn(cls_id_, oss.str());
//# endif // DCS_DEBUG
								new_share = ::std::max(ptr_vm->resource_share(res_category), default_min_share_);
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION
							}

#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
							DCS_DEBUG_TRACE("APP: " << app.id() << " - VM: " << ptr_vm->name() << " (" << ptr_vm->id() << ") - Tier: " << tier_id << ": " << res_category << " Actual Output: " << tier_measures_[tier_id].at(response_time_performance_measure)->estimate() << " (Equilibrium-Point: " << tier_eq_out_measures_[tier_id] << " - Reference-Point: " << app_perf_model.tier_measure(tier_id, response_time_performance_measure) << ") - Actual Share: " << ptr_vm->resource_share(res_category) << " ==> New Share: " << new_share << " (Equilibrium-Point: " << ::dcs::des::cloud::scale_resource_share(app.reference_resource(res_category).capacity(), pm.resource(res_category)->capacity(), tier_eq_in_measures_[tier_id]) << " - Reference-Point: " << ptr_vm->guest_system().resource_share(res_category) << ")");
::std::cerr << "APP: " << app.id() << " - VM: " << ptr_vm->name() << " (" << ptr_vm->id() << ") - Tier: " << tier_id << ": " << res_category << " Actual Output: " << tier_measures_[tier_id].at(response_time_performance_measure)->estimate() << " (Equilibrium-Point: " << tier_eq_out_measures_[tier_id] << " - Reference-Point: " << app_perf_model.tier_measure(tier_id, response_time_performance_measure) << ") - Actual Share: " << ptr_vm->resource_share(res_category) << " ==> New Share: " << new_share << " (Equilibrium-Point: " << ::dcs::des::cloud::scale_resource_share(app.reference_resource(res_category).capacity(), pm.resource(res_category)->capacity(), tier_eq_in_measures_[tier_id]) << " - Reference-Point: " << ptr_vm->guest_system().resource_share(res_category) << ")" << ::std::endl;//XXX
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
							DCS_DEBUG_TRACE("APP: " << app.id() << " - VM: " << ptr_vm->name() << " (" << ptr_vm->id() << ") - Tier: " << tier_id << ": " << res_category << " - Category: " << res_category << " - Actual Output: " << tier_measures_[tier_id].at(response_time_performance_measure)->estimate() << " (Reference-Point: " << app_perf_model.tier_measure(tier_id, response_time_performance_measure) << ") - Actual Share: " << ptr_vm->resource_share(res_category) << " ==> New Share: " << new_share);
::std::cerr << "APP: " << app.id() << " - VM: " << ptr_vm->name() << " (" << ptr_vm->id() << ") - Tier: " << tier_id << ": " << res_category << " Actual Output: " << tier_measures_[tier_id].at(response_time_performance_measure)->estimate() << " (Reference-Point: " << app_perf_model.tier_measure(tier_id, response_time_performance_measure) << ") - Actual Share: " << ptr_vm->resource_share(res_category) << " ==> New Share: " << new_share << ::std::endl;//XXX
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT

							ptr_vm->wanted_resource_share(res_category, new_share);
						}
					}

					if (ok)
					{
						typedef typename traits_type::physical_machine_identifier_type pm_identifier_type;
						::std::set<pm_identifier_type> seen_machs;
						for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
						{
							virtual_machine_pointer ptr_vm(app_sim_model.tier_virtual_machine(tier_id));
							physical_machine_type const& pm(ptr_vm->vmm().hosting_machine());

							pm_identifier_type pm_id(pm.id());

							if (!seen_machs.count(pm_id))
							{
								seen_machs.insert(pm_id);
								this->application().data_centre().physical_machine_controller(pm_id).control();
							}
						}
DCS_DEBUG_TRACE("Optimal control applied");//XXX
					}
				}
				else
				{
					++ctrl_fail_count_;

					::std::ostringstream oss;
					oss << "APP: " << app.id() << " - Control not applied: failed to solve the control problem";
					::dcs::des::cloud::log_warn(cls_id_, oss.str());

#ifdef DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
					// Dump (fake) predicted tier resource share
					for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
					{
						//FIXME: resource category is actually hard-coded to CPU
						physical_resource_category res_category(cpu_resource_category);

						ofs << "," << res_category << ",,";
					}
#endif // DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
				}

//				typedef typename traits_type::physical_machine_identifier_type pm_identifier_type;
//				typedef ::std::set<pm_identifier_type> pm_id_container;
//				typedef typename pm_id_container::const_iterator pm_id_iterator;
//
//				pm_id_container seen_machs;
//				for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
//				{
//					virtual_machine_pointer ptr_vm(app_sim_model.tier_virtual_machine(tier_id));
//
//					physical_machine_type const& pm(ptr_vm->vmm().hosting_machine());
//
//					pm_identifier_type pm_id(pm.id());
//
//					if (!seen_machs.count(pm_id))
//					{
//						seen_machs.insert(pm_id);
//					}
//
//					physical_resource_category category(cpu_resource_category); //FIXME: CPU category is hard-coded
//
//					real_type share(ptr_vm->guest_system().resource_share(category));
//
//					ptr_vm->wanted_resource_share(category, share);
//
//					::std::ostringstream oss;
//					oss << "APP: " << app.id() << " - TIER: " << tier_id << " - Control not applied: fallback to reference share: " << share;
//					::dcs::des::cloud::log_warn(cls_id_, oss.str());
//				}
//				pm_id_iterator end_it(seen_machs.end());
//				for (pm_id_iterator it = seen_machs.begin(); it != end_it; ++it)
//				{
//					pm_identifier_type pm_id(*it);
//					this->application().data_centre().physical_machine_controller(pm_id).control();
//				}
			}
			else if (!ok)
			{
				ptr_ident_strategy_->reset();

				++ident_fail_count_;

				::std::ostringstream oss;
				oss << "APP: " << app.id() << " - Control not applied: failed to solve the identification problem";
				::dcs::des::cloud::log_warn(cls_id_, oss.str());

#ifdef DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
				// Dump (fake) predicted tier resource share
				for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
				{
					//FIXME: resource category is actually hard-coded to CPU
					physical_resource_category res_category(cpu_resource_category);

					ofs << "," << res_category << ",,";
				}
#endif // DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
			}
#ifdef DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
			else
			{
				// Dump (fake) predicted tier resource share
				for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
				{
					//FIXME: resource category is actually hard-coded to CPU
					physical_resource_category res_category(cpu_resource_category);

					ofs << "," << res_category << ",,";
				}
			}
#endif // DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
		}
		else
		{
#ifdef DCS_DEBUG
			::std::ostringstream oss;
			oss << "APP: " << app.id() << " - Control not applied: SLA preserved";
			::dcs::des::cloud::log_info(cls_id_, oss.str());
#endif // DCS_DEBUG

#ifdef DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
			//FIXME: resource category is actually hard-coded to CPU
			physical_resource_category res_category(cpu_resource_category);

			ofs << ",,,," << res_category;
			// Dump (fake) actual tier resource shares
			for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
			{
				ofs << ",,";
			}
			// Dump (fake) actual tier residence times
			for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
			{
				ofs << ",,";
			}
			// Dump (fake) predicted tier residence times
			for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
			{
				ofs << ",,";
			}
			// Dump (fake) predicted tier resource shares
			for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
			{
				ofs << ",,";
			}
#endif // DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
		}

#ifdef DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
			ofs << ::std::endl;
			ofs.close();
#endif // DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA

		// Reset previously collected system measure in order to collect a new ones.
		reset_measures();

//if (((count_-1) % 1000) == 0)//XXX
//{//XXX
::std::cerr << "APP: " << this->application().id() << " - END Process CONTROL event -- Actual Output: " << measures_.at(response_time_performance_measure)->estimate() << " (Clock: " << ctx.simulated_time() << " - Counts: " << count_ << "/" << ident_fail_count_ << "/" << ctrl_fail_count_ << ")" << ::std::endl;//XXX
//}//XXX

		DCS_DEBUG_TRACE("(" << this << ") END Do Process CONTROL event (Clock: " << ctx.simulated_time() << " - Count: " << count_ << "/" << ident_fail_count_ << "/" << ctrl_fail_count_ << ")");
	}

	//@} Inteface Member Functions


	private: virtual vector_type do_optimal_control(vector_type const& x, vector_type const& u, vector_type const& y, matrix_type const& A, matrix_type const& B, matrix_type const& C, matrix_type const& D) = 0;


//	/// The LQ controller.
//	private: lq_controller_type controller_;
//	/// Matrix of system parameters estimated by RLS: [A_1 ... A_{n_a} B_1 ... B_{n_b}].
//	private: matrix_type rls_Theta_hat_;
//	/// The covariance matrix computed by RLS.
//	private: matrix_type rls_P_;
//	/// The regression vector used by RLS.
//	private: vector_type rls_phi_;
	/// The memory for the control output.
	private: size_type n_a_;
	/// The memory for the control input.
	private: size_type n_b_;
	/// Input time delay used in the RLS algorithm
	private: size_type d_;
	/// The size of the control state vector.
	private: size_type n_p_;
	/// The size of the control input vector.
	private: size_type n_s_;
	/// The size of the augmented control state vector.
	private: size_type n_x_;
	/// The size of the control output vector.
	private: size_type n_y_;
	/// The size of the augmented control input vector.
	private: size_type n_u_;
//	/// Forgetting factor used in the RLS algorithm.
//	private: real_type rls_ff_;
	/// Parameters for configuring the system identification strategy.
	private: system_identification_strategy_params_pointer ptr_ident_strategy_params_;
	/// Smoothing factor for the EWMA filter
	private: real_type ewma_smooth_;
	private: size_type x_offset_;
	private: size_type u_offset_;
	private: size_type count_;
	private: size_type ident_fail_count_;
	private: size_type ctrl_fail_count_;
	/// Flag indicating if the controller can be activated.
	private: bool ready_;
	private: vector_type x_;
	private: vector_type u_;
	/// System-level measures collected during the last control interval.
	private: category_statistic_container measures_;
	/// Tier-level measures collected during the last control interval.
	private: category_statistic_container_container tier_measures_;
	private: category_value_container ewma_s_;
	private: category_value_container_container ewma_tier_s_;
	private: system_identification_strategy_pointer ptr_ident_strategy_;
//	private: bool actual_val_ko_sla_trigger_;
//	private: bool predicted_val_ko_sla_trigger_;
	private: triggers_type triggers_;
#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
	private: uint_type num_eq_measures_;
	private: real_type next_eq_out_measure_;
	protected: real_type eq_out_measure_;//FIXME
	private: ::std::vector<real_type> next_tier_eq_out_measures_;
	private: ::std::vector<real_type> tier_eq_out_measures_;
//	private: real_type next_eq_in_measure_;
//	private: real_type eq_in_measure_;
	private: ::std::vector<real_type> next_tier_eq_in_measures_;
	private: ::std::vector<real_type> tier_eq_in_measures_;
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
}; // lq_application_controller


template <typename TraitsT>
const ::std::string lq_application_controller<TraitsT>::cls_id_ = "lq_app_ctrl";


template <typename TraitsT>
const typename lq_application_controller<TraitsT>::size_type lq_application_controller<TraitsT>::default_input_order_ = 2;


template <typename TraitsT>
const typename lq_application_controller<TraitsT>::size_type lq_application_controller<TraitsT>::default_output_order_ = 2;


template <typename TraitsT>
const typename lq_application_controller<TraitsT>::size_type lq_application_controller<TraitsT>::default_input_delay_ = 0;

//template <typename TraitsT>
//const typename lq_application_controller<TraitsT>::uint_type lq_application_controller<TraitsT>::default_ss_state_size_ = 3;


//template <typename TraitsT>
//const typename lq_application_controller<TraitsT>::uint_type lq_application_controller<TraitsT>::default_ss_input_size_ = 3;


//template <typename TraitsT>
//const typename lq_application_controller<TraitsT>::uint_type lq_application_controller<TraitsT>::default_ss_output_size_ = 1;

template <typename TraitsT>
const typename lq_application_controller<TraitsT>::real_type lq_application_controller<TraitsT>::default_min_share_ = 0.20;


//template <typename TraitsT>
//const typename lq_application_controller<TraitsT>::real_type lq_application_controller<TraitsT>::default_rls_forgetting_factor = 0.98;


template <typename TraitsT>
const typename lq_application_controller<TraitsT>::real_type lq_application_controller<TraitsT>::default_ewma_smoothing_factor = 0.7;

}} // Namespace detail::<unnamed>


/**
 * \brief Application controller based on the Linear-Quadratic-Integrator
 *  control.
 * 
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename TraitsT>
class lqi_application_controller: public detail::lq_application_controller<TraitsT>
{
	private: typedef detail::lq_application_controller<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename base_type::application_pointer application_pointer;
	public: typedef typename base_type::system_identification_strategy_params_pointer system_identification_strategy_params_pointer;
	public: typedef typename base_type::triggers_type triggers_type;
//	public: typedef base_system_identification_strategy_params<traits_type> system_identification_strategy_params_type;
//	public: typedef ::dcs::shared_ptr<system_identification_strategy_params_type> system_identification_strategy_params_pointer;
	private: typedef ::dcs::control::dlqi_controller<real_type> lq_controller_type;
	private: typedef typename base_type::vector_type vector_type;
	private: typedef typename base_type::matrix_type matrix_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;


	public: lqi_application_controller()
	: base_type()
	{
	}


	public: lqi_application_controller(application_pointer const& ptr_app, real_type ts)
	: base_type(ptr_app, ts),
	  xi_(1,0)
	{
	}


	public: template <typename QMatrixExprT, typename RMatrixExprT>
		lqi_application_controller(uint_type n_a,
								   uint_type n_b,
								   uint_type d,
								   ::boost::numeric::ublas::matrix_expression<QMatrixExprT> const& Q,
								   ::boost::numeric::ublas::matrix_expression<RMatrixExprT> const& R,
								   application_pointer const& ptr_app,
								   real_type ts,
//								   real_type rls_forgetting_factor/* = base_type::default_rls_forgetting_factor*/,
								   system_identification_strategy_params_pointer const& ptr_ident_strategy_params,
								   triggers_type const& triggers,
								   real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
//	: base_type(n_a, n_b, d, ptr_app, ts, rls_forgetting_factor, ewma_smoothing_factor),
	: base_type(n_a, n_b, d, ptr_app, ts, ptr_ident_strategy_params, triggers, ewma_smoothing_factor),
	  controller_(Q, R),
	  xi_(1,0)
	{
	}


	public: template <typename QMatrixExprT, typename RMatrixExprT, typename NMatrixExprT>
		lqi_application_controller(uint_type n_a,
								   uint_type n_b,
								   uint_type d,
								   ::boost::numeric::ublas::matrix_expression<QMatrixExprT> const& Q,
								   ::boost::numeric::ublas::matrix_expression<RMatrixExprT> const& R,
								   ::boost::numeric::ublas::matrix_expression<NMatrixExprT> const& N,
								   application_pointer const& ptr_app,
								   real_type ts,
//								   real_type rls_forgetting_factor/* = base_type::default_rls_forgetting_factor*/,
								   system_identification_strategy_params_pointer const& ptr_ident_strategy_params,
								   triggers_type const& triggers,
								   real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
//	: base_type(n_a, n_b, d, ptr_app, ts, rls_forgetting_factor, ewma_smoothing_factor),
	: base_type(n_a, n_b, d, ptr_app, ts, ptr_ident_strategy_params, triggers, ewma_smoothing_factor),
	  controller_(Q, R, N),
	  xi_(1,0)
	{
	}


	protected: void do_process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Process SYSTEM-INITIALIZATION event (Clock: " << ctx.simulated_time() << ")");

		base_type::do_process_sys_init(evt, ctx);

		xi_ = vector_type(1,0);

		DCS_DEBUG_TRACE("(" << this << ") END Do Process SYSTEM-INITIALIZATION event (Clock: " << ctx.simulated_time() << ")");
	}


	private: vector_type do_optimal_control(vector_type const& x, vector_type const& u, vector_type const& y, matrix_type const& A, matrix_type const& B, matrix_type const& C, matrix_type const& D)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( u );

		namespace ublas = ::boost::numeric::ublas;
		namespace ublasx = ::boost::numeric::ublasx;

		//FIXME: since we are using a discrete-time system, should we really use the real sampling time or simply 1?.
		//real_type ts(this->sampling_time());
		real_type ts(1);

		// Form the augmented state-vector
		//
		//  z(k+1) = [x(k+1); xi(k+1)]
		//         = [ A      0][x(k) ]+[ B     ]u(k)+[0]
		//           [-C|t_s| I][xi(k)]+[-D|t_s|]    +[r]|t_s|
		//  y(k+1) = [C 0]z(k)+Du(k)
		//
		// where r is the reference value to be tracked and xi is the current
		// integrated control error:
		//  xi(k) = xi(k-1) + e(k-1)
		//        = xi(k-1) + (r-y(k-1))
		//        = xi(k-1) + (r-Cx(k-1)-Du(k-1))|t_s|
		//

		uint_type nx(ublasx::size(x));
		uint_type nu(ublasx::size(u));
		uint_type ny(ublasx::size(y));
		uint_type nz(nx+ny);

		// Update the integrated control error.
		// NOTE: In our case the reference value r is zero
		//xi_ = xi_- ublas::subrange(x, ublasx::num_rows(A)-ublasx::num_rows(C), ublasx::num_rows(A));
		//xi_ = xi_- y;
		//FIXME: response-time performance metric is hard-coded
#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION)
# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT)
#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
		vector_type r(ublas::scalar_vector<real_type>(ny, this->application().performance_model().application_measure(response_time_performance_measure)/this->eq_out_measure_-1));
#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
		vector_type r(ublas::scalar_vector<real_type>(ny, 0));
#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
		vector_type r(ublas::scalar_vector<real_type>(ny, this->application().performance_model().application_measure(response_time_performance_measure)-this->eq_out_measure_));
#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
		vector_type r(ublas::scalar_vector<real_type>(ny, 0));
#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION
		vector_type r(ublas::scalar_vector<real_type>(ny, this->application().performance_model().application_measure(response_time_performance_measure)));
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION
		//vector_type r(ublas::scalar_vector<real_type>(ny, 1.0/this->application().performance_model().application_measure(response_time_performance_measure)));//[EXP-20120207]
		//FIXME: we need to know what y represents (i.e., either a deviation from equilibrium, or a normalized deviation, or an absolute value, ...)
		//xi_ = xi_+ ts*(r-y-ublas::scalar_vector<real_type>(ublasx::size(y), this->eq_out_measure_)); 
		xi_ = xi_+ ts*(r-y);
		//xi_ = xi_+ublas::scalar_vector<real_type>(1,1)- ublas::subrange(x, ublasx::num_rows(A)-ublasx::num_rows(C), ublasx::num_rows(A));

		vector_type z(nz);
		ublas::subrange(z, 0, nx) = x;
		ublas::subrange(z, nx, nz) = xi_;
::std::cerr << "APP: " << this->application().id() << " - Control Error: " << (r-y) << " - xi=" << xi_ << " - Extended State: " << z << ::std::endl;//XXX

		vector_type opt_u;

		controller_.solve(A, B, C, D, ts);

//::std::cerr << "[dcs::des::cloud::lqi_controller] z: " << z << ::std::endl;//XXX
//::std::cerr << "[dcs::des::cloud::lqi_controller] K: " << controller_.gain() << ::std::endl;//XXX
//::std::cerr << "[dcs::des::cloud::lqi_controller] S: " << controller_.are_solution() << ::std::endl;//XXX
//::std::cerr << "[dcs::des::cloud::lqi_controller] e: " << controller_.eigenvalues() << ::std::endl;//XXX
		opt_u = ublas::real(controller_.control(z));

		uint_type ncp(nx+nu);
		uint_type nrp(nx+ny);
		matrix_type P(nrp, ncp, 0);
		ublas::subrange(P, 0, nx, 0, nx) = ublas::identity_matrix<real_type>(nx, nx) - A;
		ublas::subrange(P, 0, nx, nx, ncp) = B;
		ublas::subrange(P, nx, nrp, 0, nx) = -C;
		ublas::subrange(P, nx, nrp, nx, ncp) = D;
		matrix_type Pt(ublas::trans(P));
		matrix_type PP(ublas::prod(P, Pt));
		bool inv = ublasx::inv_inplace(PP);
		if (inv)
		{
			PP = ublas::prod(Pt, PP);
			vector_type yd(nrp,0);
			ublas::subrange(yd, nx, nrp) = r;
			vector_type xdud(ublas::prod(PP, yd));
			::std::cerr << "COMPENSATION: P=" << P << " ==> (xd,ud)=" << xdud << ", opt_u=" << opt_u << ::std::endl;//XXX
			opt_u = opt_u + ublas::subrange(xdud, nx, ncp);
			::std::cerr << "COMPENSATION: P=" << P << " ==> (xd,ud)=" << xdud << ", NEW opt_u=" << opt_u << ::std::endl;//XXX
		}
		else
		{
			DCS_EXCEPTION_THROW( ::std::runtime_error, "Cannot compute equilibrium control input: Rosenbrock's system matrix is not invertible" );
		}

		return opt_u;
	}


	/// The LQI controller implementation.
	private: lq_controller_type controller_;
	/// The integrated control error.
	private: vector_type xi_;
}; // lqi_application_controller


//template <typename TraitsT>
//class lqiy_application_controller: public detail::lq_application_controller<TraitsT>
//{
//	private: typedef detail::lq_application_controller<TraitsT> base_type;
//	public: typedef TraitsT traits_type;
//	public: typedef typename traits_type::real_type real_type;
//	public: typedef typename traits_type::uint_type uint_type;
//	public: typedef typename base_type::application_pointer application_pointer;
//	public: typedef base_system_identification_strategy_params<traits_type> system_identification_strategy_type;
//	private: typedef ::dcs::control::dlqiy_controller<real_type> lq_controller_type;
//	private: typedef typename base_type::vector_type vector_type;
//	private: typedef typename base_type::matrix_type matrix_type;
//
//
//	public: lqiy_application_controller()
//	: base_type()
//	{
//	}
//
//
//	public: lqiy_application_controller(application_pointer const& ptr_app, real_type ts)
//	: base_type(ptr_app, ts)
//	{
//	}
//
//
//	public: template <typename QMatrixExprT, typename RMatrixExprT>
//		lqiy_application_controller(uint_type n_a,
//								    uint_type n_b,
//								    uint_type d,
//								    ::boost::numeric::ublas::matrix_expression<QMatrixExprT> const& Q,
//								    ::boost::numeric::ublas::matrix_expression<RMatrixExprT> const& R,
//								    application_pointer const& ptr_app,
//								    real_type ts,
////								    real_type rls_forgetting_factor/* = base_type::default_rls_forgetting_factor*/,
//								    system_identification_strategy_type const& ident_strategy,
//								    real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
////	: base_type(n_a, n_b, d, ptr_app, ts, rls_forgetting_factor, ewma_smoothing_factor),
//	: base_type(n_a, n_b, d, ptr_app, ts, ident_strategy, ewma_smoothing_factor),
//	  controller_(Q, R)
//	{
//	}
//
//
//	public: template <typename QMatrixExprT, typename RMatrixExprT, typename NMatrixExprT>
//		lqiy_application_controller(uint_type n_a,
//								    uint_type n_b,
//								    uint_type d,
//								    ::boost::numeric::ublas::matrix_expression<QMatrixExprT> const& Q,
//								    ::boost::numeric::ublas::matrix_expression<RMatrixExprT> const& R,
//								    ::boost::numeric::ublas::matrix_expression<NMatrixExprT> const& N,
//								    application_pointer const& ptr_app,
//								    real_type ts,
////								    real_type rls_forgetting_factor/* = base_type::default_rls_forgetting_factor*/,
//								    system_identification_strategy_type const& ident_strategy,
//								    real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
////	: base_type(n_a, n_b, d, ptr_app, ts, rls_forgetting_factor, ewma_smoothing_factor),
//	: base_type(n_a, n_b, d, ptr_app, ts, ident_strategy, ewma_smoothing_factor),
//	  controller_(Q, R, N)
//	{
//	}
//
//
//	private: vector_type do_optimal_control(vector_type const& x, vector_type const& u, vector_type const& y, matrix_type const& A, matrix_type const& B, matrix_type const& C, matrix_type const& D)
//	{
//		namespace ublas = ::boost::numeric::ublas;
//		namespace ublasx = ::boost::numeric::ublasx;
//
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( u );
//
//		vector_type opt_u;
//
//		controller_.solve(A, B, C, D, this->sampling_time());
//
//		// Form the augmented state-vector
//		//
//		//  z(k+1) = [x(k+1); xi(k+1)]
//		//         = [ A      0][x(k) ]+[ B     ]u(k)+[0]
//		//           [-C|t_s| I][xi(k)]+[-D|t_s|]    +[r]
//		//  y(k+1) = [C 0]z(k)+Du(k)
//		//
//		// where r is the reference value to be tracked and xi is the current
//		// integrated control error:
//		//  xi(k) = xi(k-1) + e(k-1)
//		//        = xi(k-1) + (r-y(k-1))
//		//        = xi(k-1) + (r-Cx(k-1)-Du(k-1))
//		//
//
//		// Update the integrated control error.
//		// NOTE: In our case the reference value r is zero
//		//xi_ = xi_- ublas::subrange(x, ublasx::num_rows(A)-ublasx::num_rows(C), ublasx::num_rows(A));
//		xi_ = xi_- y;
//		//xi_ = xi_+ublas::scalar_vector<real_type>(1,1)- ublas::subrange(x, ublasx::num_rows(A)-ublasx::num_rows(C), ublasx::num_rows(A));
//
//		vector_type z(ublasx::num_rows(A)+ublasx::num_rows(C));
//		ublas::subrange(z, 0, ublasx::num_rows(A)) = x;
//		ublas::subrange(z, ublasx::num_rows(A), ublasx::num_rows(A)+ublasx::num_rows(C)) = xi_;
//DCS_DEBUG_TRACE("Augmented x=" << z);//XXX
//
//		opt_u = ublas::real(controller_.control(z));
//
//		return opt_u;
//	}
//
//
//	/// The LQI controller
//	private: lq_controller_type controller_;
//	/// The integrated control error.
//	private: vector_type xi_;
//}; // lqiy_application_controller


template <typename TraitsT>
class lqr_application_controller: public detail::lq_application_controller<TraitsT>
{
	private: typedef detail::lq_application_controller<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename base_type::application_pointer application_pointer;
	public: typedef typename base_type::system_identification_strategy_params_pointer system_identification_strategy_params_pointer;
	public: typedef typename base_type::triggers_type triggers_type;
	private: typedef ::dcs::control::dlqr_controller<real_type> lq_controller_type;
	private: typedef typename base_type::vector_type vector_type;
	private: typedef typename base_type::matrix_type matrix_type;


	public: lqr_application_controller()
	: base_type()
	{
	}


	public: lqr_application_controller(application_pointer const& ptr_app, real_type ts)
	: base_type(ptr_app, ts)
	{
	}


	public: template <typename QMatrixExprT, typename RMatrixExprT>
		lqr_application_controller(uint_type n_a,
								   uint_type n_b,
								   uint_type d,
								   ::boost::numeric::ublas::matrix_expression<QMatrixExprT> const& Q,
								   ::boost::numeric::ublas::matrix_expression<RMatrixExprT> const& R,
								   application_pointer const& ptr_app,
								   real_type ts,
								   system_identification_strategy_params_pointer const& ptr_ident_strategy_params,
								   triggers_type const& triggers,
//								   real_type rls_forgetting_factor/* = base_type::default_rls_forgetting_factor*/,
								   real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
//	: base_type(n_a, n_b, d, ptr_app, ts, rls_forgetting_factor, ewma_smoothing_factor),
	: base_type(n_a, n_b, d, ptr_app, ts, ptr_ident_strategy_params, triggers, ewma_smoothing_factor),
	  controller_(Q, R)
	{
	}


	public: template <typename QMatrixExprT, typename RMatrixExprT, typename NMatrixExprT>
		lqr_application_controller(uint_type n_a,
								   uint_type n_b,
								   uint_type d,
								   ::boost::numeric::ublas::matrix_expression<QMatrixExprT> const& Q,
								   ::boost::numeric::ublas::matrix_expression<RMatrixExprT> const& R,
								   ::boost::numeric::ublas::matrix_expression<NMatrixExprT> const& N,
								   application_pointer const& ptr_app,
								   real_type ts,
								   system_identification_strategy_params_pointer const& ptr_ident_strategy_params,
								   triggers_type const& triggers,
//								   real_type rls_forgetting_factor/* = base_type::default_rls_forgetting_factor*/,
								   real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
//	: base_type(n_a, n_b, d, ptr_app, ts, rls_forgetting_factor, ewma_smoothing_factor),
	: base_type(n_a, n_b, d, ptr_app, ts, ptr_ident_strategy_params, triggers, ewma_smoothing_factor),
	  controller_(Q, R, N)
	{
	}


	private: vector_type do_optimal_control(vector_type const& x, vector_type const& u, vector_type const& y, matrix_type const& A, matrix_type const& B, matrix_type const& C, matrix_type const& D)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( u );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( y );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( C );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( D );

		namespace ublas = ::boost::numeric::ublas;
		namespace ublasx = ::boost::numeric::ublasx;

//		// Check: if (A,B) is controllable, then the associated DARE has a
//		//        positive semidefinite solution.
//		//        (sufficient but not necessary condition)
//		if (!::dcs::control::is_controllable(A, B))
//		{
//			//throw ::std::runtime_error("System (A,B) is not state-controllable");
//			::std::clog << "[Warning:lq_app_ctrl] System (A,B) is not state-controllable [with A=" << A << " and B=" << B << "]." << ::std::endl;
//		}
		// Check: if (A,B) is stabilizable, then the assoicated DARE has a
		//        positive semidefinite solution.
		//        (sufficient and necessary condition)
		if (!::dcs::control::is_stabilizable(A, B, true))
		{
			::std::ostringstream oss;
			oss << "APP: " << this->application().id() << " - System (A,B) is not stabilizable (the associated DARE cannot have a positive semidefinite solution) [with A=" << A << " and B=" << B << "]";
			log_warn(base_type::cls_id_, oss.str());
			throw ::std::runtime_error("System (A,B) is not stabilizable (DARE cannot have a positive semidefinite solution).");
		}
//		// Check: if (A,B) controllable and (Q,A) observable, then the
//		//        associated DARE has a unique and stabilizing solution such
//		//        that the closed-loop system:
//		//          x(k+1) = Ax(k) + Bu(k) = (A + BK)x(k)
//		//        is stable (K is the LQR-optimal state feedback gain).
//		//        (sufficient but not necessary condition)
//		if (!::dcs::control::is_observable(A, controller_.Q()))
//		{
//			//throw ::std::runtime_error("System (A,Q) is not observable (closed-loop system will not be stable).");
//			::std::clog << "[Warning:lq_app_ctrl] System (Q,A) is not observable (closed-loop system will not be stable) [with " << A << " and B=" << B << "]." << ::std::endl;
//		}
		// Check: if (A,B) stabilizable and (Q,A) detectable, then the
		//        associated DARE has a unique and stabilizing solution such
		//        that the closed-loop system:
		//          x(k+1) = Ax(k) + Bu(k) = (A + BK)x(k)
		//        is stable (K is the LQR-optimal state feedback gain).
		//        (sufficient and necessary condition)
		if (!::dcs::control::is_detectable(A, controller_.Q(), true))
		{
			::std::ostringstream oss;
			oss << "APP: " << this->application().id() << " - System (Q,A) is not detectable (closed-loop system will not be stable) [with " << A << " and Q=" << controller_.Q() << "]";
			log_warn(base_type::cls_id_, oss.str());
			throw ::std::runtime_error("System (Q,A) is not detectable (closed-loop system will not be stable).");
		}


		uint_type nx(ublas::num_columns(A));
		uint_type nu(ublas::num_columns(B));
		uint_type ny(ublas::num_rows(C));

#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION)
# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT)
#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
		vector_type r(ublas::scalar_vector<real_type>(ny, this->application().performance_model().application_measure(response_time_performance_measure)/this->eq_out_measure_-1));
#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
		vector_type r(ublas::scalar_vector<real_type>(ny, 0));
#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
		vector_type r(ublas::scalar_vector<real_type>(ny, this->application().performance_model().application_measure(response_time_performance_measure)-this->eq_out_measure_));
#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
		vector_type r(ublas::scalar_vector<real_type>(ny, 0));
#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION
		vector_type r(ublas::scalar_vector<real_type>(ny, this->application().performance_model().application_measure(response_time_performance_measure)));
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION

		vector_type opt_u;

		controller_.solve(A, B);
		opt_u = ublas::real(controller_.control(x));

		uint_type ncp(nx+nu);
		uint_type nrp(nx+ny);

		matrix_type P(nrp, ncp, 0);
		ublas::subrange(P, 0, nx, 0, nx) = ublas::identity_matrix<real_type>(nx, nx) - A;
		ublas::subrange(P, 0, nx, nx, ncp) = B;
		ublas::subrange(P, nx, nrp, 0, nx) = -C;
		ublas::subrange(P, nx, nrp, nx, ncp) = D;
		matrix_type Pt(ublas::trans(P));
		matrix_type PP(ublas::prod(P, Pt));
		bool inv = ublasx::inv_inplace(PP);
		if (inv)
		{
			PP = ublas::prod(Pt, PP);
			vector_type yd(nrp,0);
			ublas::subrange(yd, nx, nrp) = r;
			vector_type xdud(ublas::prod(PP, yd));
			::std::cerr << "COMPENSATION: P=" << P << " ==> (xd,ud)=" << xdud << ", opt_u=" << opt_u << ::std::endl;//XXX
			opt_u = opt_u + ublas::subrange(xdud, nx, ncp);
			::std::cerr << "COMPENSATION: P=" << P << " ==> (xd,ud)=" << xdud << ", NEW opt_u=" << opt_u << ::std::endl;//XXX
		}
		else
		{
			DCS_EXCEPTION_THROW( ::std::runtime_error, "Cannot compute equilibrium control input: Rosenbrock's system matrix is not invertible" );
		}

		return opt_u;
	}


//	private: template <typename SysIdentStrategyT>
//		void do_make_ss(SysIdentStrategyT const& sys_ident_strategy,
//						matrix_type& A,
//						matrix_type& B,
//						matrix_type& C,
//						matrix_type& D)
//	{
//		namespace ublas = ::boost::numeric::ublas;
//
//		typedef real_type value_type;
//		typedef ::std::size_t size_type; //FIXME: use type-promotion?
//
//		const size_type rls_n_a(sys_ident_strategy.output_order());
//		const size_type rls_n_b(sys_ident_strategy.input_order());
//	//	const size_type rls_d(sys_ident_strategy.input_delay());
//		const size_type rls_n_y(sys_ident_strategy.num_outputs());
//		const size_type rls_n_u(sys_ident_strategy.num_inputs());
//		const size_type n_x(rls_n_a*rls_n_y);
//		const size_type n_u(rls_n_b*rls_n_u);
//	//	const size_type n(::std::max(n_x,n_u));
//		const size_type n_y(1);
//
//		// Create the state matrix A
//		// A=[-A_1 -A_2 ... -A_{n_a-1} -A_{n_a};
//		//     I    0   ...  0          0      ;
//		//     0    I   ...  0          0      ;
//		//     .    .   ...  .          .      ;
//		//     .    .   ...  .          .      ;
//		//     .    .   ...  .          .      ;
//		// 	   0    0   ...  I          0      ]
//		if (n_x > 0)
//		{
//			
//			size_type rcoffs(n_x-rls_n_y); // The rightmost column offset
//
//			A().resize(n_x, n_x, false);
//
//			// The upper part of A is filled with A_1, ..., A_{n_a}
//			for (size_type i = 0; i < rls_n_a; ++i)
//			{
//				// Copy matrix -A_i from \hat{\Theta} into A.
//				// In A the matrix A_i has to go in (rls_n_a*i)-th position:
//				//   A(0:rls_n_y,rls_n_y*i:rls_n_y*(i+1)) <- -A_i
//
//				size_type c1(rls_n_y*i);
//				size_type c2(c1+rls_n_y);
//
//				ublas::subrange(A(), 0, rls_n_y, c1, c2) = -sys_ident_strategy.A(i+1);
//			}
//
//			// The lower part of A is set to [I_{k,k} 0_{rls_n_y,rls_n_y}],
//			// where: k=n_x-rls_n_y.
//			ublas::subrange(A(), rls_n_y, n_x, 0, rcoffs) = ublas::identity_matrix<value_type>(rcoffs,rcoffs);
//			ublas::subrange(A(), rls_n_y, n_x, rcoffs, n_x) = ublas::zero_matrix<value_type>(rls_n_y,rls_n_y);
//		}
//		else
//		{
//			A().resize(0, 0, false);
//		}
//
//		// Create the input matrix B
//		// B=[B_1 ... B_{n_b};
//		//    0   ... 0      ;
//		//    .	  ... .
//		//    .	  ... .
//		//    .	  ... .
//		//    0   ... 0      ;
//		if (n_x > 0)
//		{
//			B().resize(n_x, n_u, false);
//
//			// Fill B with B_1, ..., B_{n_b}
//			for (size_type i = 0; i < rls_n_b; ++i)
//			{
//				// Copy matrix B_i from \hat{\Theta} into B.
//				// In \hat{\Theta} the matrix B_i stays at:
//				//   B_i <- (\hat{\Theta}(((n_a*n_y)+i):n_b:n_u,:))^T
//				// but in B the matrix B_i has to go in (n_b-i)-th position:
//				//   B(k:(k+n_x),((n_b-i-1)*n_u):((n_a-i)*n_u)) <- B_i
//
//				size_type c1(rls_n_u*i);
//				size_type c2(c1+rls_n_u);
//
//				ublas::subrange(B(), 0, rls_n_u, c1, c2) = sys_ident_strategy.B(i+1);
//			}
//
//			// The lower part of B is set to 0_{k,n_u}
//			// where: k=n_x-rls_n_u.
//			ublas::subrange(B(), rls_n_u, n_x, 0, n_u) = ublas::zero_matrix<value_type>(n_x-rls_n_u,n_u);
//
//		}
//		else
//		{
//			B().resize(0, 0, false);
//		}
//	//DCS_DEBUG_TRACE("B="<<B);//XXX
//
//		// Create the output matrix C
//		if (n_x > 0)
//		{
//			size_type rcoffs(n_x-rls_n_y); // The right most column offset
//
//			C().resize(n_y, n_x, false);
//
//			ublas::subrange(C(), 0, n_y, 0, rcoffs) = ublas::zero_matrix<value_type>(n_y,rcoffs);
//			ublas::subrange(C(), 0, n_y, rcoffs, n_x) = ublas::scalar_matrix<value_type>(n_y, rls_n_y, 1);
//		}
//		else
//		{
//			C().resize(0, 0, false);
//		}
//	//DCS_DEBUG_TRACE("C="<<C);//XXX
//
//		// Create the transmission matrix D
//		{
//			D().resize(n_y, n_u, false);
//
//			D() = ublas::zero_matrix<value_type>(n_y, n_u);
//		}
//	}


	/// The LQ (regulator) controller
	private: lq_controller_type controller_;
}; // lqr_application_controller


template <typename TraitsT>
class lqry_application_controller: public detail::lq_application_controller<TraitsT>
{
	private: typedef detail::lq_application_controller<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename base_type::application_pointer application_pointer;
	public: typedef typename base_type::system_identification_strategy_params_pointer system_identification_strategy_params_pointer;
	public: typedef typename base_type::triggers_type triggers_type;
	private: typedef ::dcs::control::dlqry_controller<real_type> lq_controller_type;
	private: typedef typename base_type::vector_type vector_type;
	private: typedef typename base_type::matrix_type matrix_type;


	public: lqry_application_controller()
	: base_type()
	{
	}


	public: lqry_application_controller(application_pointer const& ptr_app, real_type ts)
	: base_type(ptr_app, ts)
	{
	}


	public: template <typename QMatrixExprT, typename RMatrixExprT>
		lqry_application_controller(uint_type n_a,
								    uint_type n_b,
								    uint_type d,
								    ::boost::numeric::ublas::matrix_expression<QMatrixExprT> const& Q,
								    ::boost::numeric::ublas::matrix_expression<RMatrixExprT> const& R,
								    application_pointer const& ptr_app,
								    real_type ts,
									system_identification_strategy_params_pointer const& ptr_ident_strategy_params,
								    triggers_type const& triggers,
//								    real_type rls_forgetting_factor/* = base_type::default_rls_forgetting_factor*/,
								    real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
//	: base_type(n_a, n_b, d, ptr_app, ts, rls_forgetting_factor, ewma_smoothing_factor),
	: base_type(n_a, n_b, d, ptr_app, ts, ptr_ident_strategy_params, triggers, ewma_smoothing_factor),
	  controller_(Q, R)
	{
	}


	public: template <typename QMatrixExprT, typename RMatrixExprT, typename NMatrixExprT>
		lqry_application_controller(uint_type n_a,
								    uint_type n_b,
								    uint_type d,
								    ::boost::numeric::ublas::matrix_expression<QMatrixExprT> const& Q,
								    ::boost::numeric::ublas::matrix_expression<RMatrixExprT> const& R,
								    ::boost::numeric::ublas::matrix_expression<NMatrixExprT> const& N,
								    application_pointer const& ptr_app,
								    real_type ts,
									system_identification_strategy_params_pointer const& ptr_ident_strategy_params,
								    triggers_type const& triggers,
//								    real_type rls_forgetting_factor/* = base_type::default_rls_forgetting_factor*/,
								    real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
//	: base_type(n_a, n_b, d, ptr_app, ts, rls_forgetting_factor, ewma_smoothing_factor),
	: base_type(n_a, n_b, d, ptr_app, ts, ptr_ident_strategy_params, triggers, ewma_smoothing_factor),
	  controller_(Q, R, N)
	{
	}


	private: vector_type do_optimal_control(vector_type const& x, vector_type const& u, vector_type const& y, matrix_type const& A, matrix_type const& B, matrix_type const& C, matrix_type const& D)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( u );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( y );

		namespace ublas = ::boost::numeric::ublas;
		namespace ublasx = ::boost::numeric::ublasx;

		// Check: if (A,B) is stabilizable, then the assoicated DARE has a
		//        positive semidefinite solution.
		//        (sufficient and necessary condition)
		if (!::dcs::control::is_stabilizable(A, B, true))
		{
			::std::ostringstream oss;
			oss << "APP: " << this->application().id() << " - System (A,B) is not stabilizable (the associated DARE cannot have a positive semidefinite solution) [with A=" << A << " and B=" << B << "]";
			log_warn(base_type::cls_id_, oss.str());
			throw ::std::runtime_error("System (A,B) is not stabilizable (DARE cannot have a positive semidefinite solution).");
		}
		// Check: if (A,B) stabilizable and (C'QC,A) detectable, then the
		//        associated DARE has a unique and stabilizing solution such
		//        that the closed-loop system:
		//          x(k+1) = Ax(k) + Bu(k) = (A + BK)x(k)
		//        is stable (K is the LQRY-optimal state feedback gain).
		//        (sufficient and necessary condition)
		matrix_type QQ(ublas::prod(controller_.Q(), C));
		QQ = ublas::prod(ublas::trans(C), QQ);
		if (!::dcs::control::is_detectable(A, QQ, true))
		{
			::std::ostringstream oss;
			oss << "APP: " << this->application().id() << " - System (C'QC,A) is not detectable (closed-loop system will not be stable) [with " << A << ", Q=" << controller_.Q() << " and C=" << C << "]";
			log_warn(base_type::cls_id_, oss.str());
			throw ::std::runtime_error("System (C'QC,A) is not detectable (closed-loop system will not be stable).");
		}

//namespace ublas = ::boost::numeric::ublas;
//namespace ublasx = ::boost::numeric::ublasx;
//vector_type r(ublas::scalar_vector<real_type>(ublasx::size(y), this->application().performance_model().application_measure(response_time_performance_measure)-this->eq_out_measure_));
//matrix_type Kv;
//matrix_type A1(ublas::trans(B));
//A1 = ublas::prod(A1, controller_.are_solution());
//matrix_type A2(A1);
//A1 = ublas::prod(A1, B) + controller_.R();
//A1 = ublas::prod(A1, controller_.are_solution());
//A2 = ublas::prod(A2, A);
//ublasx::lu_solve(A1, A2, Kv);
//opt_u = ::boost::numeric::ublas::real(controller_.control(x))+ublas::prod(Kv,r);

		uint_type nx(ublas::num_columns(A));
		uint_type nu(ublas::num_columns(B));
		uint_type ny(ublas::num_rows(C));

#if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION)
# if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT)
#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
		vector_type r(ublas::scalar_vector<real_type>(ny, this->application().performance_model().application_measure(response_time_performance_measure)/this->eq_out_measure_-1));
#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
		vector_type r(ublas::scalar_vector<real_type>(ny, 0));
#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
# else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
#  if defined(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT)
		vector_type r(ublas::scalar_vector<real_type>(ny, this->application().performance_model().application_measure(response_time_performance_measure)-this->eq_out_measure_));
#  else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
		vector_type r(ublas::scalar_vector<real_type>(ny, 0));
#  endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
# endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION
		vector_type r(ublas::scalar_vector<real_type>(ny, this->application().performance_model().application_measure(response_time_performance_measure)));
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION

		vector_type opt_u;

		controller_.solve(A, B, C, D);
		opt_u = ublas::real(controller_.control(x));

		uint_type ncp(nx+nu);
		uint_type nrp(nx+ny);

		matrix_type P(nrp, ncp, 0);
		ublas::subrange(P, 0, nx, 0, nx) = ublas::identity_matrix<real_type>(nx, nx) - A;
		ublas::subrange(P, 0, nx, nx, ncp) = B;
		ublas::subrange(P, nx, nrp, 0, nx) = -C;
		ublas::subrange(P, nx, nrp, nx, ncp) = D;
		matrix_type Pt(ublas::trans(P));
		matrix_type PP(ublas::prod(P, Pt));
		bool inv = ublasx::inv_inplace(PP);
		if (inv)
		{
			PP = ublas::prod(Pt, PP);
			vector_type yd(nrp,0);
			ublas::subrange(yd, nx, nrp) = r;
			vector_type xdud(ublas::prod(PP, yd));
			::std::cerr << "COMPENSATION: P=" << P << " ==> (xd,ud)=" << xdud << ", opt_u=" << opt_u << ::std::endl;//XXX
			opt_u = opt_u + ublas::subrange(xdud, nx, ncp);
			::std::cerr << "COMPENSATION: P=" << P << " ==> (xd,ud)=" << xdud << ", NEW opt_u=" << opt_u << ::std::endl;//XXX
		}
		else
		{
			DCS_EXCEPTION_THROW( ::std::runtime_error, "Cannot compute equilibrium control input: Rosenbrock's system matrix is not invertible" );
		}

		return opt_u;
	}


	/// The LQ (regulator) controller
	private: lq_controller_type controller_;
}; // lqry_application_controller


/**
 * \brief Application controller based on the Linear-Quadratic-Integrator
 *  control.
 * 
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename TraitsT>
class matlab_lqi_application_controller: public detail::lq_application_controller<TraitsT>
{
	private: typedef detail::lq_application_controller<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename base_type::application_pointer application_pointer;
	public: typedef typename base_type::system_identification_strategy_params_pointer system_identification_strategy_params_pointer;
	public: typedef typename base_type::triggers_type triggers_type;
//	public: typedef base_system_identification_strategy_params<traits_type> system_identification_strategy_params_type;
//	public: typedef ::dcs::shared_ptr<system_identification_strategy_params_type> system_identification_strategy_params_pointer;
	private: typedef detail::matlab::dlqi_controller_proxy<real_type> lq_controller_type;
	private: typedef typename base_type::vector_type vector_type;
	private: typedef typename base_type::matrix_type matrix_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;


	public: matlab_lqi_application_controller()
	: base_type()
	{
	}


	public: matlab_lqi_application_controller(application_pointer const& ptr_app, real_type ts)
	: base_type(ptr_app, ts),
	  xi_(1,0)
	{
	}


	public: template <typename QMatrixExprT, typename RMatrixExprT>
		matlab_lqi_application_controller(uint_type n_a,
								   uint_type n_b,
								   uint_type d,
								   ::boost::numeric::ublas::matrix_expression<QMatrixExprT> const& Q,
								   ::boost::numeric::ublas::matrix_expression<RMatrixExprT> const& R,
								   application_pointer const& ptr_app,
								   real_type ts,
//								   real_type rls_forgetting_factor/* = base_type::default_rls_forgetting_factor*/,
								   system_identification_strategy_params_pointer const& ptr_ident_strategy_params,
								   triggers_type const& triggers,
								   real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
//	: base_type(n_a, n_b, d, ptr_app, ts, rls_forgetting_factor, ewma_smoothing_factor),
	: base_type(n_a, n_b, d, ptr_app, ts, ptr_ident_strategy_params, triggers, ewma_smoothing_factor),
	  controller_(Q, R),
	  xi_(1,0)
	{
	}


	public: template <typename QMatrixExprT, typename RMatrixExprT, typename NMatrixExprT>
		matlab_lqi_application_controller(uint_type n_a,
								   uint_type n_b,
								   uint_type d,
								   ::boost::numeric::ublas::matrix_expression<QMatrixExprT> const& Q,
								   ::boost::numeric::ublas::matrix_expression<RMatrixExprT> const& R,
								   ::boost::numeric::ublas::matrix_expression<NMatrixExprT> const& N,
								   application_pointer const& ptr_app,
								   real_type ts,
//								   real_type rls_forgetting_factor/* = base_type::default_rls_forgetting_factor*/,
								   system_identification_strategy_params_pointer const& ptr_ident_strategy_params,
								   triggers_type const& triggers,
								   real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
//	: base_type(n_a, n_b, d, ptr_app, ts, rls_forgetting_factor, ewma_smoothing_factor),
	: base_type(n_a, n_b, d, ptr_app, ts, ptr_ident_strategy_params, triggers, ewma_smoothing_factor),
	  controller_(Q, R, N),
	  xi_(1,0)
	{
	}


	protected: void do_process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Process SYSTEM-INITIALIZATION event (Clock: " << ctx.simulated_time() << ")");

		base_type::do_process_sys_init(evt, ctx);

		xi_ = vector_type(1,0);

		DCS_DEBUG_TRACE("(" << this << ") END Do Process SYSTEM-INITIALIZATION event (Clock: " << ctx.simulated_time() << ")");
	}


	private: vector_type do_optimal_control(vector_type const& x, vector_type const& u, vector_type const& y, matrix_type const& A, matrix_type const& B, matrix_type const& C, matrix_type const& D)
	{
		namespace ublas = ::boost::numeric::ublas;
		namespace ublasx = ::boost::numeric::ublasx;

		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( u );

		vector_type opt_u;

		controller_.solve(A, B, C, D, this->sampling_time());

		// Form the augmented state-vector
		//
		//  z(k+1) = [x(k+1); xi(k+1)]
		//         = [ A      0][x(k) ]+[ B     ]u(k)+[0]
		//           [-C|t_s| I][xi(k)]+[-D|t_s|]    +[r]
		//  y(k+1) = [C 0]z(k)+Du(k)
		//
		// where r is the reference value to be tracked and xi is the current
		// integrated control error:
		//  xi(k) = xi(k-1) + e(k-1)
		//        = xi(k-1) + (r-y(k-1))
		//        = xi(k-1) + (r-Cx(k-1)-Du(k-1))
		//

		// Update the integrated control error.
		// NOTE: In our case the reference value r is zero
		//xi_ = xi_- ublas::subrange(x, ublasx::num_rows(A)-ublasx::num_rows(C), ublasx::num_rows(A));
		xi_ = xi_- y;
		//xi_ = xi_+ublas::scalar_vector<real_type>(1,1)- ublas::subrange(x, ublasx::num_rows(A)-ublasx::num_rows(C), ublasx::num_rows(A));

		vector_type z(ublasx::num_rows(A)+ublasx::num_rows(C));
		ublas::subrange(z, 0, ublasx::num_rows(A)) = x;
		ublas::subrange(z, ublasx::num_rows(A), ublasx::num_rows(A)+ublasx::num_rows(C)) = xi_;
DCS_DEBUG_TRACE("Augmented x=" << z);//XXX

		opt_u = ublas::real(controller_.control(z));

		return opt_u;
	}


	/// The LQI controller implementation.
	private: lq_controller_type controller_;
	/// The integrated control error.
	private: vector_type xi_;
}; // matlab_lqi_application_controller


template <typename TraitsT>
class matlab_lqr_application_controller: public detail::lq_application_controller<TraitsT>
{
	private: typedef detail::lq_application_controller<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename base_type::application_pointer application_pointer;
	public: typedef typename base_type::system_identification_strategy_params_pointer system_identification_strategy_params_pointer;
	public: typedef typename base_type::triggers_type triggers_type;
	private: typedef detail::matlab::dlqr_controller_proxy<real_type> lq_controller_type;
	private: typedef typename base_type::vector_type vector_type;
	private: typedef typename base_type::matrix_type matrix_type;


	public: matlab_lqr_application_controller()
	: base_type()
	{
	}


	public: matlab_lqr_application_controller(application_pointer const& ptr_app, real_type ts)
	: base_type(ptr_app, ts)
	{
	}


	public: template <typename QMatrixExprT, typename RMatrixExprT>
		matlab_lqr_application_controller(uint_type n_a,
								   uint_type n_b,
								   uint_type d,
								   ::boost::numeric::ublas::matrix_expression<QMatrixExprT> const& Q,
								   ::boost::numeric::ublas::matrix_expression<RMatrixExprT> const& R,
								   application_pointer const& ptr_app,
								   real_type ts,
								   system_identification_strategy_params_pointer const& ptr_ident_strategy_params,
								   triggers_type const& triggers,
//								   real_type rls_forgetting_factor/* = base_type::default_rls_forgetting_factor*/,
								   real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
//	: base_type(n_a, n_b, d, ptr_app, ts, rls_forgetting_factor, ewma_smoothing_factor),
	: base_type(n_a, n_b, d, ptr_app, ts, ptr_ident_strategy_params, triggers, ewma_smoothing_factor),
	  controller_(Q, R)
	{
	}


	public: template <typename QMatrixExprT, typename RMatrixExprT, typename NMatrixExprT>
		matlab_lqr_application_controller(uint_type n_a,
								   uint_type n_b,
								   uint_type d,
								   ::boost::numeric::ublas::matrix_expression<QMatrixExprT> const& Q,
								   ::boost::numeric::ublas::matrix_expression<RMatrixExprT> const& R,
								   ::boost::numeric::ublas::matrix_expression<NMatrixExprT> const& N,
								   application_pointer const& ptr_app,
								   real_type ts,
								   system_identification_strategy_params_pointer const& ptr_ident_strategy_params,
								   triggers_type const& triggers,
//								   real_type rls_forgetting_factor/* = base_type::default_rls_forgetting_factor*/,
								   real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
//	: base_type(n_a, n_b, d, ptr_app, ts, rls_forgetting_factor, ewma_smoothing_factor),
	: base_type(n_a, n_b, d, ptr_app, ts, ptr_ident_strategy_params, triggers, ewma_smoothing_factor),
	  controller_(Q, R, N)
	{
	}


	private: vector_type do_optimal_control(vector_type const& x, vector_type const& u, vector_type const& y, matrix_type const& A, matrix_type const& B, matrix_type const& C, matrix_type const& D)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( u );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( y );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( C );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( D );

		vector_type opt_u;

		controller_.solve(A, B);
		opt_u = ::boost::numeric::ublas::real(controller_.control(x));

		return opt_u;
	}


	/// The LQ (regulator) controller
	private: lq_controller_type controller_;
}; // matlab_lqr_application_controller


template <typename TraitsT>
class matlab_lqry_application_controller: public detail::lq_application_controller<TraitsT>
{
	private: typedef detail::lq_application_controller<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename base_type::application_pointer application_pointer;
	public: typedef typename base_type::system_identification_strategy_params_pointer system_identification_strategy_params_pointer;
	public: typedef typename base_type::triggers_type triggers_type;
	private: typedef detail::matlab::dlqry_controller_proxy<real_type> lq_controller_type;
	private: typedef typename base_type::vector_type vector_type;
	private: typedef typename base_type::matrix_type matrix_type;


	public: matlab_lqry_application_controller()
	: base_type()
	{
	}


	public: matlab_lqry_application_controller(application_pointer const& ptr_app, real_type ts)
	: base_type(ptr_app, ts)
	{
	}


	public: template <typename QMatrixExprT, typename RMatrixExprT>
		matlab_lqry_application_controller(uint_type n_a,
										   uint_type n_b,
										   uint_type d,
										   ::boost::numeric::ublas::matrix_expression<QMatrixExprT> const& Q,
										   ::boost::numeric::ublas::matrix_expression<RMatrixExprT> const& R,
										   application_pointer const& ptr_app,
										   real_type ts,
										   system_identification_strategy_params_pointer const& ptr_ident_strategy_params,
										   triggers_type const& triggers,
										   real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
	: base_type(n_a, n_b, d, ptr_app, ts, ptr_ident_strategy_params, triggers, ewma_smoothing_factor),
	  controller_(Q, R)
	{
	}


	public: template <typename QMatrixExprT, typename RMatrixExprT, typename NMatrixExprT>
		matlab_lqry_application_controller(uint_type n_a,
										   uint_type n_b,
										   uint_type d,
										   ::boost::numeric::ublas::matrix_expression<QMatrixExprT> const& Q,
										   ::boost::numeric::ublas::matrix_expression<RMatrixExprT> const& R,
										   ::boost::numeric::ublas::matrix_expression<NMatrixExprT> const& N,
										   application_pointer const& ptr_app,
										   real_type ts,
										   system_identification_strategy_params_pointer const& ptr_ident_strategy_params,
										   triggers_type const& triggers,
										   real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
	: base_type(n_a, n_b, d, ptr_app, ts, ptr_ident_strategy_params, triggers, ewma_smoothing_factor),
	  controller_(Q, R, N)
	{
	}


	private: vector_type do_optimal_control(vector_type const& x, vector_type const& u, vector_type const& y, matrix_type const& A, matrix_type const& B, matrix_type const& C, matrix_type const& D)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( u );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( y );

		vector_type opt_u;

		controller_.solve(A, B, C, D);
		opt_u = ::boost::numeric::ublas::real(controller_.control(x));

		return opt_u;
	}


	/// The LQ (regulator) controller
	private: lq_controller_type controller_;
}; // matlab_lqry_application_controller


template <typename TraitsT>
class fmpc_application_controller: public detail::lq_application_controller<TraitsT>
{
	private: typedef detail::lq_application_controller<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename base_type::application_pointer application_pointer;
	public: typedef typename base_type::system_identification_strategy_params_pointer system_identification_strategy_params_pointer;
	public: typedef typename base_type::triggers_type triggers_type;
	private: typedef ::dcs::control::fmpc_controller<real_type,uint_type> controller_impl_type;
	private: typedef typename base_type::vector_type vector_type;
	private: typedef typename base_type::matrix_type matrix_type;


	public: fmpc_application_controller()
	: base_type()
	{
	}


	public: fmpc_application_controller(application_pointer const& ptr_app, real_type ts)
	: base_type(ptr_app, ts)
	{
	}


	public: template <typename QMatrixExprT,
					  typename RMatrixExprT,
					  typename QfMatrixExprT,
					  typename XMinVectorExprT,
					  typename XMaxVectorExprT,
					  typename UMinVectorExprT,
					  typename UMaxVectorExprT>
		fmpc_application_controller(uint_type n_a,
								   uint_type n_b,
								   uint_type d,
								   ::boost::numeric::ublas::matrix_expression<QMatrixExprT> const& Q,
								   ::boost::numeric::ublas::matrix_expression<RMatrixExprT> const& R,
								   ::boost::numeric::ublas::matrix_expression<QfMatrixExprT> const& Qf,
								   ::boost::numeric::ublas::vector_expression<XMinVectorExprT> const& xmin,
								   ::boost::numeric::ublas::vector_expression<XMaxVectorExprT> const& xmax,
								   ::boost::numeric::ublas::vector_expression<UMinVectorExprT> const& umin,
								   ::boost::numeric::ublas::vector_expression<UMaxVectorExprT> const& umax,
								   uint_type horizon,
								   real_type barrier,
								   uint_type niters,
								   application_pointer const& ptr_app,
								   real_type ts,
								   system_identification_strategy_params_pointer const& ptr_ident_strategy_params,
								   triggers_type const& triggers,
//								   real_type rls_forgetting_factor,
								   real_type ewma_smoothing_factor)
//	: base_type(n_a, n_b, d, ptr_app, ts, rls_forgetting_factor, ewma_smoothing_factor),
	: base_type(n_a, n_b, d, ptr_app, ts, ptr_ident_strategy_params, triggers, ewma_smoothing_factor),
	  controller_(Q, R, Qf, xmin, xmax, umin, umax, horizon, barrier, niters)
	{
	}


	private: vector_type do_optimal_control(vector_type const& x, vector_type const& u, vector_type const& y, matrix_type const& A, matrix_type const& B, matrix_type const& C, matrix_type const& D)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( u );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( y );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( C );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( D );

		vector_type opt_u;

		controller_.solve(A, B);
		opt_u = ::boost::numeric::ublas::real(controller_.control(x));

		return opt_u;
	}


	/// The Fast MPC controller
	private: controller_impl_type controller_;
}; // fmpc_application_controller

}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_LQ_APPLICATION_CONTROLLER_HPP
