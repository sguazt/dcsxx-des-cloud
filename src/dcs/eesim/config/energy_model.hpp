#ifndef DCS_EESIM_CONFIG_ENERGY_MODEL_HPP
#define DCS_EESIM_CONFIG_ENERGY_MODEL_HPP


#include <iostream>


namespace dcs { namespace eesim { namespace config {

enum energy_model_category
{
	constant_energy_model,
	fan2007_energy_model
};


template <typename RealT>
struct constant_energy_model_config
{
	typedef RealT real_type;

	real_type c0;
};


template <typename RealT>
struct fan2007_energy_model_config
{
	typedef RealT real_type;

	real_type c0;
	real_type c1;
	real_type c2;
	real_type r;
};


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, constant_energy_model_config<RealT> const& model)
{
	return os << "<(constant_energy_model)"
			  << " c0: " << model.c0
			  << ">";
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, fan2007_energy_model_config<RealT> const& model)
{
	return os << "<(fan2007_energy_model)"
			  << " c0: " << model.c0
			  << ", c1: " << model.c1
			  << ", c2: " << model.c2
			  << ", r: " << model.r
			  << ">";
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_ENERGY_MODEL_HPP
