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
	lqi_application_controller,
	lqr_application_controller,
	lqry_application_controller,
	qn_application_controller
};


struct dummy_application_controller_config
{
};


template <typename RealT, typename UIntT>
struct lq_application_controller_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;

	uint_type n_a; // output order of the ARX system model
	uint_type n_b; // input order of the ARX system model
	uint_type d; // input delay of the ARX system model
	numeric_matrix<real_type> Q; // state weighting matrix
	numeric_matrix<real_type> R; // control weighting matrix
	numeric_matrix<real_type> N; // state-control cross-coupling matrix
	real_type ewma_smoothing_factor; // The smoothing factor used by the EWMA filter
	real_type rls_forgetting_factor; // The forgetting factor used by the RLS algorithm
};


template <typename RealT, typename UIntT>
struct lqi_application_controller_config: public lq_application_controller_config<RealT,UIntT>
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef lq_application_controller_config<RealT,UIntT> base_type;

//	real_type integral_weight; // The weight assigned to the integral error at each control time
};


template <typename RealT, typename UIntT>
struct lqr_application_controller_config: public lq_application_controller_config<RealT,UIntT>
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef lq_application_controller_config<RealT,UIntT> base_type;
};


template <typename RealT, typename UIntT>
struct lqry_application_controller_config: public lq_application_controller_config<RealT,UIntT>
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef lq_application_controller_config<RealT,UIntT> base_type;
};


struct qn_application_controller_config
{
};


template <typename RealT, typename UIntT>
struct application_controller_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef dummy_application_controller_config dummy_controller_config_type;
	typedef lqi_application_controller_config<real_type,uint_type> lqi_controller_config_type;
	typedef lqr_application_controller_config<real_type,uint_type> lqr_controller_config_type;
	typedef lqry_application_controller_config<real_type,uint_type> lqry_controller_config_type;
	typedef qn_application_controller_config qn_controller_config_type;

	real_type sampling_time;
	application_controller_category category;
	::boost::variant<dummy_controller_config_type,
					 lqi_controller_config_type,
					 lqr_controller_config_type,
					 lqry_controller_config_type,
					 qn_controller_config_type> category_conf;
};


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, application_controller_category category)
{
	switch (category)
	{
		case dummy_application_controller:
			os << "dummy";
			break;
		case lqr_application_controller:
			os << "lqr";
			break;
		case qn_application_controller:
			os << "qn";
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


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, lq_application_controller_config<RealT,UIntT> const& controller)
{
	os << " n_a: " << controller.n_a
	   << ", n_b: " << controller.n_b
	   << ", d: " << controller.d
	   << ", Q: " << controller.Q
	   << ", R: " << controller.R
	   << ", N: " << controller.N
	   << ", smoothing-factor: " << controller.ewma_smoothing_factor
	   << ", forgetting-factor: " << controller.rls_forgetting_factor;

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, lqi_application_controller_config<RealT,UIntT> const& controller)
{
	os << "<(lqi-application-controller)"
	   << static_cast<lq_application_controller_config<RealT,UIntT> const&>(controller)
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, lqr_application_controller_config<RealT,UIntT> const& controller)
{
	os << "<(lqr-application-controller)"
	   << static_cast<lq_application_controller_config<RealT,UIntT> const&>(controller)
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, lqry_application_controller_config<RealT,UIntT> const& controller)
{
	os << "<(lqry-application-controller)"
	   << static_cast<lq_application_controller_config<RealT,UIntT> const&>(controller)
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, qn_application_controller_config const& controller)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( controller );

	os << "<(qn-application-controller)>";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, application_controller_config<RealT,UIntT> const& controller)
{
	os << "<(application-controller)"
	   << " sampling-time: " << controller.sampling_time
	   << ", " << controller.category_conf
	   << ">";

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_APPLICATION_CONFIG_HPP
