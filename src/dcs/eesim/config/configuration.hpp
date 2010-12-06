#ifndef DCS_EESIM_CONFIG_CONFIGURATION_HPP
#define DCS_EESIM_CONFIG_CONFIGURATION_HPP


#include <dcs/eesim/config/data_center.hpp>
#include <dcs/eesim/config/rng.hpp>
#include <dcs/eesim/config/simulation.hpp>
#include <iostream>


namespace dcs { namespace eesim { namespace config {


template <typename RealT, typename UIntT>
class configuration
{
	public: typedef RealT real_type;
	public: typedef UIntT uint_type;
	public: typedef rng_config<uint_type> rng_config_type;
	public: typedef simulation_config<real_type,uint_type> simulation_config_type;
	public: typedef data_center_config<real_type,uint_type> data_center_config_type;


	public: void data_center(data_center_config_type const& dc)
	{
		dc_ = dc;
	}


	public: data_center_config_type const& data_center() const
	{
		return dc_;
	}


	public: void rng(rng_config_type const& rng)
	{
		rng_ = rng;
	}


	public: rng_config_type const& rng() const
	{
		return rng_;
	}


	public: void simulation(simulation_config_type const& sim)
	{
		sim_ = sim;
	}


	public: simulation_config_type const& simulation() const
	{
		return sim_;
	}


	private: rng_config_type rng_;
	private: simulation_config_type sim_;
	private: data_center_config_type dc_;
};


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, configuration<RealT,UIntT> const& conf)
{
	os << "<(configuration)"
	   << " " << conf.rng()
	   << ", " << conf.simulation()
	   << ", " << conf.data_center()
	   << ">";

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_CONFIGURATION_HPP
