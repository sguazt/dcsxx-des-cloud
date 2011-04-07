#ifndef DCS_EESIM_CONFIG_APPLICATION_CONFIG_HPP
#define DCS_EESIM_CONFIG_APPLICATION_CONFIG_HPP


#include <boost/variant.hpp>
#include <dcs/eesim/config/numeric_matrix.hpp>
#include <dcs/macro.hpp>
#include <iostream>


namespace dcs { namespace eesim { namespace config {

enum system_identification_category
{
	rls_bittanti1990_system_identification,
	rls_ff_system_identification,
	rls_kulhavy1984_system_identification,
	rls_park1991_system_identification
};


template <typename RealT, typename UIntT>
struct base_rls_system_identification_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;

	base_rls_system_identification_config()
	: mimo_as_miso(false),
	  enable_max_cov_heuristic(false),
	  max_cov_heuristic_value(0),
	  enable_cond_cov_heuristic(false),
	  cond_cov_heuristic_trust_digits(0)
	{
	}

	bool mimo_as_miso;
	bool enable_max_cov_heuristic;
	real_type max_cov_heuristic_value;
	bool enable_cond_cov_heuristic;
	uint_type cond_cov_heuristic_trust_digits;
//	bool enable_ewma_smoothing_filter;
//	real_type ewma_smoothing_factor;
};


template <typename RealT, typename UIntT>
struct rls_bittanti1990_system_identification_config: public base_rls_system_identification_config<RealT,UIntT>
{
	typedef base_rls_system_identification_config<RealT,UIntT> base_type;
	typedef RealT real_type;
	typedef UIntT uint_type;

	rls_bittanti1990_system_identification_config()
	: base_type(),
	  forgetting_factor(0),
	  delta(0)
	{
	}

	real_type forgetting_factor; // The forgetting factor.
	real_type delta; // The Bittanti's delta correction
};


template <typename RealT, typename UIntT>
struct rls_ff_system_identification_config: public base_rls_system_identification_config<RealT,UIntT>
{
	typedef base_rls_system_identification_config<RealT,UIntT> base_type;
	typedef RealT real_type;
	typedef UIntT uint_type;

	rls_ff_system_identification_config()
	: base_type(),
	  forgetting_factor(0)
	{
	}

	real_type forgetting_factor; // The forgetting factor
};


template <typename RealT, typename UIntT>
struct rls_kulhavy1984_system_identification_config: public base_rls_system_identification_config<RealT,UIntT>
{
	typedef base_rls_system_identification_config<RealT,UIntT> base_type;
	typedef RealT real_type;
	typedef UIntT uint_type;

	rls_kulhavy1984_system_identification_config()
	: base_type(),
	  forgetting_factor(0)
	{
	}

	real_type forgetting_factor; // The forgetting factor.
};


template <typename RealT, typename UIntT>
struct rls_park1991_system_identification_config: public base_rls_system_identification_config<RealT,UIntT>
{
	typedef base_rls_system_identification_config<RealT,UIntT> base_type;
	typedef RealT real_type;
	typedef UIntT uint_type;

	rls_park1991_system_identification_config()
	: base_type(),
	  forgetting_factor(0),
	  rho(0)
	{
	}

	real_type forgetting_factor; // The minimum forgetting factor
	real_type rho; // The sensivity gain
};


enum application_controller_category
{
	dummy_application_controller,
	lqi_application_controller,
//	lqiy_application_controller,
	lqr_application_controller,
	lqry_application_controller,
	matlab_lqi_application_controller,
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
	typedef rls_bittanti1990_system_identification_config<real_type,uint_type> rls_bittanti1990_system_identification_config_type;
	typedef rls_ff_system_identification_config<real_type,uint_type> rls_ff_system_identification_config_type;
	typedef rls_kulhavy1984_system_identification_config<real_type,uint_type> rls_kulhavy1984_system_identification_config_type;
	typedef rls_park1991_system_identification_config<real_type,uint_type> rls_park1991_system_identification_config_type;

	lq_application_controller_config()
	: n_a(0),
	  n_b(0),
	  d(0)
	{
	}

