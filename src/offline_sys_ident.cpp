#include <algorithm>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublasx/operation/size.hpp>
#include <dcs/assert.hpp>
#include <dcs/des/engine.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/replications/engine.hpp>
#include <dcs/eesim/config/configuration.hpp>
#include <dcs/eesim/config/operation/make_data_center.hpp>
#include <dcs/eesim/config/operation/make_des_engine.hpp>
#include <dcs/eesim/config/operation/make_random_number_generator.hpp>
#include <dcs/eesim/config/operation/read_file.hpp>
#include <dcs/eesim/config/yaml.hpp>
#include <dcs/eesim/data_center.hpp>
#include <dcs/eesim/data_center_manager.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/physical_resource.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
//#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/eesim/traits.hpp>
#include <dcs/eesim/user_request.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/math/stats/distribution/normal.hpp>
//#include <dcs/math/random/any_generator.hpp>
#include <dcs/math/random.hpp>
#include <dcs/memory.hpp>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>


namespace ublas = ::boost::numeric::ublas;
namespace ublasx = ::boost::numeric::ublasx;


static std::string prog_name;

enum configuration_category
{
	yaml_configuration/*, TODO:
	xml_configuration*/
};


void usage()
{
	std::cerr << "Usage: " << prog_name << " conf_file" << std::endl;
}


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
typedef std::size_t size_type;
typedef dcs::des::engine<real_type> des_engine_type;
typedef dcs::math::random::base_generator<uint_type> random_generator_type;
typedef dcs::eesim::traits<
			des_engine_type,
			random_generator_type,
			real_type,
			uint_type,
			int_type
		> traits_type;
typedef dcs::shared_ptr<des_engine_type> des_engine_pointer;
typedef dcs::shared_ptr<random_generator_type> random_generator_pointer;
typedef dcs::eesim::registry<traits_type> registry_type;
typedef dcs::eesim::multi_tier_application<traits_type> application_type;
typedef dcs::eesim::user_request<traits_type> user_request_type;
typedef std::map<uint_type,real_type> request_info_map;
typedef dcs::eesim::virtual_machine<traits_type> virtual_machine_type;
typedef dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;


namespace detail { namespace /*<unnamed>*/ {


/*
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


template <
	typename CharT,
	typename CharTraitsT,
	typename TraitsT
>
void report_stats(::std::basic_ostream<CharT,CharTraitsT>& os, ::dcs::shared_ptr< ::dcs::eesim::data_center<TraitsT> > const& ptr_dc)
{
	typedef TraitsT traits_type;
	typedef typename traits_type::uint_type uint_type;
	typedef typename traits_type::real_type real_type;
	typedef ::dcs::eesim::data_center<traits_type> data_center_type;

	::std::string indent("  ");

	// VM Placement
	{
		typedef typename data_center_type::physical_machine_identifier_type pm_identifier_type;
		typedef typename data_center_type::virtual_machine_identifier_type vm_identifier_type;
		typedef typename data_center_type::virtual_machines_placement_type vm_placement_type;
		typedef typename vm_placement_type::const_iterator vm_placement_iterator;
		typedef typename vm_placement_type::share_const_iterator vm_share_iterator;

		os << ::std::endl << "-- Last Virtual Machines Placement --" << ::std::endl;

		vm_placement_type const& vm_placement = ptr_dc->current_virtual_machines_placement();
		vm_placement_iterator place_end_it = vm_placement.end();
		for (vm_placement_iterator place_it = vm_placement.begin(); place_it != place_end_it; ++place_it)
		{
			vm_identifier_type vm_id(vm_placement.vm_id(place_it));
			vm_identifier_type pm_id(vm_placement.pm_id(place_it));

			os << indent
			   << "VM: " << vm_id << " ('" << ptr_dc->virtual_machine_ptr(vm_id)->name() << "')"
			   << " --> "
			   << "PM: " << pm_id << " ('" << ptr_dc->physical_machine_ptr(pm_id)->name() << "')"
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

		application_container apps = ptr_dc->applications();
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

		physical_machine_container machs = ptr_dc->physical_machines();
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
*/


::dcs::shared_ptr<des_engine_type> make_des_engine()
{
	typedef ::dcs::des::replications::engine<real_type,uint_type> des_engine_impl_type;
	typedef ::dcs::shared_ptr<des_engine_impl_type> des_engine_impl_pointer;

	des_engine_impl_pointer ptr_des_eng;
	ptr_des_eng = ::dcs::make_shared<des_engine_impl_type>();
	ptr_des_eng->min_num_replications(1);

	return ptr_des_eng;
}


inline
real_type relative_deviation(real_type actual, real_type reference)
{
	return actual/reference - 1;
}


template <typename ValueT>
class signal_generator
{
	public: typedef ValueT value_type;
	public: typedef ::boost::numeric::ublas::vector<value_type> vector_type;


