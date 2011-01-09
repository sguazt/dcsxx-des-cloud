#ifndef DCS_EESIM_UTILITY_HPP
#define DCS_EESIM_UTILITY_HPP


#include <dcs/macro.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/physical_resource.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <utility>
#include <vector>


namespace dcs { namespace eesim {

template <typename RealT>
inline
RealT resource_scaling_factor(RealT source_capacity,
							  RealT source_threshold,
							  RealT target_capacity,
							  RealT target_threshold)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( source_threshold );
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( target_threshold );

//	return (source_capacity*source_threshold)/(target_capacity*target_threshold);
	return target_capacity/source_capacity;
}


template <typename RealT>
inline
RealT scale_resource_share(RealT source_capacity,
						   RealT source_threshold,
						   RealT target_capacity,
						   RealT target_threshold,
						   RealT source_share)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( source_threshold );
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( target_threshold );

//	return source_share*(source_capacity*source_threshold)/(target_capacity*target_threshold);
//	return source_share*(source_capacity/target_capacity);
	return source_share/resource_scaling_factor(source_capacity, source_threshold, target_capacity, target_threshold);
}


template <typename TraitsT, typename ShareForwardIterT>
::std::vector<
	::std::pair<physical_resource_category,typename TraitsT::real_type>
> scale_resource_shares(physical_machine<TraitsT> const& source_pm,
						physical_machine<TraitsT> const& target_pm,
						ShareForwardIterT share_first,
						ShareForwardIterT share_last)
{
	typedef TraitsT traits_type;
	typedef typename traits_type::real_type real_type;
	typedef physical_resource<traits_type> physical_resource_type;
	typedef ::dcs::shared_ptr<physical_resource_type> physical_resource_pointer;
	typedef ::std::pair<physical_resource_category,real_type> share_type;
	typedef ::std::vector<share_type> share_container;

	share_container scaled_shares;

	while (share_first != share_last)
	{
		physical_resource_category category(share_first->first);
		real_type share(share_first->second);

		physical_resource_pointer ptr_src_res(source_pm.resource(category));
		physical_resource_pointer ptr_dst_res(target_pm.resource(category));

		share = scale_resource_share(ptr_src_res->capacity(),
									 ptr_src_res->utilization_threshold(),
									 ptr_dst_res->capacity(),
									 ptr_dst_res->utilization_threshold(),
									 share);

		scaled_shares.push_back(share_type(category, share));

		++share_first;
	}


	return scaled_shares;
}


template <typename TraitsT, typename ShareForwardIterT>
::std::vector<
	::std::pair<physical_resource_category,typename TraitsT::real_type>
> scale_resource_shares(application_tier<TraitsT> const& tier,
						physical_machine<TraitsT> const& target_pm)
{
	typedef TraitsT traits_type;
	typedef typename traits_type::real_type real_type;
	typedef physical_resource_view<traits_type> physical_resource_view_type;
	typedef physical_resource<traits_type> physical_resource_type;
	typedef ::dcs::shared_ptr<physical_resource_type> physical_resource_pointer;
	typedef ::std::pair<physical_resource_category,real_type> share_type;
	typedef ::std::vector<share_type> share_container;
	typedef typename share_container::iterator share_iterator;

	share_container shares(tier.resource_shares());
	share_iterator share_end_it(shares.end());
	for (share_iterator it = shares.begin(); it != share_end_it; ++it)
	{
		physical_resource_category category(it->first);
		real_type share(it->second);

		physical_resource_view_type src_res(tier.application().reference_resource(category));
		physical_resource_pointer ptr_dst_res(target_pm.resource(category));

		share = scale_resource_share(src_res.capacity(),
									 src_res.utilization_threshold(),
									 ptr_dst_res->capacity(),
									 ptr_dst_res->utilization_threshold(),
									 share);

		it->second = share;
	}


	return shares;
}

}} // Namespace dcs::eesim


#endif // DCS_EESIM_UTILITY_HPP
