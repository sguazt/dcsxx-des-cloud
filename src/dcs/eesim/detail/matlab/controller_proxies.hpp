/**
 * \file src/dcs/eesim/detail/matlab/controller_proxies.hpp
 *
 * \brief Proxies class for accessing to controllers implemented in MATLAB.
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

#ifndef DCS_EESIM_DETAIL_MATLAB_CONTROLLER_PROXIES_HPP
#define DCS_EESIM_DETAIL_MATLAB_CONTROLLER_PROXIES_HPP


#include <boost/numeric/ublas/expression_types.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <boost/numeric/ublasx/operation/num_columns.hpp>
#include <boost/numeric/ublasx/operation/num_rows.hpp>
#include <boost/numeric/ublasx/operation/size.hpp>
#include <boost/numeric/ublasx/operation/size.hpp>
#include <cstddef>
#include <dcs/eesim/detail/matlab/utility.hpp>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>


namespace dcs { namespace eesim { namespace detail { namespace matlab {


namespace /*<unnamed>*/ {

template <typename ValueT>
class lq_matlab_output_consumer
{
	public: typedef ValueT value_type;
	public: typedef ::boost::numeric::ublas::matrix<value_type> matrix_type;
	public: typedef ::boost::numeric::ublas::vector<value_type> vector_type;


	public: template <typename CharT, typename CharTraitsT>
		void operator()(::std::basic_istream<CharT,CharTraitsT>& is)
	{
		success_ = true;

		// Read from the stdin
DCS_DEBUG_TRACE("BEGIN parsing MATLAB output");//XXX
		bool parse_line(false);
		while (is.good())
		{
			::std::string line;
			::std::getline(is, line);
DCS_DEBUG_TRACE("Read from MATLAB --> " << line);//XXX
::std::cerr << "Read from MATLAB --> " << line << ::std::endl;//XXX

			if (line.find("???") != ::std::string::npos || line.find("Error:") != ::std::string::npos)
			{
				DCS_DEBUG_TRACE("An error is occurred while executing MATLAB");//XXX
				success_ = false;
				break;
			}

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
					if ((pos = line.find("K=")) != ::std::string::npos)
					{
						parse_str(line.substr(pos+2), K_);
DCS_DEBUG_TRACE("Parsed as K=" << K_);//XXX
::std::cerr << "Parsed as K=" << K_ << ::std::endl;//XXX
					}
					else if ((pos = line.find("S=")) != ::std::string::npos)
					{
						parse_str(line.substr(pos+2), S_);
DCS_DEBUG_TRACE("Parsed as S=" << S_);//XXX
::std::cerr << "Parsed as S=" << S_ << ::std::endl;//XXX
					}
					else if ((pos = line.find("e=")) != ::std::string::npos)
					{
						parse_str(line.substr(pos+2), e_);
DCS_DEBUG_TRACE("Parsed as e=" << e_);//XXX
::std::cerr << "Parsed as e=" << e_ << ::std::endl;//XXX
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
	}


	public: bool success() const
	{
		return success_;
	}


	public: matrix_type const& K() const
	{
		return K_;
	}


	public: matrix_type const& S() const
	{
		return S_;
	}


	public: vector_type const& e() const
	{
		return e_;
	}


	private: bool success_;
	private: matrix_type K_;
	private: matrix_type S_;
	private: vector_type e_;
}; // lq_matlab_output_consumer

} // Namespace <unnamed>


template <typename RealT>
class dlqi_controller_proxy
{
	public: typedef RealT real_type;
	public: typedef ::boost::numeric::ublas::matrix<real_type> matrix_type;
	public: typedef ::boost::numeric::ublas::vector<real_type> vector_type;
	public: typedef ::std::size_t size_type;


