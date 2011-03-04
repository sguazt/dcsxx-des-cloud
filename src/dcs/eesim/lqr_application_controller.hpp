/**
 * \file dcs/eesim/lqr_application_controller.hpp
 *
 * \brief Class modeling the application controller component using an LQR
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
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */

#ifndef DCS_EESIM_LQR_APPLICATION_CONTROLLER_HPP
#define DCS_EESIM_LQR_APPLICATION_CONTROLLER_HPP


#include <algorithm>
#ifdef DCS_DEBUG
#	include <boost/numeric/ublas/io.hpp>
#endif // DCS_DEBUG
#include <boost/numeric/ublas/matrix.hpp>
//#include <boost/numeric/ublas/matrix_expression.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublasx/operation/cond.hpp>
#include <boost/numeric/ublasx/operation/num_columns.hpp>
#include <boost/numeric/ublasx/operation/num_rows.hpp>
#include <boost/numeric/ublasx/operation/size.hpp>
//#include <boost/numeric/ublas/vector_expression.hpp>
#include <dcs/control/design/dlqi.hpp>
#include <dcs/control/design/dlqr.hpp>
#include <dcs/control/design/dlqry.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/mean_estimator.hpp>
#include <dcs/eesim/base_application_controller.hpp>
#include <dcs/eesim/multi_tier_application.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/eesim/utility.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <dcs/sysid/algorithm/rls.hpp>
#include <exception>
#include <iostream>
#include <map>
#if defined(DCS_EESIM_USE_MATLAB_MCR)
#	include <matlab/librls.h>
#	include <mclcppclass.h>
#elif defined(DCS_EESIM_USE_MATLAB_APP)
#	include <boost/iostreams/device/file_descriptor.hpp>
#	include <boost/iostreams/stream_buffer.hpp>
#	include <cctype>
#	include <cerrno>
#	include <csignal>
#	include <cstddef>
#	include <cstdlib>
#	include <cstring>
#	include <fcntl.h>
#	include <sstream>
#	include <sys/resource.h>
#	include <sys/time.h>
#	include <sys/types.h>
#	include <sys/wait.h>
#	include <unistd.h>
#endif // DCS_EESIM_USE_MATLAB_*
#include <stdexcept>
#include <vector>


//TODO:
// - Currently the code in this class assumes the single resource (CPU) case.
//


namespace dcs { namespace eesim {

namespace detail { namespace /*<unnamed>*/ {

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
	private: typedef ::boost::numeric::ublas::matrix<real_type> matrix_type;
	private: typedef ::boost::numeric::ublas::vector<real_type> vector_type;
	private: typedef ::std::size_t size_type;

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
::std::cerr << "HERE.1" << ::std::endl;
			rls_miso_init(3, th0, P0, phi0, nu, nn);
::std::cerr << "HERE.2" << ::std::endl;

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
};


#elif defined(DCS_EESIM_USE_MATLAB_APP)


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
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	private: typedef ::boost::numeric::ublas::matrix<real_type> matrix_type;
	private: typedef ::boost::numeric::ublas::vector<real_type> vector_type;
	private: typedef ::std::size_t size_type;

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
			// the child

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
close_fd[STDERR_FILENO] = false;//XXX: keep for debug
			for (int fd = 0; fd < maxdescs; ++fd)
			{
				if (close_fd[fd])
				{
					::close(fd);
				}
			}

//[XXX]
::std::cerr << "Executing MATLAB: " << cmd;//XXX
for (::std::size_t i=0; i < args.size(); ++i)//XXX
{//XXX
::std::cerr << " " << args[i] << ::std::flush;//XXX
}//XXX
::std::cerr << ::std::endl;//XXX
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
		else
		{
			// parent

//			// Associate the parent's stdin to the pipe read fd.
			::close(pipefd[1]);
//			::close(STDIN_FILENO);
//			::dup(pipefd[0]);
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
			::pid_t wait_pid;
			wait_pid = ::wait(&status);
DCS_DEBUG_TRACE("MATLAB exited");//XXX
			if (wait_pid != pid)
			{
				throw ::std::runtime_error("[dcs::eesim::detail::rls_miso_matlab_app_proxy::run_matlab] Unexpected child process.");
			}
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
		}
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
};


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
class rls_ff_mimo_proxy
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	private: typedef ::boost::numeric::ublas::matrix<real_type> matrix_type;
	private: typedef ::boost::numeric::ublas::vector<real_type> vector_type;
	private: typedef ::std::size_t size_type;

	public: rls_ff_mimo_proxy()
	: n_a_(0),
	  n_b_(0),
	  d_(0),
	  n_y_(0),
	  n_u_(0),
	  ff_(0)
	{
	}


