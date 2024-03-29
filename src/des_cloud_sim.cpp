/**
 * \file src/des/cloud.cpp
 *
 * \brief DCS DES Cloud application entry-point.
 *
 * Copyright (C) 2009-2011  Distributed Computing System (DCS) Group, Computer
 * Science Department - University of Piemonte Orientale, Alessandria (Italy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <dcs/assert.hpp>
#include <dcs/des/engine.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/cloud/config/configuration.hpp>
#include <dcs/des/cloud/config/operation/make_data_center.hpp>
#include <dcs/des/cloud/config/operation/make_data_center_manager.hpp>
#include <dcs/des/cloud/config/operation/make_des_engine.hpp>
#include <dcs/des/cloud/config/operation/make_logger.hpp>
#include <dcs/des/cloud/config/operation/make_random_number_generator.hpp>
#include <dcs/des/cloud/config/operation/read_file.hpp>
#include <dcs/des/cloud/config/yaml.hpp>
#include <dcs/des/cloud/data_center.hpp>
#include <dcs/des/cloud/data_center_manager.hpp>
#include <dcs/des/cloud/physical_resource_category.hpp>
#include <dcs/des/cloud/performance_measure_category.hpp>
#include <dcs/des/cloud/registry.hpp>
#include <dcs/des/cloud/virtual_machine.hpp>
#include <dcs/des/cloud/traits.hpp>
#include <dcs/des/cloud/logging/base_logger.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
//#include <dcs/math/random/any_generator.hpp>
#include <dcs/math/random.hpp>
#include <dcs/memory.hpp>
#include <exception>
#ifdef DCS_DEBUG
# if __GNUC__
#  include <execinfo.h>
# endif // __GNUC__
#endif // DCS_DEBUG
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>


static std::string prog_name;

enum configuration_category
{
	yaml_configuration/*, TODO:
	xml_configuration*/
};


//namespace detail { namespace /*<unnamed>*/ {
//
//template <typename RealT>
//void test_rng(dcs::shared_ptr< dcs::math::random::base_generator<RealT> > const& ptr_rng)
//{
//	DCS_DEBUG_TRACE("RNG: ");
//	DCS_DEBUG_TRACE("  min: " << ptr_rng->min());
//	DCS_DEBUG_TRACE("  max: " << ptr_rng->max());
//}
//
//}} // namespace detail::<unnamed>


typedef double real_type;
typedef unsigned long uint_type;
typedef long int_type;
typedef dcs::des::engine<real_type> des_engine_type;
typedef dcs::math::random::base_generator<uint_type> random_generator_type;
//typedef dcs::math::random::base_generator<uint32_t> random_generator_type;
typedef dcs::des::cloud::traits<
			des_engine_type,
			random_generator_type,
			dcs::des::cloud::config::configuration<real_type,uint_type>,
			real_type,
			uint_type,
			int_type
		> traits_type;
typedef dcs::des::cloud::registry<traits_type> registry_type;
typedef ::dcs::math::random::minstd_rand1 random_seeder_type;


