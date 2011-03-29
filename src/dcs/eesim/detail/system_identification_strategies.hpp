/**
 * \file dcs/eesim/detail/system_identification_strategies.hpp
 *
 * \brief Class containing wrappers for different system identification
 *  strategies.
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
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */

#ifndef DCS_EESIM_DETAIL_SYSTEM_IDENTIFICATION_STRATEGIES_HPP
#define DCS_EESIM_DETAIL_SYSTEM_IDENTIFICATION_STRATEGIES_HPP


#include <algorithm>
#include <boost/numeric/ublas/expression_types.hpp>
#ifdef DCS_DEBUG
#	include <boost/numeric/ublas/io.hpp>
#endif // DCS_DEBUG
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/traits.hpp>
#include <boost/numeric/ublasx/operation/max.hpp>
#include <boost/numeric/ublasx/operation/num_columns.hpp>
#include <boost/numeric/ublasx/operation/num_rows.hpp>
#include <boost/numeric/ublasx/operation/rcond.hpp>
#include <boost/numeric/ublasx/operation/size.hpp>
#include <cmath>
#include <dcs/debug.hpp>
#include <dcs/eesim/system_identification_strategy_params.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <dcs/sysid/algorithm/rls.hpp>
//#include <exception>
#if defined(DCS_EESIM_USE_MATLAB_MCR)
#	include <dcs/eesim/detail/matlab/librls.h>
#	include <iostream>
#	include <mclcppclass.h>
#elif defined(DCS_EESIM_USE_MATLAB_APP)
#	if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE
#		include <boost/iostreams/device/file_descriptor.hpp>
#		include <boost/iostreams/stream_buffer.hpp>
#		include <cctype>
#		include <cerrno>
#		include <csignal>
#		include <cstddef>
#		include <cstdlib>
#		include <cstring>
#		include <fcntl.h>
#		include <iostream>
#		include <sstream>
#		include <sys/resource.h>
#		include <sys/time.h>
#		include <sys/types.h>
#		include <sys/wait.h>
#		include <unistd.h>
#	else // _POSIX_C_SOURCE
#		error "Unable to find a POSIX compliant system."
#	endif // _POSIX_C_SOURCE
#endif // DCS_EESIM_USE_MATLAB_*
#include <stdexcept>
#include <vector>


namespace dcs { namespace eesim { namespace detail {

template <typename TraitsT>
class base_system_identification_strategy
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef ::boost::numeric::ublas::matrix<real_type> matrix_type;
	public: typedef ::boost::numeric::ublas::vector<real_type> vector_type;
	public: typedef typename ::boost::numeric::ublas::promote_traits<
							typename ::boost::numeric::ublas::matrix_traits<matrix_type>::size_type,
							typename ::boost::numeric::ublas::vector_traits<vector_type>::size_type
					>::promote_type size_type;


	public: base_system_identification_strategy()
	: n_a_(0),
	  n_b_(0),
	  d_(0),
	  n_y_(0),
	  n_u_(0),
	  count_(0)
	{
	}


	public: base_system_identification_strategy(size_type n_a, size_type n_b, size_type d, size_type n_y, size_type n_u)
	: n_a_(n_a),
	  n_b_(n_b),
	  d_(d),
	  n_y_(n_y),
	  n_u_(n_u),
	  count_(0)
	{
	}


	public: base_system_identification_strategy(base_system_identification_strategy_params<traits_type> const& params)
	: n_a_(params.output_order()),
	  n_b_(params.input_order()),
	  d_(params.input_delay()),
	  n_y_(params.num_outputs()),
	  n_u_(params.num_inputs()),
	  count_(0)
	{
	}


	public: virtual ~base_system_identification_strategy()
	{
	}


	public: size_type input_order() const
	{
		return n_b_;
	}


	public: size_type output_order() const
	{
		return n_a_;
	}


	public: size_type input_delay() const
	{
		return d_;
	}


	public: size_type num_inputs() const
	{
		return n_u_;
	}


	public: size_type num_outputs() const
	{
		return n_y_;
	}


	public: matrix_type Theta_hat() const
	{
		return do_Theta_hat();
	}


	public: matrix_type P() const
	{
		return do_P();
	}


	public: vector_type phi() const
	{
		return do_phi();
	}


	public: void init()
	{
		count_ = 0;

		do_init();
	}


	public: template <typename YVectorExprT, typename UVectorExprT>
		vector_type estimate(::boost::numeric::ublas::vector_expression<YVectorExprT> const& y,
							 ::boost::numeric::ublas::vector_expression<UVectorExprT> const& u)
	{
		++count_;

		return do_estimate(y, u);
	}


	public: matrix_type A(size_type k) const
	{
		return do_A(k);
	}


	public: matrix_type B(size_type k) const
	{
		return do_B(k);
	}


	public: size_type count() const
	{
		return count_;
	}


	private: virtual matrix_type do_Theta_hat() const = 0;


	private: virtual matrix_type do_P() const = 0;


	private: virtual vector_type do_phi() const = 0;


	private: virtual void do_init() = 0;


	private: virtual vector_type do_estimate(vector_type const& y, vector_type const& u) = 0;


	private: virtual matrix_type do_A(size_type k) const = 0;


	private: virtual matrix_type do_B(size_type k) const = 0;


	/// The memory for the control output.
	private: size_type n_a_;
	/// The memory for the control input.
	private: size_type n_b_;
	/// Input delay (dead time).
	private: size_type d_;
	/// The size of the control output vector.
	private: size_type n_y_;
	/// The size of the augmented control input vector.
	private: size_type n_u_;
	/// Count the number of times RLS has been applied
	private: size_type count_;
}; // base_system_identification_strategy


template <typename TraitsT>
class rls_system_identification_strategy: public base_system_identification_strategy<TraitsT>
{
	private: typedef base_system_identification_strategy<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename base_type::size_type size_type;


	public: rls_system_identification_strategy()
	: base_type(),
	  max_cov_heuristic_(false),
	  max_cov_heuristic_max_val_(0),
	  cond_cov_heuristic_(false),
	  cond_cov_heuristic_trust_digits_(0)
	{
	}


	public: rls_system_identification_strategy(size_type n_a, size_type n_b, size_type d, size_type n_y, size_type n_u)
	: base_type(n_a, n_b, d, n_y, n_u),
	  max_cov_heuristic_(false),
	  max_cov_heuristic_max_val_(0),
	  cond_cov_heuristic_(false),
	  cond_cov_heuristic_trust_digits_(0)
	{
	}


	public: rls_system_identification_strategy(rls_system_identification_strategy_params<traits_type> const& params)
	: base_type(params),
	  max_cov_heuristic_(false),
	  max_cov_heuristic_max_val_(0),
	  cond_cov_heuristic_(false),
	  cond_cov_heuristic_trust_digits_(0)
	{
	}


	public: void max_covariance_heuristic(bool value)
	{
		max_cov_heuristic_ = value;
	}


	public: bool max_covariance_heuristic() const
	{
		return max_cov_heuristic_;
	}


	public: void max_covariance_heuristic_max_value(real_type value)
	{
		max_cov_heuristic_max_val_ = value;
	}


	public: real_type max_covariance_heuristic_max_value() const
	{
		return max_cov_heuristic_max_val_;
	}


	public: void condition_number_covariance_heuristic(bool value)
	{
		cond_cov_heuristic_ = value;
	}


	public: bool condition_number_covariance_heuristic() const
	{
		return cond_cov_heuristic_;
	}


	public: void condition_number_covariance_heuristic_max_value(uint_type value)
	{
		cond_cov_heuristic_trust_digits_ = value;
	}


	public: uint_type condition_number_covariance_heuristic_trusted_digits() const
	{
		return cond_cov_heuristic_trust_digits_;
	}


	private: bool max_cov_heuristic_;
	private: real_type max_cov_heuristic_max_val_;
	private: bool cond_cov_heuristic_;
	private: uint_type cond_cov_heuristic_trust_digits_;
}; // rls_system_identification_strategy


#if defined(DCS_EESIM_USE_MATLAB_MCR)

/**
 * \brief Proxy to identify a MIMO system model by applying the Recursive Least
 *  Square with forgetting-factor algorithm to several MISO system models.
 *
 * MATLAB Compiler Toolbox implementation version.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class rls_ff_miso_matlab_mcr_proxy
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef ::boost::numeric::ublas::matrix<real_type> matrix_type;
	public: typedef ::boost::numeric::ublas::vector<real_type> vector_type;
	public: typedef ::std::size_t size_type;

	public: rls_ff_miso_matlab_mcr_proxy()
	: n_a_(0),
	  n_b_(0),
	  d_(0),
	  n_y_(0),
	  n_u_(0),
	  ff_(0)
	{
//		init_matlab();
	}


	public: rls_ff_miso_matlab_mcr_proxy(size_type n_a, size_type n_b, size_type d, size_type n_y, size_type n_u, real_type ff)
	: n_a_(n_a),
	  n_b_(n_b),
	  d_(d),
	  n_y_(n_y),
	  n_u_(n_u),
	  ff_(ff),
	  theta_hats_(n_y_),
	  Ps_(n_y_),
	  phis_(n_y_)
	{
//		init_matlab();
	}


	public: ~rls_ff_miso_matlab_mcr_proxy()
	{
//		finit_matlab();
	}


	public: size_type input_order() const
	{
		return n_b_;
	}


	public: size_type output_order() const
	{
		return n_a_;
	}


	public: size_type input_delay() const
	{
		return d_;
	}


	public: size_type num_inputs() const
	{
		return n_u_;
	}


	public: size_type num_outputs() const
	{
		return n_y_;
	}


	public: real_type forgetting_factor() const
	{
		return ff_;
	}


	public: matrix_type Theta_hat() const
	{
		namespace ublas = ::boost::numeric::ublas;

		const size_type nay(n_a_*n_y_);
		const size_type nbu(n_b_*n_u_);
		const size_type n(nay+nbu);
		matrix_type X(n, n_y_, real_type/*zero()*/());

		for (size_type i = 0; i < n_y_; ++i)
		{
			// ith output => ith column of Theta_hat
			// ith column of Theta_hat = [0; ...; 0; a_{ii}^{1}
			const size_type k(i*n_a_);
			ublas::matrix_column<matrix_type> mc(X,i);
			ublas::subrange(mc, k, k+n_a_) = ublas::subrange(theta_hats_[i], 0, n_a_);
			ublas::subrange(mc, nay, n) = ublas::subrange(theta_hats_[i], n_a_, n_a_+nbu);
		}

		return X;
	}


	public: matrix_type P() const
	{
		matrix_type aux_P;
//FIXME
		return aux_P;
	}


	public: vector_type phi() const
	{
		namespace ublas = ::boost::numeric::ublas;

		const size_type nay(n_a_*n_y_);
		const size_type nbu(n_b_*n_u_);
		const size_type n(nay+nbu);
		vector_type x(n);

		for (size_type i = 0; i < n_y_; ++i)
		{
			const size_type k(i*n_a_);
			ublas::subrange(x, k, k+n_a_) = ublas::subrange(phis_[i], 0, n_a_);
		}

		ublas::subrange(x, nay, n) = ublas::subrange(phis_[0], n_a_, n_a_+nbu);

		return x;
	}


	public: void init()
	{
		size_type nu_data[1];
		nu_data[0] = n_u_;

		size_type nn_sz(1+2*n_u_);
		size_type* nn_data = new size_type[nn_sz]; // [n_a_,n_b_,...,n_b_,d_,...,d_]
		nn_data[0] = n_a_;
		for (size_type i = 1; i <= n_u_; ++i)
		{
			nn_data[i] = n_b_;
			nn_data[n_u_+i] = d_;
		}

		mwArray nu(1, 1, mxINT32_CLASS, mxREAL);
		nu.SetData(nu_data, 1);
		mwArray nn(1, nn_sz, mxINT32_CLASS, mxREAL);
		nn.SetData(nn_data, nn_sz);

//		init_matlab();

            mwArray th0;
            mwArray P0;
            mwArray phi0;

		// Prepare the data structures for the RLS algorithm 
		for (size_type i = 0; i < n_y_; ++i)
		{
			//NOTE: can throw an mwException (in that case we let the caller to manage it)
			rls_miso_init(3, th0, P0, phi0, nu, nn);

			mw_to_vector(th0, theta_hats_[i]);
			mw_to_matrix(P0, Ps_[i]);
			mw_to_vector(phi0, phis_[i]);
		}

//		finit_matlab();
	}


	public: template <typename YVectorExprT, typename UVectorExprT>
		vector_type estimate(::boost::numeric::ublas::vector_expression<YVectorExprT> const& y,
							 ::boost::numeric::ublas::vector_expression<UVectorExprT> const& u)
	{
		namespace ublas = ::boost::numeric::ublas;

		// Estimate system parameters
//DCS_DEBUG_TRACE("BEGIN estimation");//XXX
//DCS_DEBUG_TRACE("y(k): " << y);//XXX
//DCS_DEBUG_TRACE("u(k): " << u);//XXX
		vector_type y_hat(n_y_);

		// Common input params
		size_type nu_data[1];
		nu_data[0] = n_u_;

		size_type nn_sz(1+2*n_u_);
		size_type* nn_data = new size_type[nn_sz]; // [n_a_,n_b_,...,n_b_,d_,...,d_]
		nn_data[0] = n_a_;
		for (size_type i = 1; i <= n_u_; i += 2)
		{
			nn_data[i] = n_b_;
			nn_data[2*i] = d_;
		}

		real_type ff_data[1];
		ff_data[0] = ff_;

		mwArray nu(1, 1, mxINT32_CLASS, mxREAL);
		nu.SetData(nu_data, 1);
		mwArray nn(1, 1+2*n_u_, mxINT32_CLASS, mxREAL);
		nn.SetData(nn_data, nn_sz);
		mwArray ff(1, 1, mxDOUBLE_CLASS, mxREAL);
		ff.SetData(ff_data, 1);

		// Partially common input params
		mwArray z;
		size_type nz(1+n_u_);
		vector_type zz(nz, real_type/*zero*/());
		ublas::subrange(zz, 1, nz) = u;
		vector_to_mw(zz, z, mxDOUBLE_CLASS, mxREAL, false);

		// The main loop
//init_matlab();
		for (size_type i = 0; i < n_y_; ++i)
		{
//DCS_DEBUG_TRACE("theta_hat["<< i << "](k): " << theta_hats_[i]);//XXX
//DCS_DEBUG_TRACE("P["<< i << "](k): " << Ps_[i]);//XXX
//DCS_DEBUG_TRACE("phi["<< i << "](k): " << phis_[i]);//XXX
			// Partially per-iteration input params
			z(1,1) = y()(i);

			// Per-iteration input params
            mwArray th0;
			vector_to_mw(theta_hats_[i], th0, mxDOUBLE_CLASS, mxREAL);
            mwArray P0;
			matrix_to_mw(Ps_[i], P0, mxDOUBLE_CLASS, mxREAL);
            mwArray phi0;
			vector_to_mw(phis_[i], phi0, mxDOUBLE_CLASS, mxREAL);

			// Output params
            mwArray thm;
            mwArray yhat;
            mwArray P;
            mwArray phi;

			//NOTE: can throw an mwException (in that case we let the caller to manage it)
			rls_miso(4, thm, yhat, P, phi, z, nn, ff, th0, P0, phi0);

			mw_to_vector(thm, theta_hats_[i]);
			mw_to_matrix(P, Ps_[i]);
			mw_to_vector(phi, phis_[i]);

			y_hat(i) = yhat(1,1);
//DCS_DEBUG_TRACE("New theta_hat["<< i << "](k): " << theta_hats_[i]);//XXX
//DCS_DEBUG_TRACE("New P["<< i << "](k): " << Ps_[i]);//XXX
//DCS_DEBUG_TRACE("New phi["<< i << "](k): " << phis_[i]);//XXX
//DCS_DEBUG_TRACE("New e["<< i << "](k): " << (y()(i)-y_hat(i)));//XXX
		}
//finit_matlab();
//DCS_DEBUG_TRACE("New y_hat(k): " << y_hat);//XXX
//DCS_DEBUG_TRACE("END estimation");//XXX

		return y_hat;
	}


	/// Return matrix A_k from \hat{\Theta}.
	public: matrix_type A(size_type k) const
	{
		namespace ublas = ::boost::numeric::ublas;

		DCS_DEBUG_ASSERT( k >= 1 && k <= n_a_ );

		// Remember, for each output i=1,...,n_y:
		//   \hat{\theta}_i = [a_{ii}^{1};
		//                     ...;
		//                     a_{ii}^{n_a};
		//                     b_{i1}^{1};
		//                     ...;
		//                     b_{i1}^{n_b};
		//                     ...;
		//                     b_{in_u}^{1};
		//                     ...;
		//                     b_{in_u}^{n_b}]
		// So in \hat{\theta}_i the ith diagonal element of matrix A_k stays at:
		//   A_k(i,i) <- \hat{\theta}_i(k)
		matrix_type A_k(n_y_, n_y_, real_type/*zero*/());
		for (size_type i = 0; i < n_y_; ++i)
		{
			A_k(i,i) = theta_hats_[i](k);
		}

		return A_k;
	}


	/// Return matrix B_k from \hat{\Theta}.
	public: matrix_type B(size_type k) const
	{
		namespace ublas = ::boost::numeric::ublas;

		DCS_DEBUG_ASSERT( k >= 1 && k <= n_b_ );

		// Remember, for each output i=1,...,n_y:
		//   \hat{\theta}_i = [a_{ii}^{1};
		//                     ...;
		//                     a_{ii}^{n_a};
		//                     b_{i1}^{1};
		//                     ...;
		//                     b_{i1}^{n_b};
		//                     ...;
		//                     b_{in_u}^{1};
		//                     ...;
		//                     b_{in_u}^{n_b}]
		// So in \hat{\theta}_i the ith row of matrix B_k stays at:
		//   B_k(i,:) <- (\hat{\theta}_i(((n_a+k):n_b:n_u))^T
		matrix_type B_k(n_y_, n_u_);
		for (size_type i = 0; i < n_y_; ++i)
		{
			ublas::row(B_k, i) = ublas::subslice(theta_hats_[i], n_a_+k-1, n_b_, n_u_);
		}

		return B_k;
	}