	public: rls_ff_mimo_proxy(size_type n_a, size_type n_b, size_type d, size_type n_y, size_type n_u, real_type ff)
	: n_a_(n_a),
	  n_b_(n_b),
	  d_(d),
	  n_y_(n_y),
	  n_u_(n_u),
	  ff_(ff)
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


	public: real_type forgetting_factor() const
	{
		return ff_;
	}


	public: matrix_type Theta_hat() const
	{
		return Theta_hat_;
	}


	public: matrix_type P() const
	{
		return P_;
	}


	public: vector_type phi() const
	{
		return phi_;
	}


	public: void init()
	{
		// Prepare the data structures for the RLS algorithm 
		::dcs::sysid::rls_arx_mimo_init(n_a_,
										n_b_,
										d_,
										n_y_,
										n_u_,
										Theta_hat_,
										P_,
										phi_);

	}


	public: template <typename YVectorExprT, typename UVectorExprT>
		vector_type estimate(::boost::numeric::ublas::vector_expression<YVectorExprT> const& y,
							 ::boost::numeric::ublas::vector_expression<UVectorExprT> const& u)
	{
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
									  n_a_,
									  n_b_,
									  d_,
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
	public: matrix_type A(size_type k) const
	{
		namespace ublas = ::boost::numeric::ublas;

		DCS_DEBUG_ASSERT( k >= 1 && k <= n_a_ );

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
		matrix_type A_k; // (n_y_, n_y_);
		A_k = ublas::trans(ublas::subslice(Theta_hat_, k-1, n_a_, n_y_, 0, 1, n_y_));

		return A_k;
	}


	/// Return matrix B_k from \hat{\Theta}.
	public: matrix_type B(size_type k) const
	{
		namespace ublas = ::boost::numeric::ublas;

		DCS_DEBUG_ASSERT( k >= 1 && k <= n_b_ );

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
		B_k = ublas::trans(ublas::subslice(Theta_hat_, n_a_*n_y_+k-1, n_b_, n_u_, 0, 1, n_y_));

		return B_k;
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
	private: matrix_type Theta_hat_;
	/// The covariance matrix.
	private: matrix_type P_;
	/// The regression vector.
	private: vector_type phi_;
};


/**
 * \brief Proxy to identify a MIMO system model by applying the Recursive Least
 *  Square with forgetting-factor algorithm to several MISO system models.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class rls_ff_miso_proxy
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	private: typedef ::boost::numeric::ublas::matrix<real_type> matrix_type;
	private: typedef ::boost::numeric::ublas::vector<real_type> vector_type;
	private: typedef ::std::size_t size_type;

	public: rls_ff_miso_proxy()
	: n_a_(0),
	  n_b_(0),
	  d_(0),
	  n_y_(0),
	  n_u_(0),
	  ff_(0)
	{
	}


	public: rls_ff_miso_proxy(size_type n_a, size_type n_b, size_type d, size_type n_y, size_type n_u, real_type ff)
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
		// Prepare the data structures for the RLS algorithm 
		for (size_type i = 0; i < n_y_; ++i)
		{
			::dcs::sysid::rls_arx_miso_init(n_a_,
											n_b_,
											d_,
											n_u_,
											theta_hats_[i],
											Ps_[i],
											phis_[i]);
		}
	}


	public: template <typename YVectorExprT, typename UVectorExprT>
		vector_type estimate(::boost::numeric::ublas::vector_expression<YVectorExprT> const& y,
							 ::boost::numeric::ublas::vector_expression<UVectorExprT> const& u)
	{
		// Estimate system parameters
DCS_DEBUG_TRACE("BEGIN estimation");//XXX
DCS_DEBUG_TRACE("y(k): " << y);//XXX
DCS_DEBUG_TRACE("u(k): " << u);//XXX
		vector_type y_hat(n_y_);
		for (size_type i = 0; i < n_y_; ++i)
		{
DCS_DEBUG_TRACE("theta_hat["<< i << "](k): " << theta_hats_[i]);//XXX
DCS_DEBUG_TRACE("P["<< i << "](k): " << Ps_[i]);//XXX
DCS_DEBUG_TRACE("phi["<< i << "](k): " << phis_[i]);//XXX
			y_hat(i) = ::dcs::sysid::rls_ff_arx_miso(y()(i),
													 u,
													 ff_,
													 n_a_,
													 n_b_,
													 d_,
													 theta_hats_[i],
													 Ps_[i],
													 phis_[i]);
DCS_DEBUG_TRACE("New theta_hat["<< i << "](k): " << theta_hats_[i]);//XXX
DCS_DEBUG_TRACE("New P["<< i << "](k): " << Ps_[i]);//XXX
DCS_DEBUG_TRACE("New cond(P["<< i << "](k)): " << ::boost::numeric::ublasx::cond(Ps_[i]));//XXX
DCS_DEBUG_TRACE("New phi["<< i << "](k): " << phis_[i]);//XXX
DCS_DEBUG_TRACE("New e["<< i << "](k): " << (y()(i)-y_hat(i)));//XXX
		}
DCS_DEBUG_TRACE("New y_hat(k): " << y_hat);//XXX
DCS_DEBUG_TRACE("END estimation");//XXX

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
};

#endif // DCS_EESIM_USE_MATLAB_*


template <
//	typename TraitsT,
	typename RLSProxyT,
	typename AMatrixExprT,
	typename BMatrixExprT,
	typename CMatrixExprT,
	typename DMatrixExprT
>
//void make_ss(rls_ff_mimo_proxy<TraitsT> const& rls_proxy,
void make_ss(RLSProxyT const& rls_proxy,
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

	const size_type rls_n_a(rls_proxy.output_order());
	const size_type rls_n_b(rls_proxy.input_order());
//	const size_type rls_d(rls_proxy.input_delay());
	const size_type rls_n_y(rls_proxy.num_outputs());
	const size_type rls_n_u(rls_proxy.num_inputs());
	const size_type n_x(rls_n_a*rls_n_y);
	const size_type n_u(rls_n_b*rls_n_u);
//	const size_type n(::std::max(n_x,n_u));
	const size_type n_y(1);

	// Create the state matrix A
	// A=[0	0 0 ... 0;
	//    .	. . ... .
	//    .	. . ... .
	//    .	. . ... .
	//    0	0 0 ... 0;
	//    0	I 0 ... 0;
	// 	  0	0 I ... 0;
	//    .	. . ... .
	//    .	. . ... .
	//    .	. . ... .
	// 	  0	0 0 ... I;
	// 	  -A_{n_a} -A_{n_a-1} -A_{n_a-2}... -A_1]
	{
		size_type broffs(n_x-rls_n_y); // The bottom row offset
//		size_type troffs((n_x < n_u) ? (n_u-n_x) : 0); // The topmost row offset

		A().resize(n_x, n_x, false);

//		// The upper part of A is set to [0_{k1,n}; 0_{k2,rls_n_y} I_{k2,k2}],
//		// where: k1=n-((n_u > n_x)?(n_u-n_x):0), and k2=n-rls_n_y.
//		if (troffs > 0)
//		{
//			ublas::subrange(A(), 0, troffs, 0, n) = ublas::zero_matrix<value_type>(troffs,n);
//		}
//		ublas::subrange(A(), troffs, broffs, 0, rls_n_y) = ublas::zero_matrix<value_type>(broffs,rls_n_y);
//		ublas::subrange(A(), troffs, broffs, rls_n_y, n) = ublas::identity_matrix<value_type>(broffs,broffs);
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

			//ublas::subrange(A(), broffs, n_x, c1, c2) = rls_proxy.A(i+1);
			ublas::subrange(A(), broffs, n_x, c1, c2) = -rls_proxy.A(i+1);
		}
	}
//DCS_DEBUG_TRACE("A="<<A);//XXX

	// Create the input matrix B
	// B=[0 ... 0;
	//    .	... .
	//    .	... .
	//    .	... .
	//    0 ... 0;
	//    B_{n_b} ... B_1]
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

			ublas::subrange(B(), broffs, n_x, c1, c2) = rls_proxy.B(i+1);
		}
	}
//DCS_DEBUG_TRACE("B="<<B);//XXX

	// Create the output matrix C
	{
		size_type rcoffs(n_x-rls_n_y); // The right most column offset

		C().resize(n_y, n_x, false);

		ublas::subrange(C(), 0, n_y, 0, rcoffs) = ublas::zero_matrix<value_type>(n_y,rcoffs);
		ublas::subrange(C(), 0, n_y, rcoffs, n_x) = ublas::scalar_matrix<value_type>(n_y, rls_n_y, 1);
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
	private: typedef ::dcs::shared_ptr<statistic_type> statistic_pointer;
	private: typedef ::std::map<performance_measure_category,statistic_pointer> category_statistic_container;
	private: typedef ::std::vector<category_statistic_container> category_statistic_container_container;
	private: typedef ::std::map<performance_measure_category,real_type> category_value_container;
	private: typedef ::std::vector<category_value_container> category_value_container_container;
	private: typedef physical_machine<traits_type> physical_machine_type;
#if defined(DCS_EESIM_USE_MATLAB_MCR)
	private: typedef detail::rls_ff_miso_matlab_mcr_proxy<traits_type> rls_ff_proxy_type;
#elif defined(DCS_EESIM_USE_MATLAB_APP)
	private: typedef detail::rls_ff_miso_matlab_app_proxy<traits_type> rls_ff_proxy_type;
#else
//	private: typedef detail::rls_ff_mimo_proxy<traits_type> rls_ff_proxy_type;
	private: typedef detail::rls_ff_miso_proxy<traits_type> rls_ff_proxy_type;
#endif // DCS_EESIM_USE_MATLAB_*


	private: static const size_type default_input_order_;
	private: static const size_type default_output_order_;
	private: static const size_type default_input_delay_;
//	private: static const uint_type default_ss_state_size_;
//	private: static const uint_type default_ss_input_size_;
//	private: static const uint_type default_ss_output_size_;
	private: static const real_type default_min_share_;
	public: static const real_type default_rls_forgetting_factor;
	public: static const real_type default_ewma_smoothing_factor;


	public: lq_application_controller()
	: base_type(),
//	  controller_(),
//	  Theta_hat_(),
//	  P_(),
//	  phi_()
	  n_a_(default_output_order_),
	  n_b_(default_input_order_),
	  d_(default_input_delay_),
	  n_p_(0),
	  n_s_(0),
	  n_x_(0),
	  n_y_(0),
	  n_u_(0),
	  rls_ff_(default_rls_forgetting_factor),
	  ewma_smooth_(default_ewma_smoothing_factor),
	  x_offset_(0),
	  u_offset_(0),
	  count_(0)
	{
		init();
	}


	public: lq_application_controller(application_pointer const& ptr_app, real_type ts)
	: base_type(ptr_app, ts),
//	  controller_(),
	  n_a_(default_output_order_),
	  n_b_(default_input_order_),
	  d_(default_input_delay_),
	  n_p_(0),
	  n_s_(0),
	  n_x_(0),
	  n_y_(0),
	  n_u_(0),
	  rls_ff_(default_rls_forgetting_factor),
	  ewma_smooth_(default_ewma_smoothing_factor),
	  x_offset_(0),
	  u_offset_(0),
	  count_(0)
	{
		init();
	}


	public: //template <typename QMatrixExprT, typename RMatrixExprT>
		lq_application_controller(uint_type n_a,
								   uint_type n_b,
								   uint_type d,
//								   ::boost::numeric::ublas::matrix_expression<QMatrixExprT> const& Q,
//								   ::boost::numeric::ublas::matrix_expression<RMatrixExprT> const& R,
								   application_pointer const& ptr_app,
								   real_type ts,
								   real_type rls_forgetting_factor/* = default_rls_forgetting_factor*/,
								   real_type ewma_smoothing_factor/* = default_ewma_smoothing_factor*/)
	: base_type(ptr_app, ts),
//	  controller_(Q, R),
	  n_a_(n_a),
	  n_b_(n_b),
	  d_(d),
	  n_p_(0),
	  n_s_(0),
	  n_x_(0),
	  n_y_(0),
	  n_u_(0),
	  rls_ff_(rls_forgetting_factor),
	  ewma_smooth_(ewma_smoothing_factor),
	  x_offset_(0),
	  u_offset_(0),
	  count_(0)
	{
		init();
	}


//	public: //template <typename QMatrixExprT, typename RMatrixExprT, typename NMatrixExprT>
//		lq_application_controller(uint_type n_a,
//								   uint_type n_b,
//								   uint_type d,
////								   ::boost::numeric::ublas::matrix_expression<QMatrixExprT> const& Q,
////								   ::boost::numeric::ublas::matrix_expression<RMatrixExprT> const& R,
////								   ::boost::numeric::ublas::matrix_expression<NMatrixExprT> const& N,
//								   application_pointer const& ptr_app,
//								   real_type ts,
//								   real_type rls_forgetting_factor/* = default_rls_forgetting_factor*/,
//								   real_type ewma_smoothing_factor/* = default_ewma_smoothing_factor*/)
//	: base_type(ptr_app, ts),
//	  controller_(Q, R, N),
//	  n_a_(n_a),
//	  n_b_(n_b),
//	  d_(d),
//	  n_p_(0),
//	  n_s_(0),
//	  n_x_(0),
//	  n_y_(0),
//	  n_u_(0),
//	  rls_ff_(rls_forgetting_factor),
//	  ewma_smooth_(ewma_smoothing_factor),
//	  x_offset_(0),
//	  u_offset_(0),
//	  count_(0)
//	{
//		init();
//	}


