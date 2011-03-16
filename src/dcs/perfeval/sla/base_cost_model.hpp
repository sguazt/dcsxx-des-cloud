/*
 * \file dcs/perfeval/sla/base_cost_model.hpp
 *
 * \brief Base class for SLA cost models implementing the SlaCostModel concept.
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

#ifndef DCS_PERFEVAL_SLA_BASE_COST_MODEL_HPP
#define DCS_PERFEVAL_SLA_BASE_COST_MODEL_HPP


#include <dcs/iterator/any_forward_iterator.hpp>
//#include <dcs/iterator/iterator_range.hpp>
#include <dcs/memory.hpp>
#include <vector>


namespace dcs { namespace perfeval { namespace sla {

template <typename ValueT>
struct base_metric_checker
{
	typedef ValueT value_type;

	virtual ~base_metric_checker() {}


	bool operator()(value_type ref_value, value_type value)
	{
		return do_check(ref_value, value);
	}


	private: virtual bool do_check(value_type ref_value, value_type value) const = 0;
};


template <typename CheckerT>
class metric_checker_adaptor: public base_metric_checker<typename CheckerT::value_type>
{
	public: typedef CheckerT checker_type;
	public: typedef typename checker_type::value_type value_type;

	public: metric_checker_adaptor(CheckerT const& checker)
	: checker_(checker)
	{
	}


	private: bool do_check(value_type ref_value, value_type value) const
	{
		return checker_(ref_value, value);
	}

	private:checker_type checker_;
};


template <typename ValueT>
struct any_metric_checker
{
	any_metric_checker()
	{
	}

	template <typename CheckerT>
	any_metric_checker(CheckerT c)
	: ptr_c_(new metric_checker_adaptor<CheckerT>(c))
	{
	}

	bool operator()(ValueT ref_value, ValueT value) const
	{
		return ptr_c_->operator()(ref_value, value);
	}

	::dcs::shared_ptr< base_metric_checker<ValueT> > ptr_c_;
};


template <typename CategoryT, typename ValueT, typename CheckerT>
class slo_model
{
	public: typedef CategoryT metric_category_type;
	public: typedef ValueT value_type;
	public: typedef CheckerT checker_type;

	public: slo_model()
	{
	}


	public: slo_model(metric_category_type category, value_type value, checker_type const& checker)
	: category_(category),
	  ref_value_(value),
	  checker_(checker)
	{
	}


	public: metric_category_type category() const
	{
		return category_;
	}


	public: value_type reference_value() const
	{
		return ref_value_;
	}


	public: checker_type const& checker() const
	{
		return checker_;
	}


	public: bool check(value_type value) const
	{
		return checker_(ref_value_, value);
	}


	private: metric_category_type category_;
	private: value_type ref_value_;
	private: checker_type checker_;
};

/**
 * \brief Base class for SLA cost models implementing the SlaCostModel concept.
 *
 * \tparam RealT The type for real numbers.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename CategoryT, typename ValueT, typename RealT=ValueT>
class base_cost_model
{
	//[TODO]
	//DCS_CONCEPT_ASSERT((SlaCostModel<base_cost_model<RealT>))
	//[/TODO]

	public: typedef CategoryT metric_category_type;
	public: typedef ValueT value_type;
	/// Alias for the type of real numbers.
	public: typedef RealT real_type;
//	public: typedef ::std::pair<metric_category_type,value_type> metric_type;
	protected: typedef slo_model<metric_category_type,value_type,any_metric_checker<value_type> > slo_model_type; 
	protected: typedef ::dcs::iterator::any_forward_iterator<metric_category_type> metric_category_iterator;
	protected: typedef ::dcs::iterator::any_forward_iterator<value_type> metric_iterator;

	/// The virtual destructor to make inherited classes destructible.
	public: virtual ~base_cost_model() { }


	public: template <typename CheckerT>
		void add_slo(metric_category_type category, value_type value, CheckerT checker)
	{
		do_add_slo(slo_model_type(category, value, any_metric_checker<value_type>(checker)));
	}


	public: bool has_slo(metric_category_type category) const
	{
		return do_has_slo(category);
	}


	public: template <typename CategoryFwdIterT, typename MeasureFwdIterT>
		real_type score(CategoryFwdIterT category_first, CategoryFwdIterT category_last, MeasureFwdIterT measure_first) const
	{
		return do_score(
				::dcs::iterator::make_any_forward_iterator(category_first),
				::dcs::iterator::make_any_forward_iterator(category_last),
				::dcs::iterator::make_any_forward_iterator(measure_first)
			);
	}


	public: template <typename CategoryFwdIterT, typename MeasureFwdIterT>
		bool satisfied(CategoryFwdIterT category_first, CategoryFwdIterT category_last, MeasureFwdIterT measure_first) const
	{
		return do_satisfied(
				::dcs::iterator::make_any_forward_iterator(category_first),
				::dcs::iterator::make_any_forward_iterator(category_last),
				::dcs::iterator::make_any_forward_iterator(measure_first)
			);
	}


	public: ::std::vector<metric_category_type> slo_categories() const
	{
		return do_slo_categories();
	}


	public: value_type slo_value(metric_category_type category) const
	{
		return do_slo_value(category);
	}


	private: virtual void do_add_slo(slo_model_type const& slo) = 0;


	private: virtual bool do_has_slo(metric_category_type category) const = 0;


	private: virtual ::std::vector<metric_category_type> do_slo_categories() const = 0;


	private: virtual value_type do_slo_value(metric_category_type category) const = 0;


	private: virtual real_type do_score(metric_category_iterator category_first, metric_category_iterator category_last, metric_iterator metric_first) const = 0;


	private: virtual bool do_satisfied(metric_category_iterator category_first, metric_category_iterator category_last, metric_iterator metric_first) const = 0;
};

}}} // Namespace dcs::perfeval::sla


#endif // DCS_PERFEVAL_SLA_BASE_COST_MODEL_HPP