//	private: void init_matlab()
//	{
//::std::cerr << "[init_matlab] .1" << ::std::endl;//XXX
//		if (!librlsInitialize())
//		{
//::std::cerr << "[init_matlab] .2" << ::std::endl;//XXX
//			std::clog << "Could not initialize the MATLAB library properly." << std::endl;
//			throw ::std::runtime_error("[dcs::eesim::rls_ff_miso_matlab_proxy] Could not initialize the MATLAB library properly.");
//		}
//::std::cerr << "[init_matlab] .3" << ::std::endl;//XXX
//	}


//	private: void finit_matlab()
//	{
//::std::cerr << "[finit_matlab] .1" << ::std::endl;//XXX
//		// Call the application and library termination routine
//		librlsTerminate();
//::std::cerr << "[finit_matlab] .2" << ::std::endl;//XXX
//	}


	private: template <typename MatrixT>
		static void mw_to_matrix(mwArray const& A, MatrixT& X)
	{
		mwArray d(A.GetDimensions());

		size_type nr(d(1,1));
		size_type nc(d(1,2));

		for (size_type r = 0; r < nr; ++r)
		{
			for (size_type c = 0; c < nc; ++c)
			{
				X(r,c) = A(r+1,c+1);
			}
		}
	}


	private: template <typename MatrixT, typename ClsT, typename CplxT>
		static void matrix_to_mw(MatrixT const& A, mwArray& X, ClsT cls, CplxT cplx)
	{
		namespace ublasx = ::boost::numeric::ublasx;

		size_type nr(ublasx::num_rows(A));
		size_type nc(ublasx::num_columns(A));

		X = mwArray(nr, nc, cls, cplx);

		for (size_type r = 0; r < nr; ++r)
		{
			for (size_type c = 0; c < nc; ++c)
			{
				X(r+1,c+1) = A(r,c);
			}
		}
	}


	private: template <typename VectorT>
		static void mw_to_vector(mwArray const& A, VectorT& x)
	{
		mwArray d(A.GetDimensions());

		size_type nr(d(1,1));
		size_type nc(d(1,2));

		if (nr != 1 && nc != 1)
		{
			throw ::std::runtime_error("[dcs::eesim::lqr_application_controller::detail::rls_ff_miso_matlab_proxy::mw_to_vector] Cannot copy a matrix into a vector.");
		}

		if (nr == 1)
		{
			// a row-vector or a vector with length 1

			for (size_type i = 0; i < nc; ++i)
			{
				x(i) = A(1,i+1);
			}
		}
		else
		{
			// a column-vector
			for (size_type i = 0; i < nr; ++i)
			{
				x(i) = A(i+1,1);
			}
		}
	}


	private: template <typename VectorT, typename ClsT, typename CplxT>
		static void vector_to_mw(VectorT const& v, mwArray& X, ClsT cls, CplxT cplx, bool col = true)
	{
		namespace ublasx = ::boost::numeric::ublasx;

		size_type n(ublasx::size(v));

		if (col)
		{
			// column vector
			X = mwArray(n, 1, cls, cplx);

			for (size_type i = 0; i < n; ++i)
			{
				X(i+1,1) = v(i);
			}
		}
		else
		{
			// row vector
			X = mwArray(1, n, cls, cplx);

			for (size_type i = 0; i < n; ++i)
			{
				X(1,i+1) = v(i);
			}
		}
	}


	/// The memory for the control output.
	private: size_type n_a_;
	/// The memory for the control input.
	private: size_type n_b_;
	/// Input delay (dead time).
	private: size_type d_;
	/// The size of the control output vector.
	private: size_type n_y_;
	/// The size of the augmented control input vector.
	private: size_type n_u_;
	/// Forgetting factor.
	private: real_type ff_;
	/// Matrix of system parameters estimated by RLS: [A_1 ... A_{n_a} B_1 ... B_{n_b}].
	private: ::std::vector<vector_type> theta_hats_;
	/// The covariance matrix.
	private: ::std::vector<matrix_type> Ps_;
	/// The regression vector.
	private: ::std::vector<vector_type> phis_;
}; // rls_ff_miso_matlab_mcr_proxy


#elif defined(DCS_EESIM_USE_MATLAB_APP)


#if defined(DCS_EESIM_USE_MATLAB_APP_RLS)


/**
 * \brief Proxy to identify a MIMO system model by applying the Recursive Least
 *  Square with forgetting-factor algorithm to several MISO system models.
 *
 * MATLAB implementation version.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class rls_ff_miso_matlab_app_proxy
{
	private: typedef rls_ff_miso_matlab_app_proxy self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef ::boost::numeric::ublas::matrix<real_type> matrix_type;
	public: typedef ::boost::numeric::ublas::vector<real_type> vector_type;
	public: typedef ::std::size_t size_type;


	public: rls_ff_miso_matlab_app_proxy()
	: n_a_(0),
	  n_b_(0),
	  d_(0),
	  n_y_(0),
	  n_u_(0),
	  ff_(0)
	{
//		init_matlab();
	}


	public: rls_ff_miso_matlab_app_proxy(size_type n_a, size_type n_b, size_type d, size_type n_y, size_type n_u, real_type ff)
	: n_a_(n_a),
	  n_b_(n_b),
	  d_(d),
	  n_y_(n_y),
	  n_u_(n_u),
	  ff_(ff),
	  theta_hats_(n_y_),
	  Ps_(n_y_),
	  phis_(n_y_)
	{
//		init_matlab();
	}


	public: ~rls_ff_miso_matlab_app_proxy()
	{
//		finit_matlab();
	}


	public: size_type input_order() const
	{
		return n_b_;
	}


	public: size_type output_order() const
	{
		return n_a_;
	}


	public: size_type input_delay() const
	{
		return d_;
	}


	public: size_type num_inputs() const
	{
		return n_u_;
	}


	public: size_type num_outputs() const
	{
		return n_y_;
	}


	public: real_type forgetting_factor() const
	{
		return ff_;
	}


	public: matrix_type Theta_hat() const
	{
		namespace ublas = ::boost::numeric::ublas;

		const size_type nay(n_a_*n_y_);
		const size_type nbu(n_b_*n_u_);
		const size_type n(nay+nbu);
		matrix_type X(n, n_y_, real_type/*zero()*/());

		for (size_type i = 0; i < n_y_; ++i)
		{
			// ith output => ith column of Theta_hat
			// ith column of Theta_hat = [0; ...; 0; a_{ii}^{1}
			const size_type k(i*n_a_);
			ublas::matrix_column<matrix_type> mc(X,i);
			ublas::subrange(mc, k, k+n_a_) = ublas::subrange(theta_hats_[i], 0, n_a_);
			ublas::subrange(mc, nay, n) = ublas::subrange(theta_hats_[i], n_a_, n_a_+nbu);
		}

		return X;
	}


	public: matrix_type P() const
	{
		matrix_type aux_P;
//FIXME
		return aux_P;
	}


	public: vector_type phi() const
	{
		namespace ublas = ::boost::numeric::ublas;

		const size_type nay(n_a_*n_y_);
		const size_type nbu(n_b_*n_u_);
		const size_type n(nay+nbu);
		vector_type x(n);

		for (size_type i = 0; i < n_y_; ++i)
		{
			const size_type k(i*n_a_);
			ublas::subrange(x, k, k+n_a_) = ublas::subrange(phis_[i], 0, n_a_);
		}

		ublas::subrange(x, nay, n) = ublas::subrange(phis_[0], n_a_, n_a_+nbu);

		return x;
	}


	public: void init()
	{
		if (initialized_.size() != n_y_)
		{
			initialized_.resize(n_y_);
		}
		for (size_type i = 0; i < n_y_; ++i)
		{
			initialized_[i] = false;
		}

		matlab_cmd_ = find_matlab_command();
	}


	public: template <typename YVectorExprT, typename UVectorExprT>
		vector_type estimate(::boost::numeric::ublas::vector_expression<YVectorExprT> const& y,
							 ::boost::numeric::ublas::vector_expression<UVectorExprT> const& u)
	{
		namespace ublas = ::boost::numeric::ublas;

		// Estimate system parameters
//DCS_DEBUG_TRACE("BEGIN estimation");//XXX
//DCS_DEBUG_TRACE("y(k): " << y);//XXX
//DCS_DEBUG_TRACE("u(k): " << u);//XXX
		vector_type y_hat(n_y_);

		// Common input params
		size_type nn_sz(1+2*n_u_);
		::std::vector<size_type> nn(nn_sz); // [n_a_,n_b_,...,n_b_,d_,...,d_]
		nn[0] = n_a_;
		for (size_type i = 1; i <= n_u_; ++i)
		{
			nn[i] = n_b_;
			nn[i+n_u_] = d_;
		}

		// Partially common input params
		size_type nz(1+n_u_);
		vector_type z(nz, real_type/*zero*/()); // [y, u_1, ..., u_{n_u_}]
		ublas::subrange(z, 1, nz) = u;

		// The main loop
		for (size_type i = 0; i < n_y_; ++i)
		{
//DCS_DEBUG_TRACE("theta_hat["<< i << "](k): " << theta_hats_[i]);//XXX
//DCS_DEBUG_TRACE("P["<< i << "](k): " << Ps_[i]);//XXX
//DCS_DEBUG_TRACE("phi["<< i << "](k): " << phis_[i]);//XXX
			// Partially per-iteration input params
			z(0) = y()(i);

			// Prepare MATLAB command
			::std::vector< ::std::string > args;
			args.push_back("-nodisplay");
			args.push_back("-nojvm");
//			args.push_back("-r");
			::std::ostringstream oss;
			oss << "-r \"[th,yh,P,phi]=rarx("
				<< matlab_form(z, false)
				<< "," << matlab_form(nn, false)
				<< ",'ff'," << ff_;
			if (initialized_[i])
			{
				oss << "," << matlab_form(theta_hats_[i], true) //<< "'" // We must pass the transpose of theta_hat
					<< "," << matlab_form(Ps_[i])
					<< "," << matlab_form(phis_[i], true);
			}
			else
			{
				initialized_[i] = true;
			}
			oss << "); format long;"
				<< " disp('--- [eesim] ---');"
				<< " disp(['th=', mat2str(th), ]);"
				<< " disp(['yh=', num2str(yh), ]);"
				<< " disp(['P=', mat2str(P), ]);"
				<< " disp(['phi=', mat2str(phi), ]);"
				<< " disp('--- [/eesim] ---');"
				<< " quit force\"";
			args.push_back(oss.str());

			// Run MATLAB and retrieve its results
			run_matlab(matlab_cmd_, args, theta_hats_[i], y_hat(i), Ps_[i], phis_[i]);

DCS_DEBUG_TRACE("New theta_hat["<< i << "](k): " << theta_hats_[i]);//XXX
DCS_DEBUG_TRACE("New P["<< i << "](k): " << Ps_[i]);//XXX
DCS_DEBUG_TRACE("New phi["<< i << "](k): " << phis_[i]);//XXX
DCS_DEBUG_TRACE("New e["<< i << "](k): " << (y()(i)-y_hat(i)));//XXX
		}

		return y_hat;
	}


	/// Return matrix A_k from \hat{\Theta}.
	public: matrix_type A(size_type k) const
	{
		namespace ublas = ::boost::numeric::ublas;

		DCS_DEBUG_ASSERT( k >= 1 && k <= n_a_ );

		// Remember, for each output i=1,...,n_y:
		//   \hat{\theta}_i = [a_{ii}^{1};
		//                     ...;
		//                     a_{ii}^{n_a};
		//                     b_{i1}^{1};
		//                     ...;
		//                     b_{i1}^{n_b};
		//                     ...;
		//                     b_{in_u}^{1};
		//                     ...;
		//                     b_{in_u}^{n_b}]
		// So in \hat{\theta}_i the ith diagonal element of matrix A_k stays at:
		//   A_k(i,i) <- \hat{\theta}_i(k)
		matrix_type A_k(n_y_, n_y_, real_type/*zero*/());
		for (size_type i = 0; i < n_y_; ++i)
		{
			A_k(i,i) = theta_hats_[i](k);
		}

		return A_k;
	}


	/// Return matrix B_k from \hat{\Theta}.
	public: matrix_type B(size_type k) const
	{
		namespace ublas = ::boost::numeric::ublas;

		DCS_DEBUG_ASSERT( k >= 1 && k <= n_b_ );

		// Remember, for each output i=1,...,n_y:
		//   \hat{\theta}_i = [a_{ii}^{1};
		//                     ...;
		//                     a_{ii}^{n_a};
		//                     b_{i1}^{1};
		//                     ...;
		//                     b_{i1}^{n_b};
		//                     ...;
		//                     b_{in_u}^{1};
		//                     ...;
		//                     b_{in_u}^{n_b}]
		// So in \hat{\theta}_i the ith row of matrix B_k stays at:
		//   B_k(i,:) <- (\hat{\theta}_i(((n_a+k):n_b:n_u))^T
		matrix_type B_k(n_y_, n_u_);
		for (size_type i = 0; i < n_y_; ++i)
		{
			ublas::row(B_k, i) = ublas::subslice(theta_hats_[i], n_a_+k-1, n_b_, n_u_);
		}

		return B_k;
	}


	private: template <typename ValueT>
		static ::std::string matlab_form(::std::vector<ValueT> const& v, bool column = true)
	{
		size_type n(v.size());
		::std::ostringstream oss;
		oss << "[";
		for (size_type i = 0; i < n; ++i)
		{
			if (i > 0)
			{
				if (column)
				{
					oss << ";";
				}
				else
				{
					oss << " ";
				}
			}
			oss << v[i];
		}
		oss << "]";

		return oss.str();
	}


	private: template <typename VectorExprT>
		static ::std::string matlab_form(::boost::numeric::ublas::vector_expression<VectorExprT> const& v, bool column = true)
	{
		namespace ublasx = ::boost::numeric::ublasx;

		size_type n(ublasx::size(v));
		::std::ostringstream oss;
		oss << "[";
		for (size_type i = 0; i < n; ++i)
		{
			if (i > 0)
			{
				if (column)
				{
					oss << ";";
				}
				else
				{
					oss << " ";
				}
			}
			oss << v()(i);
		}
		oss << "]";

		return oss.str();
	}


	private: template <typename MatrixExprT>
		static ::std::string matlab_form(::boost::numeric::ublas::matrix_expression<MatrixExprT> const& A)
	{
		namespace ublasx = ::boost::numeric::ublasx;

		size_type nr(ublasx::num_rows(A));
		size_type nc(ublasx::num_columns(A));
		::std::ostringstream oss;
		oss << "[";
		for (size_type r = 0; r < nr; ++r)
		{
			if (r > 0)
			{
				oss << ";";
			}
			for (size_type c = 0; c < nc; ++c)
			{
				if (c > 0)
				{
					oss << " ";
				}
				oss << A()(r,c);
			}
		}
		oss << "]";

		return oss.str();
	}


	private: static ::std::string find_matlab_command()
	{
		const ::std::string cmd_name("matlab");

/*XXX: this search is now useless since we use "execvp" in place of "execv".
		// Get paths from environment
		char* env_paths(::getenv("PATH"));
		char* cwd(0);
		if (!env_paths)
		{
			::std::size_t cwd_sz(20);
			char* buf(0);
			do
			{
				if (buf)
				{
					delete[] buf;
				}
				buf = new char[cwd_sz];
				cwd = ::getcwd(buf, cwd_sz);
			}
			while (!cwd && errno == ERANGE);

			if (cwd)
			{
				env_paths = cwd;
			}
			else
			{
				// Ooops! Unable to get path info.
				throw ::std::runtime_error("[dcs::eesim::detail::rls_miso_matlab_app_proxy] Unable to get path information.");
			}
		}

		::std::string cmd_path;
		::std::string paths(env_paths);
		typename ::std::string::size_type pos1(0);
		typename ::std::string::size_type pos2(0);
		bool found(false);
		do
		{
			pos2 = paths.find(':', pos1);
			::std::string dir = (pos2 != ::std::string::npos)
								? paths.substr(pos1, pos2-pos1)
								: paths.substr(pos1);
			cmd_path = ::std::string(dir + "/" + cmd_name);
			if (::access(cmd_path.c_str(), X_OK))
			{
				pos1 = pos2 + 1;
			}
			else
			{
				found = true;
			}
		}
		while (pos2 != ::std::string::npos && !found);

		if (!found)
		{
			throw ::std::runtime_error("[dcs::eesim::detail::rls_miso_matlab_app::find_matlab_command] MATLAB not found.");
		}

		if (cwd)
		{
			delete[] cwd;
		}

		return cmd_path;
*/
		return cmd_name;
	}


	private: template <typename ArgsT>
		static void run_matlab(::std::string const& cmd, ArgsT const& args, vector_type& th, real_type& yh, matrix_type& P, vector_type& phi)
	{
		int pipefd[2];

		// Create a pipe to let to communicate with MATLAB.
		// Specifically, we want to read the output from MATLAB.
		// So, the parent process read from the pipe, while the child process
		// write on it.
		if (::pipe(pipefd) == -1)
		{
			char const* err_str = ::strerror(errno);
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::rls_miso_matlab_app_rpoxy::run_matlab] pipe(2) failed: "
				<< ::std::string(err_str);
			throw ::std::runtime_error(oss.str());
		}

		// Install signal handlers
		struct ::sigaction sig_act;
		struct ::sigaction old_sigterm_act;
		struct ::sigaction old_sigint_act;
		//::memset(&sig_act, 0, sizeof(sig_act));
		::sigemptyset(&sig_act.sa_mask);
		sig_act.sa_flags = 0;
		sig_act.sa_handler = self_type::process_signals;
		::sigaction(SIGTERM, &sig_act, &old_sigterm_act);
		::sigaction(SIGINT, &sig_act, &old_sigint_act);

		// Spawn a new process

		// Between fork() and execve() only async-signal-safe functions
		// must be called if multithreaded applications should be supported.
		// That's why the following code is executed before fork() is called.

		::pid_t pid = ::fork();

		// check: pid == -1 --> error
		if (pid == -1)
		{
			char const* err_str = ::strerror(errno);
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::rls_miso_matlab_app_rpoxy::run_matlab] fork(2) failed: "
				<< ::std::string(err_str);
			throw ::std::runtime_error(oss.str());
		}

		if (pid == 0)
		{
			// The child

			// Cancel signal handler set for parent
			sig_act.sa_handler = SIG_DFL;
			::sigaction(SIGTERM, &sig_act, 0);
			::sigaction(SIGINT, &sig_act, 0);

			// Get the maximum number of files this process is allowed to open
#if defined(F_MAXFD)
    		int maxdescs = ::fcntl(-1, F_MAXFD, 0);
    		if (maxdescs == -1)
			{
#if defined(_SC_OPEN_MAX)
        		maxdescs = ::sysconf(_SC_OPEN_MAX);
#else
				::rlimit limit;
				if (::getrlimit(RLIMIT_NOFILE, &limit) < 0)
				{
					char const* err_str = ::strerror(errno);
					::std::ostringstream oss;
					oss << "[dcs::eesim::detail::rls_miso_matlab_app_rpoxy::run_matlab] getrlimit(2) failed: "
						<< ::std::string(err_str);
					throw ::std::runtime_error(oss.str());
				}
				maxdescs = limit.rlim_cur;
#endif // _SC_OPEN_MAX
			}
#else // F_MAXFD
#if defined(_SC_OPEN_MAX)
    		int maxdescs = ::sysconf(_SC_OPEN_MAX);
#else // _SC_OPEN_MAX
			::rlimit limit;
			if (::getrlimit(RLIMIT_NOFILE, &limit) < 0)
			{
				char const* err_str = ::strerror(errno);
				::std::ostringstream oss;
				oss << "[dcs::eesim::detail::rls_miso_matlab_app_rpoxy::run_matlab] getrlimit(2) failed: "
					<< ::std::string(err_str);
				throw ::std::runtime_error(oss.str());
			}
			maxdescs = limit.rlim_cur;
#endif // _SC_OPEN_MAX
#endif // F_MAXFD
			if (maxdescs == -1)
			{
				maxdescs = 1024;
			}

			::std::vector<bool> close_fd(maxdescs, true);

			// Associate the child's stdout to the pipe write fd.
			close_fd[STDOUT_FILENO] = false;
			if (pipefd[1] != STDOUT_FILENO)
			{
				if (::dup2(pipefd[1], STDOUT_FILENO) != STDOUT_FILENO)
				{
					char const* err_str = ::strerror(errno);
					::std::ostringstream oss;
					oss << "[dcs::eesim::detail::rls_miso_matlab_app_rpoxy::run_matlab] dup2(2) failed: "
						<< ::std::string(err_str);
					throw ::std::runtime_error(oss.str());
				}
			}
			else
			{
				close_fd[pipefd[1]] = false;
			}
//			::close(STDOUT_FILENO);
//			::dup(pipefd[1]);

			// Check if the command already has path information
			::std::string cmd_path;
			::std::string cmd_name;
			typename ::std::string::size_type pos;
			pos = cmd.find_last_of('/');
			if (pos != ::std::string::npos)
			{
				cmd_path = cmd.substr(0, pos);
				cmd_name = cmd.substr(pos+1);
			}

			//FIXME: use scoped_ptr in place of "new"

			::std::size_t nargs = args.size()+1;
			char** argv = new char*[nargs + 2];
			argv[0] = new char[cmd_name.size()+1];
			::std::strncpy(argv[0], cmd_name.c_str(), cmd_name.size()+1); // by convention, the first argument is always the command name
			typename ArgsT::size_type i(1);
			typename ArgsT::const_iterator end_it(args.end());
			for (typename ArgsT::const_iterator it = args.begin(); it != end_it; ++it)
			{
				argv[i] = new char[it->size()+1];
				::std::strncpy(argv[i], it->c_str(), it->size()+1);
				++i;
			}
			argv[nargs] = 0;

			//char** envp(0);

			// Close unused file descriptors
#ifdef DCS_DEBUG
			// Keep standard error open for debug
			close_fd[STDERR_FILENO] = false;
#endif // DCS_DEBUG
			for (int fd = 0; fd < maxdescs; ++fd)
			{
				if (close_fd[fd])
				{
					::close(fd);
				}
			}

//[XXX]
#ifdef DCS_DEBUG
::std::cerr << "Executing MATLAB: " << cmd;//XXX
for (::std::size_t i=0; i < args.size(); ++i)//XXX
{//XXX
::std::cerr << " " << args[i] << ::std::flush;//XXX
}//XXX
::std::cerr << ::std::endl;//XXX
#endif // DCS_DEBUG
//[/XXX]
//DCS_DEBUG_TRACE("Executing: " << cmd << " " << args[0] << " " << args[1] << " " << args[2] << " - " << args[3]);

			//::execve(cmd.c_str(), argv, envp);
			::execvp(cmd.c_str(), argv);

			// Actually we should delete argv and envp data. As we must not
			// call any non-async-signal-safe functions though we simply exit.
			::write(STDERR_FILENO, "execvp() failed\n", 17);
			//_exit(EXIT_FAILURE);
			_exit(127);
		}

		// The parent