	uint_type n_a; // output order of the ARX system model
	uint_type n_b; // input order of the ARX system model
	uint_type d; // input delay of the ARX system model
	numeric_matrix<real_type> Q; // state weighting matrix
	numeric_matrix<real_type> R; // control weighting matrix
	numeric_matrix<real_type> N; // state-control cross-coupling matrix
	real_type ewma_smoothing_factor; // The smoothing factor used by the EWMA filter
	system_identification_category ident_category;
	::boost::variant<rls_bittanti1990_system_identification_config_type,
					 rls_ff_system_identification_config_type,
					 rls_kulhavy1984_system_identification_config_type,
					 rls_park1991_system_identification_config_type> ident_category_conf;
//	real_type rls_forgetting_factor; // The forgetting factor used by the RLS algorithm
};


template <typename RealT, typename UIntT>
struct lqi_application_controller_config: public lq_application_controller_config<RealT,UIntT>
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef lq_application_controller_config<RealT,UIntT> base_type;

//	real_type integral_weight; // The weight assigned to the integral error at each control time
};


//template <typename RealT, typename UIntT>
//struct lqiy_application_controller_config: public lq_application_controller_config<RealT,UIntT>
//{
//	typedef RealT real_type;
//	typedef UIntT uint_type;
//	typedef lq_application_controller_config<RealT,UIntT> base_type;
//
////	real_type integral_weight; // The weight assigned to the integral error at each control time
//};


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


template <typename RealT, typename UIntT>
struct matlab_lqi_application_controller_config: public lq_application_controller_config<RealT,UIntT>
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef lq_application_controller_config<RealT,UIntT> base_type;

//	real_type integral_weight; // The weight assigned to the integral error at each control time
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
//	typedef lqiy_application_controller_config<real_type,uint_type> lqiy_controller_config_type;
	typedef lqr_application_controller_config<real_type,uint_type> lqr_controller_config_type;
	typedef lqry_application_controller_config<real_type,uint_type> lqry_controller_config_type;
	typedef matlab_lqi_application_controller_config<real_type,uint_type> matlab_lqi_controller_config_type;
	typedef qn_application_controller_config qn_controller_config_type;

	real_type sampling_time;
	application_controller_category category;
	::boost::variant<dummy_controller_config_type,
					 lqi_controller_config_type,
//					 lqiy_controller_config_type,
					 lqr_controller_config_type,
					 lqry_controller_config_type,
					 matlab_lqi_controller_config_type,
					 qn_controller_config_type> category_conf;
};


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, base_rls_system_identification_config<RealT,UIntT> const& sys_ident)
{
	os << " mimo-as-miso: " << ::std::boolalpha << sys_ident.mimo_as_miso
	   << ", max-covariance-heuristic: " << ::std::boolalpha << sys_ident.enable_max_cov_heuristic
	   << ", max-covariance-heuristic-max-value: " << sys_ident.max_cov_heuristic_value
	   << ", condition-number-covariance-heuristic: " << ::std::boolalpha << sys_ident.enable_cond_cov_heuristic
	   << ", condition-number-covariance-heuristic-trusted-digits: " << sys_ident.cond_cov_heuristic_trust_digits;

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, rls_bittanti1990_system_identification_config<RealT,UIntT> const& sys_ident)
{
	os << "<(rls-bittanti1990-system-identification)"
	   << static_cast<base_rls_system_identification_config<RealT,UIntT> const&>(sys_ident)
	   << ", forgetting-factor: " << sys_ident.forgetting_factor
	   << ", correction-factor: " << sys_ident.delta
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, rls_ff_system_identification_config<RealT,UIntT> const& sys_ident)
{
	os << "<(rls-ff-system-identification)"
	   << static_cast<base_rls_system_identification_config<RealT,UIntT> const&>(sys_ident)
	   << ", forgetting-factor: " << sys_ident.forgetting_factor
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, rls_kulhavy1984_system_identification_config<RealT,UIntT> const& sys_ident)
{
	os << "<(rls-kulhavy1984-system-identification)"
	   << static_cast<base_rls_system_identification_config<RealT,UIntT> const&>(sys_ident)
	   << ", forgetting-factor: " << sys_ident.forgetting_factor
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, rls_park1991_system_identification_config<RealT,UIntT> const& sys_ident)
{
	os << "<(rls-park1991-system-identification)"
	   << static_cast<base_rls_system_identification_config<RealT,UIntT> const&>(sys_ident)
	   << ", forgetting-factor: " << sys_ident.forgetting_factor
	   << ", sensitivity-gain: " << sys_ident.rho
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, application_controller_category category)
{
	switch (category)
	{
		case dummy_application_controller:
			os << "dummy";
			break;
		case lqi_application_controller:
			os << "lqi";
			break;
		case lqr_application_controller:
			os << "lqr";
			break;
		case lqry_application_controller:
			os << "lqry";
			break;
		case matlab_lqi_application_controller:
			os << "matlab_lqi";
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
//	   << ", forgetting-factor: " << controller.rls_forgetting_factor
	   << ", system-identification: " << controller.ident_category_conf
	   << ", smoothing-factor: " << controller.ewma_smoothing_factor;

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


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, matlab_lqi_application_controller_config<RealT,UIntT> const& controller)
{
	os << "<(matlab_lqi-application-controller)"
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
