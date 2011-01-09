/**
 * \file dcs/perfeval/qn/open_multi_bcmp_network.hpp
 *
 * \brief Open multi-class BCMP Queueing Network.
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
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */

#ifndef DCS_PERFEVAL_QN_OPEN_MULTI_BCMP_NETWORK_HPP
#define DCS_PERFEVAL_QN_OPEN_MULTI_BCMP_NETWORK_HPP


#include <functional>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <boost/numeric/ublas/detail/temporary.hpp>
#include <boost/numeric/ublas/expression_types.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>
#include <boost/numeric/ublas/traits.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <boost/numeric/ublasx/operation/all.hpp>
#include <boost/numeric/ublasx/operation/max.hpp>
#include <boost/numeric/ublasx/operation/num_columns.hpp>
#include <boost/numeric/ublasx/operation/num_rows.hpp>
#include <boost/numeric/ublasx/operation/size.hpp>
#include <boost/numeric/ublasx/operation/sum.hpp>
#include <boost/numeric/ublasx/operation/which.hpp>
#include <functional>
#include <stdexcept>


namespace dcs { namespace perfeval { namespace qn {

/**
 * \brief Open multi-class BCMP Queueing Network.
 *
 * References:
 * -# F. Baskett,
 *    "Open, Closed, and Mixed Networks of Queues with Different Classes of
 *     Customers",
 *    Journal of the ACM, 22(2):248-260, 1975.
 * -# E.D. Lazowska et al,
 *    "Quantitative System Performance",
 *    Prentice-Hall, Inc., 1984.
 * -# R. Jain,
 *    "The Art of Computer Systems Performance Analysis",
 *    John Wiley &amp; Sons, 1991.
 * -# G. Bolch et al,
 *    "Queueing Networks and Markov Chains",
 *    John Wiley &amp; Sons, 2006.
 * .
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
//template <typename RealVectorT, typename UIntVectorT, typename RealMatrixT>
template <typename RealT, typename UIntT>
class open_multi_bcmp_network
{
	public: typedef RealT real_type;
	public: typedef UIntT uint_type;
	public: typedef ::boost::numeric::ublas::vector<real_type> real_vector_type;
	public: typedef ::boost::numeric::ublas::vector<uint_type> uint_vector_type;
	public: typedef ::boost::numeric::ublas::matrix<real_type> real_matrix_type;
	public: typedef typename ::boost::numeric::ublas::promote_traits<
						typename real_vector_type::size_type,
						typename ::boost::numeric::ublas::promote_traits<
							typename uint_vector_type::size_type,
							typename real_matrix_type::size_type
						>::promote_type
					>::promote_type size_type;
	public: typedef ::boost::numeric::ublas::vector<size_type> size_vector_type;
	//public: typedef typename ::boost::numeric::ublas::vector_traits<uint_vector_type>::real_type uint_type;
	//public: typedef typename ::boost::numeric::ublas::promote_traits<
	//					typename ::boost::numeric::ublas::vector_traits<real_vector_type>::real_type,
	//					typename ::boost::numeric::ublas::matrix_traits<real_matrix_type>::real_type
	//				>::promote_type real_type;


	public: template <typename VE1, typename ME1, typename ME2, typename VE2>
		//open_multi_bcmp_network(vector_type const& lambda, matrix_type const& S, matrix_type const& V, vector_type const& m)
		open_multi_bcmp_network(::boost::numeric::ublas::vector_expression<VE1> const& lambda,
								::boost::numeric::ublas::matrix_expression<ME1> const& S,
								::boost::numeric::ublas::matrix_expression<ME2> const& V,
								::boost::numeric::ublas::vector_expression<VE2> const& m)
		: lambda_(lambda),
		  S_(S),
		  V_(V),
		  m_(m),
          nc_(::boost::numeric::ublasx::size(lambda_)),
          ns_(::boost::numeric::ublasx::num_columns(S_)),
		  capacity_(real_type/*zero*/()),
		  D_(nc_, ns_, real_type/*zero*/()),
		  U_(nc_, ns_, real_type/*zero*/()),
		  X_(nc_, ns_, real_type/*zero*/()),
		  R_(nc_, ns_, real_type/*zero*/()),
		  K_(nc_, ns_, real_type/*zero*/()),
		  solved_(false)
	{
		// precondition: all(lambda > 0)
		DCS_ASSERT(
			::boost::numeric::ublasx::all(
				lambda_,
				::std::bind2nd(::std::greater<real_type>(), 0)
			),
			throw ::std::invalid_argument("Arrival rates must be positive numbers.")
		);

		// precondition: size(lambda_) == num_columns(S_)
		DCS_ASSERT(
//			::boost::numeric::ublasx::size(lambda_) == ::boost::numeric::ublasx::num_columns(S_),
			nc_ == ::boost::numeric::ublasx::num_rows(S_),
			throw ::std::invalid_argument("Arrival rates and service times are of non-compliant sizes.")
		);

		// precondition: all(S > 0)
		DCS_ASSERT(
			::boost::numeric::ublasx::all(
				S_,
				::std::bind2nd(::std::greater<real_type>(), 0)
			),
			throw ::std::invalid_argument("Service times must be positive numbers.")
		);

		// precondition: size(V_) ==  size(S_)
		DCS_ASSERT(
			::boost::numeric::ublasx::num_rows(V_) == ::boost::numeric::ublasx::num_rows(S_)
			&&
			::boost::numeric::ublasx::num_columns(V_) == ::boost::numeric::ublasx::num_columns(S_),
			throw ::std::invalid_argument("Visit rates and service times are of non-compliant sizes.")
		);

		// precondition: all(V_ >= 0)
		DCS_ASSERT(
			::boost::numeric::ublasx::all(
				V_,
				::std::bind2nd(::std::greater_equal<real_type>(), 0)
			),
			throw ::std::invalid_argument("Service times must be non-negative numbers.")
		);

		// precondition: size(m_) == num_columns(S_)
		DCS_ASSERT(
			::boost::numeric::ublasx::size(m_) == ::boost::numeric::ublasx::num_columns(S_),
			throw ::std::invalid_argument("Service centers numerosity and service times are of non-compliant sizes.")
		);

		// precondition: all(m_ <= 1)
		DCS_ASSERT(
			::boost::numeric::ublasx::all(
				m_,
				::std::bind2nd(::std::less_equal<real_type>(), 1)
			),
			throw ::std::invalid_argument("Service centers numerosity must be <= 1.")
		);

		solve();
	}