//		// Associate the parent's stdin to the pipe read fd.
		::close(pipefd[1]);
//		::close(STDIN_FILENO);
//		::dup(pipefd[0]);
		if (pipefd[0] != STDIN_FILENO)
		{
			if (::dup2(pipefd[0], STDIN_FILENO) != STDIN_FILENO)
			{
				char const* err_str = ::strerror(errno);
				::std::ostringstream oss;
				oss << "[dcs::eesim::detail::rls_miso_matlab_app_rpoxy::run_matlab] dup2(2) failed: "
					<< ::std::string(err_str);
				throw ::std::runtime_error(oss.str());
			}
			::close(pipefd[0]);
		}

		typedef ::boost::iostreams::file_descriptor_source fd_device_type;
		typedef ::boost::iostreams::stream_buffer<fd_device_type> fd_streambuf_type;
		//fd_device_type fd_src(pipefd[0], ::boost::iostreams::close_handle);
		fd_device_type fd_src(STDIN_FILENO, ::boost::iostreams::close_handle);
		fd_streambuf_type fd_buf(fd_src);
		::std::istream is(&fd_buf);

		// Read from the stdin
DCS_DEBUG_TRACE("BEGIN parsing MATLAB output");//XXX
		bool parse_line(false);
		while (is.good())
		{
			::std::string line;
			::std::getline(is, line);
DCS_DEBUG_TRACE("Read from MATLAB --> " << line);//XXX

			if (parse_line)
			{
				if (line.find("[/eesim]") != ::std::string::npos)
				{
					// The end of parsable lines
					parse_line = false;
				}
				else
				{
					typename ::std::string::size_type pos;
					if ((pos = line.find("th=")) != ::std::string::npos)
					{
						parse_matlab_data(line.substr(pos+3), th);
DCS_DEBUG_TRACE("Parsed as th=" << th);//XXX
					}
					else if ((pos = line.find("yh=")) != ::std::string::npos)
					{
						parse_matlab_data(line.substr(pos+3), yh);
DCS_DEBUG_TRACE("Parsed as yh=" << yh);//XXX
					}
					else if ((pos = line.find("P=")) != ::std::string::npos)
					{
						parse_matlab_data(line.substr(pos+2), P);
DCS_DEBUG_TRACE("Parsed as P=" << P);//XXX
					}
					else if ((pos = line.find("phi=")) != ::std::string::npos)
					{
						parse_matlab_data(line.substr(pos+4), phi);
DCS_DEBUG_TRACE("Parsed as phi=" << phi);//XXX

						// Actually the RARX function of the current System
						// Identification Toolbox (MATLAB 2010b) returns a
						// \phi vector with one additional and useless entry
						// placed at the end of the vector.
						// Remove it.

						//TODO: check for current MATLAB version and issue a
						//      warning if the versio is newer than 2010b.

						phi = ::boost::numeric::ublas::subrange(phi, 0, ::boost::numeric::ublasx::size(phi)-1);
					}
				}
			}
			else
			{
				if (line.find("[eesim]") != ::std::string::npos)
				{
					// The beginning of parsable lines
					parse_line = true;
				}
			}
		}
DCS_DEBUG_TRACE("END parsing MATLAB output");//XXX
DCS_DEBUG_TRACE("IS state: " << is.good() << " - " << is.eof() << " - " << is.fail() << " - " << is.bad());//XXX

		// Wait the child termination (in order to prevent zombies)
		int status;
//		::pid_t wait_pid;
//		wait_pid = ::wait(&status);
//		if (wait_pid != pid)
//		{
//			throw ::std::runtime_error("[dcs::eesim::detail::rls_miso_matlab_app_proxy::run_matlab] Unexpected child process.");
//		}
		if (::waitpid(pid, &status, 0) == -1)
		{
			char const* err_str = ::strerror(errno);
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::rls_miso_matlab_app_rpoxy::run_matlab] waitpid(2) failed: "
				<< ::std::string(err_str);
			throw ::std::runtime_error(oss.str());
		}
DCS_DEBUG_TRACE("MATLAB exited");//XXX
		if (WIFEXITED(status))
		{
DCS_DEBUG_TRACE("MATLAB exited with a call to 'exit(" << WEXITSTATUS(status) << ")'");//XXX
			if (WEXITSTATUS(status))
			{
				// status != 0 --> error in the execution
				::std::clog << "[Warning] MATLAB command exited with status " << WEXITSTATUS(status) << ::std::endl;
			}
		}
		else if (WIFSIGNALED(status))
		{
DCS_DEBUG_TRACE("MATLAB exited with a call to 'kill(" << WTERMSIG(status) << ")'");//XXX
		   ::std::clog << "[Warning] MATLAB command received signal " << WTERMSIG(status) << ::std::endl;
		}
		else
		{
DCS_DEBUG_TRACE("MATLAB exited with an unexpected way");//XXX
		}

		// Restore signal handler
		::sigaction(SIGTERM, &old_sigterm_act, 0);
		::sigaction(SIGINT, &old_sigint_act, 0);
	}


	private: template <typename T>
		static void parse_matlab_data(::std::string const& text, T& x)
	{
		::std::istringstream iss(text);
		while (iss.good())
		{
			char ch(iss.peek());

			if (::std::isspace(ch))
			{
				// Skip space
				iss.get();
			}
			else if (::std::isdigit(ch) || ch == '+' || ch == '-' || ch == '.')
			{
				// Found the beginning of a number

				iss >> x;
			}
			else
			{
				throw ::std::runtime_error("[dcs::eesim::detail::rls_miso_matlab_app_proxy] Unable to parse a MATLAB number");
			}
		}
	}


	private: template <typename T>
		static void parse_matlab_data(::std::string const& text, ::boost::numeric::ublas::vector<T>& v)
	{
		typename ::boost::numeric::ublas::vector<T>::size_type n(0);

		::std::istringstream iss(text);
		bool inside(false);
		bool done(false);
		while (iss.good() && !done)
		{
			char ch(iss.peek());
			bool ko(false);

			if (inside)
			{
				if (::std::isspace(ch) || ch == ';')
				{
					// Found an element separator
					iss.get();
//					while (iss.good() && (ch = iss.peek()) && ::std::isspace(ch))
//					{
//						iss.get();
//					}
				}
				else if (ch == ']')
				{
					// Found the end of the vector
//					iss.get();
//					inside = false;
					done = true;
				}
				else if (::std::isdigit(ch) || ch == '+' || ch == '-' || ch == '.')
				{
					// Found the beginning of a number
					T x;
					iss >> x;
					v.resize(n+1, true);
					v(n) = x;
					++n;
				}
				else
				{
					ko = true;
				}
			}
			else
			{
				if (ch == '[')
				{
					iss.get();
					v.resize(0, false);
					inside = true;
				}
				else if (::std::isspace(ch))
				{
					iss.get();
				}
				else
				{
					ko = true;
				}
			}

			if (ko)
			{
				throw ::std::runtime_error("[dcs::eesim::detail::rls_miso_matlab_app_proxy] Unable to parse a MATLAB vector.");
			}
		}
	}


	private: template <typename T>
		static void parse_matlab_data(::std::string const& text, ::boost::numeric::ublas::matrix<T>& A)
	{
		typename ::boost::numeric::ublas::matrix<T>::size_type r(0);
		typename ::boost::numeric::ublas::matrix<T>::size_type c(0);
		typename ::boost::numeric::ublas::matrix<T>::size_type nc(0);

		::std::istringstream iss(text);
		bool inside(false);
		bool done(false);
		while (iss.good() && !done)
		{
			char ch(iss.peek());
			bool ko(false);

			if (inside)
			{
				if (::std::isspace(ch))
				{
					// Found a column separator
					iss.get();
//					while (iss.good() && (ch = iss.peek()) && ::std::isspace(ch))
//					{
//						iss.get();
//					}
				}
 				else if (ch == ';')
				{
					// Found a row separator
					iss.get();
					++r;
					c = 0;
				}
				else if (ch == ']')
				{
					// Found the end of the matrix
//					iss.get();
//					inside = false;
					done = true;
				}
				else if (::std::isdigit(ch) || ch == '+' || ch == '-' || ch == '.')
				{
					// Found the beginning of a number
					T x;
					iss >> x;
					if (nc <= c)
					{
						nc = c+1;
					}
					A.resize(r+1, nc, true);
					A(r,c) = x;
					++c;
				}
				else
				{
					ko = true;
				}
			}
			else
			{
				// Note: outside of a matrix, only two types of character are
				// allowed: spaces and '['

				if (ch == '[')
				{
					iss.get();
					A.resize(0, 0, false);
					inside = true;
				}
				else if (::std::isspace(ch))
				{
					iss.get();
				}
				else
				{
					ko = true;
				}
			}

			if (ko)
			{
				throw ::std::runtime_error("[dcs::eesim::detail::rls_miso_matlab_app_proxy] Unable to parse a MATLAB matrix.");
			}
		}
	}


	private: static void process_signals(int signum)
	{
		::std::cerr << "Caught signal " << signum << ::std::endl;

		if (signum != SIGTERM && signum != SIGINT)
		{
			::std::clog << "[Warning] Caught the unhandled signal " << signum << ::std::endl;
		}

		// Terminate all child processes.
		// Specifically, kill every process in the process group of the calling
		// process
		if (::kill(0, SIGKILL) == -1)
		{
			char const* err_str = ::strerror(errno);
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::rls_miso_matlab_app_rpoxy::process_signals] kill(2) failed: "
				<< ::std::string(err_str);
			throw ::std::runtime_error(oss.str());
		}
		::std::abort();
	}


	/// The memory for the control output.
	private: size_type n_a_;
	/// The memory for the control input.
	private: size_type n_b_;
	/// Input delay (dead time).
	private: size_type d_;
	/// The size of the control output vector.
	private: size_type n_y_;
	/// The size of the augmented control input vector.
	private: size_type n_u_;
	/// Forgetting factor.
	private: real_type ff_;
	/// Matrix of system parameters estimated by RLS: [A_1 ... A_{n_a} B_1 ... B_{n_b}].
	private: ::std::vector<vector_type> theta_hats_;
	/// The covariance matrix.
	private: ::std::vector<matrix_type> Ps_;
	/// The regression vector.
	private: ::std::vector<vector_type> phis_;
	/// Initialization flags.
	private: ::std::vector<bool> initialized_;
	private: ::std::string matlab_cmd_;
}; // rls_ff_miso_matlab_app_proxy