	public: template <
				typename AMatrixT,
				typename BMatrixT,
				typename CMatrixT,
				typename DMatrixT,
				typename QMatrixT,
				typename RMatrixT,
				typename NMatrixT
		> dlqi_controller_proxy(::boost::numeric::ublas::matrix_expression<AMatrixT> const& A,
								::boost::numeric::ublas::matrix_expression<BMatrixT> const& B,
								::boost::numeric::ublas::matrix_expression<CMatrixT> const& C,
								::boost::numeric::ublas::matrix_expression<DMatrixT> const& D,
								real_type ts,
								::boost::numeric::ublas::matrix_expression<QMatrixT> const& Q,
								::boost::numeric::ublas::matrix_expression<RMatrixT> const& R,
								::boost::numeric::ublas::matrix_expression<NMatrixT> const& N)
		: Q_(Q),
		  R_(R),
		  N_(N)
	{
		solve(A, B, C, D, ts);
	}


	public: template <
				typename AMatrixT,
				typename BMatrixT,
				typename CMatrixT,
				typename DMatrixT,
				typename QMatrixT,
				typename RMatrixT
		> dlqi_controller_proxy(::boost::numeric::ublas::matrix_expression<AMatrixT> const& A,
								::boost::numeric::ublas::matrix_expression<BMatrixT> const& B,
								::boost::numeric::ublas::matrix_expression<CMatrixT> const& C,
								::boost::numeric::ublas::matrix_expression<DMatrixT> const& D,
								real_type ts,
								::boost::numeric::ublas::matrix_expression<QMatrixT> const& Q,
								::boost::numeric::ublas::matrix_expression<RMatrixT> const& R)
		: Q_(Q),
		  R_(R),
		  N_(::boost::numeric::ublas::zero_matrix<real_type>(
					::boost::numeric::ublasx::num_rows(Q),
					::boost::numeric::ublasx::num_rows(R)
				)
			)
	{
		solve(A, B, C, D, ts);
	}


	public: template <
				typename QMatrixT,
				typename RMatrixT,
				typename NMatrixT
		> dlqi_controller_proxy(::boost::numeric::ublas::matrix_expression<QMatrixT> const& Q,
								::boost::numeric::ublas::matrix_expression<RMatrixT> const& R,
								::boost::numeric::ublas::matrix_expression<NMatrixT> const& N)
		: Q_(Q),
		  R_(R),
		  N_(N)
	{
	}


	public: template <
				typename QMatrixT,
				typename RMatrixT
		> dlqi_controller_proxy(::boost::numeric::ublas::matrix_expression<QMatrixT> const& Q,
								::boost::numeric::ublas::matrix_expression<RMatrixT> const& R)
		: Q_(Q),
		  R_(R),
		  N_(::boost::numeric::ublas::zero_matrix<real_type>(
					::boost::numeric::ublasx::num_rows(Q),
					::boost::numeric::ublasx::num_rows(R)
				)
			)
	{
	}


	public: template <typename AMatrixT,
					  typename BMatrixT,
					  typename CMatrixT,
					  typename DMatrixT>
		void solve(::boost::numeric::ublas::matrix_expression<AMatrixT> const& A,
				   ::boost::numeric::ublas::matrix_expression<BMatrixT> const& B,
				   ::boost::numeric::ublas::matrix_expression<CMatrixT> const& C,
				   ::boost::numeric::ublas::matrix_expression<DMatrixT> const& D,
					real_type ts)
	{
		// Prepare the MATLAB command
		::std::vector< ::std::string > args;
		args.push_back("-nodisplay");
		args.push_back("-nojvm");
		::std::ostringstream oss;
		oss << "-r \"try "
			<< "[K,S,e]=lqi("
			<< "ss("
			<< to_str(A)
			<< "," << to_str(B)
			<< "," << to_str(C)
			<< "," << to_str(D)
			<< "," << ts
			<< ")"
			<< "," << to_str(Q_)
			<< "," << to_str(R_)
			<< "," << to_str(N_)
			<< "); format long;"
			<< " disp('--- [eesim] ---');"
			<< " disp(['K=', mat2str(K), ]);"
			<< " disp(['S=', mat2str(S), ]);"
			<< " disp(['e=', mat2str(e), ]);"
			<< " disp('--- [/eesim] ---');"
			<< "catch me, "
			<< "disp(['??? Error: ', me.message]);"
			<< "end;"
			<< " quit force\"";
		args.push_back(oss.str());

		// Run MATLAB and retrieve its results
		bool ok;
		lq_matlab_output_consumer<real_type> consumer;
		ok = run_matlab_command(find_matlab_command(), args, consumer);
		if (!ok || !consumer.success())
		{
			throw ::std::runtime_error("[dcs::eesim::detail::matlab::dlqi_controller_proxy::solve] Wrong state dimensiion.");
		}
		K_ = consumer.K();
		S_ = consumer.S();
		e_ = consumer.e();
	}