	public: virtual ~lq_application_controller()
	{
		this->disconnect_from_event_sources();
	}


	private: void init()
	{
		init_measures();

		connect_to_event_sources();
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
		typedef ::dcs::des::mean_estimator<real_type,uint_type> statistic_impl_type;

		if (this->application_ptr())
		{
			//FIXME: statistic category (e.g., mean) is hard-coded

			uint_type num_tiers(this->application().num_tiers());

			tier_measures_.resize(num_tiers);

			typedef typename perf_category_container::const_iterator category_iterator;

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
			n_x_ = n_p_*n_a_;
			n_u_ = n_s_*n_b_;
			n_y_ = uint_type(1);
			x_offset_ = n_x_-n_p_;
			u_offset_ = n_u_-n_s_;
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

			// Apply the EWMA filter to previously observed measurements
			//real_type ewma_old_s(ewma_s_.at(category));
			ewma_s_[category] = ewma_smooth_*ptr_stat->estimate() + (1-ewma_smooth_)*ewma_s_.at(category);

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
				ewma_tier_s_[tier_id][category] = ewma_smooth_*ptr_stat->estimate() + (1-ewma_smooth_)*ewma_tier_s_[tier_id].at(category);

				// Reset stat and set as the first observation a memory of the past
				ptr_stat->reset();
				(*ptr_stat)(ewma_tier_s_[tier_id].at(category));
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

		count_ = size_type/*zero*/();
		x_ = vector_type(n_x_, 0);
		u_ = vector_type(n_u_, 0);
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
//					throw ::std::runtime_error("[dcs::eesim::lqr_application_controller::process_request_departure] LQR application controller currently handles only the response-time category.");
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
//					throw ::std::runtime_error("[dcs::eesim::lqr_application_controller::process_tier_request_departure] LQR application controller currently handles only the response-time category.");
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

		real_type app_rt(0);

		size_type num_tiers(tier_measures_.size());

		for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));
//			physical_machine_type const& actual_pm(ptr_vm->vmm().hosting_machine());
//			physical_resource_category res_category(cpu_resource_category);//FIXME

