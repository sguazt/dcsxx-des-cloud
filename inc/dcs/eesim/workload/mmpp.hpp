#ifndef DCS_EESIM_WORKLOAD_MMPP_HPP
#define DCS_EESIM_WORKLOAD_MMPP_HPP


#include <algorithm>
#include <boost/numeric/ublas/expression_types.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublasx/operation/diag.hpp>
#include <boost/random/discrete_distribution.hpp>
#include <boost/random/exponential_distribution.hpp>
#include <cmath>
#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/math/stats/function/rand.hpp>


namespace dcs { namespace eesim {

template <typename TraitsT, typename ValueT>
class mmpp_interarrivals_workload_model
{
	private: typedef mmpp_interarrivals_workload_model<TraitsT,ValueT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef ValueT value_type;
	public: typedef ::std::size_t size_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef ::boost::numeric::ublas::matrix<value_type> value_matrix_type;
	public: typedef ::boost::numeric::ublas::vector<value_type> value_vector_type;
	public: typedef ::boost::numeric::ublas::vector<size_type> size_vector_type;
	private: typedef ::boost::random::discrete_distribution<size_type,value_type> discrete_distribution_type;
	private: typedef ::boost::random::exponential_distribution<value_type> exponential_distribution_type;


	public: template <typename LambdaVectorT, typename QMatrixT, typename P0VectorT>
		mmpp_interarrivals_workload_model(::boost::numeric::ublas::vector_expression<LambdaVectorT> const& lambda,
										  ::boost::numeric::ublas::matrix_expression<QMatrixT> const& Q,
										  ::boost::numeric::ublas::vector_expression<P0VectorT> const& p0)
	: lambda_(lambda),
	  Q_(Q),
	  p0_(p0)
	{
		reset();
	}


	public: value_matrix_type const& Q() const
	{
		return Q_;
	}


	public: value_vector_type const& lambda() const
	{
		return lambda_;
	}



	public: template <typename VectorT>
		void p0(::boost::numeric::ublas::vector_expression<VectorT> const& probs)
	{
		p0_ = probs;
	}


	public: value_vector_type const& p0() const
	{
		return p0_;
	}


	public: template <typename UniformRandomGeneratorT>
		value_type rand(UniformRandomGeneratorT& rng) const
	{
		const_cast<self_type*>(this)->advance(size_type(1), rng);

		DCS_DEBUG_ASSERT( tau_.size() == 2 );

		return tau_(1)-tau_(0);
	}


	protected: size_type i() const
	{
		return i_;
	}


	protected: size_type j() const
	{
		return j_;
	}


	protected: value_vector_type const& tau() const
	{
		return tau_;
	}


	protected: value_vector_type const& x() const
	{
		return x_;
	}


	protected: size_vector_type const& y() const
	{
		return y_;
	}


	/// Generate \c n Poisson arrivals
	private: template <typename URNG>
		void advance(::std::size_t n, URNG& rng)
	{
		if (i_ == 0)
		{
			init(rng);
		}
		else
		{
			value_type tmp(tau_(tau_.size()-1));
			tau_.resize(n+1, false);
			tau_[0] = tmp;
		}

		size_type nj(j_+n);
		size_type t(1);

		while (j_ < nj)
		{
			// Transition to next state y(i)
			if (i_ == 0 || st_ >= x_(1))
			{
				++i_;

				::std::swap(y_(0), y_(1));
				::std::swap(x_(0), x_(1));

				// extend x and y if too short
				// sim time spent in Markov state y[i-1]
				value_vector_type probs(::boost::numeric::ublas::row(Pi_, y_(0)));
				discrete_distribution_type sample(probs.begin(), probs.end());
				y_(1) = sample(rng);

				// Generate new time to transition from state y(i-1) to state y(i)
				exponential_distribution_type rexp(-Q_(y_(0),y_(0)));
				x_(1) = x_(0)+rexp(rng);

				st_ = x_(0);
			}

			value_type t1(x_(1));
			// sim Poisson arrival events until transition
			while (j_ < nj && st_ < t1)
			{
				// Generate new arrival time of event in state y(i-1)
				exponential_distribution_type rexp(lambda_(y_(0)));
				value_type iat(rexp(rng));
				st_ += iat;

				if (st_ < t1)
				{
					tau_(t) = st_;
					++j_;
					++t;
				}
			}
		}
	}


	public: void reset()
	{
		st_ = value_type/*zero*/();
		j_ = i_ = size_type/*zero*/();
		tau_.resize(0, false);
		x_.resize(0, false);
		y_.resize(0, false);
	}


	private: template <typename URNG>
		void init(URNG& rng)
	{
		namespace ublas = ::boost::numeric::ublas;
		namespace ublasx = ::boost::numeric::ublasx;

		size_type nq(ublas::num_rows(Q_));

		ublas::scalar_vector<value_type> one(nq, 1);

		value_vector_type tmp;
		tmp = ublas::element_div(one, ublasx::diag(Q_));

		Pi_ = ublas::identity_matrix<value_type>(nq)-ublas::prod(ublasx::diag(tmp), Q_);
		i_ = 0;
		j_ = 1;

		tau_.resize(2, false);
		x_.resize(2, false);
		y_.resize(2, false);

		discrete_distribution_type sample(p0_.begin(), p0_.end());
		y_(0) = y_(1) = sample(rng);
		x_(0) = x_(1) = tau_(0) = tau_(1) = 0;
	}


	/// The arrival rates vector
	private: value_vector_type lambda_;
	/// The generator matrix
	private: value_matrix_type Q_;
	/// The initial states probability vector
	private: value_vector_type p0_;
	/// The phase transition matrix
	private: value_matrix_type Pi_;
	private: size_type i_;
	private: size_type j_;
	private: value_vector_type tau_;
	private: value_vector_type x_;
	private: size_vector_type y_;
	/// The sojourn time for current state y(i-1)
	private: value_type st_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_WORKLOAD_MMPP_HPP
