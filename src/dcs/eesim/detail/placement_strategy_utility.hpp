#ifndef DCS_EESIM_DETAIL_PLACEMENT_STRATEGY_UTILITY_HPP
#define DCS_EESIM_DETAIL_PLACEMENT_STRATEGY_UTILITY_HPP


#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace eesim { namespace detail {

//FIXME: only the single-resource (CPU) case is handled
template <typename PM>
struct pm_comparator
{
	bool operator()(::dcs::shared_ptr<PM> const& ptr_pm1, ::dcs::shared_ptr<PM> const& ptr_pm2)
	{
		return (((ptr_pm1->resource(cpu_resource_category)->capacity()*ptr_pm1->resource(cpu_resource_category)->utilization_threshold())
				 <
				 (ptr_pm2->resource(cpu_resource_category)->capacity()*ptr_pm2->resource(cpu_resource_category)->utilization_threshold()))
				||
				(((ptr_pm1->resource(cpu_resource_category)->capacity()*ptr_pm1->resource(cpu_resource_category)->utilization_threshold())
				 ==
				 (ptr_pm2->resource(cpu_resource_category)->capacity()*ptr_pm2->resource(cpu_resource_category)->utilization_threshold()))
				 &&
				 ptr_pm1->id() < ptr_pm2->id())
			   );
	}
};

}}} // Namespace dcs::eesim::detail


#endif // DCS_EESIM_DETAIL_PLACEMENT_STRATEGY_UTILITY_HPP
