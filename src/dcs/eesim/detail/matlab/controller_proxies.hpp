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
class lqi_matlab_output_consumer
{
	public: typedef ValueT value_type;
	public: typedef ::boost::numeric::ublas::matrix<value_type> matrix_type;
	public: typedef ::boost::numeric::ublas::vector<value_type> vector_type;


	public: template <typename CharT, typename CharTraitsT>
		void operator()(::std::basic_istream<CharT,CharTraitsT>& is)
	{
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
					if ((pos = line.find("K=")) != ::std::string::npos)
					{
						parse_str(line.substr(pos+2), K_);
DCS_DEBUG_TRACE("Parsed as K=" << K_);//XXX
					}
					else if ((pos = line.find("S=")) != ::std::string::npos)
					{
						parse_str(line.substr(pos+2), S_);
DCS_DEBUG_TRACE("Parsed as S=" << S_);//XXX
					}
					else if ((pos = line.find("e=")) != ::std::string::npos)
					{
						parse_str(line.substr(pos+2), e_);
DCS_DEBUG_TRACE("Parsed as e=" << e_);//XXX
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


	private: matrix_type K_;
	private: matrix_type S_;
	private: vector_type e_;
}; // lqi_matlab_output_consumer

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
		oss << "-r \"[K,S,e]=lqi("
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
			<< " quit force\"";
		args.push_back(oss.str());

		// Run MATLAB and retrieve its results
		lqi_matlab_output_consumer<real_type> consumer;
		run_matlab_command(find_matlab_command(), args, consumer);
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
			throw ::std::invalid_argument("[dcs::eesim::detail::matlab::dlqi_controller::control_proxy] Wrong state dimensiion.")
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

}}}} // Namespace dcs::eesim::detail::matlab

#endif // DCS_EESIM_DETAIL_MATLAB_CONTROLLER_PROXIES_HPP