    public: template <typename VectorExprT>
        vector_type control(::boost::numeric::ublas::vector_expression<VectorExprT> const& x) const
    {
		// preconditions: size(x) == num_columns(K_)
		DCS_ASSERT(
			::boost::numeric::ublasx::size(x) == ::boost::numeric::ublasx::num_columns(K_),
			throw ::std::invalid_argument("[dcs::eesim::detail::matlab::dlqi_controller_proxy::control] Wrong state dimensiion.")
		);

		return -::boost::numeric::ublas::prod(K_, x);
	}
 

    public: template <typename MatrixExprT>
        matrix_type control(::boost::numeric::ublas::matrix_expression<MatrixExprT> const& X) const
    {
		// preconditions: num_columns(X) == num_columns(K_)
		DCS_ASSERT(
			::boost::numeric::ublasx::num_columns(X) == ::boost::numeric::ublasx::num_columns(K_),
			throw ::std::invalid_argument("[dcs::eesim::detail::matlab::dlqi_controller_proxy::control] Wrong state dimensiion.")
		);
		
		return -::boost::numeric::ublas::prod(K_, ::boost::numeric::ublas::trans(X));
	}


	/// The error weigthed matrix.
	private: matrix_type Q_;
	/// The control weigthed matrix.
	private: matrix_type R_;
	/// The cross-coupling weigthed matrix.
	private: matrix_type N_;
	/// The optimal feedback gain matrix.
	private: matrix_type K_;
	/// The solution to the associated DARE.
	private: matrix_type S_;
	/// The closed-loop eigenvalues which gives the closed-loop poles of \f$A-BK\f$.
	private: vector_type e_;
}; // dlqi_controller_proxy


template <typename RealT>
class dlqr_controller_proxy
{
	public: typedef RealT real_type;
	public: typedef ::boost::numeric::ublas::matrix<real_type> matrix_type;
	public: typedef ::boost::numeric::ublas::vector<real_type> vector_type;
	public: typedef ::std::size_t size_type;


	public: template <
				typename AMatrixT,
				typename BMatrixT,
				typename QMatrixT,
				typename RMatrixT,
				typename NMatrixT
		> dlqr_controller_proxy(::boost::numeric::ublas::matrix_expression<AMatrixT> const& A,
								::boost::numeric::ublas::matrix_expression<BMatrixT> const& B,
								::boost::numeric::ublas::matrix_expression<QMatrixT> const& Q,
								::boost::numeric::ublas::matrix_expression<RMatrixT> const& R,
								::boost::numeric::ublas::matrix_expression<NMatrixT> const& N)
		: Q_(Q),
		  R_(R),
		  N_(N)
	{
		solve(A, B);
	}


	public: template <
				typename AMatrixT,
				typename BMatrixT,
				typename QMatrixT,
				typename RMatrixT
		> dlqr_controller_proxy(::boost::numeric::ublas::matrix_expression<AMatrixT> const& A,
								::boost::numeric::ublas::matrix_expression<BMatrixT> const& B,
								::boost::numeric::ublas::matrix_expression<QMatrixT> const& Q,
								::boost::numeric::ublas::matrix_expression<RMatrixT> const& R)
		: Q_(Q),
		  R_(R),
		  N_(::boost::numeric::ublas::zero_matrix<real_type>(
					::boost::numeric::ublasx::num_rows(Q),
					::boost::numeric::ublasx::num_rows(R)
				)
			)
	{
		solve(A, B);
	}


