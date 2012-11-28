#ifndef DCS_DES_CLOUD_CONFIG_PHYSICAL_RESOURCE_HPP
#define DCS_DES_CLOUD_CONFIG_PHYSICAL_RESOURCE_HPP


#include <boost/variant.hpp>
#include <dcs/des/cloud/config/energy_model.hpp>
#include <iosfwd>
#include <string>


namespace dcs { namespace des { namespace cloud { namespace config {

enum physical_resource_category
{
	cpu_resource,
	mem_resource,
	disk_resource,
	nic_resource
};


template <typename RealT>
struct physical_resource_config
{
	typedef RealT real_type;
	typedef constant_energy_model_config<real_type> constant_energy_model_config_type;
	typedef fan2007_energy_model_config<real_type> fan2007_energy_model_config_type;

	::std::string name;
	physical_resource_category type;
	real_type capacity;
	real_type threshold;
	energy_model_category energy_model_type;
	::boost::variant<constant_energy_model_config_type,
					 fan2007_energy_model_config_type> energy_model_conf;
};


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, physical_resource_category category)
{
	switch (category)
	{
		case cpu_resource:
			os << "cpu";
			break;
		case mem_resource:
			os << "mem";
			break;
		case disk_resource:
			os << "disk";
			break;
		case nic_resource:
			os << "nic";
			break;
	}

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, physical_resource_config<RealT> const& res)
{
	os << "<(physical_resource)"
	   << " name: " << res.name
	   << ", type: " << res.type
	   << ", capacity: " << res.capacity
	   << ", threshold: " << res.threshold
	   << ", " << res.energy_model_conf
	   << ">";
//	   << ", energy-model: ";
//
//	switch (res.energy_model_type)
//	{
//		case constant_energy_model:
//			{
//				typedef physical_resource_type::constant_energy_model_config_type energy_model_config_type;
//
//				energy_model_config_type const& model = ::boost::get<energy_model_config_type>(res.energy_model_conf);
//
//				os << model;
//			}
//			break;
//		case fan2007_energy_model:
//			{
//				typedef physical_resource_type::fan2007_energy_model_config_type energy_model_config_type;
//
//				energy_model_config_type const& model = ::boost::get<energy_model_config_type>(res.energy_model_conf);
//
//				os << model;
//			}
//			break;
//	}
//
//	os << ">";

	return os;
}


}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_PHYSICAL_RESOURCE_HPP