			// Actual-to-reference scaling factor
//			real_type scale_factor(1);
//			scale_factor = ::dcs::eesim::resource_scaling_factor(
//					// Actual resource capacity and threshold
//					actual_pm.resource(res_category)->capacity(),
//					actual_pm.resource(res_category)->utilization_threshold(),
//					// Reference resource capacity and threshold
//					app.reference_resource(res_category).capacity(),
//					app.reference_resource(res_category).utilization_threshold()
//				);

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
DCS_DEBUG_TRACE("HERE!!!!! tier: " << tier_id << " ==> rt: " << rt << " (aggregated: " << ptr_stat->estimate() << ")");//XXX
								(*ptr_stat)(rt);
								app_rt += rt;
							}
						}
						break;
					default:
						throw ::std::runtime_error("[dcs::eesim::lqr_application_controller::process_request_departure] LQR application controller currently handles only the response-time category.");
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
						//real_type rt(req.departure_time()-req.arrival_time());
						//rt *= scale_factor;
						//(*ptr_stat)(rt);
						(*ptr_stat)(app_rt);
DCS_DEBUG_TRACE("HERE!!!!! app ==> rt: " << app_rt << " (aggregated: " << ptr_stat->estimate() << ")");//XXX
					}
					break;
				default:
					throw ::std::runtime_error("[dcs::eesim::lqr_application_controller::process_request_departure] LQR application controller currently handles only the response-time category.");
			}
		}

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
//		::dcs::sysid::rls_arx_mimo_init(n_a_,
//										n_b_,
//										d_,
//										n_p_,
//										n_s_,
//										rls_Theta_hat_,
//										rls_P_,
//										rls_phi_);
		rls_ff_proxy_ = rls_ff_proxy_type(n_a_, n_b_, d_, n_p_, n_s_, rls_ff_);
		rls_ff_proxy_.init();

		// Completely reset all measures
		full_reset_measures();

		DCS_DEBUG_TRACE("(" << this << ") END Do Process SYSTEM-INITIALIZATION event (Clock: " << ctx.simulated_time() << ")");
	}


	private: void do_process_control(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		namespace ublas = ::boost::numeric::ublas;

		typedef typename base_type::application_type application_type;
		typedef typename application_type::simulation_model_type application_simulation_model_type;
		typedef typename application_type::performance_model_type application_performance_model_type;
		typedef typename perf_category_container::const_iterator category_iterator;
//		typedef typename category_statistic_container_container::const_iterator tier_iterator;

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Process CONTROL event (Clock: " << ctx.simulated_time() << " - Count: " << count_ << ")");

		application_type const& app(this->application());
		application_simulation_model_type const& app_sim_model(app.simulation_model());
		application_performance_model_type const& app_perf_model(app.performance_model());

		size_type num_tiers(tier_measures_.size());
		vector_type s(n_s_,0); // control input (relative error of tier resource shares)
		vector_type p(n_p_,0); // control state (relative error of tier peformance measures)
		vector_type y(n_y_,0); // control output (relative error of app peformance measures)


		++count_;

		// Rotate old with new inputs/outputs:
		//  x = [p(k-n_a+1) ... p(k)]^T
		//    = [x_{n_p:n_x} p(k)]^T
		//  u = [s(k-n_b+1) ... s(k)]^T
		//    = [u_{n_s:n_u} s(k)]^T
		// Check if a measure rotation is needed (always but the first time)
		if (count_ > 1)
		{
			// throw away old observations from x and make space for new ones.
			//detail::rotate(x_, n_a_, n_p_);
			ublas::subrange(x_, 0, (n_a_-1)*n_p_) = ublas::subrange(x_, n_p_, n_x_);
			// throw away old observations from u and make space for new ones.
			//detail::rotate(u_, n_b_, n_s_);
			ublas::subrange(u_, 0, (n_b_-1)*n_s_) = ublas::subrange(u_, n_s_, n_u_);
		}


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

			real_type ref_measure;
			real_type actual_measure;

			ref_measure = app.sla_cost_model().slo_value(category);
			if (ptr_stat->num_observations() > 0)
			{
				actual_measure = ptr_stat->estimate();
			}
			else
			{
				// No observation -> Assume perfect behavior
				actual_measure = ref_measure;
			}
DCS_DEBUG_TRACE("APP OBSERVATION: ref: " << ref_measure << " - actual: " << actual_measure);//XXX
			y(0) = actual_measure/ref_measure - real_type(1);

			for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
			{
				switch (category)
				{
					case response_time_performance_measure:
							{
								ptr_stat = tier_measures_[tier_id].at(category);

								ref_measure = app_perf_model.tier_measure(tier_id, category);
								if (ptr_stat->num_observations() > 0)
								{
									actual_measure = ptr_stat->estimate();
								}
								else
								{
									// No observation -> Assume perfect behavior
									actual_measure = ref_measure;
								}
DCS_DEBUG_TRACE("TIER " << tier_id << " OBSERVATION: ref: " << ref_measure << " - actual: " << actual_measure);//XXX
								x_(x_offset_+tier_id) = p(tier_id)
													  = actual_measure/ref_measure - real_type(1);
							}
							break;
					default:
						throw ::std::runtime_error("[dcs::eesim::lqr_application_controller::do_process_control] LQR application controller currently handles only the response-time category.");
				}
			}
		}