	public: template <
				typename QMatrixT,
				typename RMatrixT,
				typename NMatrixT
		> dlqr_controller_proxy(::boost::numeric::ublas::matrix_expression<QMatrixT> const& Q,
								::boost::numeric::ublas::matrix_expression<RMatrixT> const& R,
								::boost::numeric::ublas::matrix_expression<NMatrixT> const& N)
		: Q_(Q),
		  R_(R),
		  N_(N)
	{
	}


	public: template <
				typename QMatrixT,
				typename RMatrixT
		> dlqr_controller_proxy(::boost::numeric::ublas::matrix_expression<QMatrixT> const& Q,
								::boost::numeric::ublas::matrix_expression<RMatrixT> const& R)
		: Q_(Q),
		  R_(R),
		  N_(::boost::numeric::ublas::zero_matrix<real_type>(
					::boost::numeric::ublasx::num_rows(Q),
					::boost::numeric::ublasx::num_rows(R)
				)
			)
	{
	}


	public: template <typename AMatrixT,
					  typename BMatrixT>
		void solve(::boost::numeric::ublas::matrix_expression<AMatrixT> const& A,
				   ::boost::numeric::ublas::matrix_expression<BMatrixT> const& B)
	{
		// Prepare the MATLAB command
		::std::vector< ::std::string > args;
		args.push_back("-nodisplay");
		args.push_back("-nojvm");
		::std::ostringstream oss;
		oss << "-r \"try "
			<< "[K,S,e]=dlqr("
			<< to_str(A)
			<< "," << to_str(B)
			<< "," << to_str(Q_)
			<< "," << to_str(R_)
			<< "," << to_str(N_)
			<< "); format long;"
			<< " disp('--- [eesim] ---');"
			<< " disp(['K=', mat2str(K), ]);"
			<< " disp(['S=', mat2str(S), ]);"
			<< " disp(['e=', mat2str(e), ]);"
			<< " disp('--- [/eesim] ---');"
			<< "catch me, "
			<< "disp(['??? Error: ', me.message]);"
			<< "end;"
			<< " quit force\"";
		args.push_back(oss.str());

		// Run MATLAB and retrieve its results
		bool ok;
		lq_matlab_output_consumer<real_type> consumer;
		ok = run_matlab_command(find_matlab_command(), args, consumer);
		if (!ok || !consumer.success())
		{
			throw ::std::runtime_error("[dcs::eesim::detail::matlab::dlqr_controller_proxy::solve] Wrong state dimensiion.");
		}
		K_ = consumer.K();
		S_ = consumer.S();
		e_ = consumer.e();
	}


    public: template <typename VectorExprT>
        vector_type control(::boost::numeric::ublas::vector_expression<VectorExprT> const& x) const
    {
		// preconditions: size(x) == num_columns(K_)
		DCS_ASSERT(
			::boost::numeric::ublasx::size(x) == ::boost::numeric::ublasx::num_columns(K_),
			throw ::std::invalid_argument("[dcs::eesim::detail::matlab::dlqr_controller_proxy::control] Wrong state dimensiion.")
		);

		return -::boost::numeric::ublas::prod(K_, x);
	}
 

    public: template <typename MatrixExprT>
        matrix_type control(::boost::numeric::ublas::matrix_expression<MatrixExprT> const& X) const
    {
		// preconditions: num_columns(X) == num_columns(K_)
		DCS_ASSERT(
			::boost::numeric::ublasx::num_columns(X) == ::boost::numeric::ublasx::num_columns(K_),
			throw ::std::invalid_argument("[dcs::eesim::detail::matlab::dlqr_controller_proxy::control] Wrong state dimensiion.")
		);
		
		return -::boost::numeric::ublas::prod(K_, ::boost::numeric::ublas::trans(X));
	}


