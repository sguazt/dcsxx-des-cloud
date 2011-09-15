#ifndef DCS_EESIM_CONFIG_MIGRATION_CONTROLLER_HPP
#define DCS_EESIM_CONFIG_MIGRATION_CONTROLLER_HPP


#include <boost/variant.hpp>
#include <dcs/macro.hpp>
#include <iosfwd>


namespace dcs { namespace eesim { namespace config {

enum migration_controller_category
{
	dummy_migration_controller,
	optimal_migration_controller
};


struct dummy_migration_controller_config
{
};


struct optimal_migration_controller_config
{
};


template <typename RealT>
struct migration_controller_config
{
	typedef RealT real_type;
	typedef dummy_migration_controller_config dummy_migration_controller_config_type;
	typedef optimal_migration_controller_config optimal_migration_controller_config_type;

	real_type sampling_time;
	migration_controller_category category;
	::boost::variant<dummy_migration_controller_config_type,
					 optimal_migration_controller_config_type> category_conf;
};


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, migration_controller_category category)
{
	switch (category)
	{
		case dummy_migration_controller:
			os << "dummy";
			break;
		case optimal_migration_controller:
			os << "optimal";
			break;
	}

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, dummy_migration_controller_config const& conf)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( conf );

	os << "<(dummy-migration-controller)>";

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, optimal_migration_controller_config const& conf)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( conf );

	os << "<(optimal-migration-controller)>";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, migration_controller_config<RealT> const& controller)
{
	os << "<(migration-controller)"
	  << " sampling-time: " << controller.sampling_time
	  << ", " << controller.category_conf;

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_MIGRATION_CONTROLLER_HPP
