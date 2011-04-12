#include <dcs/assert.hpp>
#include <dcs/des/engine.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/config/configuration.hpp>
#include <dcs/eesim/config/operation/make_data_center.hpp>
#include <dcs/eesim/config/operation/make_data_center_manager.hpp>
#include <dcs/eesim/config/operation/make_des_engine.hpp>
#include <dcs/eesim/config/operation/make_logger.hpp>
#include <dcs/eesim/config/operation/make_random_number_generator.hpp>
#include <dcs/eesim/config/operation/read_file.hpp>
#include <dcs/eesim/config/yaml.hpp>
#include <dcs/eesim/data_center.hpp>
#include <dcs/eesim/data_center_manager.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/eesim/traits.hpp>
#include <dcs/eesim/logging/base_logger.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
//#include <dcs/math/random/any_generator.hpp>
#include <dcs/math/random.hpp>
#include <dcs/memory.hpp>
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>


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
typedef dcs::eesim::traits<
			des_engine_type,
			random_generator_type,
			real_type,
			uint_type,
			int_type
		> traits_type;
typedef dcs::eesim::registry<traits_type> registry_type;
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
				<< "  --conf <configuration-file>" << ::std::endl;
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


inline
::std::string to_string(::dcs::eesim::performance_measure_category category)
{
	switch (category)
	{
		case ::dcs::eesim::busy_time_performance_measure:
			return ::std::string("Busy Time");
		case ::dcs::eesim::response_time_performance_measure:
			return ::std::string("Response Time");
		case ::dcs::eesim::throughput_performance_measure:
			return ::std::string("Throughput");
		case ::dcs::eesim::utilization_performance_measure:
			return ::std::string("Utilization");
		default:
			break;
	}

	return ::std::string("Unknown Performance Measure");
}


template <typename TraitsT>
class simulated_system
{
	public: typedef TraitsT traits_type;
	public: typedef ::dcs::eesim::data_center<traits_type> data_center_type;
	public: typedef ::dcs::shared_ptr<data_center_type> data_center_pointer;
	public: typedef ::dcs::eesim::data_center_manager<traits_type> data_center_manager_type;
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
};


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

	// VM Placement
	{
		typedef typename data_center_type::physical_machine_identifier_type pm_identifier_type;
		typedef typename data_center_type::virtual_machine_identifier_type vm_identifier_type;
		typedef typename data_center_type::virtual_machines_placement_type vm_placement_type;
		typedef typename vm_placement_type::const_iterator vm_placement_iterator;
		typedef typename vm_placement_type::share_const_iterator vm_share_iterator;

		os << ::std::endl << "-- Last Virtual Machines Placement --" << ::std::endl;

		vm_placement_type const& vm_placement = dc.current_virtual_machines_placement();
		vm_placement_iterator place_end_it = vm_placement.end();
		for (vm_placement_iterator place_it = vm_placement.begin(); place_it != place_end_it; ++place_it)
		{
			vm_identifier_type vm_id(vm_placement.vm_id(place_it));
			vm_identifier_type pm_id(vm_placement.pm_id(place_it));

			os << indent
			   << "VM: " << vm_id << " ('" << dc.virtual_machine_ptr(vm_id)->name() << "')"
			   << " --> "
			   << "PM: " << pm_id << " ('" << dc.physical_machine_ptr(pm_id)->name() << "')"
			   << ::std::endl;

			vm_share_iterator share_end_it = vm_placement.shares_end(place_it);
			for (vm_share_iterator share_it = vm_placement.shares_begin(place_it); share_it != share_end_it; ++share_it)
			{
				::dcs::eesim::physical_resource_category category(vm_placement.resource_category(share_it));
				real_type share(vm_placement.resource_share(share_it));

				os << indent << indent
				   << "Resource: " << category << ", Share: " << share << ::std::endl;
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
		typedef ::dcs::eesim::performance_measure_category statistic_category_type;
		typedef ::std::vector<statistic_category_type> statistic_category_container;
		typedef typename statistic_category_container::const_iterator statistic_category_iterator;

		os << ::std::endl << "-- Applications --" << ::std::endl;

		application_container apps = dc.applications();
		application_iterator app_end_it = apps.end();
		for (application_iterator app_it = apps.begin(); app_it != app_end_it; ++app_it)
		{
			application_pointer ptr_app = *app_it;

			os << indent
			   << "Application: '" << ptr_app->name() << "'" << ::std::endl;
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
			//stat_categories.push_back(::dcs::eesim::response_time_performance_measure);
			//[/FIXME]

			stat_categories = ::dcs::eesim::performance_measure_categories();
			statistic_category_iterator stat_cat_end_it = stat_categories.end();
			for (statistic_category_iterator stat_cat_it = stat_categories.begin(); stat_cat_it != stat_cat_end_it; ++stat_cat_it)
			{
				statistic_category_type stat_category(*stat_cat_it);

				if (::dcs::eesim::for_application(stat_category))
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
						   << "Mean estimator: " << *ptr_stat << ::std::endl;//FIXME: statistic type is hard-coded
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

					if (::dcs::eesim::for_application_tier(stat_category))
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
							   << "Mean estimator: " << *ptr_tier_stat << ::std::endl;//FIXME: statistic type is hard-coded
						}
					}
				}
			}
		}
	}

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
			   << "Physical Machine: '" << ptr_mach->name() << "'" << ::std::endl;
			os << indent << indent
			   << "Uptime: " << ptr_mach->simulation_model().uptime() << ::std::endl;
			os << indent << indent
			   << "Consumed Energy: " << ptr_mach->simulation_model().consumed_energy() << ::std::endl;
		}
	}
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
//void process_sys_finit_sim_event(des_event_type const& evt, des_engine_context_type& ctx, ::std::basic_ostream<CharT,CharTraitsT>& os, ::dcs::shared_ptr< ::dcs::eesim::data_center<TraitsT> > const& ptr_dc)
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

}} // Namespace detail::<unnamed>