//FIXME: resource category is actually hard-coded to CPU
		// Collect new input observations:
		// u_j(k) = (<actual-resource-share-at-tier-j>-<ref-resource-share-at-tier-j>)/<ref-resource-share-at-tier-j>
		for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			virtual_machine_pointer ptr_vm = app_sim_model.tier_virtual_machine(tier_id);
			physical_machine_type const& actual_pm(ptr_vm->vmm().hosting_machine());
			physical_resource_category res_category(cpu_resource_category);//FIXME

			// Get the reference resource share for the tier from the application specs
//			//real_type ref_share(ptr_vm->wanted_resource_share(ptr_vm->vmm().hosting_machine(), category));
//			//real_type ref_share(ptr_vm->wanted_resource_share(res_category));
			real_type ref_share(ptr_vm->guest_system().resource_share(res_category));

			// Get the actual resource share from the VM and scale w.r.t. the reference machine
			real_type actual_share;
			actual_share = ::dcs::eesim::scale_resource_share(actual_pm.resource(res_category)->capacity(),
															  actual_pm.resource(res_category)->utilization_threshold(),
															  app.reference_resource(res_category).capacity(),
															  app.reference_resource(res_category).utilization_threshold(),
															  ptr_vm->resource_share(res_category));

			//FIXME: should u contain relative errore w.r.t. resource share given from performance model?
