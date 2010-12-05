/**
 * \file dcs/perfeval/sla/step_cost_model.hpp
 *
 * \brief SLA model with a step-based cost function.
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

#ifndef DCS_PERFEVAL_SLA_STEP_COST_MODEL_HPP
#define DCS_PERFEVAL_SLA_STEP_COST_MODEL_HPP


#include <dcs/iterator/any_forward_iterator.hpp>
#include <dcs/iterator/iterator_range.hpp>
#include <functional>
#include <iterator>


namespace dcs { namespace perfeval { namespace sla {

namespace detail {

/**
 * \brief Default SLA checker functor.
 * \tparam RealT The type used for real numbers.
 *
 * Given a pair \f$\langle x,y \rangle\f$ of measures sequences, the default SLA
 * checker function return \c true if \f$x_i \le y_i$ for all \f$i\f$, and
 * \c false otherwise.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename RealT>
struct default_checker
{
	/// The type used for real numbers.
	typedef RealT real_type;

	/// The type of the range of iterators over the measures sequence.
	typedef ::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator<real_type> > iterator_range_type;
//	typedef ::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator<real_type const> > const_iterator_range_type;

	/**
	 * \brief The checker functor.
	 * \param r The iterator range for reference measures.
	 * \param c The iterator range for current measures.
	 */
	bool operator()(iterator_range_type const& r, iterator_range_type const& c) const
	{
		return (c < r || c == r); // this use the std::lexicographical_compare function
	}
};

} // Namespace detail