namespace detail { namespace /*<unnamed>*/ {

typedef ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
typedef ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;


inline
void usage()
{
//	::std::cerr << "Usage: " << prog_name << " conf_file" << std::endl;
	::std::cerr << "Usage: " << prog_name << " <options>" << ::std::endl
				<< "Options:" << ::std::endl
				<< "  --partial-stats" << ::std::endl
				<< "  --conf <configuration-file>" << ::std::endl
				<< "  --out-data-file <output-data-file>" << ::std::endl;
}


inline
void info()
{
//	::std::cerr << "Usage: " << prog_name << " conf_file" << std::endl;
	::std::cerr << "Executable name: " << prog_name << ::std::endl
				<< "Features:" << ::std::endl
#ifdef DCS_DES_CLOUD_EXP_OUTPUT_VM_SHARES
				<< "  OUTPUT_VM_SHARES=on" << ::std::endl
#else // DCS_DES_CLOUD_EXP_OUTPUT_VM_SHARES
				<< "  OUTPUT_VM_SHARES=off" << ::std::endl
#endif // DCS_DES_CLOUD_EXP_OUTPUT_VM_SHARES
#ifdef DCS_DES_CLOUD_EXP_OUTPUT_VM_MEASURES
				<< "  OUTPUT_VM_MEASURES=on" << ::std::endl
#else // DCS_DES_CLOUD_EXP_OUTPUT_VM_MEASURES
				<< "  OUTPUT_VM_MEASURES=off" << ::std::endl
#endif // DCS_DES_CLOUD_EXP_OUTPUT_VM_MEASURES
#ifdef DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
				<< "  OUTPUT_RLS_DATA=on" << ::std::endl
#else // DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
				<< "  OUTPUT_RLS_DATA=off" << ::std::endl
#endif // DCS_DES_CLOUD_EXP_OUTPUT_RLS_DATA
#ifdef DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION
				<< "  LQ_APP_CONTROLLER_USE_INPUT_DEVIATION=on" << ::std::endl
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION
				<< "  LQ_APP_CONTROLLER_USE_INPUT_DEVIATION=off" << ::std::endl
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_INPUT_DEVIATION
#ifdef DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
				<< "  LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT=on" << ::std::endl
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
				<< "  LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT=off" << ::std::endl
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_INPUT
#ifdef DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION
				<< "  LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION=on" << ::std::endl
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION
				<< "  LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION=off" << ::std::endl
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_OUTPUT_DEVIATION
#ifdef DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
				<< "  LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT=on" << ::std::endl
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
				<< "  LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT=off" << ::std::endl
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_NORMALIZED_OUTPUT
#ifdef DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
				<< "  LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT=on" << ::std::endl
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
				<< "  LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT=off" << ::std::endl
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_USE_DYNAMIC_EQUILIBRIUM_POINT
#ifdef DCS_DES_CLOUD_EXP_MIGR_CONTROLLER_MONITOR_VMS
				<< "  MIGR_CONTROLLER_MONITOR_VMS=on" << ::std::endl
#else // DCS_DES_CLOUD_EXP_MIGR_CONTROLLER_MONITOR_VMS
				<< "  MIGR_CONTROLLER_MONITOR_VMS=off" << ::std::endl
#endif // DCS_DES_CLOUD_EXP_MIGR_CONTROLLER_MONITOR_VMS
#ifdef DCS_DES_CLOUD_EXP_PHYSICAL_MACHINES_AUTO_POWER_OFF
				<< "  PHYSICAL_MACHINES_AUTO_POWER_OFF=on" << ::std::endl
#else // DCS_DES_CLOUD_EXP_PHYSICAL_MACHINES_AUTO_POWER_OFF
				<< "  PHYSICAL_MACHINES_AUTO_POWER_OFF=off" << ::std::endl
#endif // DCS_DES_CLOUD_EXP_PHYSICAL_MACHINES_AUTO_POWER_OFF
#ifdef DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION
				<< "  LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION=" << static_cast<int>(DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION) << ::std::endl
#else // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION
				<< "  LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION=default" << ::std::endl
#endif // DCS_DES_CLOUD_EXP_LQ_APP_CONTROLLER_NEGATIVE_SHARE_ACTION
				;
}


inline
::std::string strtime()
{
	::std::time_t t(::std::time(0));//XXX
	::std::tm tm;
	::localtime_r(&t, &tm);

	const ::std::size_t st_max_len(30);
	char* st = new char[st_max_len];
	::std::size_t st_len;
	st_len = ::std::strftime(st, st_max_len, "%Y-%m-%d %H:%M:%S (%Z)", &tm);
	if (!st_len)
	{
		throw ::std::runtime_error("Unable to compute current time.");
	}
	::std::string sst(st, st_len);
	delete[] st;

	return sst;
}


template <typename ForwardIterT>
inline
ForwardIterT find_option(ForwardIterT begin, ForwardIterT end, std::string const& option)
{
	ForwardIterT it = ::std::find(begin, end, option);
//    if (it != end && ++it != end)
	if (it != end)
	{
		return it;
	}
	return end;
}


template <typename T, typename ForwardIterT>
T get_option(ForwardIterT begin, ForwardIterT end, std::string const& option)
{
    ForwardIterT it = find_option(begin, end, option);

    if (it == end || ++it == end)
    {
		::std::ostringstream oss;
		oss << "Unable to find option: '" << option << "'";
    	throw ::std::runtime_error(oss.str());
    }

	T value;

	::std::istringstream iss(*it);
	iss >> value;

    return value;
}


template <typename T, typename ForwardIterT>
T get_option(ForwardIterT begin, ForwardIterT end, std::string const& option, T default_value)
{
    ForwardIterT it = find_option(begin, end, option);

    if (it == end || ++it == end)
    {
		return default_value;
    }

	T value;

	::std::istringstream iss(*it);
	iss >> value;

    return value;
}


/// Get a boolean option; also tell if a given option does exist.
template <typename ForwardIterT>
bool get_option(ForwardIterT begin, ForwardIterT end, std::string const& option)
{
    ForwardIterT it = find_option(begin, end, option);

	return it != end;
}


inline
::std::string to_string(::dcs::des::cloud::performance_measure_category category)
{
	switch (category)
	{
		case ::dcs::des::cloud::busy_time_performance_measure:
			return ::std::string("Busy Time");
		case ::dcs::des::cloud::queue_length_performance_measure:
			return ::std::string("Queue Length");
		case ::dcs::des::cloud::response_time_performance_measure:
			return ::std::string("Response Time");
		case ::dcs::des::cloud::throughput_performance_measure:
			return ::std::string("Throughput");
		case ::dcs::des::cloud::utilization_performance_measure:
			return ::std::string("Utilization");
		default:
			break;
	}

	return ::std::string("Unknown Performance Measure");
}


inline
::std::string to_yaml_id(::dcs::des::cloud::performance_measure_category category)
{
	switch (category)
	{
		case ::dcs::des::cloud::busy_time_performance_measure:
			return ::std::string("busy-time");
		case ::dcs::des::cloud::queue_length_performance_measure:
			return ::std::string("queue-length");
		case ::dcs::des::cloud::response_time_performance_measure:
			return ::std::string("response-time");
		case ::dcs::des::cloud::throughput_performance_measure:
			return ::std::string("throughput");
		case ::dcs::des::cloud::utilization_performance_measure:
			return ::std::string("utilization");
		default:
			break;
	}

	return ::std::string("unknown");
}


template <typename TraitsT>
class simulated_system
{
	public: typedef TraitsT traits_type;
	public: typedef ::dcs::des::cloud::data_center<traits_type> data_center_type;
	public: typedef ::dcs::shared_ptr<data_center_type> data_center_pointer;
	public: typedef ::dcs::des::cloud::data_center_manager<traits_type> data_center_manager_type;
	public: typedef ::dcs::shared_ptr<data_center_manager_type> data_center_manager_pointer;


	public: void data_center(data_center_pointer const& ptr_dc)
	{
		ptr_dc_ = ptr_dc;
	}


	public: data_center_type const& data_center() const
	{
		// pre: data center must have been set
		DCS_ASSERT(
				ptr_dc_,
				throw ::std::runtime_error("Invalid data center pointer.")
			);

		return *ptr_dc_;
	}


	public: void data_center_manager(data_center_manager_pointer const& ptr_dc_mngr)
	{
		ptr_dc_mngr_ = ptr_dc_mngr;
	}


	public: data_center_manager_type const& data_center_manager() const
	{
		// pre: data center must have been set
		DCS_ASSERT(
				ptr_dc_mngr_,
				throw ::std::runtime_error("Invalid data center manager pointer.")
			);

		return *ptr_dc_mngr_;
	}


