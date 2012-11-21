#ifndef DCS_EESIM_CONFIG_APPLICATION_BUILDER_HPP
#define DCS_EESIM_CONFIG_APPLICATION_BUILDER_HPP


#include <dcs/eesim/config/probability_distribution.hpp>
#include <iomanip>
#include <iosfwd>


namespace dcs { namespace eesim { namespace config {

template <typename RealT, typename UIntT>
struct application_builder_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef probability_distribution_config<real_type> probability_distribution_type;

	uint_type min_num_instances;
	uint_type max_num_instances;
	uint_type num_preallocated_instances;
	bool preallocated_is_endless;
	probability_distribution_type arrival_distribution;
	probability_distribution_type runtime_distribution;
};


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, application_builder_config<RealT,UIntT> const& conf)
{
	os << "<(application-builder)"
	   << " min-num-instances: " << conf.min_num_instances
	   << ", max-num-instances: " << conf.max_num_instances
	   << ", num-preallocated-instances: " << conf.num_preallocated_instances
	   << ", preallocated-is-endless: " << ::std::boolalpha << conf.preallocated_is_endless
	   << ", arrival-distribution: " << conf.arrival_distribution
	   << ", runtime-distribution: " << conf.runtime_distribution
	   << ">";

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_APPLICATION_BUILDER_HPP