	/// The error weigthed matrix.
	private: matrix_type Q_;
	/// The control weigthed matrix.
	private: matrix_type R_;
	/// The cross-coupling weigthed matrix.
	private: matrix_type N_;
	/// The optimal feedback gain matrix.
	private: matrix_type K_;
	/// The solution to the associated DARE.
	private: matrix_type S_;
	/// The closed-loop eigenvalues which gives the closed-loop poles of \f$A-BK\f$.
	private: vector_type e_;
}; // dlqr_controller_proxy


template <typename RealT>
class dlqry_controller_proxy
{
	public: typedef RealT real_type;
	public: typedef ::boost::numeric::ublas::matrix<real_type> matrix_type;
	public: typedef ::boost::numeric::ublas::vector<real_type> vector_type;
	public: typedef ::std::size_t size_type;


	public: template <
				typename AMatrixT,
				typename BMatrixT,
				typename CMatrixT,
				typename DMatrixT,
				typename QMatrixT,
				typename RMatrixT,
				typename NMatrixT
		> dlqry_controller_proxy(::boost::numeric::ublas::matrix_expression<AMatrixT> const& A,
								 ::boost::numeric::ublas::matrix_expression<BMatrixT> const& B,
								 ::boost::numeric::ublas::matrix_expression<CMatrixT> const& C,
								 ::boost::numeric::ublas::matrix_expression<DMatrixT> const& D,
								 ::boost::numeric::ublas::matrix_expression<QMatrixT> const& Q,
								 ::boost::numeric::ublas::matrix_expression<RMatrixT> const& R,
								 ::boost::numeric::ublas::matrix_expression<NMatrixT> const& N)
		: Q_(Q),
		  R_(R),
		  N_(N)
	{
		solve(A, B, C, D);
	}


	public: template <
				typename AMatrixT,
				typename BMatrixT,
				typename CMatrixT,
				typename DMatrixT,
				typename QMatrixT,
				typename RMatrixT
		> dlqry_controller_proxy(::boost::numeric::ublas::matrix_expression<AMatrixT> const& A,
								 ::boost::numeric::ublas::matrix_expression<BMatrixT> const& B,
								 ::boost::numeric::ublas::matrix_expression<CMatrixT> const& C,
								 ::boost::numeric::ublas::matrix_expression<DMatrixT> const& D,
								 ::boost::numeric::ublas::matrix_expression<QMatrixT> const& Q,
								 ::boost::numeric::ublas::matrix_expression<RMatrixT> const& R)
		: Q_(Q),
		  R_(R),
		  N_(::boost::numeric::ublas::zero_matrix<real_type>(
					::boost::numeric::ublasx::num_rows(Q),
					::boost::numeric::ublasx::num_rows(R)
				)
			)
	{
		solve(A, B, C, D);
	}


	public: template <
				typename QMatrixT,
				typename RMatrixT,
				typename NMatrixT
		> dlqry_controller_proxy(::boost::numeric::ublas::matrix_expression<QMatrixT> const& Q,
								 ::boost::numeric::ublas::matrix_expression<RMatrixT> const& R,
								 ::boost::numeric::ublas::matrix_expression<NMatrixT> const& N)
		: Q_(Q),
		  R_(R),
		  N_(N)
	{
	}


	public: template <
				typename QMatrixT,
				typename RMatrixT
		> dlqry_controller_proxy(::boost::numeric::ublas::matrix_expression<QMatrixT> const& Q,
								 ::boost::numeric::ublas::matrix_expression<RMatrixT> const& R)
		: Q_(Q),
		  R_(R),
		  N_(::boost::numeric::ublas::zero_matrix<real_type>(
					::boost::numeric::ublasx::num_rows(Q),
					::boost::numeric::ublasx::num_rows(R)
				)
			)
	{
	}