int main(int argc, char* argv[])
{
//	typedef double real_type;
//	typedef unsigned long uint_type;
//	typedef long int_type;
//	typedef dcs::des::engine<real_type> des_engine_type;
////	typedef dcs::math::random::base_generator<real_type> random_generator_type;
//	typedef dcs::math::random::base_generator<uint_type> random_generator_type;
//	typedef dcs::eesim::traits<
//				des_engine_type,
//				random_generator_type,
//				real_type,
//				uint_type,
//				int_type
//			> traits_type;
//	typedef dcs::eesim::registry<traits_type> registry_type;
	typedef dcs::shared_ptr<des_engine_type> des_engine_pointer;
	typedef dcs::shared_ptr<random_generator_type> random_generator_pointer;
	//typedef dcs::eesim::data_center<traits_type> data_center_type;
	typedef dcs::shared_ptr< dcs::eesim::data_center<traits_type> > data_center_pointer;
	typedef dcs::shared_ptr< dcs::eesim::data_center_manager<traits_type> > data_center_manager_pointer;


	prog_name = argv[0];

	if (argc < 2)
	{
		detail::usage();
		return -1;
	}


	// Parse command line arguments

	std::string conf_fname; // (argv[1]);
	bool partial_stats(false);

	if (argc > 2)
	{
		try
		{
//			partial_stats = detail::get_option<bool>(argv, argv+argc, "--partial-stats");
			conf_fname = detail::get_option<std::string>(argv, argv+argc, "--conf");
		}
		catch (std::exception const& e)
		{
			std::cerr << "[Error] Error while parsing command-line options: " << e.what() << std::endl;
			detail::usage();
			std::abort();
		}
	}
	else
	{
		// old-style usage: exec_name <conf-file>
		conf_fname = argv[1];
	}

	// Read configuration

	configuration_category conf_cat = yaml_configuration;

	dcs::eesim::config::configuration<real_type,uint_type> conf;

	switch (conf_cat)
	{
		case yaml_configuration:

			conf = dcs::eesim::config::read_file(
				conf_fname,
				::dcs::eesim::config::yaml_reader<real_type,uint_type>()
			);
			break;
		default:
			return -2;
	}

	DCS_DEBUG_TRACE("Configuration: " << conf); //XXX

	// Print configuration (for ease later info retrieval)
	::std::cout << "CONFIGURATION:" << ::std::endl
				<< conf << ::std::endl
				<< "--------------------------------------------------------------------------------" << ::std::endl
				<< ::std::endl;

	// Build the registry

	registry_type& reg(registry_type::instance());
	des_engine_pointer ptr_des_eng;
	ptr_des_eng = dcs::eesim::config::make_des_engine(conf);
	reg.des_engine(ptr_des_eng);
	random_generator_pointer ptr_rng;
	ptr_rng = dcs::eesim::config::make_random_number_generator(conf);
	reg.uniform_random_generator(ptr_rng);

//	detail::test_rng(ptr_rng);//XXX

	random_seeder_type seeder(conf.rng().seed);

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
	ptr_des_eng->system_finalization_event_source().connect(
			::dcs::functional::bind(
				&detail::process_sys_finit_sim_event<traits_type>,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2,
				&sys
			)
		);

	// Attach a simulation observer
	dcs::shared_ptr< dcs::eesim::logging::base_logger<traits_type> > ptr_sim_log;
	ptr_sim_log = dcs::eesim::config::make_logger<traits_type>(conf);
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
	ptr_dc = dcs::eesim::config::make_data_center<traits_type>(conf, ptr_rng, ptr_des_eng);
	ptr_dc_mngr = dcs::eesim::config::make_data_center_manager<traits_type>(conf, ptr_dc);

	sys.data_center(ptr_dc);
	sys.data_center_manager(ptr_dc_mngr);

	// Run the simulation
	ptr_des_eng->run();

	// Detach the simulation observer
	ptr_sim_log->detach(*ptr_des_eng);

	// Report statistics
	::std::cout << "STATISTICS:" << ::std::endl;
	detail::report_stats(::std::cout, sys);
	::std::cout << "--------------------------------------------------------------------------------" << ::std::endl;
}