	public: real_type processing_capacity() const
	{
		return capacity_;
	}


	/// Per-class average service time at each station.
	public: real_matrix_type service_times() const
	{
		return S_;
	}


	/// Per-class average visit ratio at each station.
	public: real_matrix_type visit_ratios() const
	{
		return V_;
	}


	/// Per-class external arrival rate.
	public: real_vector_type arrival_rates() const
	{
		return lambda_;
	}


	public: size_type num_classes() const
	{
		return nc_;
	}


	public: size_type num_stations() const
	{
		return ns_;
	}


	public: uint_vector_type num_servers() const
	{
		return m_;
	}


	/**
	 * Check the stability condition:
	 * \f{align*}
	 *   \forall s=\{1,\ldots,n_s\} :& \sum_{c=1}^{n_c}{\lambda_c D_{c,s}} \le 1\\
	 *   & U_i \le 1
	 * \f}
	 */
	public: bool saturated() const
	{
		return capacity_ >= static_cast<real_type>(1);
	}


	/// Per-class utilization of each station.
	public: real_matrix_type utilizations() const
	{
//		if (!solved_)
//		{
//			solve();
//		}

		return U_;
	}


	/// Per-class throughput at each station.
	public: real_matrix_type throughputs() const
	{
//		if (!solved_)
//		{
//			solve();
//		}

		return X_;
	}


	/// Per-class average response time per visit at each station.
	public: real_matrix_type response_times() const
	{
//		if (!solved_)
//		{
//			solve();
//		}

		return R_;
	}


	/// Per-class average number of customers (waiting + in service) at each
	/// station.
	public: real_matrix_type customers_numbers() const
	{
		return K_;
	}