DCS_DEBUG_TRACE("TIER " << tier_id << " SHARE: ref: " << ref_share << " - actual: " << ptr_vm->resource_share(res_category) << " - actual-scaled: " << actual_share);//XXX
			u_(u_offset_+tier_id) = s(tier_id)
								  = actual_share/ref_share - real_type(1);
		}

		// Estimate system parameters
		vector_type p_hat;
		p_hat = rls_ff_proxy_.estimate(p, s);
DCS_DEBUG_TRACE("RLS estimation:");//XXX
DCS_DEBUG_TRACE("p=" << p);//XXX
DCS_DEBUG_TRACE("s=" << s);//XXX
DCS_DEBUG_TRACE("p_hat=" << p_hat);//XXX
DCS_DEBUG_TRACE("Theta_hat=" << rls_ff_proxy_.Theta_hat());//XXX
DCS_DEBUG_TRACE("P=" << rls_ff_proxy_.P());//XXX
DCS_DEBUG_TRACE("phi=" << rls_ff_proxy_.phi());//XXX

		// Check if RLS (and LQR) can be applied.
		// If not, then no control is performed.
//		if (count_ >= ::std::max(n_a_,n_b_))
		if (count_ > ::std::min(n_a_,n_b_))
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

			detail::make_ss(rls_ff_proxy_, A, B, C, D);

			// Compute the optimal control