	public: vector_type operator()()
	{
		return do_generate();
	}


	private: virtual vector_type do_generate() = 0;
};

template <typename ValueT>
class step_signal_generator: public signal_generator<ValueT>
{
	private: typedef signal_generator<ValueT> base_type;
	public: typedef ValueT value_type;
	public: typedef typename base_type::vector_type vector_type;


	public: step_signal_generator(vector_type const& u0)
	: u_(u0)
	{
	}

 
	private: vector_type do_generate()
	{
		return u_;
	}


	private: vector_type u_;
};

template <typename ValueT>
class gaussian_signal_generator: public signal_generator<ValueT>
{
	private: typedef signal_generator<ValueT> base_type;
	public: typedef ValueT value_type;
	public: typedef typename base_type::vector_type vector_type;
	private: typedef ::dcs::math::stats::normal_distribution<value_type> normal_distribution_type;
	private: typedef ::std::vector<normal_distribution_type> normal_distribution_container;


	public: gaussian_signal_generator(vector_type const& mu0, vector_type const& sigma0)
	{
		// pre: size(mu0) == size(sigma0)
		DCS_ASSERT(
				ublasx::size(mu0) == ublasx::size(sigma0),
				throw ::std::invalid_argument("[guassian_signal_generator::ctor] Invalid size.")
			);

		::std::size_t n(ublasx::size(mu0));
		for (::std::size_t i = 0; i < n; ++i)
		{
			distrs_.push_back(normal_distribution_type(mu0(i), sigma0(i)));
		}
	}

 
	private: vector_type do_generate()
	{
		random_generator_pointer ptr_rng(::dcs::eesim::registry<traits_type>::instance().uniform_random_generator_ptr());

		::std::size_t n(distrs_.size());
		vector_type u(n);
		for (::std::size_t i = 0; i < n; ++i)
		{
			u(i) = ::dcs::math::stats::rand(distrs_[i], *ptr_rng);
		}

		return u;
	}


	private: normal_distribution_container distrs_;
};


template <typename TraitsT>
struct sysid_state
{
	typedef TraitsT traits_type;
	typedef typename traits_type::uint_type uint_type;

	uint_type num_arrs;
	uint_type num_deps;
	::std::vector<request_info_map> req_info_maps;
};


}} // Namespace detail::<unnamed>


template <typename TraitsT>
class system_identificator
{
	private: typedef system_identificator<TraitsT> self_type;
	public: typedef TraitsT traits_type;
//	public: typedef dcs::eesim::multi_tier_application<traits_type> application_type;
	public: typedef ::dcs::eesim::physical_machine<traits_type> physical_machine_type;
	public: typedef ::dcs::shared_ptr<physical_machine_type> physical_machine_pointer;
	public: typedef ::dcs::eesim::physical_resource<traits_type> physical_resource_type;
	public: typedef ::dcs::shared_ptr<physical_resource_type> physical_resource_pointer;
	public: typedef ::application_type::reference_physical_resource_type reference_resource_type;
	private: typedef ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	private: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
	private: typedef detail::signal_generator<real_type> signal_generator_type;
	private: typedef ::dcs::shared_ptr<signal_generator_type> signal_generator_pointer;
	private: typedef detail::sysid_state<traits_type> sysid_state_type;
	private: typedef ::dcs::shared_ptr<sysid_state_type> sysid_state_pointer;