	public: template <typename AMatrixT,
					  typename BMatrixT,
					  typename CMatrixT,
					  typename DMatrixT>
		void solve(::boost::numeric::ublas::matrix_expression<AMatrixT> const& A,
				   ::boost::numeric::ublas::matrix_expression<BMatrixT> const& B,
				   ::boost::numeric::ublas::matrix_expression<CMatrixT> const& C,
				   ::boost::numeric::ublas::matrix_expression<DMatrixT> const& D)
	{
		// Prepare the MATLAB command
		::std::vector< ::std::string > args;
		args.push_back("-nodisplay");
		args.push_back("-nojvm");
		::std::ostringstream oss;
		oss << "-r \"try "
			<< "[K,S,e]=lqry("
			<< "ss("
			<< to_str(A)
			<< "," << to_str(B)
			<< "," << to_str(C)
			<< "," << to_str(D)
			<< ",1)" // dummy sampling time (in MATLAB is only used for discriminating between the continuous and discrete case)
			<< "," << to_str(Q_)
			<< "," << to_str(R_)
			<< "," << to_str(N_)
			<< "); format long;"
			<< " disp('--- [eesim] ---');"
			<< " disp(['K=', mat2str(K), ]);"
			<< " disp(['S=', mat2str(S), ]);"
			<< " disp(['e=', mat2str(e), ]);"
			<< " disp('--- [/eesim] ---');"
			<< "catch me, "
			<< "disp(['??? Error: ', me.message]);"
			<< "end;"
			<< " quit force\"";
		args.push_back(oss.str());

		// Run MATLAB and retrieve its results
		bool ok;
		lq_matlab_output_consumer<real_type> consumer;
		ok = run_matlab_command(find_matlab_command(), args, consumer);
		if (!ok || !consumer.success())
		{
			throw ::std::runtime_error("[dcs::eesim::detail::matlab::dlqry_controller_proxy::solve] Wrong state dimensiion.");
		}
		K_ = consumer.K();
		S_ = consumer.S();
		e_ = consumer.e();
	}


    public: template <typename VectorExprT>
        vector_type control(::boost::numeric::ublas::vector_expression<VectorExprT> const& x) const
    {
		// preconditions: size(x) == num_columns(K_)
		DCS_ASSERT(
			::boost::numeric::ublasx::size(x) == ::boost::numeric::ublasx::num_columns(K_),
			throw ::std::invalid_argument("[dcs::eesim::detail::matlab::dlqry_controller_proxy::control] Wrong state dimensiion.")
		);

		return -::boost::numeric::ublas::prod(K_, x);
	}
 

    public: template <typename MatrixExprT>
        matrix_type control(::boost::numeric::ublas::matrix_expression<MatrixExprT> const& X) const
    {
		// preconditions: num_columns(X) == num_columns(K_)
		DCS_ASSERT(
			::boost::numeric::ublasx::num_columns(X) == ::boost::numeric::ublasx::num_columns(K_),
			throw ::std::invalid_argument("[dcs::eesim::detail::matlab::dlqry_controller_proxy::control] Wrong state dimensiion.")
		);
		
		return -::boost::numeric::ublas::prod(K_, ::boost::numeric::ublas::trans(X));
	}


	/// The error weigthed matrix.
	private: matrix_type Q_;
	/// The control weigthed matrix.
	private: matrix_type R_;
	/// The cross-coupling weigthed matrix.
	private: matrix_type N_;
	/// The optimal feedback gain matrix.
	private: matrix_type K_;
	/// The solution to the associated DARE.
	private: matrix_type S_;
	/// The closed-loop eigenvalues which gives the closed-loop poles of \f$A-BK\f$.
	private: vector_type e_;
}; // dlqr_controller_proxy

}}}} // Namespace dcs::eesim::detail::matlab

#endif // DCS_EESIM_DETAIL_MATLAB_CONTROLLER_PROXIES_HPP