DCS_DEBUG_TRACE("Solving LQR with");//XXX
DCS_DEBUG_TRACE("A=" << A);//XXX
DCS_DEBUG_TRACE("B=" << B);//XXX
DCS_DEBUG_TRACE("C=" << C);//XXX
DCS_DEBUG_TRACE("D=" << D);//XXX
DCS_DEBUG_TRACE("y= " << y);//XXX
DCS_DEBUG_TRACE("x= " << x_);//XXX
DCS_DEBUG_TRACE("u= " << u_);//XXX
			bool ok(true);
			vector_type opt_u;
			try
			{
				opt_u = this->do_optimal_control(x_, u_, y, A, B, C, D);
			}
			catch (::std::exception const& e)
			{
				::std::clog << "[Warning] Unable to compute control input: " << e.what() << "." << ::std::endl;
				DCS_DEBUG_TRACE( "Caught exception: " << e.what() );

				ok = false;
			}
//			try
//			{
//				controller_.solve(A, B, C, D);
//			}
//			catch (::std::exception const& e)
//			{
//				::std::clog << "[Warning] Unable to compute control input." << ::std::endl;
//				DCS_DEBUG_TRACE( "Caught exception: " << e.what() );
//
//				ok = false;
//			}
			if (ok)
			{
DCS_DEBUG_TRACE("Solved!");//XXX
DCS_DEBUG_TRACE("Optimal Control u*=> " << opt_u);//XXX
DCS_DEBUG_TRACE("Expected application response time: " << (app.sla_cost_model().slo_value(response_time_performance_measure)+(ublas::prod(C, ublas::prod(A,x_)+ublas::prod(B,opt_u))+ublas::prod(D,opt_u))(0)));//XXX

DCS_DEBUG_TRACE("Applying optimal control");//XXX
				for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
				{
					physical_resource_category res_category(cpu_resource_category);//FIXME

					virtual_machine_pointer ptr_vm(app_sim_model.tier_virtual_machine(tier_id));
					physical_machine_type const& pm(ptr_vm->vmm().hosting_machine());
					real_type ref_share(ptr_vm->guest_system().resource_share(res_category));
					real_type actual_share;
					actual_share = ::dcs::eesim::scale_resource_share(pm.resource(res_category)->capacity(),
																	  pm.resource(res_category)->utilization_threshold(),
																	  app.reference_resource(res_category).capacity(),
																	  app.reference_resource(res_category).utilization_threshold(),
																	  ptr_vm->resource_share(res_category));


					real_type new_share;
DCS_DEBUG_TRACE("Tier " << tier_id << " --> Actual share: " << actual_share);//XXX
DCS_DEBUG_TRACE("Tier " << tier_id << " --> New Unscaled share: " << (ref_share/(opt_u(u_offset_+tier_id)+real_type(1))));//XXX
//DCS_DEBUG_TRACE("Tier " << tier_id << " --> Unscaled share: " << (actual_share/(opt_u(tier_id)+1)));//XXX
					new_share = ::dcs::eesim::scale_resource_share(
									// Reference resource capacity and threshold
									app.reference_resource(res_category).capacity(),
									app.reference_resource(res_category).utilization_threshold(),
									// Actual resource capacity and threshold
									pm.resource(res_category)->capacity(),
									pm.resource(res_category)->utilization_threshold(),
									//// Old resource share + computed deviation
									//ptr_vm->wanted_resource_share(res_category)+opt_u(tier_id)
									ref_share*(opt_u(u_offset_+tier_id)+real_type(1))
//									actual_share/(opt_u(tier_id)+1)
						);
DCS_DEBUG_TRACE("Tier " << tier_id << " --> New Scaled share: " << new_share);//XXX

					if (new_share >= 0)
					{
						new_share = ::std::max(new_share, default_min_share_);
					}
					else
					{
						new_share = actual_share;
					}

DCS_DEBUG_TRACE("Assigning new wanted share: VM: " << ptr_vm->name() << " (" << ptr_vm->id() << ") - Category: " << res_category << " ==> Share: " << new_share);//XXX
					ptr_vm->wanted_resource_share(res_category, new_share);
				}
DCS_DEBUG_TRACE("Optimal control applied");//XXX
			}
		}

		// Reset previously collected system measure in order to collect a new ones.
		reset_measures();

		DCS_DEBUG_TRACE("(" << this << ") END Do Process CONTROL event (Clock: " << ctx.simulated_time() << " - Count: " << count_ << ")");
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
	/// Forgetting factor used in the RLS algorithm.
	private: real_type rls_ff_;
	/// Smoothing factor for the EWMA filter
	private: real_type ewma_smooth_;
	private: size_type x_offset_;
	private: size_type u_offset_;
	private: size_type count_;
	private: vector_type x_;
	private: vector_type u_;
	/// System-level measures collected during the last control interval.
	private: category_statistic_container measures_;
	/// Tier-level measures collected during the last control interval.
	private: category_statistic_container_container tier_measures_;
	private: category_value_container ewma_s_;
	private: category_value_container_container ewma_tier_s_;
	private: rls_ff_proxy_type rls_ff_proxy_;
};


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
const typename lq_application_controller<TraitsT>::real_type lq_application_controller<TraitsT>::default_min_share_ = 0.01;