#elif defined(DCS_EESIM_USE_MATLAB_APP_RPEM)


/**
 * \brief Proxy to identify a MIMO system model by applying the recursive
 *  Prediction-Error Minimization (PEM) with forgetting factor algorithm to
 *  several MISO system models.
 *
 * MATLAB implementation version.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class rpem_ff_miso_matlab_app_proxy
{
	private: typedef rpem_ff_miso_matlab_app_proxy self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef ::boost::numeric::ublas::matrix<real_type> matrix_type;
	public: typedef ::boost::numeric::ublas::vector<real_type> vector_type;
	public: typedef ::std::size_t size_type;


	public: rpem_ff_miso_matlab_app_proxy()
	: n_a_(0),
	  n_b_(0),
	  n_c_(0),
	  d_(0),
	  n_y_(0),
	  n_u_(0),
	  ff_(0)
	{
//		init_matlab();
	}


	public: rpem_ff_miso_matlab_app_proxy(size_type n_a, size_type n_b, size_type n_c, size_type d, size_type n_y, size_type n_u, real_type ff)
	: n_a_(n_a),
	  n_b_(n_b),
	  n_c_(n_c),
	  d_(d),
	  n_y_(n_y),
	  n_u_(n_u),
	  ff_(ff),
	  theta_hats_(n_y_),
	  Ps_(n_y_),
	  phis_(n_y_),
	  psis_(n_y_)
	{
	}


	public: size_type noise_order() const
	{
		return n_c_;
	}


	public: size_type input_order() const
	{
		return n_b_;
	}


	public: size_type output_order() const
	{
		return n_a_;
	}


	public: size_type input_delay() const
	{
		return d_;
	}


	public: size_type num_inputs() const
	{
		return n_u_;
	}


	public: size_type num_outputs() const
	{
		return n_y_;
	}


	public: real_type forgetting_factor() const
	{
		return ff_;
	}


	public: matrix_type Theta_hat() const
	{
		namespace ublas = ::boost::numeric::ublas;

		const size_type nay(n_a_*n_y_);
		const size_type nbu(n_b_*n_u_);
		const size_type nce(n_c_);
		const size_type n(nay+nbu+nce);
		matrix_type X(n, n_y_, real_type/*zero()*/());

		for (size_type i = 0; i < n_y_; ++i)
		{
			// ith output => ith column of Theta_hat
			// ith column of Theta_hat = [0; ...; 0; a_{ii}^{1}
			const size_type k(i*n_a_);
			ublas::matrix_column<matrix_type> mc(X,i);
			ublas::subrange(mc, k, k+n_a_) = ublas::subrange(theta_hats_[i], 0, n_a_);
			ublas::subrange(mc, nay, nay+nbu) = ublas::subrange(theta_hats_[i], n_a_, n_a_+nbu);
			ublas::subrange(mc, nay+nbu, n) = ublas::subrange(theta_hats_[i], n_a_+nbu, n_a_+nbu+nce);
		}

		return X;
	}


	public: matrix_type P() const
	{
		matrix_type aux_P;
//FIXME
		return aux_P;
	}


	public: vector_type phi() const
	{
		namespace ublas = ::boost::numeric::ublas;

		const size_type nay(n_a_*n_y_);
		const size_type nbu(n_b_*n_u_);
		const size_type n(nay+nbu);
		vector_type x(n);

		for (size_type i = 0; i < n_y_; ++i)
		{
			const size_type k(i*n_a_);
			ublas::subrange(x, k, k+n_a_) = ublas::subrange(phis_[i], 0, n_a_);
		}

		ublas::subrange(x, nay, n) = ublas::subrange(phis_[0], n_a_, n_a_+nbu);

		return x;
	}


	public: vector_type psi() const
	{
		namespace ublas = ::boost::numeric::ublas;

		const size_type nay(n_a_*n_y_);
		const size_type nbu(n_b_*n_u_);
		const size_type n(nay+nbu);
		vector_type x(n);

		for (size_type i = 0; i < n_y_; ++i)
		{
			const size_type k(i*n_a_);
			ublas::subrange(x, k, k+n_a_) = ublas::subrange(psis_[i], 0, n_a_);
		}

		ublas::subrange(x, nay, n) = ublas::subrange(psis_[0], n_a_, n_a_+nbu);

		return x;
	}


	public: void init()
	{
		if (initialized_.size() != n_y_)
		{
			initialized_.resize(n_y_);
		}
		for (size_type i = 0; i < n_y_; ++i)
		{
			initialized_[i] = false;
		}

		matlab_cmd_ = find_matlab_command();
	}


	public: template <typename YVectorExprT, typename UVectorExprT>
		vector_type estimate(::boost::numeric::ublas::vector_expression<YVectorExprT> const& y,
							 ::boost::numeric::ublas::vector_expression<UVectorExprT> const& u)
	{
		namespace ublas = ::boost::numeric::ublas;

		// Estimate system parameters
//DCS_DEBUG_TRACE("BEGIN estimation");//XXX
//DCS_DEBUG_TRACE("y(k): " << y);//XXX
//DCS_DEBUG_TRACE("u(k): " << u);//XXX
		vector_type y_hat(n_y_);

		// Common input params
		// Orders vector:
		//   nn = [na nb nc nd nf nk]
		size_type nn_sz(3+3*n_u_);
		::std::vector<size_type> nn(nn_sz); // [n_a_,n_b_,...,n_b_,n_c_,0,0,...0,d_,...,d_]
		nn[0] = n_a_; // na
		nn[1+n_u_] = n_c_; // nc
		nn[2+n_u_] = 0; // nd
		for (size_type i = 1; i <= n_u_; ++i)
		{
			nn[i] = n_b_; // nb
			nn[i+2+n_u_] = 0; // nf
			nn[i+2+2*n_u_] = d_; // nk
		}

		// Partially common input params
		size_type nz(1+n_u_);
		vector_type z(nz, real_type/*zero*/()); // [y, u_1, ..., u_{n_u_}]
		ublas::subrange(z, 1, nz) = u;

		// The main loop
		for (size_type i = 0; i < n_y_; ++i)
		{
//DCS_DEBUG_TRACE("theta_hat["<< i << "](k): " << theta_hats_[i]);//XXX
//DCS_DEBUG_TRACE("P["<< i << "](k): " << Ps_[i]);//XXX
//DCS_DEBUG_TRACE("phi["<< i << "](k): " << phis_[i]);//XXX
			// Partially per-iteration input params
			z(0) = y()(i);

			// Prepare MATLAB command
			::std::vector< ::std::string > args;
			args.push_back("-nodisplay");
			args.push_back("-nojvm");
//			args.push_back("-r");
			::std::ostringstream oss;
			oss << "-r \"[th,yh,P,phi,psi]=rpem("
				<< matlab_form(z, false)
				<< "," << matlab_form(nn, false)
				<< ",'ff'," << ff_;
			if (initialized_[i])
			{
				oss << "," << matlab_form(theta_hats_[i], true) //<< "'" // We must pass the transpose of theta_hat
					<< "," << matlab_form(Ps_[i])
					<< "," << matlab_form(phis_[i], true)
					<< "," << matlab_form(psis_[i], true);
			}
			else
			{
				initialized_[i] = true;
			}
			oss << "); format long;"
				<< " disp('--- [eesim] ---');"
				<< " disp(['th=', mat2str(th), ]);"
				<< " disp(['yh=', num2str(yh), ]);"
				<< " disp(['P=', mat2str(P), ]);"
				<< " disp(['phi=', mat2str(phi), ]);"
				<< " disp(['psi=', mat2str(psi), ]);"
				<< " disp('--- [/eesim] ---');"
				<< " quit force\"";
			args.push_back(oss.str());

			// Run MATLAB and retrieve its results
			run_matlab(matlab_cmd_, args, theta_hats_[i], y_hat(i), Ps_[i], phis_[i], psis_[i]);

DCS_DEBUG_TRACE("New theta_hat["<< i << "](k): " << theta_hats_[i]);//XXX
DCS_DEBUG_TRACE("New P["<< i << "](k): " << Ps_[i]);//XXX
DCS_DEBUG_TRACE("New phi["<< i << "](k): " << phis_[i]);//XXX
DCS_DEBUG_TRACE("New psi["<< i << "](k): " << psis_[i]);//XXX
DCS_DEBUG_TRACE("New e["<< i << "](k): " << (y()(i)-y_hat(i)));//XXX
		}

		return y_hat;
	}


	/// Return matrix A_k from \hat{\Theta}.
	public: matrix_type A(size_type k) const
	{
		namespace ublas = ::boost::numeric::ublas;

		DCS_DEBUG_ASSERT( k >= 1 && k <= n_a_ );

		// Remember, for each output i=1,...,n_y:
		//   \hat{\theta}_i = [a_{ii}^{1};
		//                     ...;
		//                     a_{ii}^{n_a};
		//                     b_{i1}^{1};
		//                     ...;
		//                     b_{i1}^{n_b};
		//                     ...;
		//                     b_{in_u}^{1};
		//                     ...;
		//                     b_{in_u}^{n_b}]
		// So in \hat{\theta}_i the ith diagonal element of matrix A_k stays at:
		//   A_k(i,i) <- \hat{\theta}_i(k)
		matrix_type A_k(n_y_, n_y_, real_type/*zero*/());
		for (size_type i = 0; i < n_y_; ++i)
		{
			A_k(i,i) = theta_hats_[i](k);
		}

		return A_k;
	}


	/// Return matrix B_k from \hat{\Theta}.
	public: matrix_type B(size_type k) const
	{
		namespace ublas = ::boost::numeric::ublas;

		DCS_DEBUG_ASSERT( k >= 1 && k <= n_b_ );

		// Remember, for each output i=1,...,n_y:
		//   \hat{\theta}_i = [a_{ii}^{1};
		//                     ...;
		//                     a_{ii}^{n_a};
		//                     b_{i1}^{1};
		//                     ...;
		//                     b_{i1}^{n_b};
		//                     ...;
		//                     b_{in_u}^{1};
		//                     ...;
		//                     b_{in_u}^{n_b}]
		// So in \hat{\theta}_i the ith row of matrix B_k stays at:
		//   B_k(i,:) <- (\hat{\theta}_i(((n_a+k):n_b:n_u))^T
		matrix_type B_k(n_y_, n_u_);
		for (size_type i = 0; i < n_y_; ++i)
		{
			ublas::row(B_k, i) = ublas::subslice(theta_hats_[i], n_a_+k-1, n_b_, n_u_);
		}

		return B_k;
	}


	/// Return matrix C_k from \hat{\Theta}.
	public: matrix_type C(size_type k) const
	{
		namespace ublas = ::boost::numeric::ublas;

		DCS_DEBUG_ASSERT( k >= 1 && k <= n_c_ );

		// Remember, for each output i=1,...,n_y:
		//   \hat{\theta}_i = [a_{ii}^{1};
		//                     ...;
		//                     a_{ii}^{n_a};
		//                     b_{i1}^{1};
		//                     ...;
		//                     b_{i1}^{n_b};
		//                     ...;
		//                     b_{in_u}^{1};
		//                     ...;
		//                     b_{in_u}^{n_b};
		//                     c_{i1}^{1};
		//                     ...;
		//                     c_{i1}^{n_c};
		//                     ...;
		//                     c_{in_y}^{1};
		//                     ...;
		//                     c_{in_y}^{n_c}]
		// So in \hat{\theta}_i the ith row of matrix C_k stays at:
		//   C_k(i,:) <- (\hat{\theta}_i(((n_a+k):n_b:n_u))^T
		matrix_type C_k(n_y_, n_y_);
		for (size_type i = 0; i < n_y_; ++i)
		{
			ublas::row(C_k, i) = ublas::subslice(theta_hats_[i], n_a_+n_b_*n_u_+k-1, n_c_, n_y_);
		}

		return C_k;
	}


	private: template <typename ValueT>
		static ::std::string matlab_form(::std::vector<ValueT> const& v, bool column = true)
	{
		size_type n(v.size());
		::std::ostringstream oss;
		oss << "[";
		for (size_type i = 0; i < n; ++i)
		{
			if (i > 0)
			{
				if (column)
				{
					oss << ";";
				}
				else
				{
					oss << " ";
				}
			}
			oss << v[i];
		}
		oss << "]";

		return oss.str();
	}


	private: template <typename VectorExprT>
		static ::std::string matlab_form(::boost::numeric::ublas::vector_expression<VectorExprT> const& v, bool column = true)
	{
		namespace ublasx = ::boost::numeric::ublasx;

		size_type n(ublasx::size(v));
		::std::ostringstream oss;
		oss << "[";
		for (size_type i = 0; i < n; ++i)
		{
			if (i > 0)
			{
				if (column)
				{
					oss << ";";
				}
				else
				{
					oss << " ";
				}
			}
			oss << v()(i);
		}
		oss << "]";

		return oss.str();
	}


	private: template <typename MatrixExprT>
		static ::std::string matlab_form(::boost::numeric::ublas::matrix_expression<MatrixExprT> const& A)
	{
		namespace ublasx = ::boost::numeric::ublasx;

		size_type nr(ublasx::num_rows(A));
		size_type nc(ublasx::num_columns(A));
		::std::ostringstream oss;
		oss << "[";
		for (size_type r = 0; r < nr; ++r)
		{
			if (r > 0)
			{
				oss << ";";
			}
			for (size_type c = 0; c < nc; ++c)
			{
				if (c > 0)
				{
					oss << " ";
				}
				oss << A()(r,c);
			}
		}
		oss << "]";

		return oss.str();
	}


	private: static ::std::string find_matlab_command()
	{
		const ::std::string cmd_name("matlab");

/*XXX: this search is now useless since we use "execvp" in place of "execv".
		// Get paths from environment
		char* env_paths(::getenv("PATH"));
		char* cwd(0);
		if (!env_paths)
		{
			::std::size_t cwd_sz(20);
			char* buf(0);
			do
			{
				if (buf)
				{
					delete[] buf;
				}
				buf = new char[cwd_sz];
				cwd = ::getcwd(buf, cwd_sz);
			}
			while (!cwd && errno == ERANGE);

			if (cwd)
			{
				env_paths = cwd;
			}
			else
			{
				// Ooops! Unable to get path info.
				throw ::std::runtime_error("[dcs::eesim::detail::rls_miso_matlab_app_proxy] Unable to get path information.");
			}
		}

		::std::string cmd_path;
		::std::string paths(env_paths);
		typename ::std::string::size_type pos1(0);
		typename ::std::string::size_type pos2(0);
		bool found(false);
		do
		{
			pos2 = paths.find(':', pos1);
			::std::string dir = (pos2 != ::std::string::npos)
								? paths.substr(pos1, pos2-pos1)
								: paths.substr(pos1);
			cmd_path = ::std::string(dir + "/" + cmd_name);
			if (::access(cmd_path.c_str(), X_OK))
			{
				pos1 = pos2 + 1;
			}
			else
			{
				found = true;
			}
		}
		while (pos2 != ::std::string::npos && !found);

		if (!found)
		{
			throw ::std::runtime_error("[dcs::eesim::detail::rls_miso_matlab_app::find_matlab_command] MATLAB not found.");
		}

		if (cwd)
		{
			delete[] cwd;
		}

		return cmd_path;
*/
		return cmd_name;
	}


	private: template <typename ArgsT>
		static void run_matlab(::std::string const& cmd, ArgsT const& args, vector_type& th, real_type& yh, matrix_type& P, vector_type& phi, vector_type& psi)
	{
		int pipefd[2];

		// Create a pipe to let to communicate with MATLAB.
		// Specifically, we want to read the output from MATLAB.
		// So, the parent process read from the pipe, while the child process
		// write on it.
		if (::pipe(pipefd) == -1)
		{
			char const* err_str = ::strerror(errno);
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::rls_miso_matlab_app_rpoxy::run_matlab] pipe(2) failed: "
				<< ::std::string(err_str);
			throw ::std::runtime_error(oss.str());
		}

		// Install signal handlers
		struct ::sigaction sig_act;
		struct ::sigaction old_sigterm_act;
		struct ::sigaction old_sigint_act;
		//::memset(&sig_act, 0, sizeof(sig_act));
		::sigemptyset(&sig_act.sa_mask);
		sig_act.sa_flags = 0;
		sig_act.sa_handler = self_type::process_signals;
		::sigaction(SIGTERM, &sig_act, &old_sigterm_act);
		::sigaction(SIGINT, &sig_act, &old_sigint_act);

		// Spawn a new process

		// Between fork() and execve() only async-signal-safe functions
		// must be called if multithreaded applications should be supported.
		// That's why the following code is executed before fork() is called.

		::pid_t pid = ::fork();

		// check: pid == -1 --> error
		if (pid == -1)
		{
			char const* err_str = ::strerror(errno);
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::rls_miso_matlab_app_rpoxy::run_matlab] fork(2) failed: "
				<< ::std::string(err_str);
			throw ::std::runtime_error(oss.str());
		}

		if (pid == 0)
		{
			// The child

			// Cancel signal handler set for parent
			sig_act.sa_handler = SIG_DFL;
			::sigaction(SIGTERM, &sig_act, 0);
			::sigaction(SIGINT, &sig_act, 0);

			// Get the maximum number of files this process is allowed to open
#if defined(F_MAXFD)
    		int maxdescs = ::fcntl(-1, F_MAXFD, 0);
    		if (maxdescs == -1)
			{
#if defined(_SC_OPEN_MAX)
        		maxdescs = ::sysconf(_SC_OPEN_MAX);
#else
				::rlimit limit;
				if (::getrlimit(RLIMIT_NOFILE, &limit) < 0)
				{
					char const* err_str = ::strerror(errno);
					::std::ostringstream oss;
					oss << "[dcs::eesim::detail::rls_miso_matlab_app_rpoxy::run_matlab] getrlimit(2) failed: "
						<< ::std::string(err_str);
					throw ::std::runtime_error(oss.str());
				}
				maxdescs = limit.rlim_cur;
#endif // _SC_OPEN_MAX
			}
#else // F_MAXFD
#if defined(_SC_OPEN_MAX)
    		int maxdescs = ::sysconf(_SC_OPEN_MAX);
#else // _SC_OPEN_MAX
			::rlimit limit;
			if (::getrlimit(RLIMIT_NOFILE, &limit) < 0)
			{
				char const* err_str = ::strerror(errno);
				::std::ostringstream oss;
				oss << "[dcs::eesim::detail::rls_miso_matlab_app_rpoxy::run_matlab] getrlimit(2) failed: "
					<< ::std::string(err_str);
				throw ::std::runtime_error(oss.str());
			}
			maxdescs = limit.rlim_cur;
#endif // _SC_OPEN_MAX
#endif // F_MAXFD
			if (maxdescs == -1)
			{
				maxdescs = 1024;
			}

			::std::vector<bool> close_fd(maxdescs, true);

			// Associate the child's stdout to the pipe write fd.
			close_fd[STDOUT_FILENO] = false;
			if (pipefd[1] != STDOUT_FILENO)
			{
				if (::dup2(pipefd[1], STDOUT_FILENO) != STDOUT_FILENO)
				{
					char const* err_str = ::strerror(errno);
					::std::ostringstream oss;
					oss << "[dcs::eesim::detail::rls_miso_matlab_app_rpoxy::run_matlab] dup2(2) failed: "
						<< ::std::string(err_str);
					throw ::std::runtime_error(oss.str());
				}
			}
			else
			{
				close_fd[pipefd[1]] = false;
			}
//			::close(STDOUT_FILENO);
//			::dup(pipefd[1]);

			// Check if the command already has path information
			::std::string cmd_path;
			::std::string cmd_name;
			typename ::std::string::size_type pos;
			pos = cmd.find_last_of('/');
			if (pos != ::std::string::npos)
			{
				cmd_path = cmd.substr(0, pos);
				cmd_name = cmd.substr(pos+1);
			}

			//FIXME: use scoped_ptr in place of "new"

			::std::size_t nargs = args.size()+1;
			char** argv = new char*[nargs + 2];
			argv[0] = new char[cmd_name.size()+1];
			::std::strncpy(argv[0], cmd_name.c_str(), cmd_name.size()+1); // by convention, the first argument is always the command name
			typename ArgsT::size_type i(1);
			typename ArgsT::const_iterator end_it(args.end());
			for (typename ArgsT::const_iterator it = args.begin(); it != end_it; ++it)
			{
				argv[i] = new char[it->size()+1];
				::std::strncpy(argv[i], it->c_str(), it->size()+1);
				++i;
			}
			argv[nargs] = 0;

			//char** envp(0);

			// Close unused file descriptors
#ifdef DCS_DEBUG
			// Keep standard error open for debug
			close_fd[STDERR_FILENO] = false;
#endif // DCS_DEBUG
			for (int fd = 0; fd < maxdescs; ++fd)
			{
				if (close_fd[fd])
				{
					::close(fd);
				}
			}

//[XXX]
#ifdef DCS_DEBUG
::std::cerr << "Executing MATLAB: " << cmd;//XXX
for (::std::size_t i=0; i < args.size(); ++i)//XXX
{//XXX
::std::cerr << " " << args[i] << ::std::flush;//XXX
}//XXX
::std::cerr << ::std::endl;//XXX
#endif // DCS_DEBUG
//[/XXX]
//DCS_DEBUG_TRACE("Executing: " << cmd << " " << args[0] << " " << args[1] << " " << args[2] << " - " << args[3]);

			//::execve(cmd.c_str(), argv, envp);
			::execvp(cmd.c_str(), argv);

			// Actually we should delete argv and envp data. As we must not
			// call any non-async-signal-safe functions though we simply exit.
			::write(STDERR_FILENO, "execvp() failed\n", 17);
			//_exit(EXIT_FAILURE);
			_exit(127);
		}

		// The parent

