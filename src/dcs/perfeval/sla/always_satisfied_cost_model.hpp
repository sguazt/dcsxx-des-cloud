/**
 * \file dcs/perfeval/sla/always_satisfied_cost_model.hpp
 *
 * \brief A SLA model whereby SLAs are always satisfied.
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

#ifndef DCS_PERFEVAL_SLA_ALWAYS_SATISFIED_COST_MODEL_HPP
#define DCS_PERFEVAL_SLA_ALWAYS_SATISFIED_COST_MODEL_HPP


#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/perfeval/sla/base_cost_model.hpp>
#include <map>
#include <stdexcept>
#include <vector>


namespace dcs { namespace perfeval { namespace sla {

/**
 * \brief SLA model whereby SLAs are always satisfied.
 *
 * \tparam RealT The type used for real numbers.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <
	typename CategoryT,
	typename ValueT,
	typename RealT=ValueT
>
class always_satisfied_cost_model: public base_cost_model<CategoryT,ValueT,RealT>
{
	private: typedef base_cost_model<CategoryT,ValueT,RealT> base_type;
	public: typedef CategoryT metric_category_type;
	public: typedef ValueT value_type;
	/// The type used for real numbers.
	public: typedef RealT real_type;
	private: typedef typename base_type::metric_iterator metric_iterator;
	private: typedef typename base_type::metric_category_iterator metric_category_iterator;
	private: typedef typename base_type::slo_model_type slo_model_type;
	private: typedef ::std::map<metric_category_type,slo_model_type> slo_model_container;


	/// Default constructor.
	public: always_satisfied_cost_model()
	: base_type()
	{
	}


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
			throw ::std::invalid_argument("[dcs::perfeval::sla::always_satisfied_cost_model::do_slo_value] SLO Category not found.")
		);

		return slos_.at(category).reference_value();
	}


	private: bool do_satisfied(metric_category_iterator category_first, metric_category_iterator category_last, metric_iterator metric_first) const
	{
		return true;
	}


	private: real_type do_score(metric_category_iterator category_first, metric_category_iterator category_last, metric_iterator metric_first) const
	{
		return 0;
	}


	private: slo_model_container slos_;
};

}}} // Namespace dcs::perfeval::sla


#endif // DCS_PERFEVAL_SLA_ALWAYS_SATISFIED_COST_MODEL_HPP