	private: data_center_pointer ptr_dc_;
	private: data_center_manager_pointer ptr_dc_mngr_;
}; // simulated_system


template <
	typename CharT,
	typename CharTraitsT,
	typename TraitsT
>
void report_stats(::std::basic_ostream<CharT,CharTraitsT>& os, simulated_system<TraitsT> const& sys)
{
	typedef TraitsT traits_type;
	typedef typename traits_type::uint_type uint_type;
	typedef typename traits_type::real_type real_type;
	typedef typename simulated_system<TraitsT>::data_center_type data_center_type;

	data_center_type const& dc(sys.data_center());

	::std::string indent("  ");

//	// VM Placement
//	{
//		typedef typename data_center_type::physical_machine_identifier_type pm_identifier_type;
//		typedef typename data_center_type::virtual_machine_identifier_type vm_identifier_type;
//		typedef typename data_center_type::virtual_machines_placement_type vm_placement_type;
//		typedef typename vm_placement_type::const_iterator vm_placement_iterator;
//		typedef typename vm_placement_type::share_const_iterator vm_share_iterator;
//
//		os << ::std::endl << "-- Virtual Machine Placement --" << ::std::endl;
//
//		vm_placement_type const& vm_placement = dc.current_virtual_machines_placement();
//		vm_placement_iterator place_end_it = vm_placement.end();
//		for (vm_placement_iterator place_it = vm_placement.begin(); place_it != place_end_it; ++place_it)
//		{
//			vm_identifier_type vm_id(vm_placement.vm_id(place_it));
//			vm_identifier_type pm_id(vm_placement.pm_id(place_it));
//
//			os << indent
//			   << "VM: " << vm_id << " ('" << dc.virtual_machine_ptr(vm_id)->name() << "')"
//			   << " --> "
//			   << "PM: " << pm_id << " ('" << dc.physical_machine_ptr(pm_id)->name() << "')"
//			   << ::std::endl;
//
//			vm_share_iterator share_end_it = vm_placement.shares_end(place_it);
//			for (vm_share_iterator share_it = vm_placement.shares_begin(place_it); share_it != share_end_it; ++share_it)
//			{
//				::dcs::des::cloud::physical_resource_category category(vm_placement.resource_category(share_it));
//				real_type share(vm_placement.resource_share(share_it));
//
//				os << indent << indent
//				   << "Resource: " << category << ", Share: " << share << ::std::endl;
//			}
//		}
//	}

	// Virtual Machines
	{
		typedef typename data_center_type::virtual_machine_type virtual_machine_type;
		typedef typename data_center_type::virtual_machine_pointer virtual_machine_pointer;
		typedef ::std::vector<virtual_machine_pointer> virtual_machine_container;
		typedef typename virtual_machine_container::const_iterator virtual_machine_iterator;
		typedef typename virtual_machine_type::statistic_pointer statistic_pointer;
		typedef ::std::vector<statistic_pointer> vm_statistic_container;
		typedef typename vm_statistic_container::const_iterator vm_statistic_iterator;
		typedef ::std::map< ::dcs::des::cloud::physical_resource_category, vm_statistic_container> vm_resource_statistic_map;
		typedef typename vm_resource_statistic_map::const_iterator vm_resource_statistic_iterator;

		os << ::std::endl << "-- Virtual Machines --" << ::std::endl;

		virtual_machine_container vms = dc.virtual_machines();
		virtual_machine_iterator vm_end_it = vms.end();
		for (virtual_machine_iterator vm_it = vms.begin(); vm_it != vm_end_it; ++vm_it)
		{
			virtual_machine_pointer ptr_vm = *vm_it;
			vm_resource_statistic_map vm_res_stats;
			vm_resource_statistic_iterator res_stat_end_it;

			os << indent
			   << "Virtual Machine: '" << ptr_vm->name() << "' (ID: " << ptr_vm->id() << ")" << ::std::endl;

			os << indent << indent
			   << "Wanted Resource Shares: " << ::std::endl;

			vm_res_stats = ptr_vm->wanted_resource_share_statistics();
			res_stat_end_it = vm_res_stats.end();
			for (vm_resource_statistic_iterator res_stat_it = vm_res_stats.begin(); res_stat_it != res_stat_end_it; ++res_stat_it)
			{
				os << indent << indent << indent
				   << "Resource: " << res_stat_it->first << ::std::endl;

				vm_statistic_container vm_stats(res_stat_it->second);
				vm_statistic_iterator stat_end_it(vm_stats.end());
				for (vm_statistic_iterator stat_it = vm_stats.begin(); stat_it != stat_end_it; ++stat_it)
				{
					statistic_pointer ptr_stat(*stat_it);

					os << indent << indent << indent << indent
					   << ptr_stat->name() << ": " << *ptr_stat << ::std::endl;
				}
			}

			os << indent << indent
			   << "Assigned Resource Shares: " << ::std::endl;

			vm_res_stats = ptr_vm->resource_share_statistics();
			res_stat_end_it = vm_res_stats.end();
			for (vm_resource_statistic_iterator res_stat_it = vm_res_stats.begin(); res_stat_it != res_stat_end_it; ++res_stat_it)
			{
				os << indent << indent << indent
				   << "Resource: " << res_stat_it->first << ::std::endl;

				vm_statistic_container vm_stats(res_stat_it->second);
				vm_statistic_iterator stat_end_it(vm_stats.end());
				for (vm_statistic_iterator stat_it = vm_stats.begin(); stat_it != stat_end_it; ++stat_it)
				{
					statistic_pointer ptr_stat(*stat_it);

					os << indent << indent << indent << indent
					   << ptr_stat->name() << ": " << *ptr_stat << ::std::endl;
				}
			}
		}
	}

	// Application statistics
	{
		typedef typename data_center_type::application_type application_type;
		typedef typename data_center_type::application_pointer application_pointer;
		typedef ::std::vector<application_pointer> application_container;
		typedef typename application_container::const_iterator application_iterator;
		typedef typename application_type::simulation_model_type application_simulation_model_type;
		typedef typename application_simulation_model_type::output_statistic_pointer output_statistic_pointer;
		typedef ::std::vector<output_statistic_pointer> statistic_container;
		typedef typename statistic_container::const_iterator statistic_iterator;
		typedef typename application_type::application_tier_pointer application_tier_pointer;
		typedef ::std::vector<application_tier_pointer> tier_container;
		typedef typename tier_container::const_iterator tier_iterator;
		typedef ::dcs::des::cloud::performance_measure_category statistic_category_type;
		typedef ::std::vector<statistic_category_type> statistic_category_container;
		typedef typename statistic_category_container::const_iterator statistic_category_iterator;

		os << ::std::endl << "-- Applications --" << ::std::endl;

		application_container apps = dc.applications();
		application_iterator app_end_it = apps.end();
		for (application_iterator app_it = apps.begin(); app_it != app_end_it; ++app_it)
		{
			application_pointer ptr_app = *app_it;

			os << indent
			   << "Application: '" << ptr_app->name() << "' (ID: " << ptr_app->id() << ")" << ::std::endl;
			os << indent << indent
			   << "Overall: " << ::std::endl;

			os << indent << indent << indent
			   << "# Arrivals: " << ptr_app->simulation_model().num_arrivals() << ::std::endl;
			os << indent << indent << indent
			   << "# Departures: " << ptr_app->simulation_model().num_departures() << ::std::endl;
			os << indent << indent << indent
			   << "# SLA violations: " << ptr_app->simulation_model().num_sla_violations() << ::std::endl;

			statistic_category_container stat_categories;

			//[FIXME] statistic categories are hard-coded
			//stat_categories.push_back(::dcs::des::cloud::response_time_performance_measure);
			//[/FIXME]

			stat_categories = ::dcs::des::cloud::performance_measure_categories();
			statistic_category_iterator stat_cat_end_it = stat_categories.end();
			for (statistic_category_iterator stat_cat_it = stat_categories.begin(); stat_cat_it != stat_cat_end_it; ++stat_cat_it)
			{
				statistic_category_type stat_category(*stat_cat_it);

				if (::dcs::des::cloud::for_application(stat_category))
				{
					os << indent << indent << indent
					   << to_string(stat_category) << ": " << ::std::endl;

					statistic_container ptr_stats;
					ptr_stats = ptr_app->simulation_model().statistic(stat_category);
					statistic_iterator stat_end_it = ptr_stats.end();
					for (statistic_iterator stat_it = ptr_stats.begin(); stat_it != stat_end_it; ++stat_it)
					{
						output_statistic_pointer ptr_stat(*stat_it);

						os << indent << indent << indent << indent
						   << ptr_stat->name() << ": " << *ptr_stat << ::std::endl;//FIXME: statistic type is hard-coded
					}
				}
			}

			// Tier statistics
			tier_container tiers;
			tiers = ptr_app->tiers();
			tier_iterator tier_end_it = tiers.end();
			for (tier_iterator tier_it = tiers.begin(); tier_it != tier_end_it; ++tier_it)
			{
				application_tier_pointer ptr_tier(*tier_it);
				uint_type tier_id(ptr_tier->id());

				os << indent << indent
				   << "Tier '" << ptr_tier->name() << "': " << ::std::endl;

				os << indent << indent << indent
				   << "# Arrivals: " << ptr_app->simulation_model().tier_num_arrivals(tier_id) << ::std::endl;
				os << indent << indent << indent
				   << "# Departures: " << ptr_app->simulation_model().tier_num_departures(tier_id) << ::std::endl;

				statistic_category_iterator stat_cat_end_it = stat_categories.end();
				for (statistic_category_iterator stat_cat_it = stat_categories.begin(); stat_cat_it != stat_cat_end_it; ++stat_cat_it)
				{
					statistic_category_type stat_category(*stat_cat_it);

					if (::dcs::des::cloud::for_application_tier(stat_category))
					{
						os << indent << indent << indent
						   << to_string(stat_category) << ": " << ::std::endl;

						statistic_container ptr_tier_stats;
						ptr_tier_stats = ptr_app->simulation_model().tier_statistic(tier_id, stat_category);
						statistic_iterator tier_stat_end_it = ptr_tier_stats.end();
						for (statistic_iterator tier_stat_it = ptr_tier_stats.begin(); tier_stat_it != tier_stat_end_it; ++tier_stat_it)
						{
							output_statistic_pointer ptr_tier_stat(*tier_stat_it);

							os << indent << indent << indent << indent
							   << ptr_tier_stat->name() << ": " << *ptr_tier_stat << ::std::endl;//FIXME: statistic type is hard-coded
						}
					}
				}
			}
		}
	}

	real_type tot_energy(0);

	// Machine statistics
	{
		typedef typename data_center_type::physical_machine_type physical_machine_type;
		typedef typename data_center_type::physical_machine_pointer physical_machine_pointer;
		typedef ::std::vector<physical_machine_pointer> physical_machine_container;
		typedef typename physical_machine_container::const_iterator physical_machine_iterator;

		os << ::std::endl << "-- Physical Machines --" << ::std::endl;

		physical_machine_container machs = dc.physical_machines();
		physical_machine_iterator mach_end_it = machs.end();
		for (physical_machine_iterator mach_it = machs.begin(); mach_it != mach_end_it; ++mach_it)
		{
			physical_machine_pointer ptr_mach = *mach_it;

			os << indent
			   << "Physical Machine: '" << ptr_mach->name() << "' (ID: " << ptr_mach->id() << ")" << ::std::endl;
			os << indent << indent
			   << "Uptime: " << ptr_mach->simulation_model().uptime() << ::std::endl;
			os << indent << indent
			   << "Consumed Energy: " << ptr_mach->simulation_model().consumed_energy() << ::std::endl;
			os << indent << indent
			   << "Utilization: " << ptr_mach->simulation_model().utilization() << ::std::endl;
			os << indent << indent
			   << "Share: " << ptr_mach->simulation_model().share() << ::std::endl;

			tot_energy += ptr_mach->simulation_model().consumed_energy().estimate();
		}
	}

	// Data Center statistics
	{
		os << ::std::endl << "-- Data Center --" << ::std::endl;
		os << indent
		   << "Consumed Energy: " << tot_energy << ::std::endl;
		os << indent
		   << "# VM Migrations: " << sys.data_center_manager().migration_controller().num_migrations() << ::std::endl;
		os << indent
		   << "VM Migration Ratio: " << sys.data_center_manager().migration_controller().migration_rate() << ::std::endl;
	}
}


template <
	typename CharT,
	typename CharTraitsT,
	typename TraitsT
>
void yaml_report_stats(::std::basic_ostream<CharT,CharTraitsT>& os, simulated_system<TraitsT> const& sys)
{
	typedef TraitsT traits_type;
	typedef typename traits_type::uint_type uint_type;
	typedef typename traits_type::real_type real_type;
	typedef typename simulated_system<TraitsT>::data_center_type data_center_type;

	data_center_type const& dc(sys.data_center());

	::YAML::Emitter yaml;

	yaml << ::YAML::BeginMap;

	// Virtual Machines
	{
		typedef typename data_center_type::virtual_machine_type virtual_machine_type;
		typedef typename data_center_type::virtual_machine_pointer virtual_machine_pointer;
		typedef ::std::vector<virtual_machine_pointer> virtual_machine_container;
		typedef typename virtual_machine_container::const_iterator virtual_machine_iterator;
		typedef typename virtual_machine_type::statistic_pointer statistic_pointer;
		typedef ::std::vector<statistic_pointer> vm_statistic_container;
		typedef typename vm_statistic_container::const_iterator vm_statistic_iterator;
		typedef ::std::map< ::dcs::des::cloud::physical_resource_category, vm_statistic_container> vm_resource_statistic_map;
		typedef typename vm_resource_statistic_map::const_iterator vm_resource_statistic_iterator;

		yaml << ::YAML::Key << "virtual-machines" << ::YAML::Value;
		yaml << ::YAML::BeginSeq;

		virtual_machine_container vms = dc.virtual_machines();
		virtual_machine_iterator vm_end_it = vms.end();
		for (virtual_machine_iterator vm_it = vms.begin(); vm_it != vm_end_it; ++vm_it)
		{
			virtual_machine_pointer ptr_vm = *vm_it;
			vm_resource_statistic_map vm_res_stats;
			vm_resource_statistic_iterator res_stat_end_it;

			yaml << ::YAML::BeginMap;

			yaml << ::YAML::Key << "id" << ::YAML::Value << ptr_vm->id();

			yaml << ::YAML::Key << "name" << ::YAML::Value << ptr_vm->name();

			yaml << ::YAML::Key << "wanted-shares" << ::YAML::Value;
			yaml << ::YAML::BeginSeq;
			vm_res_stats = ptr_vm->wanted_resource_share_statistics();
			res_stat_end_it = vm_res_stats.end();
			for (vm_resource_statistic_iterator res_stat_it = vm_res_stats.begin(); res_stat_it != res_stat_end_it; ++res_stat_it)
			{
				yaml << ::YAML::BeginMap;
				yaml << ::YAML::Key << "resource-category" << ::YAML::Value << res_stat_it->first;
				yaml << ::YAML::Key << "stats" << ::YAML::Value;
				yaml << ::YAML::BeginSeq;
				vm_statistic_container vm_stats(res_stat_it->second);
				vm_statistic_iterator stat_end_it(vm_stats.end());
				for (vm_statistic_iterator stat_it = vm_stats.begin(); stat_it != stat_end_it; ++stat_it)
				{
					statistic_pointer ptr_stat(*stat_it);

					yaml << ::YAML::BeginMap;
					yaml << ::YAML::Key << "type" << ::YAML::Value << ptr_stat->name();
					yaml << ::YAML::Key << "estimate" << ::YAML::Value << ptr_stat->estimate();
					yaml << ::YAML::Key << "stddev" << ::YAML::Value << ptr_stat->standard_deviation();
					yaml << ::YAML::EndMap;
				}
				yaml << ::YAML::EndSeq; // statistic
				yaml << ::YAML::EndMap; // wanted-share
			}
			yaml << ::YAML::EndSeq; // wanted-shares

			yaml << ::YAML::Key << "assigned-shares" << ::YAML::Value;
			yaml << ::YAML::BeginSeq;
			vm_res_stats = ptr_vm->resource_share_statistics();
			res_stat_end_it = vm_res_stats.end();
			for (vm_resource_statistic_iterator res_stat_it = vm_res_stats.begin(); res_stat_it != res_stat_end_it; ++res_stat_it)
			{
				yaml << ::YAML::BeginMap;
				yaml << ::YAML::Key << "resource-category" << ::YAML::Value << res_stat_it->first;
				yaml << ::YAML::Key << "stats" << ::YAML::Value;
				yaml << ::YAML::BeginSeq;
				vm_statistic_container vm_stats(res_stat_it->second);
				vm_statistic_iterator stat_end_it(vm_stats.end());
				for (vm_statistic_iterator stat_it = vm_stats.begin(); stat_it != stat_end_it; ++stat_it)
				{
					statistic_pointer ptr_stat(*stat_it);

					yaml << ::YAML::BeginMap;
					yaml << ::YAML::Key << "type" << ::YAML::Value << ptr_stat->name();
					yaml << ::YAML::Key << "estimate" << ::YAML::Value << ptr_stat->estimate();
					yaml << ::YAML::Key << "stddev" << ::YAML::Value << ptr_stat->standard_deviation();
					yaml << ::YAML::EndMap;
				}
				yaml << ::YAML::EndSeq; // statistics
				yaml << ::YAML::EndMap; // assigned-share
			}
			yaml << ::YAML::EndSeq; // assigned-shares

			yaml << ::YAML::EndMap; // virtual-machine
		}

		yaml << ::YAML::EndSeq; // virtual-machines
	}

	// Application statistics
	{
		typedef typename data_center_type::application_type application_type;
		typedef typename data_center_type::application_pointer application_pointer;
		typedef ::std::vector<application_pointer> application_container;
		typedef typename application_container::const_iterator application_iterator;
		typedef typename application_type::simulation_model_type application_simulation_model_type;
		typedef typename application_simulation_model_type::output_statistic_pointer output_statistic_pointer;
		typedef ::std::vector<output_statistic_pointer> statistic_container;
		typedef typename statistic_container::const_iterator statistic_iterator;
		typedef typename application_type::application_tier_pointer application_tier_pointer;
		typedef ::std::vector<application_tier_pointer> tier_container;
		typedef typename tier_container::const_iterator tier_iterator;
		typedef ::dcs::des::cloud::performance_measure_category statistic_category_type;
		typedef ::std::vector<statistic_category_type> statistic_category_container;
		typedef typename statistic_category_container::const_iterator statistic_category_iterator;

		yaml << ::YAML::Key << "applications" << ::YAML::Value;

		yaml << ::YAML::BeginSeq;

		application_container apps = dc.applications();
		application_iterator app_end_it = apps.end();
		for (application_iterator app_it = apps.begin(); app_it != app_end_it; ++app_it)
		{
			application_pointer ptr_app = *app_it;

			yaml << ::YAML::BeginMap;

			yaml << ::YAML::Key << "id" << ::YAML::Value << ptr_app->id();

			yaml << ::YAML::Key << "name" << ::YAML::Value << ptr_app->name();

			yaml << ::YAML::Key << "overall" << ::YAML::Value;
			yaml << ::YAML::BeginMap;
			yaml << ::YAML::Key << "num-arrivals" << ::YAML::Value;
			yaml << ::YAML::BeginMap;
			yaml << ::YAML::Key << "type" << ::YAML::Value << ptr_app->simulation_model().num_arrivals().name();
			yaml << ::YAML::Key << "estimate" << ::YAML::Value << ptr_app->simulation_model().num_arrivals().estimate();
			yaml << ::YAML::Key << "stddev" << ::YAML::Value << ptr_app->simulation_model().num_arrivals().standard_deviation();
			yaml << ::YAML::EndMap;
			yaml << ::YAML::Key << "num-departures" << ::YAML::Value;
			yaml << ::YAML::BeginMap;
			yaml << ::YAML::Key << "type" << ::YAML::Value << ptr_app->simulation_model().num_departures().name();
			yaml << ::YAML::Key << "estimate" << ::YAML::Value << ptr_app->simulation_model().num_departures().estimate();
			yaml << ::YAML::Key << "stddev" << ::YAML::Value << ptr_app->simulation_model().num_departures().standard_deviation();
			yaml << ::YAML::EndMap;
			yaml << ::YAML::Key << "num-sla-violations" << ::YAML::Value;
			yaml << ::YAML::BeginMap;
			yaml << ::YAML::Key << "type" << ::YAML::Value << ptr_app->simulation_model().num_sla_violations().name();
			yaml << ::YAML::Key << "estimate" << ::YAML::Value << ptr_app->simulation_model().num_sla_violations().estimate();
			yaml << ::YAML::Key << "stddev" << ::YAML::Value << ptr_app->simulation_model().num_sla_violations().standard_deviation();
			yaml << ::YAML::EndMap;

			statistic_category_container stat_categories(::dcs::des::cloud::performance_measure_categories());
			statistic_category_iterator stat_cat_end_it = stat_categories.end();
			for (statistic_category_iterator stat_cat_it = stat_categories.begin(); stat_cat_it != stat_cat_end_it; ++stat_cat_it)
			{
				statistic_category_type stat_category(*stat_cat_it);

				if (::dcs::des::cloud::for_application(stat_category))
				{
					yaml << ::YAML::Key << to_yaml_id(stat_category) << ::YAML::Value;

					yaml << ::YAML::BeginSeq;

					statistic_container ptr_stats;
					ptr_stats = ptr_app->simulation_model().statistic(stat_category);
					statistic_iterator stat_end_it = ptr_stats.end();
					for (statistic_iterator stat_it = ptr_stats.begin(); stat_it != stat_end_it; ++stat_it)
					{
						output_statistic_pointer ptr_stat(*stat_it);

						yaml << ::YAML::BeginMap;
						yaml << ::YAML::Key << "type" << ::YAML::Value << ptr_stat->name();
						yaml << ::YAML::Key << "estimate" << ::YAML::Value << ptr_stat->estimate();
						yaml << ::YAML::Key << "stddev" << ::YAML::Value << ptr_stat->standard_deviation();
						yaml << ::YAML::EndMap;
					}

					yaml << ::YAML::EndSeq;
				}
			}

			yaml << ::YAML::EndMap;

			yaml << ::YAML::Key << "tiers" << ::YAML::Value;

			yaml << ::YAML::BeginSeq;

			// Tier statistics
			tier_container tiers;
			tiers = ptr_app->tiers();
			tier_iterator tier_end_it = tiers.end();
			for (tier_iterator tier_it = tiers.begin(); tier_it != tier_end_it; ++tier_it)
			{
				application_tier_pointer ptr_tier(*tier_it);
				uint_type tier_id(ptr_tier->id());

				yaml << ::YAML::BeginMap;

				yaml << ::YAML::Key << "id" << ::YAML::Value << tier_id;
				yaml << ::YAML::Key << "name" << ::YAML::Value << ptr_tier->name();
				yaml << ::YAML::Key << "num-arrivals" << ::YAML::Value;
				yaml << ::YAML::BeginMap;
				yaml << ::YAML::Key << "type" << ::YAML::Value << ptr_app->simulation_model().tier_num_arrivals(tier_id).name();
				yaml << ::YAML::Key << "estimate" << ::YAML::Value << ptr_app->simulation_model().tier_num_arrivals(tier_id).estimate();
				yaml << ::YAML::Key << "stddev" << ::YAML::Value << ptr_app->simulation_model().tier_num_arrivals(tier_id).standard_deviation();
				yaml << ::YAML::EndMap;
				yaml << ::YAML::Key << "num-departures" << ::YAML::Value;
				yaml << ::YAML::BeginMap;
				yaml << ::YAML::Key << "type" << ::YAML::Value << ptr_app->simulation_model().tier_num_departures(tier_id).name();
				yaml << ::YAML::Key << "estimate" << ::YAML::Value << ptr_app->simulation_model().tier_num_departures(tier_id).estimate();
				yaml << ::YAML::Key << "stddev" << ::YAML::Value << ptr_app->simulation_model().tier_num_departures(tier_id).standard_deviation();
				yaml << ::YAML::EndMap;

				statistic_category_iterator stat_cat_end_it = stat_categories.end();
				for (statistic_category_iterator stat_cat_it = stat_categories.begin(); stat_cat_it != stat_cat_end_it; ++stat_cat_it)
				{
					statistic_category_type stat_category(*stat_cat_it);

					if (::dcs::des::cloud::for_application_tier(stat_category))
					{
						yaml << ::YAML::Key << to_yaml_id(stat_category) << ::YAML::Value;

						yaml << ::YAML::BeginSeq;

						statistic_container ptr_tier_stats;
						ptr_tier_stats = ptr_app->simulation_model().tier_statistic(tier_id, stat_category);
						statistic_iterator tier_stat_end_it = ptr_tier_stats.end();
						for (statistic_iterator tier_stat_it = ptr_tier_stats.begin(); tier_stat_it != tier_stat_end_it; ++tier_stat_it)
						{
							output_statistic_pointer ptr_tier_stat(*tier_stat_it);

							yaml << ::YAML::BeginMap;
							yaml << ::YAML::Key << "type" << ::YAML::Value << ptr_tier_stat->name();
							yaml << ::YAML::Key << "estimate" << ::YAML::Value << ptr_tier_stat->estimate();
							yaml << ::YAML::Key << "stddev" << ::YAML::Value << ptr_tier_stat->standard_deviation();
							yaml << ::YAML::EndMap;
						}

						yaml << ::YAML::EndSeq; // statistics
					}
				}

				yaml << ::YAML::EndMap; // tier
			}

			yaml << ::YAML::EndSeq; // tiers

			yaml << ::YAML::EndMap; // application
		}

		yaml << ::YAML::EndSeq; // applications
	}

	real_type tot_energy(0);

	// Machine statistics
	{
		typedef typename data_center_type::physical_machine_type physical_machine_type;
		typedef typename data_center_type::physical_machine_pointer physical_machine_pointer;
		typedef ::std::vector<physical_machine_pointer> physical_machine_container;
		typedef typename physical_machine_container::const_iterator physical_machine_iterator;

		yaml << ::YAML::Key << "physical-machines" << ::YAML::Value;
		yaml << ::YAML::BeginSeq;

		physical_machine_container machs = dc.physical_machines();
		physical_machine_iterator mach_end_it = machs.end();
		for (physical_machine_iterator mach_it = machs.begin(); mach_it != mach_end_it; ++mach_it)
		{
			physical_machine_pointer ptr_mach = *mach_it;

			yaml << ::YAML::BeginMap;

			yaml << ::YAML::Key << "id" << ::YAML::Value << ptr_mach->id();

			yaml << ::YAML::Key << "name" << ::YAML::Value << ptr_mach->name();

			yaml << ::YAML::Key << "uptime" << ::YAML::Value;
			yaml << ::YAML::BeginMap;
			yaml << ::YAML::Key << "type" << ::YAML::Value << ptr_mach->simulation_model().uptime().name();
			yaml << ::YAML::Key << "estimate" << ::YAML::Value << ptr_mach->simulation_model().uptime().estimate();
			yaml << ::YAML::Key << "stddev" << ::YAML::Value << ptr_mach->simulation_model().uptime().standard_deviation();
			yaml << ::YAML::EndMap;
			yaml << ::YAML::Key << "consumed-energy" << ::YAML::Value;
			yaml << ::YAML::BeginMap;
			yaml << ::YAML::Key << "type" << ::YAML::Value << ptr_mach->simulation_model().consumed_energy().name();
			yaml << ::YAML::Key << "estimate" << ::YAML::Value << ptr_mach->simulation_model().consumed_energy().estimate();
			yaml << ::YAML::Key << "stddev" << ::YAML::Value << ptr_mach->simulation_model().consumed_energy().standard_deviation();
			yaml << ::YAML::EndMap;
			yaml << ::YAML::Key << "utilization" << ::YAML::Value;
			yaml << ::YAML::BeginMap;
			yaml << ::YAML::Key << "type" << ::YAML::Value << ptr_mach->simulation_model().utilization().name();
			yaml << ::YAML::Key << "estimate" << ::YAML::Value << ptr_mach->simulation_model().utilization().estimate();
			yaml << ::YAML::Key << "stddev" << ::YAML::Value << ptr_mach->simulation_model().utilization().standard_deviation();
			yaml << ::YAML::EndMap;
			yaml << ::YAML::Key << "share" << ::YAML::Value;
			yaml << ::YAML::BeginMap;
			yaml << ::YAML::Key << "type" << ::YAML::Value << ptr_mach->simulation_model().share().name();
			yaml << ::YAML::Key << "estimate" << ::YAML::Value << ptr_mach->simulation_model().share().estimate();
			yaml << ::YAML::Key << "stddev" << ::YAML::Value << ptr_mach->simulation_model().share().standard_deviation();
			yaml << ::YAML::EndMap;

			yaml << ::YAML::EndMap; // physical-machine

			tot_energy += ptr_mach->simulation_model().consumed_energy().estimate();
		}

		yaml << ::YAML::EndSeq; // physical-machines
	}

	// Data Center statistics
	{
		yaml << ::YAML::Key << "data-center" << ::YAML::Value;
		yaml << ::YAML::BeginMap;

		yaml << ::YAML::Key << "consumed-energy" << ::YAML::Value;
		yaml << ::YAML::BeginMap;
		yaml << ::YAML::Key << "type" << ::YAML::Value << "mean";
		yaml << ::YAML::Key << "estimate" << ::YAML::Value << tot_energy;
		yaml << ::YAML::Key << "stddev" << ::YAML::Value << 0;
		yaml << ::YAML::EndMap;
		yaml << ::YAML::Key << "num-vm-migrations" << ::YAML::Value;
		yaml << ::YAML::BeginMap;
		yaml << ::YAML::Key << "type" << ::YAML::Value << sys.data_center_manager().migration_controller().num_migrations().name();
		yaml << ::YAML::Key << "estimate" << ::YAML::Value << sys.data_center_manager().migration_controller().num_migrations().estimate();
		yaml << ::YAML::Key << "stddev" << ::YAML::Value << sys.data_center_manager().migration_controller().num_migrations().standard_deviation();
		yaml << ::YAML::EndMap;
		yaml << ::YAML::Key << "vm-migration-rate" << ::YAML::Value;
		yaml << ::YAML::BeginMap;
		yaml << ::YAML::Key << "type" << ::YAML::Value << sys.data_center_manager().migration_controller().migration_rate().name();
		yaml << ::YAML::Key << "estimate" << ::YAML::Value << sys.data_center_manager().migration_controller().migration_rate().estimate();
		yaml << ::YAML::Key << "stddev" << ::YAML::Value << sys.data_center_manager().migration_controller().migration_rate().standard_deviation();
		yaml << ::YAML::EndMap;

		yaml << ::YAML::EndMap;
	}

	yaml << ::YAML::EndMap;

	os << yaml.c_str() << ::std::endl;
}


void process_sys_init_sim_event(des_event_type const& evt, des_engine_context_type& ctx, random_seeder_type& seeder)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

