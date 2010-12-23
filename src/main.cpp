#include <dcs/des/engine.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/config/configuration.hpp>
#include <dcs/eesim/config/operation/make_data_center.hpp>
#include <dcs/eesim/config/operation/make_data_center_manager.hpp>
#include <dcs/eesim/config/operation/make_des_engine.hpp>
#include <dcs/eesim/config/operation/make_random_number_generator.hpp>
#include <dcs/eesim/config/operation/read_file.hpp>
#include <dcs/eesim/config/yaml.hpp>
#include <dcs/eesim/data_center.hpp>
#include <dcs/eesim/data_center_manager.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/eesim/traits.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
//#include <dcs/math/random/any_generator.hpp>
#include <dcs/math/random.hpp>
#include <dcs/memory.hpp>
#include <iostream>
#include <string>
#include <vector>


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


namespace detail {

typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;


void process_sys_init_sim_event(des_event_type const& evt, des_engine_context_type& ctx, random_seeder_type& seeder)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

	typedef typename random_seeder_type::result_type seed_type;

	DCS_DEBUG_TRACE("BEGIN Process System Initialization at Clock: " << ctx.simulated_time());

	seed_type seed = seeder();

	DCS_DEBUG_TRACE("Generated new seed: " << seed);//XXX

	registry_type& ref_reg = registry_type::instance();
	ref_reg.uniform_random_generator_ptr()->seed(seed);

	DCS_DEBUG_TRACE("END Process System Initialization at Clock: " << ctx.simulated_time());
}


template <
	typename CharT,
	typename CharTraitsT,
	typename TraitsT
>
void report_stats(::std::basic_ostream<CharT,CharTraitsT>& os, ::dcs::shared_ptr< ::dcs::eesim::data_center<TraitsT> > const& ptr_dc)
{
	typedef TraitsT traits_type;
	typedef ::dcs::eesim::data_center<traits_type> data_center_type;

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

		application_container apps = ptr_dc->applications();
		application_iterator app_end_it = apps.end();
		for (application_iterator app_it = apps.begin(); app_it != app_end_it; ++app_it)
		{
			application_pointer ptr_app = *app_it;

			os << "Application: '" << ptr_app->name() << "'" << ::std::endl;
			os << " Overall: " << ::std::endl;


			os << "   # Arrivals: " << ptr_app->simulation_model().num_arrivals() << ::std::endl;
			os << "   # Departures: " << ptr_app->simulation_model().num_departures() << ::std::endl;
			os << "   # SLA violations: " << ptr_app->simulation_model().num_sla_violations() << ::std::endl;

			statistic_category_container stat_categories;

			//[FIXME] statistic categories are hard-coded
			stat_categories.push_back(::dcs::eesim::response_time_performance_measure);
			//[/FIXME]

			statistic_category_iterator stat_cat_end_it = stat_categories.end();
			for (statistic_category_iterator stat_cat_it = stat_categories.begin(); stat_cat_it != stat_cat_end_it; ++stat_cat_it)
			{
				statistic_category_type stat_category(*stat_cat_it);

				os << "   Response Time: " << ::std::endl;//FIXME: statistic category name is hard-coded

				statistic_container ptr_stats;
				ptr_stats = ptr_app->simulation_model().statistic(stat_category);
				statistic_iterator stat_end_it = ptr_stats.end();
				for (statistic_iterator stat_it = ptr_stats.begin(); stat_it != stat_end_it; ++stat_it)
				{
					output_statistic_pointer ptr_stat(*stat_it);

 					os << "     Mean estimator: " << *ptr_stat << ::std::endl;//FIXME: statistic type is hard-coded
				}
			}

			tier_container tiers;
			tiers = ptr_app->tiers();
			tier_iterator tier_end_it = tiers.end();
			for (tier_iterator tier_it = tiers.begin(); tier_it != tier_end_it; ++tier_it)
			{
				application_tier_pointer ptr_tier(*tier_it);

				os << " Tier '" << ptr_tier->name() << "': " << ::std::endl;

				statistic_category_iterator stat_cat_end_it = stat_categories.end();
				for (statistic_category_iterator stat_cat_it = stat_categories.begin(); stat_cat_it != stat_cat_end_it; ++stat_cat_it)
				{
					statistic_category_type stat_category(*stat_cat_it);

					os << "   Response Time: " << ::std::endl;//FIXME: statistic category name is hard-coded

					statistic_container ptr_tier_stats;
					ptr_tier_stats = ptr_app->simulation_model().tier_statistic(ptr_tier->id(), stat_category);
					statistic_iterator tier_stat_end_it = ptr_tier_stats.end();
					for (statistic_iterator tier_stat_it = ptr_tier_stats.begin(); tier_stat_it != tier_stat_end_it; ++tier_stat_it)
					{
						output_statistic_pointer ptr_tier_stat(*tier_stat_it);

						os << "     Mean estimator: " << *ptr_tier_stat << ::std::endl;//FIXME: statistic type is hard-coded
					}
				}
			}
		}
	}
}

} // Namespace detail


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
	ptr_des_eng = dcs::eesim::config::make_des_engine(conf);
	reg.des_engine(ptr_des_eng);
	random_generator_pointer ptr_rng;
	ptr_rng = dcs::eesim::config::make_random_number_generator(conf);
	reg.uniform_random_generator(ptr_rng);

//	detail::test_rng(ptr_rng);//XXX

	random_seeder_type seeder(conf.rng().seed);

	// Register some DES event hooks
	ptr_des_eng->system_initialization_event_source().connect(
		::dcs::functional::bind(
			&detail::process_sys_init_sim_event,
			::dcs::functional::placeholders::_1,
			::dcs::functional::placeholders::_2,
			seeder
		)
	);

	// Build the Data Center

	data_center_pointer ptr_dc;
	ptr_dc = dcs::eesim::config::make_data_center<traits_type>(conf, ptr_rng, ptr_des_eng);

	data_center_manager_pointer ptr_dc_mngr;
	ptr_dc_mngr = dcs::eesim::config::make_data_center_manager<traits_type>(conf, ptr_dc);

	ptr_des_eng->run();

	detail::report_stats(::std::cout, ptr_dc);
}