//		// Associate the parent's stdin to the pipe read fd.
		::close(pipefd[1]);
//		::close(STDIN_FILENO);
//		::dup(pipefd[0]);
		if (pipefd[0] != STDIN_FILENO)
		{
			if (::dup2(pipefd[0], STDIN_FILENO) != STDIN_FILENO)
			{
				char const* err_str = ::strerror(errno);
				::std::ostringstream oss;
				oss << "[dcs::eesim::detail::rls_miso_matlab_app_rpoxy::run_matlab] dup2(2) failed: "
					<< ::std::string(err_str);
				throw ::std::runtime_error(oss.str());
			}
			::close(pipefd[0]);
		}

		typedef ::boost::iostreams::file_descriptor_source fd_device_type;
		typedef ::boost::iostreams::stream_buffer<fd_device_type> fd_streambuf_type;
		//fd_device_type fd_src(pipefd[0], ::boost::iostreams::close_handle);
		fd_device_type fd_src(STDIN_FILENO, ::boost::iostreams::close_handle);
		fd_streambuf_type fd_buf(fd_src);
		::std::istream is(&fd_buf);

		// Read from the stdin
DCS_DEBUG_TRACE("BEGIN parsing MATLAB output");//XXX
		bool parse_line(false);
		while (is.good())
		{
			::std::string line;
			::std::getline(is, line);
DCS_DEBUG_TRACE("Read from MATLAB --> " << line);//XXX

			if (parse_line)
			{
				if (line.find("[/eesim]") != ::std::string::npos)
				{
					// The end of parsable lines
					parse_line = false;
				}
				else
				{
					typename ::std::string::size_type pos;
					if ((pos = line.find("th=")) != ::std::string::npos)
					{
						parse_matlab_data(line.substr(pos+3), th);
DCS_DEBUG_TRACE("Parsed as th=" << th);//XXX
					}
					else if ((pos = line.find("yh=")) != ::std::string::npos)
					{
						parse_matlab_data(line.substr(pos+3), yh);
DCS_DEBUG_TRACE("Parsed as yh=" << yh);//XXX
					}
					else if ((pos = line.find("P=")) != ::std::string::npos)
					{
						parse_matlab_data(line.substr(pos+2), P);
DCS_DEBUG_TRACE("Parsed as P=" << P);//XXX
					}
					else if ((pos = line.find("phi=")) != ::std::string::npos)
					{
						parse_matlab_data(line.substr(pos+4), phi);
DCS_DEBUG_TRACE("Parsed as phi=" << phi);//XXX
					}
					else if ((pos = line.find("psi=")) != ::std::string::npos)
					{
						parse_matlab_data(line.substr(pos+4), psi);
DCS_DEBUG_TRACE("Parsed as psi=" << psi);//XXX
					}
				}
			}
			else
			{
				if (line.find("[eesim]") != ::std::string::npos)
				{
					// The beginning of parsable lines
					parse_line = true;
				}
			}
		}
DCS_DEBUG_TRACE("END parsing MATLAB output");//XXX
DCS_DEBUG_TRACE("IS state: " << is.good() << " - " << is.eof() << " - " << is.fail() << " - " << is.bad());//XXX

		// Wait the child termination (in order to prevent zombies)
		int status;
//		::pid_t wait_pid;
//		wait_pid = ::wait(&status);
//		if (wait_pid != pid)
//		{
//			throw ::std::runtime_error("[dcs::eesim::detail::rls_miso_matlab_app_proxy::run_matlab] Unexpected child process.");
//		}
		if (::waitpid(pid, &status, 0) == -1)
		{
			char const* err_str = ::strerror(errno);
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::rls_miso_matlab_app_rpoxy::run_matlab] waitpid(2) failed: "
				<< ::std::string(err_str);
			throw ::std::runtime_error(oss.str());
		}
DCS_DEBUG_TRACE("MATLAB exited");//XXX
		if (WIFEXITED(status))
		{
DCS_DEBUG_TRACE("MATLAB exited with a call to 'exit(" << WEXITSTATUS(status) << ")'");//XXX
			if (WEXITSTATUS(status))
			{
				// status != 0 --> error in the execution
				::std::clog << "[Warning] MATLAB command exited with status " << WEXITSTATUS(status) << ::std::endl;
			}
		}
		else if (WIFSIGNALED(status))
		{
DCS_DEBUG_TRACE("MATLAB exited with a call to 'kill(" << WTERMSIG(status) << ")'");//XXX
		   ::std::clog << "[Warning] MATLAB command received signal " << WTERMSIG(status) << ::std::endl;
		}
		else
		{
DCS_DEBUG_TRACE("MATLAB exited with an unexpected way");//XXX
		}

		// Restore signal handler
		::sigaction(SIGTERM, &old_sigterm_act, 0);
		::sigaction(SIGINT, &old_sigint_act, 0);
	}


	private: template <typename T>
		static void parse_matlab_data(::std::string const& text, T& x)
	{
		::std::istringstream iss(text);
		while (iss.good())
		{
			char ch(iss.peek());

			if (::std::isspace(ch))
			{
				// Skip space
				iss.get();
			}
			else if (::std::isdigit(ch) || ch == '+' || ch == '-' || ch == '.')
			{
				// Found the beginning of a number

				iss >> x;
			}
			else
			{
				throw ::std::runtime_error("[dcs::eesim::detail::rls_miso_matlab_app_proxy] Unable to parse a MATLAB number");
			}
		}
	}


	private: template <typename T>
		static void parse_matlab_data(::std::string const& text, ::boost::numeric::ublas::vector<T>& v)
	{
		typename ::boost::numeric::ublas::vector<T>::size_type n(0);

		::std::istringstream iss(text);
		bool inside(false);
		bool done(false);
		while (iss.good() && !done)
		{
			char ch(iss.peek());
			bool ko(false);

			if (inside)
			{
				if (::std::isspace(ch) || ch == ';')
				{
					// Found an element separator
					iss.get();
//					while (iss.good() && (ch = iss.peek()) && ::std::isspace(ch))
//					{
//						iss.get();
//					}
				}
				else if (ch == ']')
				{
					// Found the end of the vector
//					iss.get();
//					inside = false;
					done = true;
				}
				else if (::std::isdigit(ch) || ch == '+' || ch == '-' || ch == '.')
				{
					// Found the beginning of a number
					T x;
					iss >> x;
					v.resize(n+1, true);
					v(n) = x;
					++n;
				}
				else
				{
					ko = true;
				}
			}
			else
			{
				if (ch == '[')
				{
					iss.get();
					v.resize(0, false);
					inside = true;
				}
				else if (::std::isspace(ch))
				{
					iss.get();
				}
				else
				{
					ko = true;
				}
			}

			if (ko)
			{
				throw ::std::runtime_error("[dcs::eesim::detail::rls_miso_matlab_app_proxy] Unable to parse a MATLAB vector.");
			}
		}
	}


	private: template <typename T>
		static void parse_matlab_data(::std::string const& text, ::boost::numeric::ublas::matrix<T>& A)
	{
		typename ::boost::numeric::ublas::matrix<T>::size_type r(0);
		typename ::boost::numeric::ublas::matrix<T>::size_type c(0);
		typename ::boost::numeric::ublas::matrix<T>::size_type nc(0);

		::std::istringstream iss(text);
		bool inside(false);
		bool done(false);
		while (iss.good() && !done)
		{
			char ch(iss.peek());
			bool ko(false);

			if (inside)
			{
				if (::std::isspace(ch))
				{
					// Found a column separator
					iss.get();
//					while (iss.good() && (ch = iss.peek()) && ::std::isspace(ch))
//					{
//						iss.get();
//					}
				}
 				else if (ch == ';')
				{
					// Found a row separator
					iss.get();
					++r;
					c = 0;
				}
				else if (ch == ']')
				{
					// Found the end of the matrix
//					iss.get();
//					inside = false;
					done = true;
				}
				else if (::std::isdigit(ch) || ch == '+' || ch == '-' || ch == '.')
				{
					// Found the beginning of a number
					T x;
					iss >> x;
					if (nc <= c)
					{
						nc = c+1;
					}
					A.resize(r+1, nc, true);
					A(r,c) = x;
					++c;
				}
				else
				{
					ko = true;
				}
			}
			else
			{
				// Note: outside of a matrix, only two types of character are
				// allowed: spaces and '['

				if (ch == '[')
				{
					iss.get();
					A.resize(0, 0, false);
					inside = true;
				}
				else if (::std::isspace(ch))
				{
					iss.get();
				}
				else
				{
					ko = true;
				}
			}

			if (ko)
			{
				throw ::std::runtime_error("[dcs::eesim::detail::rls_miso_matlab_app_proxy] Unable to parse a MATLAB matrix.");
			}
		}
	}


	private: static void process_signals(int signum)
	{
		::std::cerr << "Caught signal " << signum << ::std::endl;

		if (signum != SIGTERM && signum != SIGINT)
		{
			::std::clog << "[Warning] Caught the unhandled signal " << signum << ::std::endl;
		}

		// Terminate all child processes.
		// Specifically, kill every process in the process group of the calling
		// process
		if (::kill(0, SIGKILL) == -1)
		{
			char const* err_str = ::strerror(errno);
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::rls_miso_matlab_app_rpoxy::process_signals] kill(2) failed: "
				<< ::std::string(err_str);
			throw ::std::runtime_error(oss.str());
		}
		::std::abort();
	}


	/// The memory for the control output.
	private: size_type n_a_;
	/// The memory for the control input.
	private: size_type n_b_;
	/// The memory for the noise.
	private: size_type n_c_;
	/// Input delay (dead time).
	private: size_type d_;
	/// The size of the control output vector.
	private: size_type n_y_;
	/// The size of the augmented control input vector.
	private: size_type n_u_;
	/// Forgetting factor.
	private: real_type ff_;
	/// Matrix of system parameters estimated by RLS: [A_1 ... A_{n_a} B_1 ... B_{n_b} C_1 ... C_{n_c}].
	private: ::std::vector<vector_type> theta_hats_;
	/// The covariance matrix.
	private: ::std::vector<matrix_type> Ps_;
	/// The regression vector.
	private: ::std::vector<vector_type> phis_;
	/// The ...
	private: ::std::vector<vector_type> psis_;
	/// Initialization flags.
	private: ::std::vector<bool> initialized_;
	private: ::std::string matlab_cmd_;
}; // rpem_ff_miso_matlab_app_proxy


#else


#	error "Unable to find a suitable algorithm for recursive identification."


#endif // DCS_EESIM_USE_MATLAB_APP_*


#else // DCS_EESIM_USE_MATLAB_*


//template <typename VectorExprT>
//void rotate(::boost::numeric::ublas::vector_expression<VectorExprT>& v, ::std::size_t num_rot_grp, ::std::size_t rot_grp_size)
//{
//	namespace ublas = ::boost::numeric::ublas;
//
//	// Alternative #1
//	//
//	//::std::rotate(v().begin(), v().begin()+rot_grp_size, v().end());
//
//	// Alternative #2 (perhaps faster than solution #1 since performs less swaps))
//
//	DCS_DEBUG_ASSERT( v().size() == (num_rot_grp+1)*rot_grp_size );
//
//	for (::std::size_t i = 1; i <= num_rot_grp; ++i)
//	{
//		::std::size_t j1((i-1)*rot_grp_size);
//		::std::size_t j2(i*rot_grp_size);
//		::std::size_t j3((i+1)*rot_grp_size);
//		ublas::subrange(v(), j1, j2) = ublas::subrange(v(), j2, j3);
//	}
//}


