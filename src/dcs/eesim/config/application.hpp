#ifndef DCS_EESIM_CONFIG_APPLICATION_HPP
#define DCS_EESIM_CONFIG_APPLICATION_HPP


#include <algorithm>
#include <dcs/eesim/config/application_controller.hpp>
#include <dcs/eesim/config/application_performance_model.hpp>
#include <dcs/eesim/config/application_simulation_model.hpp>
#include <dcs/eesim/config/application_sla.hpp>
#include <dcs/eesim/config/application_tier.hpp>
#include <dcs/eesim/config/physical_resource.hpp>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <vector>


namespace dcs { namespace eesim { namespace config {

template <typename RealT, typename UIntT>
struct application_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef ::std::map<physical_resource_category,real_type> reference_resource_container;
	typedef ::std::vector< application_tier_config<real_type> > tier_container;
	typedef application_performance_model_config<real_type,uint_type> performance_model_config_type;
	typedef application_simulation_model_config<real_type,uint_type> simulation_model_config_type;
	typedef application_sla_config<real_type> sla_config_type;
	typedef application_controller_config<RealT> controller_config_type;

	::std::string name;
	performance_model_config_type perf_model;
	simulation_model_config_type sim_model;
	sla_config_type sla;
	reference_resource_container reference_resources;
	tier_container tiers;
	controller_config_type controller;
};

template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, application_config<RealT,UIntT> const& app)
{
	os << "<(application)"
	   <<  " name: " << app.name
	   << ", sla: " << app.sla
	   << ", tiers: [";
	::std::copy(app.tiers.begin(),
				app.tiers.end(),
				::std::ostream_iterator< application_tier_config<RealT> >(os, ", "));

	os << "]";
	os << ", perf-model: " << app.perf_model
	   << ", sim-model: " << app.sim_model;
	typedef typename application_config<RealT,UIntT>::reference_resource_container::const_iterator iterator;
	iterator begin_it = app.reference_resources.begin();
	iterator end_it = app.reference_resources.end();
	os << ", {";
	for (iterator it = begin_it; it != end_it; ++it)
	{
		if (it != begin_it)
		{
			os << ", ";
		}
//		::std::string res;
//		switch (it->first)
//		{
//			case cpu_resource:
//				res = "cpu";
//				break;
//			case mem_resource:
//				res = "mem";
//				break;
//			case disk_resource:
//				res = "disk";
//				break;
//			case nic_resource:
//				res = "nic";
//				break;
//		}
//		os << res << ": " << it->second;
		os << it->first << ": " << it->second;
	}
	os << "}";

	os << ", " << app.controller;

	os << ">";

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_APPLICATION_HPP