	/// Per-class average time spent at each station over all visits.
	public: real_matrix_type residence_times() const
	{
		return ::boost::numeric::ublas::element_prod(R_, V_);
	}


	/// Per-class average waiting time per visit at each station.
	public: real_matrix_type waiting_times() const
	{
		return R_ - S_;
	}


	/// Per-class average number of customers waiting for a service at each
	/// station.
	public: real_matrix_type queue_lengths() const
	{
		//return ::boost::numeric::ublas::element_prod(X_, waiting_times());
		return K_ - U_;
	}


//XXX: Meaningless
//	public: real_vector_type class_utilizations() const
//	{
//		return ::boost::numeric::ublasx::sum<1>(U_);
//	}


	/// Per-station utilizations
	public: real_vector_type station_utilizations() const
	{
		return ::boost::numeric::ublasx::sum<2>(U_);
	}


	/// Per class (aggregate) average response time.
	public: real_vector_type class_response_times() const
	{
		return ::boost::numeric::ublasx::sum<1>(R_);
	}


	/// Per station (aggregate) average response time.
	public: real_vector_type station_response_times() const
	{
		// See Chapter 4 of (Lazowska, 1984)

		//return ::boost::numeric::ublas::prod(::boost::numeric::ublas::trans(R_), class_throughputs()) / system_throughput();
		return ::boost::numeric::ublas::prod(class_throughputs(), R_) / system_throughput();
	}


	/// Per class (aggregate) throughput.
	public: real_vector_type class_throughputs() const
	{
		return lambda_;
	}


	/// Per station (aggregate) throughput.
	public: real_vector_type station_throughputs() const
	{
		return ::boost::numeric::ublasx::sum<2>(X_);
	}


	/// Per class (aggregate) average number of customers.
	public: real_vector_type class_customers_numbers() const
	{
		return ::boost::numeric::ublasx::sum<1>(K_);
	}


	/// Per station (aggregate) average number of customers.
	public: real_vector_type station_customers_numbers() const
	{
		return ::boost::numeric::ublasx::sum<2>(K_);
	}


	/// Per class (aggregate) average residence times.
	public: real_vector_type class_residence_times() const
	{
		return ::boost::numeric::ublasx::sum<1>(residence_times());
	}


	/// Per station (aggregate) average residence times.
	public: real_vector_type station_residence_times() const
	{
		// See Chapter 4 of (Lazowska, 1984)

		//return ::boost::numeric::ublas::prod(::boost::numeric::ublas::trans(residence_times()), class_throughputs()) / system_throughput();
		return ::boost::numeric::ublas::prod(class_throughputs(), residence_times()) / system_throughput();
	}


	/// Per class (aggregate) average waiting times.
	public: real_vector_type class_waiting_times() const
	{
		return ::boost::numeric::ublasx::sum<1>(waiting_times());
	}


	/// Per station (aggregate) average waiting times.
	public: real_vector_type station_waiting_times() const
	{
		return ::boost::numeric::ublasx::sum<2>(waiting_times());
	}


	/// Per class (aggregate) average number of waiting customers.
	public: real_vector_type class_queue_lengths() const
	{
		return ::boost::numeric::ublasx::sum<1>(queue_lengths());
	}


	/// Per station (aggregate) average number of waiting customers.
	public: real_vector_type station_queue_lengths() const
	{
		return ::boost::numeric::ublasx::sum<2>(queue_lengths());
	}


//XXX: Meaningless
//	/// System utilization.
//	public: real_type system_utilizations() const
//	{
//		return ::boost::numeric::ublasx::sum<1>(U_);
//	}


	/// System response time.
	public: real_type system_response_time() const
	{
		// See Chapter 4 of (Lazowska, 1984)
		// R = \sum_c \frac{R_c X_c}{X}
		//   = \sum_c \frac{\sum_k R_{ck} X_c}{X}
		//   = \sum_k \frac{\sum_c R_{ck} X_c}{X}
		//   = \sum_k R_k

		//return ::boost::numeric::ublasx::sum(
		//		::boost::numeric::ublas::element_prod(class_response_times(), class_throughputs())
		//		) / system_throughput();
		return ::boost::numeric::ublasx::sum(station_response_times());
	}