/**
 * \brief Proxy for directly applying the Recursive Least Square with
 *  forgetting-factor algorithm to a MIMO system model.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class rls_ff_mimo_proxy: public rls_system_identification_strategy<TraitsT>
{
	private: typedef rls_system_identification_strategy<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename base_type::matrix_type matrix_type;
	public: typedef typename base_type::vector_type vector_type;
	public: typedef typename base_type::size_type size_type;


	public: rls_ff_mimo_proxy()
	: base_type(),
	  ff_(0),
	  Theta_hat_(),
	  P_(),
	  phi_()
	{
	}


	public: rls_ff_mimo_proxy(size_type n_a, size_type n_b, size_type d, size_type n_y, size_type n_u, real_type ff)
	: base_type(n_a, n_b, d, n_y, n_u),
	  ff_(ff)
	  Theta_hat_(),
	  P_(),
	  phi_()
	{
	}


	public: rls_ff_mimo_proxy(rls_ff_system_identification_strategy_params<traits_type> const& params)
	: base_type(params),
	  ff_(params.forgetting_factor()),
	  Theta_hat_(),
	  P_(),
	  phi_()
	{
	}


	public: real_type forgetting_factor() const
	{
		return ff_;
	}


	private: matrix_type do_Theta_hat() const
	{
		return Theta_hat_;
	}


	private: matrix_type do_P() const
	{
		return P_;
	}


	private: vector_type do_phi() const
	{
		return phi_;
	}


	private: void do_init()
	{
		// Prepare the data structures for the RLS algorithm 
		::dcs::sysid::rls_arx_mimo_init(this->output_order(),
										this->input_order(),
										this->input_delay(),
										this->num_outputs(),
										this->num_inputs(),
										Theta_hat_,
										P_,
										phi_);

	}


	private: vector_type do_estimate(vector_type const& y, vector_type const& u)
	{
		// Apply enabled heuristics
		bool reset(false);
		// Apply the "max-covariance" heuristic (if enabled)
		if (!reset && this->max_covariance_heuristic())
		{
			if (::boost::numeric::ublasx::max(P_) > this->max_covariance_heuristic_max_value())
			{
				reset = true;
			}
		}
		// Apply the "condition-number-covariance" heuristic (if enabled)
		if (!reset && this->max_covariance_heuristic())
		{
			real_type check_val = ::std::floor(
							::std::numeric_limits<real_type>::epsilon()
							*static_cast<real_type>(2)
							*::std::pow(10, this->condition_number_covariance_heuristic())
				);

			if (::boost::numeric::ublasx::rcond(P_) > check_val)
			{
				reset = true;
			}
		}
		if (reset)
		{
			this->init();
		}

		// Estimate system parameters
//DCS_DEBUG_TRACE("BEGIN estimation");//XXX
//DCS_DEBUG_TRACE("y(k): " << y);//XXX
//DCS_DEBUG_TRACE("u(k): " << u);//XXX
//DCS_DEBUG_TRACE("Theta_hat(k): " << Theta_hat_);//XXX
//DCS_DEBUG_TRACE("P(k): " << P_);//XXX
//DCS_DEBUG_TRACE("phi(k): " << phi_);//XXX
		vector_type y_hat;
		y_hat = ::dcs::sysid::rls_ff_arx_mimo(y,
											  u,
											  ff_,
											  this->output_order(),
											  this->input_order(),
											  this->input_delay(),
											  Theta_hat_,
											  P_,
											  phi_);
//DCS_DEBUG_TRACE("New Theta_hat(k): " << Theta_hat_);//XXX
//DCS_DEBUG_TRACE("New P(k): " << P_);//XXX
//DCS_DEBUG_TRACE("New phi(k): " << phi_);//XXX
//DCS_DEBUG_TRACE("END estimation");//XXX
		return y_hat;
	}


	/// Return matrix A_k from \hat{\Theta}.
	private: matrix_type do_A(size_type k) const
	{
		namespace ublas = ::boost::numeric::ublas;

		DCS_DEBUG_ASSERT( k >= 1 && k <= this->output_order() );

		// Remember:
		//   \hat{\Theta} = [a_{11}^{1},     a_{21}^{1},     ...,  a_{n_y1}^{1};
		//                   ...,            ...,            ...,  ...;
		//                   a_{11}^{n_a},   a_{21}^{n_a},   ...,  a_{n_y1}^{n_a};
		//                   ...,            ...,            ...,  ...;
		//                   a_{1n_y}^{1},   a_{2n_y}^{1},   ...,  a_{n_yn_y}^{1};
		//                   ...,            ...,            ...,  ...;
		//                   a_{1n_y}^{n_a}, a_{2n_y}^{n_a}, ...,  a_{n_yn_y}^{n_a};
		//                   b_{11}^{1},     b_{21}^{1},     ...,  b_{n_y1}^{1};
		//                   ...,            ...,            ...,  ...;
		//                   b_{11}^{n_b},   b_{21}^{n_b},   ...,  b_{n_y1}^{n_b};
		//                   ...,            ...,            ...,  ...;
		//                   b_{1n_u}^{1},   b_{2n_u}^{1},   ...,  b_{n_yn_u}^{1};
		//                   ...,            ...,            ...,  ...;
		//                   b_{1n_u}^{n_b}, b_{2n_u}^{n_b}, ...,  b_{n_yn_u}^{n_b}]
		// So in \hat{\Theta} the matrix A_k stays at:
		//   A_k <- (\hat{\Theta}(k:n_a:n_y,:))^T
		matrix_type A_k; // (this->num_outputs(), this->num_outputs());
		A_k = ublas::trans(ublas::subslice(Theta_hat_, k-1, this->output_order(), this->num_outputs(), 0, 1, this->num_outputs()));

		return A_k;
	}


	/// Return matrix B_k from \hat{\Theta}.
	private: matrix_type do_B(size_type k) const
	{
		namespace ublas = ::boost::numeric::ublas;

		DCS_DEBUG_ASSERT( k >= 1 && k <= this->input_order() );

		// Remember:
		//   \hat{\Theta} = [a_{11}^{1},     a_{21}^{1},     ...,  a_{n_y1}^{1};
		//                   ...,            ...,            ...,  ...;
		//                   a_{11}^{n_a},   a_{21}^{n_a},   ...,  a_{n_y1}^{n_a};
		//                   ...,            ...,            ...,  ...;
		//                   a_{1n_y}^{1},   a_{2n_y}^{1},   ...,  a_{n_yn_y}^{1};
		//                   ...,            ...,            ...,  ...;
		//                   a_{1n_y}^{n_a}, a_{2n_y}^{n_a}, ...,  a_{n_yn_y}^{n_a};
		//                   b_{11}^{1},     b_{21}^{1},     ...,  b_{n_y1}^{1};
		//                   ...,            ...,            ...,  ...;
		//                   b_{11}^{n_b},   b_{21}^{n_b},   ...,  b_{n_y1}^{n_b};
		//                   ...,            ...,            ...,  ...;
		//                   b_{1n_u}^{1},   b_{2n_u}^{1},   ...,  b_{n_yn_u}^{1};
		//                   ...,            ...,            ...,  ...;
		//                   b_{1n_u}^{n_b}, b_{2n_u}^{n_b}, ...,  b_{n_yn_u}^{n_b}]
		// So in \hat{\Theta} the matrix B_k stays at:
		//   B_k <- (\hat{\Theta}(((n_a*n_y)+k):n_b:n_u,:))^T
		matrix_type B_k; // (n_y_, n_u_);
		B_k = ublas::trans(ublas::subslice(Theta_hat_, this->output_order()*this->num_outputs()+k-1, this->input_order(), this->num_inputs(), 0, 1, this->num_outputs()));

		return B_k;
	}


	/// Forgetting factor.
	private: real_type ff_;
	/// Matrix of system parameters estimated by RLS: [A_1 ... A_{n_a} B_1 ... B_{n_b}].
	private: matrix_type Theta_hat_;
	/// The covariance matrix.
	private: matrix_type P_;
	/// The regression vector.
	private: vector_type phi_;
}; // rls_ff_mimo_proxy


/**
 * \brief Proxy to identify a MIMO system model by applying the Recursive Least
 *  Square with forgetting-factor algorithm to several MISO system models.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class rls_ff_miso_proxy: public rls_system_identification_strategy<TraitsT>
{
	private: typedef rls_system_identification_strategy<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename base_type::matrix_type matrix_type;
	public: typedef typename base_type::vector_type vector_type;
	public: typedef typename base_type::size_type size_type;


	public: rls_ff_miso_proxy()
	: base_type(),
	  ff_(0),
	  theta_hats_(),
	  Ps_(),
	  phis_()
	{
	}


	public: rls_ff_miso_proxy(size_type n_a, size_type n_b, size_type d, size_type n_y, size_type n_u, real_type ff)
	: base_type(n_a,n_b,d,n_y,n_u),
	  ff_(ff),
	  theta_hats_(n_y),
	  Ps_(n_y),
	  phis_(n_y)
	{
	}


	public: rls_ff_miso_proxy(rls_ff_system_identification_strategy_params<traits_type> const& params)
	: base_type(params),
	  ff_(params.forgetting_factor()),
	  theta_hats_(params.num_outputs()),
	  Ps_(params.num_outputs()),
	  phis_(params.num_outputs())
	{
	}


	public: real_type forgetting_factor() const
	{
		return ff_;
	}


	private: matrix_type do_Theta_hat() const
	{
		namespace ublas = ::boost::numeric::ublas;

		const size_type na(this->output_order());
		const size_type nb(this->input_order());
		const size_type ny(this->num_outputs());
		const size_type nu(this->num_inputs());
		const size_type nay(na*ny);
		const size_type nbu(nb*nu);
		const size_type n(nay+nbu);
		matrix_type X(n, ny, real_type/*zero()*/());

		for (size_type i = 0; i < ny; ++i)
		{
			// ith output => ith column of Theta_hat
			// ith column of Theta_hat = [0; ...; 0; a_{ii}^{1}
			const size_type k(i*na);
			ublas::matrix_column<matrix_type> mc(X,i);
			ublas::subrange(mc, k, k+na) = ublas::subrange(theta_hats_[i], 0, na);
			ublas::subrange(mc, nay, n) = ublas::subrange(theta_hats_[i], na, na+nbu);
		}

		return X;
	}


	private: matrix_type do_P() const
	{
		matrix_type aux_P;
//FIXME
		return aux_P;
	}


	private: vector_type do_phi() const
	{
		namespace ublas = ::boost::numeric::ublas;

		const size_type na(this->output_order());
		const size_type ny(this->num_outputs());
		const size_type nay(na*ny);
		const size_type nbu(this->input_order()*this->num_inputs());
		const size_type n(nay+nbu);
		vector_type x(n);

		for (size_type i = 0; i < ny; ++i)
		{
			const size_type k(i*na);
			ublas::subrange(x, k, k+na) = ublas::subrange(phis_[i], 0, na);
		}

		ublas::subrange(x, nay, n) = ublas::subrange(phis_[0], na, na+nbu);

		return x;
	}


	private: void do_init()
	{
		const size_type na(this->output_order());
		const size_type nb(this->input_order());
		const size_type d(this->input_delay());
		const size_type ny(this->num_outputs());
		const size_type nu(this->num_inputs());

		// Prepare the data structures for the RLS algorithm 
		for (size_type i = 0; i < ny; ++i)
		{
			::dcs::sysid::rls_arx_miso_init(na,
											nb,
											d,
											nu,
											theta_hats_[i],
											Ps_[i],
											phis_[i]);
		}
	}


	private: vector_type do_estimate(vector_type const& y, vector_type const& u)
	{
		const size_type na(this->output_order());
		const size_type nb(this->input_order());
		const size_type d(this->input_delay());
		const size_type ny(this->num_outputs());

		// Apply enabled heuristics
		bool reset(false);
		// Apply the "max-covariance" heuristic (if enabled)
		if (this->max_covariance_heuristic())
		{
			for (size_type i = 0; i < ny && !reset; ++i)
			{
				if (::boost::numeric::ublasx::max(Ps_[i]) > this->max_covariance_heuristic_max_value())
				{
					reset = true;
				}
			}
		}
		// Apply the "condition-number-covariance" heuristic (if enabled)
		if (this->max_covariance_heuristic())
		{
			real_type check_val = ::std::floor(
							::std::numeric_limits<real_type>::epsilon()
							*static_cast<real_type>(2)
							*::std::pow(10, this->condition_number_covariance_heuristic())
				);

			for (size_type i = 0; i < ny && !reset; ++i)
			{
				if (::boost::numeric::ublasx::rcond(Ps_[i]) > check_val)
				{
					reset = true;
				}
			}
		}
		if (reset)
		{
			this->init();
		}

		// Estimate system parameters
DCS_DEBUG_TRACE("BEGIN estimation");//XXX
DCS_DEBUG_TRACE("y(k): " << y);//XXX
DCS_DEBUG_TRACE("u(k): " << u);//XXX
		vector_type y_hat(ny);
		for (size_type i = 0; i < ny; ++i)
		{
DCS_DEBUG_TRACE("theta_hat["<< i << "](k): " << theta_hats_[i]);//XXX
DCS_DEBUG_TRACE("P["<< i << "](k): " << Ps_[i]);//XXX
DCS_DEBUG_TRACE("phi["<< i << "](k): " << phis_[i]);//XXX
			y_hat(i) = ::dcs::sysid::rls_ff_arx_miso(y(i),
													 u,
													 ff_,
													 na,
													 nb,
													 d,
													 theta_hats_[i],
													 Ps_[i],
													 phis_[i]);
DCS_DEBUG_TRACE("New theta_hat["<< i << "](k): " << theta_hats_[i]);//XXX
DCS_DEBUG_TRACE("New P["<< i << "](k): " << Ps_[i]);//XXX
DCS_DEBUG_TRACE("New rcond(P["<< i << "](k)): " << ::boost::numeric::ublasx::rcond(Ps_[i]));//XXX
DCS_DEBUG_TRACE("New phi["<< i << "](k): " << phis_[i]);//XXX
DCS_DEBUG_TRACE("New e["<< i << "](k): " << (y(i)-y_hat(i)));//XXX
		}
DCS_DEBUG_TRACE("New y_hat(k): " << y_hat);//XXX
DCS_DEBUG_TRACE("END estimation");//XXX

		return y_hat;
	}


	/// Return matrix A_k from \hat{\Theta}.
	private: matrix_type do_A(size_type k) const
	{
		namespace ublas = ::boost::numeric::ublas;

		DCS_DEBUG_ASSERT( k >= 1 && k <= this->output_order() );

		const size_type ny(this->num_outputs());

		// Remember, for each output i=1,...,n_y:
		//   \hat{\theta}_i = [a_{ii}^{1};
		//                     ...;
		//                     a_{ii}^{n_a};
		//                     b_{i1}^{1};
		//                     ...;
		//                     b_{i1}^{n_b};
		//                     ...;
		//                     b_{in_u}^{1};
		//                     ...;
		//                     b_{in_u}^{n_b}]
		// So in \hat{\theta}_i the ith diagonal element of matrix A_k stays at:
		//   A_k(i,i) <- \hat{\theta}_i(k)
		matrix_type A_k(ny, ny, real_type/*zero*/());
		for (size_type i = 0; i < ny; ++i)
		{
			A_k(i,i) = theta_hats_[i](k);
		}

		return A_k;
	}


	/// Return matrix B_k from \hat{\Theta}.
	private: matrix_type do_B(size_type k) const
	{
		namespace ublas = ::boost::numeric::ublas;

		DCS_DEBUG_ASSERT( k >= 1 && k <= this->input_order() );

		const size_type na(this->output_order());
		const size_type nb(this->input_order());
		const size_type ny(this->num_outputs());
		const size_type nu(this->num_inputs());

		// Remember, for each output i=1,...,n_y:
		//   \hat{\theta}_i = [a_{ii}^{1};
		//                     ...;
		//                     a_{ii}^{n_a};
		//                     b_{i1}^{1};
		//                     ...;
		//                     b_{i1}^{n_b};
		//                     ...;
		//                     b_{in_u}^{1};
		//                     ...;
		//                     b_{in_u}^{n_b}]
		// So in \hat{\theta}_i the ith row of matrix B_k stays at:
		//   B_k(i,:) <- (\hat{\theta}_i(((n_a+k):n_b:n_u))^T
		matrix_type B_k(ny, nu);
		for (size_type i = 0; i < ny; ++i)
		{
			ublas::row(B_k, i) = ublas::subslice(theta_hats_[i], na+k-1, nb, nu);
		}

		return B_k;
	}


	/// Forgetting factor.
	private: real_type ff_;
	/// Matrix of system parameters estimated by RLS: [A_1 ... A_{n_a} B_1 ... B_{n_b}].
	private: ::std::vector<vector_type> theta_hats_;
	/// The covariance matrix.
	private: ::std::vector<matrix_type> Ps_;
	/// The regression vector.
	private: ::std::vector<vector_type> phis_;
}; // rls_ff_miso_proxy