template <typename TraitsT>
const typename lq_application_controller<TraitsT>::real_type lq_application_controller<TraitsT>::default_rls_forgetting_factor = 0.98;


template <typename TraitsT>
const typename lq_application_controller<TraitsT>::real_type lq_application_controller<TraitsT>::default_ewma_smoothing_factor = 0.7;

}} // Namespace detail::<unnamed>


/**
 * \brief Application controller based on the Linear-Quadratic-Integrator
 *  control.
 * 
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class lqi_application_controller: public detail::lq_application_controller<TraitsT>
{
	private: typedef detail::lq_application_controller<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename base_type::application_pointer application_pointer;
	private: typedef ::dcs::control::dlqi_controller<real_type> lq_controller_type;
	private: typedef typename base_type::vector_type vector_type;
	private: typedef typename base_type::matrix_type matrix_type;


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
								   real_type rls_forgetting_factor/* = base_type::default_rls_forgetting_factor*/,
								   real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
	: base_type(n_a, n_b, d, ptr_app, ts, rls_forgetting_factor, ewma_smoothing_factor),
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
								   real_type rls_forgetting_factor/* = base_type::default_rls_forgetting_factor*/,
								   real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
	: base_type(n_a, n_b, d, ptr_app, ts, rls_forgetting_factor, ewma_smoothing_factor),
	  controller_(Q, R, N),
	  xi_(1,0)
	{
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
};


template <typename TraitsT>
class lqr_application_controller: public detail::lq_application_controller<TraitsT>
{
	private: typedef detail::lq_application_controller<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename base_type::application_pointer application_pointer;
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
								   real_type rls_forgetting_factor/* = base_type::default_rls_forgetting_factor*/,
								   real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
	: base_type(n_a, n_b, d, ptr_app, ts, rls_forgetting_factor, ewma_smoothing_factor),
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
								   real_type rls_forgetting_factor/* = base_type::default_rls_forgetting_factor*/,
								   real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
	: base_type(n_a, n_b, d, ptr_app, ts, rls_forgetting_factor, ewma_smoothing_factor),
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
};


template <typename TraitsT>
class lqry_application_controller: public detail::lq_application_controller<TraitsT>
{
	private: typedef detail::lq_application_controller<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename base_type::application_pointer application_pointer;
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
								    real_type rls_forgetting_factor/* = base_type::default_rls_forgetting_factor*/,
								    real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
	: base_type(n_a, n_b, d, ptr_app, ts, rls_forgetting_factor, ewma_smoothing_factor),
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
								    real_type rls_forgetting_factor/* = base_type::default_rls_forgetting_factor*/,
								    real_type ewma_smoothing_factor/* = base_type::default_ewma_smoothing_factor*/)
	: base_type(n_a, n_b, d, ptr_app, ts, rls_forgetting_factor, ewma_smoothing_factor),
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
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_LQR_APPLICATION_CONTROLLER_HPP