/**
 * \brief SLA model with a step-based cost function.
 *
 * \tparam RealT The type used for real numbers.
 * \tparam SlaCheckT The type used for the SLA checker functor.
 *
 * This model uses a step-based function for determing the cost (gain) of a SLA
 * violation (satisfaction).
 * Specifically, given:
 * - a sequence \f$r\f$ of reference measures,
 * - a constant number \f$P\f$ denoting the cost paid in case of SLA violation,
 * - a constant number \f$R\f$ denoting the reward obtained in case of SLA
 *   satisfaction,
 * - a SLA checker function \f$c(r,x)\f$ which return \f$\mathrm{true}\f$ if
 *   \f$x\f$ does not violate SLA constraints according to reference measures
 *   \f$r\f$ and \f$\mathrm{false}\f$ otherwise,
 * - and a sequence \f$x\f$ of current measures,
 * .
 * the cost function \f$s(x)\f$ is defined as:
 * \f{equation*}{
 *   s(x)=
 *     \begin{cases}
 *     R, \quad c(r,x) == \mathrm{true} \\
 *     P, \quad c(r,x) == \mathrm{false}
 *     \end{cases}
 * \f}
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <
	typename RealT=double,
	typename SlaCheckerT=detail::default_checker<RealT>
>
class step_cost_model
{
	/// The type used for real numbers.
	public: typedef RealT real_type;
	/// The type used for the SLA checker functor.
	public: typedef SlaCheckerT checker_type;
	private: typedef step_cost_model<real_type,checker_type> self_type;
	public: typedef ::dcs::iterator::any_forward_iterator<real_type> measures_iterator;
	public: typedef ::dcs::iterator::any_forward_iterator<real_type const> measures_const_iterator;
	public: typedef ::dcs::iterator::iterator_range<measures_iterator> measures_iterator_range_type;
	public: typedef ::dcs::iterator::iterator_range<measures_const_iterator> measures_const_iterator_range_type;
	private: typedef ::std::vector<real_type> measures_sequence_type;
	private: typedef typename measures_sequence_type::iterator measures_internal_iterator;
	private: typedef typename measures_sequence_type::const_iterator measures_internal_const_iterator;


	/// Default constructor.
	public: step_cost_model()
	{
	}


	/// A constructor.
	public: template <typename ForwardIteratorT>
		step_cost_model(real_type penalty_cost, real_type reward_cost, ForwardIteratorT ref_measures_begin, ForwardIteratorT ref_measures_end, checker_type const& checker=checker_type())
		: penalty_(penalty_cost),
		  reward_(reward_cost),
		  ref_measures_(ref_measures_begin, ref_measures_end),
		  ref_measures_range_(ref_measures_.begin(), ref_measures_.end()),
		  checker_(checker)
	{
		// empty
	}


	/// A constructor.
	public: step_cost_model(real_type penalty_cost, real_type reward_cost, measures_iterator_range_type const& ref_measures, checker_type const& checker=checker_type())
		: penalty_(penalty_cost),
		  reward_(reward_cost),
		  ref_measures_(ref_measures.begin(), ref_measures.end()),
		  ref_measures_range_(ref_measures_.begin(), ref_measures_.end()),
		  checker_(checker)
	{
		// empty
	}


	public: step_cost_model(step_cost_model const& that)
		: penalty_(that.penalty_),
		  reward_(that.reward_),
		  ref_measures_(that.ref_measures_),
		  ref_measures_range_(dcs::iterator::make_iterator_range(
			  dcs::iterator::make_any_forward_iterator(ref_measures_.begin()),
			  dcs::iterator::make_any_forward_iterator(ref_measures_.end())
		  )),
		  checker_(that.checker_)
	{

		ref_measures_range_ = dcs::iterator::make_iterator_range(
			dcs::iterator::make_any_forward_iterator(ref_measures_.begin()),
			dcs::iterator::make_any_forward_iterator(ref_measures_.end())
		);
	}


	public: void penalty_cost(real_type value)
	{
		penalty_ = value;
	}


	public: void reward_cost(real_type value)
	{
		reward_ = value;
	}


	public: template <typename ForwardIteratorT>
		void reference_measures(ForwardIteratorT measures_begin, ForwardIteratorT measures_end)
	{
		  ref_measures_ = measures_sequence_type(measures_begin, measures_end);
		  ref_measures_range_ = measures_iterator_range_type(ref_measures_.begin(), ref_measures_.end());
	}


	public: void reference_measures(measures_iterator_range_type const& measures)
	{
		  ref_measures_ = measures_sequence_type(measures.begin(), measures.end());
		  ref_measures_range_ = measures_iterator_range_type(ref_measures_.begin(), ref_measures_.end());
	}


	public: void checker(checker_type const& checker)
	{
		checker_ = checker;
	}


	public: bool satisfied(measures_iterator_range_type const& measures) const
	{
		bool ok = false;

		ok = checker_(
					const_cast<self_type*>(this)->ref_measures_range_,
					measures
		);

		return ok;
	}

	public: real_type cost(measures_iterator_range_type const& measures) const
	{
		bool ok = false;

//		measures_internal_iterator b = const_cast<self_type*>(this)->ref_measures_.begin();
//		measures_internal_iterator e = const_cast<self_type*>(this)->ref_measures_.end();
//
//		measures_iterator_range_type ref_measures_range_ = ::dcs::iterator::make_iterator_range(b, e);
//
//		ok = checker_(
//					ref_measures_range_,
//					measures
//		);

		ok = checker_(
					const_cast<self_type*>(this)->ref_measures_range_,
					measures
		);

		return ok ? reward_ : penalty_;
	}

	/**
	 * \brief Compute the SLA cost/gain for to the given measures and according
	 *  to reference measures.
	 * \param cur_measures Range of iterators over the sequence of current SLA measures.
	 * \return The SLA cost/gain.
	 */
/*
	public: real_type cost(measures_const_iterator_range_type const& measures) const
	{
		bool ok = false;

		measures_internal_const_iterator b = ref_measures_.begin();
		measures_internal_const_iterator e = ref_measures_.end();

		measures_const_iterator_range_type ref_measures_range_ = ::dcs::iterator::make_iterator_range(b, e);

		ok = checker_(
					ref_measures_range_,
					measures
		);

		return ok ? reward_ : penalty_;
	}
*/


	/// The penalty paid when performance measures violate SLA.
	private: real_type penalty_;
	/// The reward gained when performance measures satisfy SLA.
	private: real_type reward_;
	/// The collection of reference performance measures
	private: measures_sequence_type ref_measures_;
	/// Convenience data member for storing the pair of iterators to the measures collection.
	private: mutable measures_iterator_range_type ref_measures_range_;
	/// The SLA checker functor.
	private: checker_type checker_;
};

}}} // Namespace dcs::perfeval::sla


#endif // DCS_PERFEVAL_SLA_STEP_COST_MODEL_HPP