/**
 * \brief Proxy to identify a MIMO system model by applying the Recursive Least
 *  Square with forgetting-factor algorithm to several MISO system models.
 *
 * The forgetting-factor is varied according to the following law [1]:
 * \f[
 *   \lambda(t) = \lambda_{\text{min}}+(1-\lambda_{\text{min}})2^{-\text{NINT}[\rho \epsilon^2(t)]}
 * \f]
 * where
 * - \f$\rho\f$, the <em>sensitivity gain</em>, is a design parameter.
 * - \f$\epsilon\f$, is the estimation error (i.e., the difference between the
 *    value of the current observed output and the one of the current estimated output).
 * - \f$\text{NINT}[\cdot]\f$ is the nearest integer of \f$[\cdot\]\f$.
 * .
 *
 * References:
 * -# Park et al.
 *    "Fast Tracking RLS Algorithm Using Novel Variable Forgetting Factor with Unity Zone",
 *    Electronic Letters, Vol. 23, 1991.
 * .
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class rls_park1991_miso_proxy: public rls_system_identification_strategy<TraitsT>
{
	private: typedef rls_system_identification_strategy<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename base_type::matrix_type matrix_type;
	public: typedef typename base_type::vector_type vector_type;
	public: typedef typename base_type::size_type size_type;


	public: rls_park1991_miso_proxy()
	: base_type(),
	  ff_(0),
	  rho_(0),
	  theta_hats_(),
	  Ps_(),
	  phis_()
	{
	}


	public: rls_park1991_miso_proxy(size_type n_a, size_type n_b, size_type d, size_type n_y, size_type n_u, real_type ff, real_type rho)
	: base_type(n_a,n_b,d,n_y,n_u),
	  ff_(ff),
	  rho_(rho),
	  theta_hats_(n_y),
	  Ps_(n_y),
	  phis_(n_y)
	{
	}


	public: rls_park1991_miso_proxy(rls_park1991_system_identification_strategy_params<traits_type> const& params)
	: base_type(params),
	  ff_(params.forgetting_factor()),
	  rho_(params.sensitivity_gain()),
	  theta_hats_(params.num_outputs()),
	  Ps_(params.num_outputs()),
	  phis_(params.num_outputs())
	{
	}


	public: real_type forgetting_factor() const
	{
		return ff_;
	}


	public: real_type sensitivity_gain() const
	{
		return rho_;
	}


	private: matrix_type do_Theta_hat() const
	{
		namespace ublas = ::boost::numeric::ublas;

		const size_type na(this->output_order());
		const size_type nb(this->input_order());
		const size_type ny(this->num_outputs());
		const size_type nu(this->num_inputs());
		const size_type nay(na*ny);
		const size_type nbu(nb*nu);
		const size_type n(nay+nbu);
		matrix_type X(n, ny, real_type/*zero()*/());

		for (size_type i = 0; i < ny; ++i)
		{
			// ith output => ith column of Theta_hat
			// ith column of Theta_hat = [0; ...; 0; a_{ii}^{1}
			const size_type k(i*na);
			ublas::matrix_column<matrix_type> mc(X,i);
			ublas::subrange(mc, k, k+na) = ublas::subrange(theta_hats_[i], 0, na);
			ublas::subrange(mc, nay, n) = ublas::subrange(theta_hats_[i], na, na+nbu);
		}

		return X;
	}


	private: matrix_type do_P() const
	{
		matrix_type aux_P;
//FIXME
		return aux_P;
	}


	private: vector_type do_phi() const
	{
		namespace ublas = ::boost::numeric::ublas;

		const size_type na(this->output_order());
		const size_type ny(this->num_outputs());
		const size_type nay(na*ny);
		const size_type nbu(this->input_order()*this->num_inputs());
		const size_type n(nay+nbu);
		vector_type x(n);

		for (size_type i = 0; i < ny; ++i)
		{
			const size_type k(i*na);
			ublas::subrange(x, k, k+na) = ublas::subrange(phis_[i], 0, na);
		}

		ublas::subrange(x, nay, n) = ublas::subrange(phis_[0], na, na+nbu);

		return x;
	}


	private: void do_init()
	{
		const size_type na(this->output_order());
		const size_type nb(this->input_order());
		const size_type d(this->input_delay());
		const size_type ny(this->num_outputs());
		const size_type nu(this->num_inputs());

		// Prepare the data structures for the RLS algorithm 
		for (size_type i = 0; i < ny; ++i)
		{
			::dcs::sysid::rls_arx_miso_init(na,
											nb,
											d,
											nu,
											theta_hats_[i],
											Ps_[i],
											phis_[i]);
		}
	}


	private: vector_type do_estimate(vector_type const& y, vector_type const& u)
	{
		const size_type na(this->output_order());
		const size_type nb(this->input_order());
		const size_type d(this->input_delay());
		const size_type ny(this->num_outputs());

		// Apply enabled heuristics
		bool reset(false);
		// Apply the "max-covariance" heuristic (if enabled)
		if (this->max_covariance_heuristic())
		{
			for (size_type i = 0; i < ny && !reset; ++i)
			{
				if (::boost::numeric::ublasx::max(Ps_[i]) > this->max_covariance_heuristic_max_value())
				{
					reset = true;
				}
			}
		}
		// Apply the "condition-number-covariance" heuristic (if enabled)
		if (this->max_covariance_heuristic())
		{
			real_type check_val = ::std::floor(
							::std::numeric_limits<real_type>::epsilon()
							*static_cast<real_type>(2)
							*::std::pow(10, this->condition_number_covariance_heuristic())
				);

			for (size_type i = 0; i < ny && !reset; ++i)
			{
				if (::boost::numeric::ublasx::rcond(Ps_[i]) > check_val)
				{
					reset = true;
				}
			}
		}
		if (reset)
		{
			this->init();
		}

		// Estimate system parameters
DCS_DEBUG_TRACE("BEGIN estimation");//XXX
DCS_DEBUG_TRACE("y(k): " << y);//XXX
DCS_DEBUG_TRACE("u(k): " << u);//XXX
		vector_type y_hat(ny);
		for (size_type i = 0; i < ny; ++i)
		{
DCS_DEBUG_TRACE("theta_hat["<< i << "](k): " << theta_hats_[i]);//XXX
DCS_DEBUG_TRACE("P["<< i << "](k): " << Ps_[i]);//XXX
DCS_DEBUG_TRACE("phi["<< i << "](k): " << phis_[i]);//XXX
			y_hat(i) = ::dcs::sysid::rls_park1991_arx_miso(y(i),
													 u,
													 ff_,
													 rho_,
													 na,
													 nb,
													 d,
													 theta_hats_[i],
													 Ps_[i],
													 phis_[i]);
DCS_DEBUG_TRACE("New theta_hat["<< i << "](k): " << theta_hats_[i]);//XXX
DCS_DEBUG_TRACE("New P["<< i << "](k): " << Ps_[i]);//XXX
DCS_DEBUG_TRACE("New rcond(P["<< i << "](k)): " << ::boost::numeric::ublasx::rcond(Ps_[i]));//XXX
DCS_DEBUG_TRACE("New phi["<< i << "](k): " << phis_[i]);//XXX
DCS_DEBUG_TRACE("New e["<< i << "](k): " << (y(i)-y_hat(i)));//XXX
		}
DCS_DEBUG_TRACE("New y_hat(k): " << y_hat);//XXX
DCS_DEBUG_TRACE("END estimation");//XXX

		return y_hat;
	}


	/// Return matrix A_k from \hat{\Theta}.
	private: matrix_type do_A(size_type k) const
	{
		namespace ublas = ::boost::numeric::ublas;

		DCS_DEBUG_ASSERT( k >= 1 && k <= this->output_order() );

		const size_type ny(this->num_outputs());

		// Remember, for each output i=1,...,n_y:
		//   \hat{\theta}_i = [a_{ii}^{1};
		//                     ...;
		//                     a_{ii}^{n_a};
		//                     b_{i1}^{1};
		//                     ...;
		//                     b_{i1}^{n_b};
		//                     ...;
		//                     b_{in_u}^{1};
		//                     ...;
		//                     b_{in_u}^{n_b}]
		// So in \hat{\theta}_i the ith diagonal element of matrix A_k stays at:
		//   A_k(i,i) <- \hat{\theta}_i(k)
		matrix_type A_k(ny, ny, real_type/*zero*/());
		for (size_type i = 0; i < ny; ++i)
		{
			A_k(i,i) = theta_hats_[i](k);
		}

		return A_k;
	}


	/// Return matrix B_k from \hat{\Theta}.
	private: matrix_type do_B(size_type k) const
	{
		namespace ublas = ::boost::numeric::ublas;

		DCS_DEBUG_ASSERT( k >= 1 && k <= this->input_order() );

		const size_type na(this->output_order());
		const size_type nb(this->input_order());
		const size_type ny(this->num_outputs());
		const size_type nu(this->num_inputs());

		// Remember, for each output i=1,...,n_y:
		//   \hat{\theta}_i = [a_{ii}^{1};
		//                     ...;
		//                     a_{ii}^{n_a};
		//                     b_{i1}^{1};
		//                     ...;
		//                     b_{i1}^{n_b};
		//                     ...;
		//                     b_{in_u}^{1};
		//                     ...;
		//                     b_{in_u}^{n_b}]
		// So in \hat{\theta}_i the ith row of matrix B_k stays at:
		//   B_k(i,:) <- (\hat{\theta}_i(((n_a+k):n_b:n_u))^T
		matrix_type B_k(ny, nu);
		for (size_type i = 0; i < ny; ++i)
		{
			ublas::row(B_k, i) = ublas::subslice(theta_hats_[i], na+k-1, nb, nu);
		}

		return B_k;
	}


	/// Forgetting factor.
	private: real_type ff_;
	/// Sensitivity gain.
	private: real_type rho_;
	/// Matrix of system parameters estimated by RLS: [A_1 ... A_{n_a} B_1 ... B_{n_b}].
	private: ::std::vector<vector_type> theta_hats_;
	/// The covariance matrix.
	private: ::std::vector<matrix_type> Ps_;
	/// The regression vector.
	private: ::std::vector<vector_type> phis_;
}; // rls_park1991_miso_proxy


/**
 * \brief Proxy to identify a MIMO system model by applying the Recursive Least
 *  Square with forgetting-factor algorithm to several MISO system models.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class rls_kulhavy1984_miso_proxy: public rls_system_identification_strategy<TraitsT>
{
	private: typedef rls_system_identification_strategy<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename base_type::matrix_type matrix_type;
	public: typedef typename base_type::vector_type vector_type;
	public: typedef typename base_type::size_type size_type;


	public: rls_kulhavy1984_miso_proxy()
	: base_type(),
	  ff_(0),
	  theta_hats_(),
	  Ps_(),
	  phis_()
	{
	}


	public: rls_kulhavy1984_miso_proxy(size_type n_a, size_type n_b, size_type d, size_type n_y, size_type n_u, real_type ff)
	: base_type(n_a,n_b,d,n_y,n_u),
	  ff_(ff),
	  theta_hats_(n_y),
	  Ps_(n_y),
	  phis_(n_y)
	{
	}


	public: rls_kulhavy1984_miso_proxy(rls_kulhavy1984_system_identification_strategy_params<traits_type> const& params)
	: base_type(params),
	  ff_(params.forgetting_factor()),
	  theta_hats_(params.num_outputs()),
	  Ps_(params.num_outputs()),
	  phis_(params.num_outputs())
	{
	}


	public: real_type forgetting_factor() const
	{
		return ff_;
	}


	private: matrix_type do_Theta_hat() const
	{
		namespace ublas = ::boost::numeric::ublas;

		const size_type na(this->output_order());
		const size_type nb(this->input_order());
		const size_type ny(this->num_outputs());
		const size_type nu(this->num_inputs());
		const size_type nay(na*ny);
		const size_type nbu(nb*nu);
		const size_type n(nay+nbu);
		matrix_type X(n, ny, real_type/*zero()*/());

		for (size_type i = 0; i < ny; ++i)
		{
			// ith output => ith column of Theta_hat
			// ith column of Theta_hat = [0; ...; 0; a_{ii}^{1}
			const size_type k(i*na);
			ublas::matrix_column<matrix_type> mc(X,i);
			ublas::subrange(mc, k, k+na) = ublas::subrange(theta_hats_[i], 0, na);
			ublas::subrange(mc, nay, n) = ublas::subrange(theta_hats_[i], na, na+nbu);
		}

		return X;
	}


	private: matrix_type do_P() const
	{
		matrix_type aux_P;
//FIXME
		return aux_P;
	}


	private: vector_type do_phi() const
	{
		namespace ublas = ::boost::numeric::ublas;

		const size_type na(this->output_order());
		const size_type ny(this->num_outputs());
		const size_type nay(na*ny);
		const size_type nbu(this->input_order()*this->num_inputs());
		const size_type n(nay+nbu);
		vector_type x(n);

		for (size_type i = 0; i < ny; ++i)
		{
			const size_type k(i*na);
			ublas::subrange(x, k, k+na) = ublas::subrange(phis_[i], 0, na);
		}

		ublas::subrange(x, nay, n) = ublas::subrange(phis_[0], na, na+nbu);

		return x;
	}


	private: void do_init()
	{
		const size_type na(this->output_order());
		const size_type nb(this->input_order());
		const size_type d(this->input_delay());
		const size_type ny(this->num_outputs());
		const size_type nu(this->num_inputs());

		// Prepare the data structures for the RLS algorithm 
		for (size_type i = 0; i < ny; ++i)
		{
			::dcs::sysid::rls_arx_miso_init(na,
											nb,
											d,
											nu,
											theta_hats_[i],
											Ps_[i],
											phis_[i]);
		}
	}


	private: vector_type do_estimate(vector_type const& y, vector_type const& u)
	{
		const size_type na(this->output_order());
		const size_type nb(this->input_order());
		const size_type d(this->input_delay());
		const size_type ny(this->num_outputs());

		// Apply enabled heuristics
		bool reset(false);
		// Apply the "max-covariance" heuristic (if enabled)
		if (this->max_covariance_heuristic())
		{
			for (size_type i = 0; i < ny && !reset; ++i)
			{
				if (::boost::numeric::ublasx::max(Ps_[i]) > this->max_covariance_heuristic_max_value())
				{
					reset = true;
				}
			}
		}
		// Apply the "condition-number-covariance" heuristic (if enabled)
		if (this->max_covariance_heuristic())
		{
			real_type check_val = ::std::floor(
							::std::numeric_limits<real_type>::epsilon()
							*static_cast<real_type>(2)
							*::std::pow(10, this->condition_number_covariance_heuristic())
				);

			for (size_type i = 0; i < ny && !reset; ++i)
			{
				if (::boost::numeric::ublasx::rcond(Ps_[i]) > check_val)
				{
					reset = true;
				}
			}
		}
		if (reset)
		{
			this->init();
		}

		// Estimate system parameters
DCS_DEBUG_TRACE("BEGIN estimation");//XXX
DCS_DEBUG_TRACE("y(k): " << y);//XXX
DCS_DEBUG_TRACE("u(k): " << u);//XXX
		vector_type y_hat(ny);
		for (size_type i = 0; i < ny; ++i)
		{
DCS_DEBUG_TRACE("theta_hat["<< i << "](k): " << theta_hats_[i]);//XXX
DCS_DEBUG_TRACE("P["<< i << "](k): " << Ps_[i]);//XXX
DCS_DEBUG_TRACE("phi["<< i << "](k): " << phis_[i]);//XXX
			y_hat(i) = ::dcs::sysid::rls_kulhavy1984_arx_miso(y(i),
													 u,
													 ff_,
													 na,
													 nb,
													 d,
													 theta_hats_[i],
													 Ps_[i],
													 phis_[i]);
DCS_DEBUG_TRACE("New theta_hat["<< i << "](k): " << theta_hats_[i]);//XXX
DCS_DEBUG_TRACE("New P["<< i << "](k): " << Ps_[i]);//XXX
DCS_DEBUG_TRACE("New rcond(P["<< i << "](k)): " << ::boost::numeric::ublasx::rcond(Ps_[i]));//XXX
DCS_DEBUG_TRACE("New phi["<< i << "](k): " << phis_[i]);//XXX
DCS_DEBUG_TRACE("New e["<< i << "](k): " << (y(i)-y_hat(i)));//XXX
		}
DCS_DEBUG_TRACE("New y_hat(k): " << y_hat);//XXX
DCS_DEBUG_TRACE("END estimation");//XXX

		return y_hat;
	}


	/// Return matrix A_k from \hat{\Theta}.
	private: matrix_type do_A(size_type k) const
	{
		namespace ublas = ::boost::numeric::ublas;

		DCS_DEBUG_ASSERT( k >= 1 && k <= this->output_order() );

		const size_type ny(this->num_outputs());

		// Remember, for each output i=1,...,n_y:
		//   \hat{\theta}_i = [a_{ii}^{1};
		//                     ...;
		//                     a_{ii}^{n_a};
		//                     b_{i1}^{1};
		//                     ...;
		//                     b_{i1}^{n_b};
		//                     ...;
		//                     b_{in_u}^{1};
		//                     ...;
		//                     b_{in_u}^{n_b}]
		// So in \hat{\theta}_i the ith diagonal element of matrix A_k stays at:
		//   A_k(i,i) <- \hat{\theta}_i(k)
		matrix_type A_k(ny, ny, real_type/*zero*/());
		for (size_type i = 0; i < ny; ++i)
		{
			A_k(i,i) = theta_hats_[i](k);
		}

		return A_k;
	}


	/// Return matrix B_k from \hat{\Theta}.
	private: matrix_type do_B(size_type k) const
	{
		namespace ublas = ::boost::numeric::ublas;

		DCS_DEBUG_ASSERT( k >= 1 && k <= this->input_order() );

		const size_type na(this->output_order());
		const size_type nb(this->input_order());
		const size_type ny(this->num_outputs());
		const size_type nu(this->num_inputs());

		// Remember, for each output i=1,...,n_y:
		//   \hat{\theta}_i = [a_{ii}^{1};
		//                     ...;
		//                     a_{ii}^{n_a};
		//                     b_{i1}^{1};
		//                     ...;
		//                     b_{i1}^{n_b};
		//                     ...;
		//                     b_{in_u}^{1};
		//                     ...;
		//                     b_{in_u}^{n_b}]
		// So in \hat{\theta}_i the ith row of matrix B_k stays at:
		//   B_k(i,:) <- (\hat{\theta}_i(((n_a+k):n_b:n_u))^T
		matrix_type B_k(ny, nu);
		for (size_type i = 0; i < ny; ++i)
		{
			ublas::row(B_k, i) = ublas::subslice(theta_hats_[i], na+k-1, nb, nu);
		}

		return B_k;
	}


	/// Forgetting factor.
	private: real_type ff_;
	/// Matrix of system parameters estimated by RLS: [A_1 ... A_{n_a} B_1 ... B_{n_b}].
	private: ::std::vector<vector_type> theta_hats_;
	/// The covariance matrix.
	private: ::std::vector<matrix_type> Ps_;
	/// The regression vector.
	private: ::std::vector<vector_type> phis_;
}; // rls_kulhavy1984_miso_proxy


