#ifndef DCS_EESIM_CONFIG_MIGRATION_CONTROLLER_HPP
#define DCS_EESIM_CONFIG_MIGRATION_CONTROLLER_HPP


#include <boost/variant.hpp>
#include <dcs/macro.hpp>
#include <iosfwd>


namespace dcs { namespace eesim { namespace config {

enum migration_controller_category
{
	minlp_migration_controller,
	dummy_migration_controller
};


struct minlp_migration_controller_config
{
};


struct dummy_migration_controller_config
{
};


template <typename RealT>
struct migration_controller_config
{
	typedef RealT real_type;
	typedef minlp_migration_controller_config minlp_migration_controller_config_type;
	typedef dummy_migration_controller_config dummy_migration_controller_config_type;

	real_type sampling_time;
	migration_controller_category category;
	::boost::variant<minlp_migration_controller_config_type,
					 dummy_migration_controller_config_type> category_conf;
};


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, migration_controller_category category)
{
	switch (category)
	{
		case minlp_migration_controller:
			os << "lp";
			break;
		case dummy_migration_controller:
			os << "dummy";
			break;
	}

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, minlp_migration_controller_config const& conf)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( conf );

	os << "<(minlp-migration-controller)>";

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, dummy_migration_controller_config const& conf)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( conf );

	os << "<(dummy-migration-controller)>";

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
