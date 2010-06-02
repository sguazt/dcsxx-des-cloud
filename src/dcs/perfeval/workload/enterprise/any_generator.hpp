/**
 * \file dcs/perfeval/workload/enterprise/any_generator.hpp
 *
 * \brief Generic class for workload generators implementing the
 *  WorkloadGenerator concept.
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

#ifndef DCS_PERFEVAL_WORKLOAD_ENTERPRISE_ANY_GENERATOR_HPP
#define DCS_PERFEVAL_WORKLOAD_ENTERPRISE_ANY_GENERATOR_HPP


#include <dcs/meta/not.hpp>
#include <dcs/type_traits/is_same.hpp>
#include <dcs/meta/enable_if.hpp>
#include <dcs/perfeval/workload/enterprise/base_generator.hpp>
#include <dcs/perfeval/workload/enterprise/generator_adaptor.hpp>
#include <dcs/math/random/any_generator.hpp>
#include <dcs/memory.hpp>
#include <dcs/type_traits/remove_reference.hpp>
#include <dcs/util/holder.hpp>


namespace dcs { namespace perfeval { namespace workload { namespace enterprise {

/**
 * \brief Class for any workload generator implementing the WorkloadGenerator
 *  concept.
 *
 * \tparam RealT The type for real numbers.
 * \tparam RequestCategoryT The type for user request categories.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename RequestCategoryT, typename RealT>
struct any_generator
{
	//[TODO]
	//DCS_CONCEPT_ASSERT((WorkloadGenerator<any_generator<RealT>))
	//[/TODO]


	/// Alias for the type of user request categories.
	public: typedef RequestCategoryT request_category_type;
	/// Alias for the type of real numbers.
	public: typedef RealT real_type;


	/// Default constructor.
	public: any_generator()
	{
		// empty
	}


	/*
	 * \brief Wrap a concrete workload generator.
	 * \tparam WorkloadGeneratorT The concrete workload generator type.
	 */
	public: template <typename WorkloadGeneratorT>
		explicit any_generator(WorkloadGeneratorT const& generator)
		: ptr_generator_(new generator_adaptor<WorkloadGeneratorT>(generator))
	{
		// empty
	}


	/*
	 * \brief Wrap a concrete workload generator.
	 * \tparam WorkloadGeneratorT The concrete workload generator type.
	 *
	 * This is used for example when you want to pass a reference value instead
	 * of a const reference value.
	 */
	public: template <typename WorkloadGeneratorT>
		explicit any_generator(::dcs::util::holder<WorkloadGeneratorT> const& wrap_generator)
		: ptr_generator_(new generator_adaptor<WorkloadGeneratorT>(wrap_generator.get()))
	{
		// empty
	}


	// Set a new workload generator.
	public: template <typename WorkloadGeneratorT>
		void generator(WorkloadGeneratorT const& generator)
	{
		ptr_generator_ = new generator_adaptor<WorkloadGeneratorT>(generator);
	}


	/**
	 * \brief Compute the workload consumed for the given system utilization.
	 * \param u The system utilization.
	 * \return The workload consumed for the given system utilization.
	 */
	public: ::std::pair<request_category_type,real_type> operator()(::dcs::math::random::any_generator<real_type>& rng) const
	{
		return ptr_generator_->operator()(rng);
	}


	/**
	 * \brief Compute the workload consumed for the given system utilization.
	 * \param u The system utilization.
	 * \return The workload consumed for the given system utilization.
	 */
//	public: template <typename UniformRandomGeneratorT>
//		::std::pair<request_category_type,real_type> operator()(UniformRandomGeneratorT& rng) const
	public: template <typename UniformRandomGeneratorT>
		typename ::dcs::meta::enable_if<
			::dcs::meta::not_<
				::dcs::type_traits::is_same<
					UniformRandomGeneratorT,
					::dcs::math::random::any_generator<real_type>
				>
			>,
			::std::pair<request_category_type,real_type>
		>::type
		operator()(UniformRandomGeneratorT& rng) const
	{
		//FIXME: The code below does not work because the result type defined by
		// UniformRandomGeneratorT might be different from the one requested by
		// the return type (i.e., real_type)
		//
		//[don't work]
		//typedef typename ::dcs::math::random::make_any_generator_type<UniformRandomGeneratorT&>::type any_rng_type;
		//any_rng_type any_rng(::dcs::math::random::make_any_generator<UniformRandomGeneratorT&>(rng));
		//[/don't work]

		// So in order to keep stuff working we used this dirt trick, that is
		// we wrap the original random generator inside a uniform generator with
		// real result type.
		typedef typename ::dcs::math::random::make_any_generator_type< dcs::math::random::uniform_real_adaptor<UniformRandomGeneratorT&,real_type> >::type any_rng_type;
		any_rng_type any_rng(::dcs::math::random::make_any_generator< dcs::math::random::uniform_real_adaptor<UniformRandomGeneratorT&,real_type> >(rng));

		return ptr_generator_->operator()(any_rng);
	}


	private: ::dcs::shared_ptr< base_generator<request_category_type,real_type> > ptr_generator_; // shared_ptr needed in order to keep alive the pointer during object copying
};


template <typename WorkloadGeneratorT, typename WorkloadGeneratorTraitsT=typename ::dcs::type_traits::remove_reference<WorkloadGeneratorT>::type>
struct make_any_generator_type
{
	typedef any_generator<
				typename WorkloadGeneratorTraitsT::request_category_type,
				typename WorkloadGeneratorTraitsT::real_type
		> type;
};


namespace detail {

template <typename WorkloadGeneratorT, typename WorkloadGeneratorTraitsT=typename ::dcs::type_traits::remove_reference<WorkloadGeneratorT>::type>
struct make_any_generator_impl;


template <typename WorkloadGeneratorT, typename WorkloadGeneratorTraitsT>
struct make_any_generator_impl
{
	typedef typename make_any_generator_type<WorkloadGeneratorT,WorkloadGeneratorTraitsT>::type any_generator_type;
	static any_generator_type apply(WorkloadGeneratorT& generator)
	{
		return any_generator_type(generator);
	}
};


template <typename WorkloadGeneratorT, typename WorkloadGeneratorTraitsT>
struct make_any_generator_impl<WorkloadGeneratorT&,WorkloadGeneratorTraitsT>
{
	typedef typename make_any_generator_type<WorkloadGeneratorT,WorkloadGeneratorTraitsT>::type any_generator_type;
	static any_generator_type apply(WorkloadGeneratorT& generator)
	{
		::dcs::util::holder<WorkloadGeneratorT&> wrap_generator(generator);
		return any_generator_type(wrap_generator);
	}
};

} // Namespace detail


template <typename WorkloadGeneratorT>
typename make_any_generator_type<WorkloadGeneratorT>::type make_any_generator(WorkloadGeneratorT generator)
{
	return detail::make_any_generator_impl<WorkloadGeneratorT>::apply(generator);
}

}}}} // Namespace dcs::perfeval::enterprise::workload


#endif // DCS_PERFEVAL_WORKLOAD_ENTERPRISE_ANY_GENERATOR_HPP
