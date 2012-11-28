#ifndef DCS_DES_CLOUD_CONFIG_PHYSICAL_MACHINE_CONFIG_HPP
#define DCS_DES_CLOUD_CONFIG_PHYSICAL_MACHINE_CONFIG_HPP


#include <boost/variant.hpp>
#include <dcs/macro.hpp>
#include <iosfwd>


namespace dcs { namespace des { namespace cloud { namespace config {

enum physical_machine_controller_category
{
	conservative_physical_machine_controller,
	proportional_physical_machine_controller,
	dummy_physical_machine_controller
};


struct conservative_physical_machine_controller_config
{
};


struct proportional_physical_machine_controller_config
{
};


struct dummy_physical_machine_controller_config
{
};


template <typename RealT>
struct physical_machine_controller_config
{
	typedef RealT real_type;
	typedef conservative_physical_machine_controller_config conservative_controller_config_type;
	typedef proportional_physical_machine_controller_config proportional_controller_config_type;
	typedef dummy_physical_machine_controller_config dummy_controller_config_type;

	real_type sampling_time;
	physical_machine_controller_category category;
	::boost::variant<conservative_controller_config_type,
					 proportional_controller_config_type,
					 dummy_controller_config_type> category_conf;
};


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, physical_machine_controller_category category)
{
	switch (category)
	{
		case conservative_physical_machine_controller:
			os << "conservative";
			break;
		case proportional_physical_machine_controller:
			os << "proportional";
			break;
		case dummy_physical_machine_controller:
			os << "dummy";
			break;
	}

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, conservative_physical_machine_controller_config const& controller)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( controller );

	os << "<(conservative-physical-machine-controller)>";

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, proportional_physical_machine_controller_config const& controller)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( controller );

	os << "<(proportional-physical-machine-controller)>";

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, dummy_physical_machine_controller_config const& controller)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( controller );

	os << "<(dummy-physical-machine-controller)>";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, physical_machine_controller_config<RealT> const& controller)
{
	os << "<(physical-machine-controller)"
	   << " sampling-time: " << controller.sampling_time
	   << ", " << controller.category_conf
	   << ">";

	return os;
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_PHYSICAL_MACHINE_CONFIG_HPP