	typedef random_seeder_type::result_type seed_type;

	DCS_DEBUG_TRACE("BEGIN Process System Initialization at Clock: " << ctx.simulated_time());

	seed_type seed = seeder();

	DCS_DEBUG_TRACE("Generated new seed: " << seed);//XXX

	registry_type& ref_reg = registry_type::instance();
	ref_reg.uniform_random_generator_ptr()->seed(seed);

	DCS_DEBUG_TRACE("END Process System Initialization at Clock: " << ctx.simulated_time());
}


//FIXME: Add a fourth parameter for the output stream
//template <
//	typename CharT,
//	typename CharTraitsT,
//	typename TraitsT
//>
//void process_sys_finit_sim_event(des_event_type const& evt, des_engine_context_type& ctx, ::std::basic_ostream<CharT,CharTraitsT>& os, ::dcs::shared_ptr< ::dcs::des::cloud::data_center<TraitsT> > const& ptr_dc)
template <typename TraitsT>
void process_sys_finit_sim_event(des_event_type const& evt, des_engine_context_type& ctx, simulated_system<TraitsT> const* sys)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

	DCS_DEBUG_TRACE("BEGIN Process System Finalization at Clock: " << ctx.simulated_time());

	//FIXME: std::cout hard-coded... Add an additional parameter to this function
	::std::cout << "PARTIAL STATISTICS:" << ::std::endl;
	::std::cout << "Simulator state: " << ::std::endl
				<< ctx << ::std::endl;
	::std::cout << "Statistics: " << ::std::endl;
	detail::report_stats(::std::cout, *sys);
	::std::cout << "--------------------------------------------------------------------------------" << ::std::endl;

