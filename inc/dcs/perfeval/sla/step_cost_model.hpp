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
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_PERFEVAL_SLA_STEP_COST_MODEL_HPP
#define DCS_PERFEVAL_SLA_STEP_COST_MODEL_HPP


#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/perfeval/sla/base_cost_model.hpp>
#include <dcs/iterator/any_forward_iterator.hpp>
//#include <dcs/iterator/iterator_range.hpp>
#include <functional>
#include <iterator>
#include <map>
#include <stdexcept>
#include <vector>


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
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
//template <typename RealT>
struct default_checker
{
//	/// The type used for real numbers.
//	typedef RealT real_type;

	/// The type of the range of iterators over the measures sequence.
//	typedef ::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator<real_type> > iterator_range_type;
//	typedef ::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator<real_type const> > const_iterator_range_type;

	/**
	 * \brief The checker functor.
	 * \param r The iterator range for reference measures.
	 * \param c The iterator range for current measures.
	 */
	template <typename ForwardIterT, typename RefForwardIterT>
	bool operator()(ForwardIterT measure_first, ForwardIterT measure_last, RefForwardIterT ref_measure_first) const
	{
		while (measure_first != measure_last)
		{
			if (*measure_first > *ref_measure_first)
			{
				return false;
			}
			//return (c < r || c == r); // this use the std::lexicographical_compare function
			++measure_first;
		}

		return true;
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
 * - a constant number \f$R\f$ denoting the revenue obtained in case of SLA
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
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <
	typename CategoryT,
	typename ValueT,
	typename RealT=ValueT
>
class step_cost_model: public base_cost_model<CategoryT,ValueT,RealT>
{
	private: typedef base_cost_model<CategoryT,ValueT,RealT> base_type;
	public: typedef CategoryT metric_category_type;
	public: typedef ValueT value_type;
	/// The type used for real numbers.
	public: typedef RealT real_type;
	/// The type used for the SLA checker functor.
//	private: typedef step_cost_model<real_type,checker_type> self_type;
	private: typedef typename base_type::metric_iterator metric_iterator;
	private: typedef typename base_type::metric_category_iterator metric_category_iterator;
	private: typedef typename base_type::slo_model_type slo_model_type;
//	public: typedef ::dcs::iterator::any_forward_iterator<real_type const> measures_const_iterator;
//	public: typedef ::dcs::iterator::iterator_range<measures_iterator> measures_iterator_range_type;
//	public: typedef ::dcs::iterator::iterator_range<measures_const_iterator> measures_const_iterator_range_type;
	private: typedef ::std::map<metric_category_type,slo_model_type> slo_model_container;
//	private: typedef typename measures_container::iterator measures_internal_iterator;
//	private: typedef typename measures_container::const_iterator measures_internal_const_iterator;


	/// Default constructor.
	public: step_cost_model()
	: base_type()
	{
	}


	/// A constructor.
	public: step_cost_model(real_type penalty_cost, real_type revenue_cost)
		: penalty_(penalty_cost),
		  revenue_(revenue_cost)
	{
		// empty
	}


	// Compiler-generated copy constructor and copy-assignment are fine.

//	/// The copy constructor
//	public: step_cost_model(step_cost_model const& that)
//		: base_type(that),
//		  penalty_(that.penalty_),
//		  revenue_(that.revenue_)
//	{
//	}


	public: void penalty(real_type value)
	{
		penalty_ = value;
	}


	public: real_type penalty() const
	{
		return penalty_;
	}


	public: void revenue(real_type value)
	{
		revenue_ = value;
	}


	public: real_type revenue() const
	{
		return revenue_;
	}


//	public: template <typename ForwardIterT>
//		void reference_measures(ForwardIterT measures_begin, ForwardIterT measures_end)
//	{
//		  ref_measures_ = measures_container(measures_begin, measures_end);
////		  ref_measures_range_ = measures_iterator_range_type(ref_measures_.begin(), ref_measures_.end());
//	}


	private: void do_add_slo(slo_model_type const& slo)
	{
		slos_[slo.category()] = slo;
	}


	private: bool do_has_slo(metric_category_type category) const
	{
		return slos_.count(category) > 0;
	}


	private: ::std::vector<metric_category_type> do_slo_categories() const
	{
		typedef typename slo_model_container::const_iterator iterator;

		::std::vector<metric_category_type> categories;

		iterator end_it = slos_.end();
		for (iterator it = slos_.begin(); it != end_it; ++it)
		{
			categories.push_back(it->first);
		}

		return categories;
	}


	private: value_type do_slo_value(metric_category_type category) const
	{
		// pre: category must already be inserted.
		DCS_ASSERT(
			this->has_slo(category),
			throw ::std::invalid_argument("[dcs::perfeval::sla::step_cost_model::do_slo_value] SLO Category not found.")
		);

		return slos_.at(category).reference_value();
	}


	private: bool do_satisfied(metric_category_iterator category_first, metric_category_iterator category_last, metric_iterator metric_first) const
	{
		bool ok(true);

		while (category_first != category_last && ok)
		{
			ok &= slos_.at(*category_first).check(*metric_first);

			++category_first;
			++metric_first;
		}

		return ok;
	}


	private: real_type do_score(metric_category_iterator category_first, metric_category_iterator category_last, metric_iterator metric_first) const
	{
		bool ok(false);

		ok = this->satisfied(category_first, category_last, metric_first);

		return ok ? revenue_ : penalty_;
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

		return ok ? revenue_ : penalty_;
	}
*/


	/// The penalty paid when performance measures violate SLA.
	private: real_type penalty_;
	/// The revenue gained when performance measures satisfy SLA.
	private: real_type revenue_;
	private: slo_model_container slos_;
	/// The collection of reference performance measures
//	private: measures_container ref_measures_;
//	/// Convenience data member for storing the pair of iterators to the measures collection.
//	private: mutable measures_iterator_range_type ref_measures_range_;
	/// The SLA checker functor.
//	private: checker_type checker_;
};

}}} // Namespace dcs::perfeval::sla


#endif // DCS_PERFEVAL_SLA_STEP_COST_MODEL_HPP
