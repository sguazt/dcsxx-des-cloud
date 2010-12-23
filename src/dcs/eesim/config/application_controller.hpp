#ifndef DCS_EESIM_CONFIG_APPLICATION_CONFIG_HPP
#define DCS_EESIM_CONFIG_APPLICATION_CONFIG_HPP


#include <boost/variant.hpp>
#include <dcs/eesim/config/numeric_matrix.hpp>
#include <dcs/macro.hpp>
#include <iostream>


namespace dcs { namespace eesim { namespace config {

enum application_controller_category
{
	dummy_application_controller,
	lqr_application_controller
};


struct dummy_application_controller_config
{
};


template <typename RealT>
struct lqr_application_controller_config
{
	typedef RealT real_type;

	numeric_matrix<real_type> Q; // state weighting matrix
	numeric_matrix<real_type> R; // control weighting matrix
	numeric_matrix<real_type> N; // state-control cross-coupling matrix
};


template <typename RealT>
struct application_controller_config
{
	typedef RealT real_type;
	typedef lqr_application_controller_config<real_type> lqr_controller_config_type;
	typedef dummy_application_controller_config dummy_controller_config_type;

	real_type sampling_time;
	application_controller_category category;
	::boost::variant<lqr_controller_config_type,
					 dummy_controller_config_type> category_conf;
};


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, application_controller_category category)
{
	switch (category)
	{
		case lqr_application_controller:
			os << "lqr";
			break;
		case dummy_application_controller:
			os << "lqr";
			break;
	}

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, dummy_application_controller_config const& controller)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( controller );

	os << "<(dummy-application-controller)>";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, lqr_application_controller_config<RealT> const& controller)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( controller );

	os << "<(lqr-application-controller)"
	   << " Q: " << controller.Q
	   << ", R: " << controller.R
	   << ", N: " << controller.N
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, application_controller_config<RealT> const& controller)
{
	os << "<(application-controller)"
	   << " sampling-time: " << controller.sampling_time
	   << ", " << controller.category_conf
	   << ">";

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_APPLICATION_CONFIG_HPP