	/// System throughput.
	public: real_type system_throughput() const
	{
		return ::boost::numeric::ublasx::sum(lambda_);
	}


	/// Average number of customers in system.
	public: real_type system_customers_number() const
	{
		return ::boost::numeric::ublasx::sum_all(K_);
	}


	/// System residence time.
	public: real_type system_residence_time() const
	{
		return ::boost::numeric::ublasx::sum(station_residence_times());
	}


	/// System waiting time.
	public: real_type system_waiting_time() const
	{
		return ::boost::numeric::ublasx::sum(station_waiting_times());
	}


	/// System queue length.
	public: real_type system_queue_length() const
	{
		return ::boost::numeric::ublasx::sum(station_queue_lengths());
	}


//	/// For each class, return the station(s) with highest utilization.
//	FIXME; what type should be returned? Currently is a map to <class, vector of
//	       bottleneck stations> pairs.
//	public: ::std::map<size_type,uint_vector_type> bottleneck_stations() const
//	{
//		::std::map<size_type,size_vector_type> m;
//
//		// Find the per-class max utilization
//		real_vector_type u_max = ::boost::numeric::ublasx::max<2>(U_);
//		// For each class, find the station having the above utilization
//		for (size_type c = 0; c < nc_; ++c)
//		{
//			m[c] = ::boost::numeric::ublasx::which(
//					::boost::numeric::ublas::row(U_, c),
//					::std::bind2nd(::std::greater_equal<real_type>(), u_max(c))
//				);
//		}
//
//		return m;
//	}


	/// For the given class, return the station(s) with highest utilization.
	public: size_vector_type bottleneck_stations(size_type c) const
	{
		// Find the per-class max utilization
		real_type u_max = ::boost::numeric::ublasx::max(
							::boost::numeric::ublas::row(U_, c)
			);
		// Find the station having the above utilization
		return ::boost::numeric::ublasx::which(
				::boost::numeric::ublas::row(U_, c),
				::std::bind2nd(::std::greater_equal<real_type>(), u_max)
			);
	}


