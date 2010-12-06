#ifndef DCS_EESIM_CONFIG_DATA_CENTER_CONFIG_HPP
#define DCS_EESIM_CONFIG_DATA_CENTER_CONFIG_HPP


#include <algorithm>
#include <boost/variant.hpp>
#include <dcs/eesim/config/application.hpp>
#include <dcs/eesim/config/initial_placement_strategy.hpp>
#include <dcs/eesim/config/physical_machine.hpp>
#include <iostream>
#include <iterator>
#include <vector>


namespace dcs { namespace eesim { namespace config {

template <typename RealT, typename UIntT>
class data_center_config
{
	public: typedef RealT real_type;
	public: typedef UIntT uint_type;
	public: typedef application_config<real_type,uint_type> application_config_type;
	public: typedef physical_machine_config<real_type> physical_machine_config_type;
	public: typedef ::std::vector<application_config_type> application_config_container;
	public: typedef ::std::vector<physical_machine_config_type> physical_machine_config_container;
	public: typedef first_fit_initial_placement_strategy_config first_fit_initial_placement_strategy_config_type;
	public: typedef ::boost::variant<first_fit_initial_placement_strategy_config_type> initial_placement_strategy_config_type;


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


	public: void initial_placement_strategy_conf(initial_placement_strategy_config_type const& strategy_conf)
	{
		init_place_conf_ = strategy_conf;
	}


	public: initial_placement_strategy_config_type const& initial_placement_strategy_conf() const
	{
		return init_place_conf_;
	}


	public: void initial_placement_category(initial_placement_strategy_category category)
	{
		init_place_cat_ = category;
	}


	public: initial_placement_strategy_category initial_placement_category() const
	{
		return init_place_cat_;
	}


	private: application_config_container apps_;
	private: physical_machine_config_container machs_;
	private: initial_placement_strategy_category init_place_cat_;
	private: initial_placement_strategy_config_type init_place_conf_;
};


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, data_center_config<RealT,UIntT> const& dc)
{
	typedef typename data_center_config<RealT,UIntT>::application_config_type application_config_type;
	typedef typename data_center_config<RealT,UIntT>::physical_machine_config_type physical_machine_config_type;

	os << "<(data-center)";

	os << " [";
	::std::copy(dc.applications().begin(),
				dc.applications().end(),
				::std::ostream_iterator<application_config_type>(os, ", "));
	os << "]";

	os << ", [";
	::std::copy(dc.physical_machines().begin(),
				dc.physical_machines().end(),
				::std::ostream_iterator<physical_machine_config_type>(os, ", "));
	os << "]";

	os << ", " << dc.initial_placement_strategy_conf();

	os << ">";

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_DATA_CENTER_CONFIG_HPP