/**
 * \brief Proxy to identify a MIMO system model by applying the Recursive Least
 *  Square with forgetting-factor algorithm to several MISO system models.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class rls_bittanti1990_miso_proxy: public rls_system_identification_strategy<TraitsT>
{
	private: typedef rls_system_identification_strategy<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename base_type::matrix_type matrix_type;
	public: typedef typename base_type::vector_type vector_type;
	public: typedef typename base_type::size_type size_type;


	public: rls_bittanti1990_miso_proxy()
	: base_type(),
	  ff_(0),
	  delta_(0),
	  theta_hats_(),
	  Ps_(),
	  phis_()
	{
	}


	public: rls_bittanti1990_miso_proxy(size_type n_a, size_type n_b, size_type d, size_type n_y, size_type n_u, real_type ff, real_type delta)
	: base_type(n_a,n_b,d,n_y,n_u),
	  ff_(ff),
	  delta_(delta),
	  theta_hats_(n_y),
	  Ps_(n_y),
	  phis_(n_y)
	{
	}


	public: rls_bittanti1990_miso_proxy(rls_bittanti1990_system_identification_strategy_params<traits_type> const& params)
	: base_type(params),
	  ff_(params.forgetting_factor()),
	  delta_(params.correction_factor()),
	  theta_hats_(params.num_outputs()),
	  Ps_(params.num_outputs()),
	  phis_(params.num_outputs())
	{
	}


	public: real_type forgetting_factor() const
	{
		return ff_;
	}


	public: real_type correction_factor() const
	{
		return delta_;
	}


	private: matrix_type do_Theta_hat() const
	{
		namespace ublas = ::boost::numeric::ublas;

		const size_type na(this->output_order());
		const size_type nb(this->input_order());
		const size_type ny(this->num_outputs());
		const size_type nu(this->num_inputs());
		const size_type nay(na*ny);
		const size_type nbu(nb*nu);
		const size_type n(nay+nbu);
		matrix_type X(n, ny, real_type/*zero()*/());

		for (size_type i = 0; i < ny; ++i)
		{
			// ith output => ith column of Theta_hat
			// ith column of Theta_hat = [0; ...; 0; a_{ii}^{1}
			const size_type k(i*na);
			ublas::matrix_column<matrix_type> mc(X,i);
			ublas::subrange(mc, k, k+na) = ublas::subrange(theta_hats_[i], 0, na);
			ublas::subrange(mc, nay, n) = ublas::subrange(theta_hats_[i], na, na+nbu);
		}

		return X;
	}


	private: matrix_type do_P() const
	{
		matrix_type aux_P;
//FIXME
		return aux_P;
	}


	private: vector_type do_phi() const
	{
		namespace ublas = ::boost::numeric::ublas;

		const size_type na(this->output_order());
		const size_type ny(this->num_outputs());
		const size_type nay(na*ny);
		const size_type nbu(this->input_order()*this->num_inputs());
		const size_type n(nay+nbu);
		vector_type x(n);

		for (size_type i = 0; i < ny; ++i)
		{
			const size_type k(i*na);
			ublas::subrange(x, k, k+na) = ublas::subrange(phis_[i], 0, na);
		}

		ublas::subrange(x, nay, n) = ublas::subrange(phis_[0], na, na+nbu);

		return x;
	}


	private: void do_init()
	{
		const size_type na(this->output_order());
		const size_type nb(this->input_order());
		const size_type d(this->input_delay());
		const size_type ny(this->num_outputs());
		const size_type nu(this->num_inputs());

		// Prepare the data structures for the RLS algorithm 
		for (size_type i = 0; i < ny; ++i)
		{
			::dcs::sysid::rls_arx_miso_init(na,
											nb,
											d,
											nu,
											theta_hats_[i],
											Ps_[i],
											phis_[i]);
		}
	}


	private: vector_type do_estimate(vector_type const& y, vector_type const& u)
	{
		const size_type na(this->output_order());
		const size_type nb(this->input_order());
		const size_type d(this->input_delay());
		const size_type ny(this->num_outputs());

		// Apply enabled heuristics
		bool reset(false);
		// Apply the "max-covariance" heuristic (if enabled)
		if (this->max_covariance_heuristic())
		{
			for (size_type i = 0; i < ny && !reset; ++i)
			{
				if (::boost::numeric::ublasx::max(Ps_[i]) > this->max_covariance_heuristic_max_value())
				{
					reset = true;
				}
			}
		}
		// Apply the "condition-number-covariance" heuristic (if enabled)
		if (this->max_covariance_heuristic())
		{
			real_type check_val = ::std::floor(
							::std::numeric_limits<real_type>::epsilon()
							*static_cast<real_type>(2)
							*::std::pow(10, this->condition_number_covariance_heuristic())
				);

			for (size_type i = 0; i < ny && !reset; ++i)
			{
				if (::boost::numeric::ublasx::rcond(Ps_[i]) > check_val)
				{
					reset = true;
				}
			}
		}
		if (reset)
		{
			this->init();
		}

		// Estimate system parameters
DCS_DEBUG_TRACE("BEGIN estimation");//XXX
DCS_DEBUG_TRACE("y(k): " << y);//XXX
DCS_DEBUG_TRACE("u(k): " << u);//XXX
		vector_type y_hat(ny);
		for (size_type i = 0; i < ny; ++i)
		{
DCS_DEBUG_TRACE("theta_hat["<< i << "](k): " << theta_hats_[i]);//XXX
DCS_DEBUG_TRACE("P["<< i << "](k): " << Ps_[i]);//XXX
DCS_DEBUG_TRACE("phi["<< i << "](k): " << phis_[i]);//XXX
			y_hat(i) = ::dcs::sysid::rls_bittanti1990_arx_miso(y(i),
															   u,
															   ff_,
															   na,
															   nb,
															   d,
															   theta_hats_[i],
															   Ps_[i],
															   phis_[i],
															   delta_);
DCS_DEBUG_TRACE("New theta_hat["<< i << "](k): " << theta_hats_[i]);//XXX
DCS_DEBUG_TRACE("New P["<< i << "](k): " << Ps_[i]);//XXX
DCS_DEBUG_TRACE("New rcond(P["<< i << "](k)): " << ::boost::numeric::ublasx::rcond(Ps_[i]));//XXX
DCS_DEBUG_TRACE("New phi["<< i << "](k): " << phis_[i]);//XXX
DCS_DEBUG_TRACE("New e["<< i << "](k): " << (y(i)-y_hat(i)));//XXX
		}
DCS_DEBUG_TRACE("New y_hat(k): " << y_hat);//XXX
DCS_DEBUG_TRACE("END estimation");//XXX

		return y_hat;
	}


	/// Return matrix A_k from \hat{\Theta}.
	private: matrix_type do_A(size_type k) const
	{
		namespace ublas = ::boost::numeric::ublas;

		DCS_DEBUG_ASSERT( k >= 1 && k <= this->output_order() );

		const size_type ny(this->num_outputs());

		// Remember, for each output i=1,...,n_y:
		//   \hat{\theta}_i = [a_{ii}^{1};
		//                     ...;
		//                     a_{ii}^{n_a};
		//                     b_{i1}^{1};
		//                     ...;
		//                     b_{i1}^{n_b};
		//                     ...;
		//                     b_{in_u}^{1};
		//                     ...;
		//                     b_{in_u}^{n_b}]
		// So in \hat{\theta}_i the ith diagonal element of matrix A_k stays at:
		//   A_k(i,i) <- \hat{\theta}_i(k)
		matrix_type A_k(ny, ny, real_type/*zero*/());
		for (size_type i = 0; i < ny; ++i)
		{
			A_k(i,i) = theta_hats_[i](k);
		}

		return A_k;
	}


	/// Return matrix B_k from \hat{\Theta}.
	private: matrix_type do_B(size_type k) const
	{
		namespace ublas = ::boost::numeric::ublas;

		DCS_DEBUG_ASSERT( k >= 1 && k <= this->input_order() );

		const size_type na(this->output_order());
		const size_type nb(this->input_order());
		const size_type ny(this->num_outputs());
		const size_type nu(this->num_inputs());

		// Remember, for each output i=1,...,n_y:
		//   \hat{\theta}_i = [a_{ii}^{1};
		//                     ...;
		//                     a_{ii}^{n_a};
		//                     b_{i1}^{1};
		//                     ...;
		//                     b_{i1}^{n_b};
		//                     ...;
		//                     b_{in_u}^{1};
		//                     ...;
		//                     b_{in_u}^{n_b}]
		// So in \hat{\theta}_i the ith row of matrix B_k stays at:
		//   B_k(i,:) <- (\hat{\theta}_i(((n_a+k):n_b:n_u))^T
		matrix_type B_k(ny, nu);
		for (size_type i = 0; i < ny; ++i)
		{
			ublas::row(B_k, i) = ublas::subslice(theta_hats_[i], na+k-1, nb, nu);
		}

		return B_k;
	}


	/// Forgetting factor.
	private: real_type ff_;
	/// Bittanti's correction factor.
	private: real_type delta_;
	/// Matrix of system parameters estimated by RLS: [A_1 ... A_{n_a} B_1 ... B_{n_b}].
	private: ::std::vector<vector_type> theta_hats_;
	/// The covariance matrix.
	private: ::std::vector<matrix_type> Ps_;
	/// The regression vector.
	private: ::std::vector<vector_type> phis_;
}; // rls_bittanti1990_miso_proxy


#endif // DCS_EESIM_USE_MATLAB_*


template <typename TraitsT>
::dcs::shared_ptr< base_system_identification_strategy<TraitsT> > make_system_identification_strategy(base_system_identification_strategy_params<TraitsT> const& params)
{
	typedef TraitsT traits_type;
	typedef base_system_identification_strategy<traits_type> strategy_type;
	typedef ::dcs::shared_ptr<strategy_type> strategy_pointer;

	strategy_pointer ptr_strategy;

DCS_DEBUG_TRACE("HERE.1 --> " << params.category());//XXX
	switch (params.category())
	{
		case rls_bittanti1990_system_identification_strategy:
			{
				typedef rls_bittanti1990_system_identification_strategy_params<traits_type> const* strategy_params_impl_pointer;

				strategy_params_impl_pointer ptr_params_impl = dynamic_cast<strategy_params_impl_pointer>(&params);
				if (!ptr_params_impl)
				{
					throw ::std::runtime_error("[dcs::eesim::detail::make_system_identification_strategy] Failed to retrieve RLS FF strategy parameters.");
				}
				if (ptr_params_impl->mimo_as_miso())
				{
					typedef rls_bittanti1990_miso_proxy<traits_type> strategy_impl_type;

					ptr_strategy = ::dcs::make_shared<strategy_impl_type>(*ptr_params_impl);
				}
				else
				{
					throw ::std::runtime_error("[dcs::eesim::detail::make_system_identification_strategy] MIMO RLS (Bittanti, 1990) has not been implemented yet.");
//					typedef rls_bittanti1990_mimo_proxy<traits_type> strategy_impl_type;
//
//					ptr_strategy = ::dcs::make_shared<strategy_impl_type>(*ptr_params_impl);
				}
			}
			break;
		case rls_ff_system_identification_strategy:
			{
				typedef rls_ff_system_identification_strategy_params<traits_type> const* strategy_params_impl_pointer;

				strategy_params_impl_pointer ptr_params_impl = dynamic_cast<strategy_params_impl_pointer>(&params);
				if (!ptr_params_impl)
				{
					throw ::std::runtime_error("[dcs::eesim::detail::make_system_identification_strategy] Failed to retrieve RLS FF strategy parameters.");
				}
				if (ptr_params_impl->mimo_as_miso())
				{
					typedef rls_ff_miso_proxy<traits_type> strategy_impl_type;

					ptr_strategy = ::dcs::make_shared<strategy_impl_type>(*ptr_params_impl);
				}
				else
				{
					typedef rls_ff_mimo_proxy<traits_type> strategy_impl_type;

					ptr_strategy = ::dcs::make_shared<strategy_impl_type>(*ptr_params_impl);
				}
			}
			break;
		case rls_kulhavy1984_system_identification_strategy:
			{
				typedef rls_kulhavy1984_system_identification_strategy_params<traits_type> const* strategy_params_impl_pointer;

				strategy_params_impl_pointer ptr_params_impl = dynamic_cast<strategy_params_impl_pointer>(&params);
				if (!ptr_params_impl)
				{
					throw ::std::runtime_error("[dcs::eesim::detail::make_system_identification_strategy] Failed to retrieve RLS FF strategy parameters.");
				}
				if (ptr_params_impl->mimo_as_miso())
				{
					typedef rls_kulhavy1984_miso_proxy<traits_type> strategy_impl_type;

					ptr_strategy = ::dcs::make_shared<strategy_impl_type>(*ptr_params_impl);
				}
				else
				{
					throw ::std::runtime_error("[dcs::eesim::detail::make_system_identification_strategy] MIMO RLS (Kulhavy, 1984) has not been implemented yet.");
//					typedef rls_kulhavy1984_mimo_proxy<traits_type> strategy_impl_type;
//
//					ptr_strategy = ::dcs::make_shared<strategy_impl_type>(*ptr_params_impl);
				}
			}
			break;
		case rls_park1991_system_identification_strategy:
			{
DCS_DEBUG_TRACE("HERE.2");//XXX
				typedef rls_park1991_system_identification_strategy_params<traits_type> const* strategy_params_impl_pointer;

DCS_DEBUG_TRACE("HERE.3");//XXX
				strategy_params_impl_pointer ptr_params_impl = dynamic_cast<strategy_params_impl_pointer>(&params);
				if (!ptr_params_impl)
				{
DCS_DEBUG_TRACE("HERE.4");//XXX
					throw ::std::runtime_error("[dcs::eesim::detail::make_system_identification_strategy] Failed to retrieve RLS FF strategy parameters.");
				}
				if (ptr_params_impl->mimo_as_miso())
				{
DCS_DEBUG_TRACE("HERE.5");//XXX
					typedef rls_park1991_miso_proxy<traits_type> strategy_impl_type;

					ptr_strategy = ::dcs::make_shared<strategy_impl_type>(*ptr_params_impl);
				}
				else
				{
					throw ::std::runtime_error("[dcs::eesim::detail::make_system_identification_strategy] MIMO RLS (Park, 1991) has not been implemented yet.");
//					typedef rls_park1991_mimo_proxy<traits_type> strategy_impl_type;
//
//					ptr_strategy = ::dcs::make_shared<strategy_impl_type>(*ptr_params_impl);
				}
DCS_DEBUG_TRACE("HERE.6");//XXX
			}
			break;
	}

DCS_DEBUG_TRACE("HERE.7 --> " << ptr_strategy);//XXX
	return ptr_strategy;
}

}}} // Namespace dcs::eesim::detail


#endif // DCS_EESIM_DETAIL_SYSTEM_IDENTIFICATION_STRATEGIES_HPP
