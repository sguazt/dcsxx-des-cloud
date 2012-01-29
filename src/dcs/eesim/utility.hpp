#ifndef DCS_EESIM_UTILITY_HPP
#define DCS_EESIM_UTILITY_HPP


#include <algorithm>
#include <cmath>
#include <dcs/macro.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/physical_resource.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <limits>
#include <utility>
#include <vector>


namespace dcs { namespace eesim {

template <typename RealT>
inline
RealT resource_scaling_factor(RealT source_capacity,
							  RealT target_capacity)
{
	return target_capacity/source_capacity;
}


template <typename RealT>
inline
RealT resource_scaling_factor(RealT source_capacity,
							  RealT source_share,
							  RealT target_capacity,
							  RealT target_share)
{
	return resource_scaling_factor(source_capacity*source_share, target_capacity*target_share);
}


template <typename RealT>
inline
RealT scale_resource_share(RealT source_capacity,
						   RealT target_capacity,
						   RealT source_share,
						   RealT target_share_threshold = ::std::numeric_limits<RealT>::infinity())
{
	RealT target_share = source_share/resource_scaling_factor(source_capacity, target_capacity);

	return ::std::isfinite(target_share_threshold)
		   ? ::std::min(target_share, target_share_threshold)
		   : target_share;
}


//template <typename RealT>
//inline
//RealT scale_resource_share(RealT source_capacity,
//						   RealT source_threshold,
//						   RealT target_capacity,
//						   RealT target_threshold,
//						   RealT source_share,
//						   RealT target_share_threshold = ::std::numeric_limits<RealT>::infinity())
//{
////	return source_share*(source_capacity*source_threshold)/(target_capacity*target_threshold);
////	return source_share*(source_capacity/target_capacity);
//	RealT target_share = source_share/resource_scaling_factor(source_capacity, source_threshold, target_capacity, target_threshold);
//
//	return ::std::isfinite(target_share_threshold)
//		   ? ::std::min(target_share, target_share_threshold)
//		   : target_share;
//}


//template <typename TraitsT, typename ShareForwardIterT>
//::std::vector<
//	::std::pair<physical_resource_category,typename TraitsT::real_type>
//> scale_resource_shares(physical_machine<TraitsT> const& source_pm,
//						physical_machine<TraitsT> const& target_pm,
//						ShareForwardIterT share_first,
//						ShareForwardIterT share_last)
//{
//	typedef TraitsT traits_type;
//	typedef typename traits_type::real_type real_type;
//	typedef physical_resource<traits_type> physical_resource_type;
//	typedef ::dcs::shared_ptr<physical_resource_type> physical_resource_pointer;
//	typedef ::std::pair<physical_resource_category,real_type> share_type;
//	typedef ::std::vector<share_type> share_container;
//
//	share_container scaled_shares;
//
//	while (share_first != share_last)
//	{
//		physical_resource_category category(share_first->first);
//		real_type share(share_first->second);
//
//		physical_resource_pointer ptr_src_res(source_pm.resource(category));
//		physical_resource_pointer ptr_dst_res(target_pm.resource(category));
//
//		share = scale_resource_share(ptr_src_res->capacity(),
//									 ptr_src_res->utilization_threshold(),
//									 ptr_dst_res->capacity(),
//									 ptr_dst_res->utilization_threshold(),
//									 share);
//
//		scaled_shares.push_back(share_type(category, share));
//
//		++share_first;
//	}
//
//
//	return scaled_shares;
//}


//template <typename TraitsT, typename ShareForwardIterT>
//::std::vector<
//	::std::pair<physical_resource_category,typename TraitsT::real_type>
//> scale_resource_shares(application_tier<TraitsT> const& tier,
//						physical_machine<TraitsT> const& target_pm)
//{
//	typedef TraitsT traits_type;
//	typedef typename traits_type::real_type real_type;
//	typedef physical_resource_view<traits_type> physical_resource_view_type;
//	typedef physical_resource<traits_type> physical_resource_type;
//	typedef ::dcs::shared_ptr<physical_resource_type> physical_resource_pointer;
//	typedef ::std::pair<physical_resource_category,real_type> share_type;
//	typedef ::std::vector<share_type> share_container;
//	typedef typename share_container::iterator share_iterator;
//
//	share_container shares(tier.resource_shares());
//	share_iterator share_end_it(shares.end());
//	for (share_iterator it = shares.begin(); it != share_end_it; ++it)
//	{
//		physical_resource_category category(it->first);
//		real_type share(it->second);
//
//		physical_resource_view_type src_res(tier.application().reference_resource(category));
//		physical_resource_pointer ptr_dst_res(target_pm.resource(category));
//
//		share = scale_resource_share(src_res.capacity(),
//									 src_res.utilization_threshold(),
//									 ptr_dst_res->capacity(),
//									 ptr_dst_res->utilization_threshold(),
//									 share);
//
//		it->second = share;
//	}
//
//
//	return shares;
//}


template <typename RealT>
inline
RealT scale_resource_utilization(RealT source_capacity,
								 RealT source_share,
								 RealT target_capacity,
								 RealT target_share,
								 RealT source_utilization,
								 RealT target_threshold = ::std::numeric_limits<RealT>::infinity())
{
	/// Utilization is insensible to the share.
	/// As a matter of fact, utilization is computed as busy-time/uptime:
	/// $U=\frac{B}{T}$
	///
	/// Suppose that a VM induces on a physical machine a utilization of $U=0.5$, when it owns the 100% of the physical resource capacity.
	/// If this VM gets a resource share of $s=0.5$, then the busy-time $B^{\prime}$, approximately, double since the VM will take the double of the time to complete the same operations.
	/// So, one can think that also the utilization double too, that is $U^{\prime}=1$.
	///
	/// However, that formula for computing the utilization assumes that the CPU is always used at 100%.
	/// Instead, it would be right to estimate the utilization as:
	/// 	$U=s\frac{B}{T}$
	/// so that the utilization induced by the VM is relative to the fraction of obtained CPU capacity.
	/// This is implies that the VMM is working in a non-conservative approach, so that if the physical machine has free capacity -- in excess to the one allocated to the VM -- this capacity is not assigned to the VM).
	///
	/// Thus, in the preceeding example we can state that
	/// $U^{\prime}=0.5\frac{B^{\prime}}{T}=0.5\frac{2B}{T}=\frac{B}{T}=U$
	/// that is, utilization is insensible to the share.
	/// Obviously, utilization will remain sensible to the CPU capacity.
	/// Specifically, if a VM is moved to a machine with capacity $C^{\prime}$ different from the initial one $C$, the scaling of utilization must be take care of this difference, that is:
	///     $U^{\prime}=U\frac{C}{C^{\prime}}$
	/// For instance, if the target machine has a capacity $C^{\prime}$ which is double of the initial one $C$, the new utilization is the half of the original one.

	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(source_share);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(target_share);

	//RealT target_utilization(source_utilization/resource_scaling_factor(source_capacity, source_share, target_capacity, target_share));
	RealT target_utilization(source_utilization/resource_scaling_factor(source_capacity, target_capacity));

	return ::std::isfinite(target_threshold)
		   ? ::std::min(target_utilization, target_threshold)
		   : target_utilization;
}


template <typename RealT>
inline
RealT scale_resource_utilization(RealT source_capacity,
								 RealT target_capacity,
								 RealT source_utilization,
								 RealT target_threshold = ::std::numeric_limits<RealT>::infinity())
{
	return scale_resource_utilization(source_capacity,
									  RealT(1),
									  target_capacity,
									  RealT(1),
									  source_utilization,
									  target_threshold);
}


}} // Namespace dcs::eesim


#endif // DCS_EESIM_UTILITY_HPP