	private: bool solve()
	{
		//typedef ::boost::numeric::ublas::vector<size_type> ix_vector_type;

		// Clean/reset measures
		U_ = real_matrix_type(nc_, ns_, 0);
		R_ = real_matrix_type(nc_, ns_, 0);
		K_ = real_matrix_type(nc_, ns_, 0);
		X_ = real_matrix_type(nc_, ns_, 0);

		// Compute service demands: D_{c,i}=V_{c,i}*S_{c,i}
		D_ = ::boost::numeric::ublas::element_prod(S_, V_);

		// Check that the system is not saturated
		// - Compute the processing capacity: max(lambda*D)
		capacity_ = ::boost::numeric::ublasx::max(::boost::numeric::ublas::prod(lambda_, D_));
//		DCS_ASSERT(
//			capacity_ < real_type(1),
//			throw ::std::runtime_error("Processing capacity exceeded.")
//		);
		// - Check max(lambda*D) < 1
		if (saturated())
		{
			DCS_DEBUG_TRACE("Saturated system: processing capacity >= 1.")
			return false;
		}

		// Compute the per station throughtput X_{c,i} and utilization U_{c,i}.
		// For each class c and station i:
		// - Throughtput: X_{c,i} = X_c*V_{c,i}       (forced flow law)
		// - Utilization: U_{c,i} = X_{c,i}*S_{c,i}   (utilization law)
		//                        = X_c*D_{c,i}       (forced flow law)
		// Supposing that the job flow balance assumption holds, we can replace
		// X_c with \lambda_c.
		for (size_type c = 0; c < nc_; ++c)
		{
			if (lambda_(c) > 0)
			{
				::boost::numeric::ublas::row(U_, c) = lambda_(c)*::boost::numeric::ublas::row(D_, c); // U(c,.) = lambda_(c)*D(c,.);
				::boost::numeric::ublas::row(X_, c) = lambda_(c)*::boost::numeric::ublas::row(V_, c); // X(c,.) = lambda_(c)*V(c,.);
			}
		}

		size_vector_type which_ix;
		size_type n;

		// Find delay stations (i.e., stations for which m < 1)
		//ix_vector_type delay_ix = ::boost::numeric::ublasx::which(m_, ::std::bind2nd(::std::less<real_type>(), real_type(1)));
		which_ix = ::boost::numeric::ublasx::which(m_, ::std::bind2nd(::std::less<real_type>(), real_type(1)));

		// delay centers
		n = ::boost::numeric::ublasx::size(which_ix);
		for (size_type i = 0; i < n; ++i)
		{
			::boost::numeric::ublas::column(R_, which_ix(i)) = ::boost::numeric::ublas::column(S_, which_ix(i));
			::boost::numeric::ublas::column(K_, which_ix(i)) = ::boost::numeric::ublas::column(U_, which_ix(i));
		}

		// Find single stations (i.e., stations for which m == 1)
		//ix_vector_type single_ix = ::boost::numeric::ublasx::which(m_, ::std::bind2nd(::std::equal_to<real_type>(), real_type(1)));
		which_ix = ::boost::numeric::ublasx::which(m_, ::std::bind2nd(::std::equal_to<real_type>(), real_type(1)));

		// queueing centers
		n = ::boost::numeric::ublasx::size(which_ix);
		real_vector_type single_sums(n, real_type/*zero*/());
		for (size_type i = 0; i < n; ++i)
		{
			single_sums(i) = 1 - ::boost::numeric::ublasx::sum(::boost::numeric::ublas::column(U_, which_ix(i)));
		}
		for (size_type i = 0; i < n; ++i)
		{
			::boost::numeric::ublas::column(R_, which_ix(i)) = ::boost::numeric::ublas::element_div(
																	::boost::numeric::ublas::column(S_, which_ix(i)),
																	::boost::numeric::ublas::scalar_vector<real_type>(nc_, single_sums(i))
			);
			::boost::numeric::ublas::column(K_, which_ix(i)) = ::boost::numeric::ublas::element_div(
																	::boost::numeric::ublas::column(U_, which_ix(i)),
																	::boost::numeric::ublas::scalar_vector<real_type>(nc_, single_sums(i))
			);
		}

		solved_ = true;

		return true;
	}


	/// The arrival rates vector (one element for each user class)
	private: real_vector_type lambda_;
	// The per-class mean service time for each service center.
	private: real_matrix_type S_;
	/// The per-class mean number of visits to each service center.
	private: real_matrix_type V_;
	/// The number of servers vector (one element for each service center);
	/// Valid values are \f$m_k < 1\f$ to denote a delay center
	/// (\f$-/G/\infty\f$), and \f$m_k==1\f$ to denote a single server queueing
	/// center (\f$M/M/1\f$--FCFS, \f$-/G/1\f$--LCFS-PR or \f$-/G/1\f$--PS).
	private: uint_vector_type m_;
	/// Number of classes.
	private: size_type nc_;
	/// Number of service stations.
	private: size_type ns_;
	/// Processing capacity.
	private: real_type capacity_;
	/// The per-class service demands
	private: real_matrix_type D_;
	/// The per-class utilization for each service center; if \c k is a queueing
	/// center, then \c U(c,k) is the class \c c utilization of center \c k; if
	/// \c k is an IS node, then \c U(c,k) is the class \c c <em>traffic
	/// intensity</em> defined as <em>X(c,k)*S(c,k)</em>.
	private: real_matrix_type U_;
	/// The per-class throughput for each service center; if \c X(c,k) is the class
	/// \c c throughput at center \c k.
	private: real_matrix_type X_;
	/// The per-class response time for each service center; if \c R(c,k) is the class \c c response time at center \c k. The system response time for class \c c requests can be computed as \c dot(@var{R}, @var{V}, 2)}.
	private: real_matrix_type R_;
	private: real_matrix_type K_;
	private: bool solved_;
};

}}} // Namespace dcs::perfeval::qn


#endif // DCS_PERFEVAL_QN_OPEN_MULTI_BCMP_NETWORK_HPP