	DCS_DEBUG_TRACE("END Process System Finalization at Clock: " << ctx.simulated_time());
}


#ifdef DCS_DEBUG
void stack_tracer()
{
#if __GNUC__
	void *trace_elems[20];
	int trace_elem_count(backtrace(trace_elems, 20));
	char **stack_syms(backtrace_symbols(trace_elems, trace_elem_count));
	for (int i = 0; i < trace_elem_count; ++i)
	{
		::std::cerr << stack_syms[i] << ::std::endl;
	}
	::free(stack_syms);

	::std::exit(1);
#endif // __GNUC__
}
#endif // DCS_DEBUG

}} // Namespace detail::<unnamed>


int main(int argc, char* argv[])
{
//	typedef double real_type;
//	typedef unsigned long uint_type;
//	typedef long int_type;
//	typedef dcs::des::engine<real_type> des_engine_type;
////	typedef dcs::math::random::base_generator<real_type> random_generator_type;
//	typedef dcs::math::random::base_generator<uint_type> random_generator_type;
//	typedef dcs::des::cloud::traits<
//				des_engine_type,
//				random_generator_type,
//				real_type,
//				uint_type,
//				int_type
//			> traits_type;
//	typedef dcs::des::cloud::registry<traits_type> registry_type;
	typedef dcs::des::cloud::config::configuration<real_type,uint_type> configuration_type;
	typedef dcs::shared_ptr<configuration_type> configuration_pointer;
	typedef dcs::shared_ptr<des_engine_type> des_engine_pointer;
	typedef dcs::shared_ptr<random_generator_type> random_generator_pointer;
	//typedef dcs::des::cloud::data_center<traits_type> data_center_type;
	typedef dcs::shared_ptr< dcs::des::cloud::data_center<traits_type> > data_center_pointer;
	typedef dcs::shared_ptr< dcs::des::cloud::data_center_manager<traits_type> > data_center_manager_pointer;


#ifdef DCS_DEBUG
	::std::set_terminate(detail::stack_tracer);
#endif // DCS_DEBUG

	prog_name = argv[0];

	if (argc < 2)
	{
		detail::usage();
		return -1;
	}


	std::cout << "--- DCS DES Cloud start at " << detail::strtime() << "." << std::endl;
	std::cout << "--------------------------------------------------------------------------------" << std::endl;


	// Parse command line arguments

	std::string conf_fname; // (argv[1]);
	bool partial_stats(false);
	std::string outdata_fname;
	bool output_info(false);
	bool output_help(false);

	output_help = detail::get_option(argv, argv+argc, "--help");
	output_info = detail::get_option(argv, argv+argc, "--info");

	if (output_help)
	{
		detail::usage();
		return 0;
	}
	if (output_info)
	{
		detail::info();
		return 0;
	}

	try
	{
		partial_stats = detail::get_option(argv, argv+argc, "--partial-stats");
		conf_fname = detail::get_option<std::string>(argv, argv+argc, "--conf");
		outdata_fname = detail::get_option<std::string>(argv, argv+argc, "--out-data-file", "");
	}
	catch (std::exception const& e)
	{
		std::cerr << "[Error] Error while parsing command-line options: " << e.what() << std::endl;
		detail::usage();
		std::abort();
	}

	std::cout << "CLI OPTIONS:" << std::endl;
	std::cout << " - Partial Statistics: " << std::boolalpha << partial_stats << std::endl;
	std::cout << " - Configuration File: " << conf_fname << std::endl;
	std::cout << " - Output Data File: " << outdata_fname << std::endl;
	std::cout << "--------------------------------------------------------------------------------" << std::endl;

	// Read configuration

	configuration_category conf_cat = yaml_configuration;

	configuration_pointer ptr_conf;

	try
	{
		switch (conf_cat)
		{
			case yaml_configuration:

				ptr_conf = dcs::make_shared<configuration_type>(
							dcs::des::cloud::config::read_file(
								conf_fname,
								::dcs::des::cloud::config::yaml_reader<real_type,uint_type>()
							)
						);
				break;
			default:
				throw ::std::runtime_error("Unknown configuration category.");
		}
	}
	catch (::std::exception const& e)
	{
		::std::clog << "[Error] Unable to read configuration: " << e.what() << ::std::endl;
		return -2;
	}

	DCS_DEBUG_TRACE("Configuration: " << *ptr_conf); //XXX

	// Print configuration (for ease later info retrieval)
	::std::cout << "CONFIGURATION:" << ::std::endl
				<< *ptr_conf << ::std::endl
				<< "--------------------------------------------------------------------------------" << ::std::endl
				<< ::std::endl;

	// Build the registry

	registry_type& reg(registry_type::instance());
	reg.configuration(ptr_conf);
	des_engine_pointer ptr_des_eng;
	ptr_des_eng = dcs::des::cloud::config::make_des_engine(*ptr_conf);
	reg.des_engine(ptr_des_eng);
	random_generator_pointer ptr_rng;
	ptr_rng = dcs::des::cloud::config::make_random_number_generator(*ptr_conf);//OK
//	ptr_rng = dcs::make_shared<dcs::math::random::mt19937>(5489UL);//XXX
	reg.uniform_random_generator(ptr_rng);

//	detail::test_rng(ptr_rng);//XXX

	random_seeder_type seeder(ptr_conf->rng().seed);

	detail::simulated_system<traits_type> sys;

	// Register some DES event hooks
	ptr_des_eng->system_initialization_event_source().connect(
			::dcs::functional::bind(
				&detail::process_sys_init_sim_event,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2,
				seeder
			)
		);
	if (partial_stats)
	{
		ptr_des_eng->system_finalization_event_source().connect(
				::dcs::functional::bind(
					&detail::process_sys_finit_sim_event<traits_type>,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2,
					&sys
				)
			);
	}

	// Attach a simulation observer
	dcs::shared_ptr< dcs::des::cloud::logging::base_logger<traits_type> > ptr_sim_log;
	ptr_sim_log = dcs::des::cloud::config::make_logger<traits_type>(*ptr_conf);
	//FIXME: makes it user configurable
	//FIXME: makes sinks more flexible like:
	//       ...->sink(text_file_sink("sim-obs.log"))
	//       ...->sink(console_sink(::std::cout))
	//       ...->sink(ostream_sink())
	//ptr_sim_log->sink("sim-obs.log");
	ptr_sim_log->attach(*ptr_des_eng);

	// Build the Data Center
	data_center_pointer ptr_dc;
	data_center_manager_pointer ptr_dc_mngr;
	ptr_dc = dcs::des::cloud::config::make_data_center<traits_type>(*ptr_conf, ptr_rng, ptr_des_eng);
	ptr_dc_mngr = dcs::des::cloud::config::make_data_center_manager<traits_type>(*ptr_conf, ptr_dc);

	sys.data_center(ptr_dc);
	sys.data_center_manager(ptr_dc_mngr);

	std::cerr.precision(16);
	std::cout.precision(16);

	// Run the simulation
	ptr_des_eng->run();

	// Detach the simulation observer
	ptr_sim_log->detach(*ptr_des_eng);

	// Report statistics
	std::cout << "STATISTICS:" << std::endl;
	detail::report_stats(std::cout, sys);
	std::cout << "--------------------------------------------------------------------------------" << std::endl;


	std::cout << "--- DCS DES Cloud stop at " << detail::strtime() << "." << std::endl;

	if (!outdata_fname.empty())
	{
		::std::ofstream ofs(outdata_fname.c_str());

		detail::yaml_report_stats(ofs, sys);

		ofs.close();
	}
}