	public: system_identificator()
	: ptr_excite_sys_evt_src_(new des_event_source_type())
	{
	}


	public: void identify(application_type& app, signal_generator_pointer const& ptr_sig_gen)
	{
		sysid_state_pointer ptr_sysid_state;

		::std::vector<physical_machine_pointer> pms;
		::std::vector<virtual_machine_pointer> vms;

		size_type num_tiers(app.num_tiers());

		ptr_sysid_state = ::dcs::make_shared<sysid_state_type>();
		ptr_sysid_state->num_arrs = uint_type/*zero*/();
		ptr_sysid_state->num_deps = uint_type/*zero*/();
		ptr_sysid_state->req_info_maps.resize(num_tiers);

		des_engine_pointer ptr_des_eng(::dcs::eesim::registry<traits_type>::instance().des_engine_ptr());

		ptr_des_eng->system_initialization_event_source().connect(
				::dcs::functional::bind(
					&self_type::process_sys_init_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
		ptr_des_eng->system_finalization_event_source().connect(
				::dcs::functional::bind(
					&self_type::process_sys_finit_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);

		// Build one reference physical machine and one virtual machine for each tier
		for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			//request_info_map req_info_map;
			//ptr_sysid_state->req_info_maps.push_back(ptr_req_info_map);

			// Build the reference machine for this tier

			physical_machine_pointer ptr_pm;

			::std::ostringstream oss;
			oss << "Machine for " << app.tier(tier_id)->name();

			ptr_pm = dcs::make_shared<physical_machine_type>(oss.str());
			ptr_pm->id(pms.size());
			pms.push_back(ptr_pm);

			typedef std::vector<reference_resource_type> reference_resource_container;
			typedef typename reference_resource_container::const_iterator reference_resource_iterator;
			reference_resource_container reference_resources(app.reference_resources());
			reference_resource_iterator ref_res_end_it(reference_resources.end());
			for (reference_resource_iterator ref_res_it = reference_resources.begin(); ref_res_it != ref_res_end_it; ++ref_res_it)
			{
				::dcs::shared_ptr<physical_resource_type> ptr_resource;

				oss.str("");
				oss.clear();
				oss << "Reference resource for " << app.tier(tier_id)->name();

				ptr_resource = dcs::make_shared<physical_resource_type>(
								oss.str(),
								ref_res_it->category(),
								ref_res_it->capacity(),
								ref_res_it->utilization_threshold()
					);
				ptr_pm->add_resource(ptr_resource);
			}

			// Build the virtual machine for this tier

			virtual_machine_pointer ptr_vm;

			oss.str("");
			oss.clear();
			oss << "VM for " << app.tier(tier_id)->name();

			ptr_vm = dcs::make_shared<virtual_machine_type>(oss.str());
			ptr_vm->id(vms.size());
			ptr_vm->guest_system(app.tier(tier_id));
			//app.simulation_model().tier_virtual_machine(ptr_vm);
			vms.push_back(ptr_vm);

			// Place the virtual machine on the reference physical machine
			// - Power-on the machine
			ptr_pm->power_on();
			// - Assign the maximum allowable resource share
			typedef std::vector<physical_resource_pointer> resource_container;
			typedef typename resource_container::const_iterator resource_iterator;
			resource_container resources(ptr_pm->resources());
			resource_iterator res_end_it(resources.end());
			for (resource_iterator res_it = resources.begin(); res_it != res_end_it; ++res_it)
			{
				real_type share(app.tier(tier_id)->resource_share((*res_it)->category()));

				share = ::std::min(share, (*res_it)->utilization_threshold());

				ptr_vm->wanted_resource_share((*res_it)->category(), share);
				ptr_vm->resource_share((*res_it)->category(), share);
			}
			ptr_pm->vmm().create_domain(ptr_vm);
			ptr_vm->power_on();

			// Register some DES event hooks for this tier
			app.simulation_model().request_tier_arrival_event_source(tier_id).connect(
					::dcs::functional::bind(
						&self_type::process_tier_request_arrival_event,
						this,
						::dcs::functional::placeholders::_1,
						::dcs::functional::placeholders::_2,
						tier_id,
						app,
						ptr_sysid_state
					)
				);
			app.simulation_model().request_tier_departure_event_source(tier_id).connect(
					::dcs::functional::bind(
						&self_type::process_tier_request_departure_event,
						this,
						::dcs::functional::placeholders::_1,
						::dcs::functional::placeholders::_2,
						tier_id,
						app,
						ptr_sysid_state
					)
				);
		}

		// Register some DES event hooks
		app.simulation_model().request_arrival_event_source().connect(
				::dcs::functional::bind(
					&self_type::process_request_arrival_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2,
					ptr_sysid_state
				)
			);
		app.simulation_model().request_departure_event_source().connect(
				::dcs::functional::bind(
					&self_type::process_request_departure_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2,
					ptr_sysid_state
				)
			);

		app.start(vms.begin(), vms.end());

		// Register the event for changing resource shares
		ptr_excite_sys_evt_src_->connect(
				::dcs::functional::bind(
					&self_type::process_excite_system_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2,
					app,
					ptr_sig_gen,
					ptr_sysid_state
				)
			);

		// Run the simulation
		ptr_des_eng->run();

		// Deregister some DES event hooks for tiers
/*FIXME: Does not compile. Why??
		for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			//::dcs::shared_ptr<request_info_map> ptr_req_info_map(req_info_maps[tier_id]);

			app.simulation_model().request_tier_arrival_event_source(tier_id).disconnect(
					::dcs::functional::bind(
						&self_type::process_tier_request_arrival_event,
						this,
						::dcs::functional::placeholders::_1,
						::dcs::functional::placeholders::_2,
						tier_id,
						app,
						ptr_sysid_state
					)
				);
			app.simulation_model().request_tier_departure_event_source(tier_id).disconnect(
					::dcs::functional::bind(
						&self_type::process_tier_request_departure_event,
						this,
						::dcs::functional::placeholders::_1,
						::dcs::functional::placeholders::_2,
						tier_id,
						app,
						ptr_sysid_state
					)
				);
		}
*/

		// Deregister some global DES event hooks
		app.simulation_model().request_arrival_event_source().disconnect(
				::dcs::functional::bind(
					&self_type::process_request_departure_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2,
					ptr_sysid_state
				)
			);
		app.simulation_model().request_departure_event_source().disconnect(
				::dcs::functional::bind(
					&self_type::process_request_departure_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2,
					ptr_sysid_state
				)
			);

	}


	private: void schedule_excite_system_event()
	{
		des_engine_pointer ptr_des_eng(::dcs::eesim::registry<traits_type>::instance().des_engine_ptr());
		ptr_des_eng->schedule_event(ptr_excite_sys_evt_src_, ptr_des_eng->simulated_time()+5);
	}


	//@{ Event Handlers


	private: void process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("BEGIN Process SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");

		schedule_excite_system_event();

		DCS_DEBUG_TRACE("END Process SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("BEGIN Process SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ")");

		DCS_DEBUG_TRACE("END Process SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("BEGIN Process REQUEST-ARRIVAL (Clock: " << ctx.simulated_time() << ")");

		ptr_sysid_state->num_arrs += 1;

		DCS_DEBUG_TRACE("END Process REQUEST-ARRIVAL (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("BEGIN Process REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");

		ptr_sysid_state->num_deps += 1;

//::std::cerr << "# deps -> " << ptr_sysid_state->num_deps << ::std::endl;//XXX
		if (ptr_sysid_state->num_deps == 1e+6)
		{
::std::cerr << "STOP!" << ::std::endl;//XXX
			::dcs::eesim::registry<traits_type>::instance().des_engine_ptr()->stop_now();
		}

		DCS_DEBUG_TRACE("END Process REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_tier_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("BEGIN Process TIER-REQUEST-ARRIVAL (Clock: " << ctx.simulated_time() << ")");

		user_request_type req = app.simulation_model().request_state(evt);

		ptr_sysid_state->req_info_maps[tier_id][req.id()] = ctx.simulated_time();

		DCS_DEBUG_TRACE("END Process TIER-REQUEST-ARRIVAL (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_tier_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("BEGIN Process TIER-REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");

		user_request_type req = app.simulation_model().request_state(evt);

		::std::cout << tier_id
					<< "," << req.id();

		virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);
		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;
		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator end_it(resource_shares.end());
		for (resource_share_iterator it = resource_shares.begin(); it != end_it; ++it)
		{
			::std::cout << "," << it->first
						<< "," << it->second
						<< "," << detail::relative_deviation(it->second, app.tier(tier_id)->resource_share(it->first));
		}

		real_type rt(ctx.simulated_time()-ptr_sysid_state->req_info_maps[tier_id].at(req.id()));

		::std::cout << "," << rt
					<< "," << detail::relative_deviation(rt, app.performance_model().tier_measure(tier_id, ::dcs::eesim::response_time_performance_measure))
					<< ::std::endl;

		ptr_sysid_state->req_info_maps[tier_id].erase(req.id());

		DCS_DEBUG_TRACE("END Process TIER-REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_excite_system_event(des_event_type const& evt, des_engine_context_type& ctx, application_type& app, signal_generator_pointer const& ptr_sig_gen, sysid_state_pointer const& ptr_sysid_state)
	{
		size_type num_tiers(app.num_tiers());

		for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

			typedef std::vector<physical_resource_pointer> resource_container;
			typedef typename resource_container::const_iterator resource_iterator;
			resource_container resources(ptr_vm->vmm().hosting_machine().resources());
			resource_iterator res_end_it(resources.end());

			ublas::vector<real_type> u((*ptr_sig_gen)());
			for (resource_iterator res_it = resources.begin(); res_it != res_end_it; ++res_it)
			{
				real_type ref_share(app.tier(tier_id)->resource_share((*res_it)->category()));
				real_type new_share;

				new_share = ::std::max(real_type(0), ::std::min(ref_share+u(tier_id), (*res_it)->utilization_threshold()));

				ptr_vm->wanted_resource_share((*res_it)->category(), new_share);
				ptr_vm->resource_share((*res_it)->category(), new_share);
			}
		}

		// Reschedule this event
		schedule_excite_system_event();
	}


	//@} Event Handlers


	private: des_event_source_pointer ptr_excite_sys_evt_src_;
};


int main(int argc, char* argv[])
{
	typedef dcs::shared_ptr<des_engine_type> des_engine_pointer;
	typedef dcs::shared_ptr<random_generator_type> random_generator_pointer;
	typedef dcs::shared_ptr< dcs::eesim::data_center<traits_type> > data_center_pointer;
	typedef dcs::shared_ptr< dcs::eesim::data_center_manager<traits_type> > data_center_manager_pointer;
	typedef dcs::eesim::config::configuration<real_type,uint_type> configuration_type;


	prog_name = argv[0];

	if (argc < 2)
	{
		usage();
		return -1;
	}


	// Read configuration

	std::string conf_fname(argv[1]);
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

	// Build the registry

	registry_type& reg(registry_type::instance());
	des_engine_pointer ptr_des_eng;
//	ptr_des_eng = dcs::eesim::config::make_des_engine(conf);
	ptr_des_eng = detail::make_des_engine();
	reg.des_engine(ptr_des_eng);
	random_generator_pointer ptr_rng;
	ptr_rng = dcs::eesim::config::make_random_number_generator(conf);
	reg.uniform_random_generator(ptr_rng);

//	// Build the Data Center
//	data_center_pointer ptr_dc;
//	data_center_manager_pointer ptr_dc_mngr;
//	ptr_dc = dcs::eesim::config::make_data_center<traits_type>(conf, ptr_rng, ptr_des_eng);
//	ptr_dc_mngr = dcs::eesim::config::make_data_center_manager<traits_type>(conf, ptr_dc);

	typedef configuration_type::data_center_config_type data_center_config_type;
	typedef data_center_config_type::application_config_container::const_iterator app_iterator;
//	typedef dcs::eesim::multi_tier_application<traits_type> application_type;
	typedef dcs::eesim::physical_machine<traits_type> physical_machine_type;
	typedef dcs::shared_ptr<physical_machine_type> physical_machine_pointer;
	typedef dcs::eesim::physical_resource<traits_type> physical_resource_type;
	typedef dcs::shared_ptr<physical_resource_type> physical_resource_pointer;
	typedef application_type::reference_physical_resource_type reference_resource_type;

	app_iterator app_end_it = conf.data_center().applications().end();
	for (app_iterator app_it = conf.data_center().applications().begin(); app_it != app_end_it; ++app_it)
	{
		dcs::shared_ptr<application_type> ptr_app;
		dcs::shared_ptr< detail::signal_generator<real_type> > ptr_sig_gen;

		// Build the application
		ptr_app = dcs::eesim::config::make_multi_tier_application<traits_type>(*app_it, conf, ptr_rng, ptr_des_eng);

		// Build the signal generator
		//ptr_sig_gen = dcs::make_shared< detail::step_signal_generator<real_type> >(ublas::scalar_vector<real_type>(ptr_app->num_tiers(), 1));
		ptr_sig_gen = dcs::make_shared< detail::gaussian_signal_generator<real_type> >(
						ublas::zero_vector<real_type>(ptr_app->num_tiers()),
						ublas::scalar_vector<real_type>(ptr_app->num_tiers(), 1)
			);

		system_identificator<traits_type> sysid;

//		sysid.des_engine(ptr_des_eng);
//		sysid.uniform_random_generator(ptr_rng);
		sysid.identify(*ptr_app, ptr_sig_gen);

/*
		dcs::shared_ptr<uint_type> ptr_num_arrs;
		dcs::shared_ptr<uint_type> ptr_num_deps;

		ptr_num_arrs = dcs::make_shared<uint_type>(0);
		ptr_num_deps = dcs::make_shared<uint_type>(0);

		::std::vector<physical_machine_pointer> pms;
		::std::vector<virtual_machine_pointer> vms;
		::std::vector< ::dcs::shared_ptr<request_info_map> > req_info_maps;

		size_type num_tiers(ptr_app->num_tiers());

		// Build one reference physical machine and one virtual machine for each tier
		for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			dcs::shared_ptr<request_info_map> ptr_req_info_map;
			ptr_req_info_map = dcs::make_shared<request_info_map>();

			req_info_maps.push_back(ptr_req_info_map);

			// Build the reference machine for this tier

			physical_machine_pointer ptr_pm;

			std::ostringstream oss;
			oss << "Machine for " << ptr_app->tier(tier_id)->name();

			ptr_pm = dcs::make_shared<physical_machine_type>(oss.str());
			ptr_pm->id(pms.size());
			pms.push_back(ptr_pm);

			typedef std::vector<reference_resource_type> reference_resource_container;
			typedef reference_resource_container::const_iterator reference_resource_iterator;
			reference_resource_container reference_resources(ptr_app->reference_resources());
			reference_resource_iterator ref_res_end_it(reference_resources.end());
			for (reference_resource_iterator ref_res_it = reference_resources.begin(); ref_res_it != ref_res_end_it; ++ref_res_it)
			{
				dcs::shared_ptr<physical_resource_type> ptr_resource;

				oss.str("");
				oss.clear();
				oss << "Reference resource for " << ptr_app->tier(tier_id)->name();

				ptr_resource = dcs::make_shared<physical_resource_type>(
								oss.str(),
								ref_res_it->category(),
								ref_res_it->capacity(),
								ref_res_it->utilization_threshold()
					);
				ptr_pm->add_resource(ptr_resource);
			}

			// Build the virtual machine for this tier

			virtual_machine_pointer ptr_vm;

			oss.str("");
			oss.clear();
			oss << "VM for " << ptr_app->tier(tier_id)->name();

			ptr_vm = dcs::make_shared<virtual_machine_type>(oss.str());
			ptr_vm->id(vms.size());
			ptr_vm->guest_system(ptr_app->tier(tier_id));
			//ptr_app->simulation_model().tier_virtual_machine(ptr_vm);
			vms.push_back(ptr_vm);

			// Place the virtual machine on the reference physical machine
			// - Power-on the machine
			ptr_pm->power_on();
			// - Assign the maximum allowable resource share
			typedef std::vector<physical_resource_pointer> resource_container;
			typedef resource_container::const_iterator resource_iterator;
			resource_container resources(ptr_pm->resources());
			resource_iterator res_end_it(resources.end());
			for (resource_iterator res_it = resources.begin(); res_it != res_end_it; ++res_it)
			{
				ptr_vm->wanted_resource_share((*res_it)->category(), (*res_it)->utilization_threshold());
				ptr_vm->resource_share((*res_it)->category(), (*res_it)->utilization_threshold());
			}
			ptr_pm->vmm().create_domain(ptr_vm);
			ptr_vm->power_on();

			// Register some DES event hooks for this tier
			ptr_app->simulation_model().request_tier_arrival_event_source(tier_id).connect(
					dcs::functional::bind(
						&detail::process_tier_request_arrival_event,
						dcs::functional::placeholders::_1,
						dcs::functional::placeholders::_2,
						tier_id,
						*ptr_app,
						ptr_req_info_map
					)
				);
			ptr_app->simulation_model().request_tier_departure_event_source(tier_id).connect(
					dcs::functional::bind(
						&detail::process_tier_request_departure_event,
						dcs::functional::placeholders::_1,
						dcs::functional::placeholders::_2,
						tier_id,
						*ptr_app,
						ptr_req_info_map
					)
				);
		}

		// Register some DES event hooks
		ptr_app->simulation_model().request_arrival_event_source().connect(
				dcs::functional::bind(
					&detail::process_request_arrival_event,
					dcs::functional::placeholders::_1,
					dcs::functional::placeholders::_2,
					ptr_num_arrs
				)
			);
		ptr_app->simulation_model().request_departure_event_source().connect(
				dcs::functional::bind(
					&detail::process_request_departure_event,
					dcs::functional::placeholders::_1,
					dcs::functional::placeholders::_2,
					ptr_num_deps
				)
			);

		ptr_app->start(vms.begin(), vms.end());

		// Run the simulation
		ptr_des_eng->run();

		// Deregister some DES event hooks for tiers
//FIXME: Does not compile. Why??
//		for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
//		{
//			dcs::shared_ptr<request_info_map> ptr_req_info_map(req_info_maps[tier_id]);
//
//			ptr_app->simulation_model().request_tier_arrival_event_source(tier_id).disconnect(
//					dcs::functional::bind(
//						&detail::process_tier_request_arrival_event,
//						dcs::functional::placeholders::_1,
//						dcs::functional::placeholders::_2,
//						tier_id,
//						*ptr_app,
//						ptr_req_info_map
//					)
//				);
//			ptr_app->simulation_model().request_tier_departure_event_source(tier_id).disconnect(
//					dcs::functional::bind(
//						&detail::process_tier_request_departure_event,
//						dcs::functional::placeholders::_1,
//						dcs::functional::placeholders::_2,
//						tier_id,
//						*ptr_app,
//						ptr_req_info_map
//					)
//				);
//		}
//

		// Deregister some global DES event hooks
		ptr_app->simulation_model().request_arrival_event_source().disconnect(
				dcs::functional::bind(
					&detail::process_request_departure_event,
					dcs::functional::placeholders::_1,
					dcs::functional::placeholders::_2,
					ptr_num_arrs
				)
			);
		ptr_app->simulation_model().request_departure_event_source().disconnect(
				dcs::functional::bind(
					&detail::process_request_departure_event,
					dcs::functional::placeholders::_1,
					dcs::functional::placeholders::_2,
					ptr_num_deps
				)
			);

		//// Report statistics
		//detail::report_stats(::std::cout, ptr_dc);
*/
	}
}
