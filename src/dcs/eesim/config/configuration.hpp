#ifndef DCS_EESIM_CONFIG_CONFIGURATION_HPP
#define DCS_EESIM_CONFIG_CONFIGURATION_HPP


#include <algorithm>
#include <dcs/debug.hpp>
#include <dcs/eesim/config/application.hpp>
#include <dcs/eesim/config/physical_machine.hpp>
#include <dcs/eesim/config/rng.hpp>
#include <dcs/eesim/config/simulation.hpp>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>


namespace dcs { namespace eesim { namespace config {


template <typename RealT, typename UIntT>
class configuration
{
	public: typedef RealT real_type;
	public: typedef UIntT uint_type;
	public: typedef rng_config<uint_type> rng_config_type;
	public: typedef simulation_config<real_type,uint_type> simulation_config_type;
	public: typedef application_config<real_type,uint_type> application_config_type;
	public: typedef physical_machine_config<real_type> physical_machine_config_type;
	public: typedef ::std::vector<application_config_type> application_config_container;
	public: typedef ::std::vector<physical_machine_config_type> physical_machine_config_container;


	public: void add_application(application_config_type const& app)
	{
		apps_.push_back(app);
	}


	public: application_config_container const& applications() const
	{
		return apps_;
	}


	public: void add_physical_machine(physical_machine_config_type const& mach)
	{
		machs_.push_back(mach);
	}


	public: physical_machine_config_container const& physical_machines() const
	{
		return machs_;
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
	private: application_config_container apps_;
	private: physical_machine_config_container machs_;
};


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, configuration<RealT,UIntT> const& conf)
{
	os << "<(configuration)";
	os << " " << conf.rng();
	os << ", " << conf.simulation();
	os << ", [";
	::std::copy(conf.applications().begin(),
				conf.applications().end(),
				::std::ostream_iterator< application_config<RealT,UIntT> >(os, ", "));
	os << "]";
	os << ",[";
	::std::copy(conf.physical_machines().begin(),
				conf.physical_machines().end(),
				::std::ostream_iterator< physical_machine_config<RealT> >(os, ", "));
	os << "]";
	os << ">";

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_CONFIGURATION_HPP
