/**
 * \file src/offline_bench.cpp
 *
 * \brief Simulation of offline system benchmarking.
 *
 * The purpose of this program is to benchmark the system under test in order to
 * evaluate its performance.
 *
 * Copyright (C) 2009-2012  Distributed Computing System (DCS) Group, Computer
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

#ifdef BOOST_PARAMETER_MAX_ARITY
#	undef BOOST_PARAMETER_MAX_ARITY
#endif
#define BOOST_PARAMETER_MAX_ARITY 7

#include <algorithm>
//#include <boost/accumulators/accumulators.hpp>
//#include <boost/accumulators/statistics/count.hpp>
//#include <boost/accumulators/statistics/extended_p_square.hpp>
//#include <boost/accumulators/statistics/max.hpp>
//#include <boost/accumulators/statistics/mean.hpp>
//#include <boost/accumulators/statistics/min.hpp>
//#include <boost/accumulators/statistics/stats.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublasx/operation/size.hpp>
#include <cmath>
#include <dcs/assert.hpp>
#include <dcs/des/engine.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/max_estimator.hpp>
#include <dcs/des/mean_estimator.hpp>
#include <dcs/des/min_estimator.hpp>
#include <dcs/des/null_transient_detector.hpp>
#include <dcs/des/quantile_estimator.hpp>
#include <dcs/des/replications/engine.hpp>
#include <dcs/des/replications/dummy_num_replications_detector.hpp>
#include <dcs/des/replications/dummy_replication_size_detector.hpp>
#include <dcs/des/cloud/config/configuration.hpp>
#include <dcs/des/cloud/config/operation/make_application.hpp>
#include <dcs/des/cloud/config/operation/make_des_engine.hpp>
#include <dcs/des/cloud/config/operation/make_random_number_generator.hpp>
#include <dcs/des/cloud/config/operation/read_file.hpp>
#include <dcs/des/cloud/config/yaml.hpp>
#include <dcs/des/cloud/data_center.hpp>
#include <dcs/des/cloud/data_center_manager.hpp>
#include <dcs/des/cloud/performance_measure_category.hpp>
#include <dcs/des/cloud/physical_machine.hpp>
#include <dcs/des/cloud/physical_resource.hpp>
#include <dcs/des/cloud/physical_resource_category.hpp>
//#include <dcs/des/cloud/performance_measure_category.hpp>
#include <dcs/des/cloud/registry.hpp>
#include <dcs/des/cloud/traits.hpp>
#include <dcs/des/cloud/user_request.hpp>
#include <dcs/perfeval/sla/base_cost_model.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/math/constants.hpp>
#include <dcs/math/stats/distribution/normal.hpp>
//#include <dcs/math/random/any_generator.hpp>
#include <dcs/math/random.hpp>
#include <dcs/memory.hpp>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>


namespace ublas = ::boost::numeric::ublas;
namespace ublasx = ::boost::numeric::ublasx;


static std::string prog_name;

enum configuration_category
{
	yaml_configuration/*, TODO:
	xml_configuration*/
};


typedef double real_type;
typedef unsigned long uint_type;
typedef long int_type;
typedef std::size_t size_type;
typedef dcs::des::engine<real_type> des_engine_type;
typedef dcs::math::random::base_generator<uint_type> random_generator_type;
typedef dcs::des::cloud::traits<
			des_engine_type,
			random_generator_type,
			dcs::des::cloud::config::configuration<real_type,uint_type>,
			real_type,
			uint_type,
			int_type
		> traits_type;
typedef dcs::shared_ptr<des_engine_type> des_engine_pointer;
typedef dcs::shared_ptr<random_generator_type> random_generator_pointer;
typedef dcs::des::cloud::registry<traits_type> registry_type;
typedef dcs::des::cloud::multi_tier_application<traits_type> application_type;
typedef dcs::des::cloud::user_request<traits_type> user_request_type;
typedef dcs::des::cloud::virtual_machine<traits_type> virtual_machine_type;
typedef dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
typedef ::dcs::math::random::minstd_rand1 random_seeder_type;


namespace detail { namespace /*<unnamed>*/ {


typedef ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
typedef ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;


void usage()
{
//	::std::cerr << "Usage: " << prog_name << " <options>" << ::std::endl
//				<< "Options:" << ::std::endl
//				<< "  --ts <sampling-time>" << ::std::endl
//				<< "  --ns <number-of-samples>" << ::std::endl
//				<< "  --sys {'siso'|'miso'}" << ::std::endl
//				<< "  --sig {'step'|'gaussian'|'sine'}" << ::std::endl
//				<< "  --conf <configuration-file>" << ::std::endl;
}


template <typename ForwardIterT>
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


/// Get a boolean option; also tell if a given option does exist.
template <typename ForwardIterT>
bool get_option(ForwardIterT begin, ForwardIterT end, std::string const& option)
{
	ForwardIterT it = find_option(begin, end, option);

	return it != end;
}


template <typename TraitsT>
::dcs::shared_ptr<typename TraitsT::des_engine_type> make_des_engine()
{
	typedef ::dcs::des::replications::engine<
				typename TraitsT::real_type,
				typename TraitsT::uint_type
			> des_engine_impl_type;
	typedef ::dcs::shared_ptr<des_engine_impl_type> des_engine_impl_pointer;

	des_engine_impl_pointer ptr_des_eng;
	ptr_des_eng = ::dcs::make_shared<des_engine_impl_type>();
	ptr_des_eng->min_num_replications(1);

	return ptr_des_eng;
}


template <typename ValueT>
inline
ValueT md1_residence_time(ValueT lambda, ValueT s)
{
	ValueT rho(lambda*s);

	return  rho*s/(ValueT(2)*(1-rho)) + s;
}


template <typename ValueT>
inline
ValueT relative_deviation(ValueT actual, ValueT reference)
{
	return actual/reference - ValueT(1);
}


template <typename TraitsT>
struct request_info
{
	typedef TraitsT traits_type;
	typedef typename traits_type::real_type real_type;
	typedef typename traits_type::uint_type uint_type;

	real_type arrival_time;
};


template <typename ValueT>
class dummy_sla_cost_model: public ::dcs::perfeval::sla::base_cost_model<
										::dcs::des::cloud::performance_measure_category,
										ValueT
									>
{
	private: typedef ::dcs::perfeval::sla::base_cost_model<
						::dcs::des::cloud::performance_measure_category,
						ValueT
				> base_type;
	public: typedef typename base_type::metric_category_type metric_category_type;
	public: typedef typename base_type::value_type value_type;
	public: typedef typename base_type::real_type real_type;
	private: typedef typename base_type::metric_category_iterator metric_category_iterator;
	private: typedef typename base_type::metric_iterator metric_iterator;
	private: typedef typename base_type::slo_model_type slo_model_type;
	private: typedef ::std::map<metric_category_type,value_type> slo_map;


	private: void do_add_slo(slo_model_type const& slo)
	{
		slos_[slo.category()] = slo.reference_value();
	}


	private: bool do_has_slo(metric_category_type category) const
	{
		return slos_.count(category) > 0;
	}


	private: ::std::vector<metric_category_type> do_slo_categories() const
	{
		typedef typename slo_map::const_iterator iterator;

		::std::vector<metric_category_type> res;

		iterator end_it(slos_.end());
		for (iterator it = slos_.begin(); it != end_it; ++it)
		{
			res.push_back(it->first);
		}
		return res;
	}


	private: value_type do_slo_value(metric_category_type category) const
	{
		typename slo_map::const_iterator it(slos_.find(category));

		if (it == slos_.end())
		{
			throw ::std::invalid_argument("[detail::dummy_sla_cost_model] Category not found.");
		}

		return it->second;
	}


	private: real_type do_score(metric_category_iterator category_first, metric_category_iterator category_last, metric_iterator metric_first) const
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(category_first);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(category_last);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(metric_first);

		return real_type(0);
	}


	private: bool do_satisfied(metric_category_iterator category_first, metric_category_iterator category_last, metric_iterator metric_first) const
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(category_first);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(category_last);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(metric_first);

		return true;
	}


	private: slo_map slos_;
};


template <typename TraitsT>
class benchmark
{
	private: typedef benchmark<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef dcs::des::cloud::config::configuration<real_type,uint_type> configuration_type;
//	public: typedef dcs::des::cloud::multi_tier_application<traits_type> application_type;
	public: typedef ::dcs::des::cloud::physical_machine<traits_type> physical_machine_type;
	public: typedef ::dcs::shared_ptr<physical_machine_type> physical_machine_pointer;
	public: typedef ::dcs::des::cloud::physical_resource<traits_type> physical_resource_type;
	public: typedef ::dcs::shared_ptr<physical_resource_type> physical_resource_pointer;
	public: typedef ::application_type::reference_physical_resource_type reference_resource_type;
	private: typedef ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	private: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
//	private: typedef ::boost::accumulators::accumulator_set<
//						real_type,
//						::boost::accumulators::features<
//							::boost::accumulators::tag::count,
//							::boost::accumulators::tag::min,
//							::boost::accumulators::tag::mean,
//							::boost::accumulators::tag::extended_p_square,
//							::boost::accumulators::tag::max
//						>
//				> accumulator_type;
//	private: typedef ::dcs::shared_ptr<accumulator_type> accumulator_pointer;
//	private: typedef detail::request_info<traits_type> request_info_type;
//	private: typedef ::std::map<uint_type,request_info_type> request_info_map;
//	private: typedef ::std::map<uint_type,request_info_map> tier_request_info_map;
	private: typedef ::dcs::des::base_statistic<real_type,uint_type> statistic_type;
	private: typedef ::dcs::shared_ptr<statistic_type> statistic_pointer;
	private: typedef ::dcs::des::base_analyzable_statistic<real_type,uint_type> analyzable_statistic_type;
	private: typedef ::dcs::shared_ptr<analyzable_statistic_type> analyzable_statistic_pointer;
	private: typedef ::dcs::shared_ptr<application_type> application_pointer;
	private: typedef ::std::vector<application_pointer> application_container;
	private: typedef ::std::vector<analyzable_statistic_pointer> analyzable_statistic_container;
	private: typedef ::std::map<typename traits_type::application_identifier_type, analyzable_statistic_container> application_statistic_map;
	private: typedef ::std::map<typename traits_type::application_identifier_type, ::std::map<uint_type, analyzable_statistic_container> > tier_statistic_map;


	private: enum quantiles { q25, q50, q75, q90, q95, q99, q99_5, q99_9, q99_99, q99_999 };
	private: static const real_type probs_[10]/* = {0.25, 0.50, 0.75, 0.90, 0.95, 0.99, 0.995, 0.999, 0.9999, 0.99999}*/;


	public: benchmark()
	{
	}


	public: void run(configuration_type const& conf)
	{
		typedef typename configuration_type::data_center_config_type data_center_config_type;
		typedef typename data_center_config_type::application_config_container::const_iterator app_iterator;

		// Clear previous state
		reset();

		::std::vector<physical_machine_pointer> pms;
		::std::vector<virtual_machine_pointer> vms;

		::dcs::des::cloud::registry<traits_type> const& reg(::dcs::des::cloud::registry<traits_type>::instance());

		des_engine_pointer ptr_des_eng(reg.des_engine_ptr());
		random_generator_pointer ptr_rng(reg.uniform_random_generator_ptr());
		real_type conf_level(conf.simulation().output_analysis.confidence_level);

		// Build applications
		app_iterator app_end_it = conf.data_center().applications().end();
		for (app_iterator app_it = conf.data_center().applications().begin(); app_it != app_end_it; ++app_it)
		{
			dcs::shared_ptr<application_type> ptr_app;

			ptr_app = dcs::des::cloud::config::make_application<traits_type>(*app_it, conf, ptr_rng, ptr_des_eng);

			ptr_app->id(apps_.size());
			apps_.push_back(ptr_app);

			size_type num_tiers(ptr_app->num_tiers());

//			ptr_des_eng->system_initialization_event_source().connect(
//					::dcs::functional::bind(
//						&self_type::process_sys_init_event,
//						this,
//						::dcs::functional::placeholders::_1,
//						::dcs::functional::placeholders::_2,
//						*ptr_app
//					)
//				);
//			ptr_des_eng->system_finalization_event_source().connect(
//					::dcs::functional::bind(
//						&self_type::process_sys_finit_event,
//						this,
//						::dcs::functional::placeholders::_1,
//						::dcs::functional::placeholders::_2,
//						*ptr_app
//					)
//				);

			// Set a fake SLA cost model
			ptr_app->sla_cost_model(dummy_sla_cost_model<real_type>());

			// Set some stat to monitor
			analyzable_statistic_pointer ptr_stat;

			// Response Time - Mean estimator
			ptr_stat = make_analyzable_statistic(
					::dcs::des::mean_estimator<real_type,uint_type>(conf_level),
					*dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type>* >(ptr_des_eng.get())
				);
			ptr_app->simulation_model().statistic(
					::dcs::des::cloud::response_time_performance_measure,
					ptr_stat
				);
			app_stat_map_[ptr_app->id()].push_back(ptr_stat);
			// Response Time - Minimum estimator
			ptr_stat = make_analyzable_statistic(
					::dcs::des::min_estimator<real_type,uint_type>(conf_level),
					*dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type>* >(ptr_des_eng.get())
				);
			ptr_app->simulation_model().statistic(
					::dcs::des::cloud::response_time_performance_measure,
					ptr_stat
				);
			app_stat_map_[ptr_app->id()].push_back(ptr_stat);
			// Response Time - Maximum estimator
			ptr_stat = make_analyzable_statistic(
					::dcs::des::max_estimator<real_type,uint_type>(conf_level),
					*dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type>* >(ptr_des_eng.get())
				);
			ptr_app->simulation_model().statistic(
					::dcs::des::cloud::response_time_performance_measure,
					ptr_stat
				);
			app_stat_map_[ptr_app->id()].push_back(ptr_stat);
			// Response Time - Quantiles estimator
			for (::std::size_t p = 0; p < num_quantiles(); ++p)
			{
				ptr_stat = make_analyzable_statistic(
						::dcs::des::quantile_estimator<real_type,uint_type>(probs_[p], conf_level),
						*dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type>* >(ptr_des_eng.get())
					);
				ptr_app->simulation_model().statistic(
						::dcs::des::cloud::response_time_performance_measure,
						ptr_stat
					);
				app_stat_map_[ptr_app->id()].push_back(ptr_stat);
			}

			// Build one reference physical machine and one virtual machine for each tier
			for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
			{
				//request_info_map req_info_map;

				// Build the reference machine for this tier

				physical_machine_pointer ptr_pm;

				::std::ostringstream oss;
				oss << "Machine for " << ptr_app->tier(tier_id)->name();

				ptr_pm = dcs::make_shared<physical_machine_type>(oss.str());
				ptr_pm->id(pms.size());
				pms.push_back(ptr_pm);

				typedef std::vector<reference_resource_type> reference_resource_container;
				typedef typename reference_resource_container::const_iterator reference_resource_iterator;
				reference_resource_container reference_resources(ptr_app->reference_resources());
				reference_resource_iterator ref_res_end_it(reference_resources.end());
				for (reference_resource_iterator ref_res_it = reference_resources.begin(); ref_res_it != ref_res_end_it; ++ref_res_it)
				{
					::dcs::shared_ptr<physical_resource_type> ptr_resource;

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
				ptr_app->simulation_model().tier_virtual_machine(ptr_vm);
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
					real_type share(ptr_app->tier(tier_id)->resource_share((*res_it)->category()));

					share = ::std::min(share, (*res_it)->utilization_threshold());

					ptr_vm->wanted_resource_share((*res_it)->category(), share);
					ptr_vm->resource_share((*res_it)->category(), share);
				}
				ptr_pm->vmm().create_domain(ptr_vm);
				ptr_vm->power_on();

//				// Register some DES event hooks for this tier
//				ptr_app->simulation_model().request_tier_arrival_event_source(tier_id).connect(
//						::dcs::functional::bind(
//							&self_type::process_tier_request_arrival_event,
//							this,
//							::dcs::functional::placeholders::_1,
//							::dcs::functional::placeholders::_2,
//							tier_id,
//							*ptr_app
//						)
//					);
//				ptr_app->simulation_model().request_tier_service_event_source(tier_id).connect(
//						::dcs::functional::bind(
//							&self_type::process_tier_request_service_event,
//							this,
//							::dcs::functional::placeholders::_1,
//							::dcs::functional::placeholders::_2,
//							tier_id,
//							*ptr_app
//						)
//					);
//				ptr_app->simulation_model().request_tier_departure_event_source(tier_id).connect(
//						::dcs::functional::bind(
//							&self_type::process_tier_request_departure_event,
//							this,
//							::dcs::functional::placeholders::_1,
//							::dcs::functional::placeholders::_2,
//							tier_id,
//							*ptr_app
//						)
//					);

				// Set some stat to monitor
				// Response Time - Mean estimator
				ptr_stat = make_analyzable_statistic(
						::dcs::des::mean_estimator<real_type,uint_type>(conf_level),
						*dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type>* >(ptr_des_eng.get())
					);
				ptr_app->simulation_model().tier_statistic(
						tier_id,
						::dcs::des::cloud::response_time_performance_measure,
						ptr_stat
					);
				tier_stat_map_[ptr_app->id()][tier_id].push_back(ptr_stat);
				// Response Time - Minimum estimator
				ptr_stat = make_analyzable_statistic(
						::dcs::des::min_estimator<real_type,uint_type>(conf_level),
						*dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type>* >(ptr_des_eng.get())
					);
				ptr_app->simulation_model().tier_statistic(
						tier_id,
						::dcs::des::cloud::response_time_performance_measure,
						ptr_stat
					);
				tier_stat_map_[ptr_app->id()][tier_id].push_back(ptr_stat);
				// Response Time - Maximum estimator
				ptr_stat = make_analyzable_statistic(
						::dcs::des::max_estimator<real_type,uint_type>(conf_level),
						*dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type>* >(ptr_des_eng.get())
					);
				ptr_app->simulation_model().tier_statistic(
						tier_id,
						::dcs::des::cloud::response_time_performance_measure,
						ptr_stat
					);
				tier_stat_map_[ptr_app->id()][tier_id].push_back(ptr_stat);
				// Response Time - Quantiles estimator
				for (::std::size_t p = 0; p < num_quantiles(); ++p)
				{
					ptr_stat = make_analyzable_statistic(
							::dcs::des::quantile_estimator<real_type,uint_type>(probs_[p], conf_level),
							*dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type>* >(ptr_des_eng.get())
						);
					ptr_app->simulation_model().tier_statistic(
							tier_id,
							::dcs::des::cloud::response_time_performance_measure,
							ptr_stat
						);
					tier_stat_map_[ptr_app->id()][tier_id].push_back(ptr_stat);
				}
				// Utilization - Mean estimator
				ptr_stat = make_analyzable_statistic(
						::dcs::des::mean_estimator<real_type,uint_type>(conf_level),
						*dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type>* >(ptr_des_eng.get())
					);
				ptr_app->simulation_model().tier_statistic(
						tier_id,
						::dcs::des::cloud::utilization_performance_measure,
						ptr_stat
					);
				tier_stat_map_[ptr_app->id()][tier_id].push_back(ptr_stat);
			}

//			// Register some DES event hooks
//			ptr_app->simulation_model().request_arrival_event_source().connect(
//					::dcs::functional::bind(
//						&self_type::process_request_arrival_event,
//						this,
//						::dcs::functional::placeholders::_1,
//						::dcs::functional::placeholders::_2,
//						*ptr_app
//					)
//				);
//			ptr_app->simulation_model().request_departure_event_source().connect(
//					::dcs::functional::bind(
//						&self_type::process_request_departure_event,
//						this,
//						::dcs::functional::placeholders::_1,
//						::dcs::functional::placeholders::_2,
//						*ptr_app
//					)
//				);

			ptr_app->start(vms.begin(), vms.end());

//			app_acc_map_[ptr_app->id()] = ::dcs::make_shared<accumulator_type>(::boost::accumulators::tag::extended_p_square::probabilities = probs_);
		}

		// Run the simulation
		ptr_des_eng->run();
	}


	public: template <typename CharT, typename CharTraitsT>
		void report_stats(::std::basic_ostream<CharT,CharTraitsT>& os)
	{
		typedef typename application_container::const_iterator application_iterator;
//		typedef typename ::std::vector<statistic_pointer> statistic_container;
//		typedef typename statistic_container::const_iterator statistic_iterator;
		typedef typename analyzable_statistic_container::const_iterator statistic_iterator;

		::std::string indent("  ");

		os << ::std::endl << "-- Applications --" << ::std::endl;

		application_iterator app_end_it(apps_.end());
		for (application_iterator app_it = apps_.begin(); app_it != app_end_it; ++app_it)
		{
			application_pointer ptr_app(*app_it);

			os << indent
			   << "Application: '" << ptr_app->name() << "'" << ::std::endl;

//			statistic_container stats;
//			stats = ptr_app->simulation_model().statistic(::dcs::des::cloud::response_time_performance_measure);
//
//			statistic_iterator stat_end_it(stats.end());
//			for (statistic_iterator stat_it = stats.begin(); stat_it != stat_end_it; ++stat_it)
//			{
//				os << *(*stat_it) << ::std::endl;
//			}

			os << indent << indent
			   << "Overall: " << ::std::endl;

			os << indent << indent << indent
			   << "Response Time: " << ::std::endl;

			statistic_iterator stat_end_it;

			stat_end_it = app_stat_map_.at(ptr_app->id()).end();
			for (statistic_iterator stat_it = app_stat_map_.at(ptr_app->id()).begin(); stat_it != stat_end_it; ++stat_it)
			{
				statistic_pointer ptr_stat(*stat_it);

				os << indent << indent << indent << indent
				   << ptr_stat->name() << ": " << *ptr_stat << ::std::endl;
			}

			for (uint_type tier_id = 0; tier_id < ptr_app->num_tiers(); ++tier_id)
			{
				os << indent << indent
				   << "Tier '" << ptr_app->tier(tier_id)->name() << "': " << ::std::endl;

				stat_end_it = tier_stat_map_.at(ptr_app->id()).at(tier_id).end();
				for (statistic_iterator stat_it = tier_stat_map_.at(ptr_app->id()).at(tier_id).begin(); stat_it != stat_end_it; ++stat_it)
				{
					statistic_pointer ptr_stat(*stat_it);

					os << indent << indent << indent << indent
					   << ptr_stat->name() << ": " << *ptr_stat << ::std::endl;
				}
			}
		}
	}


	public: void reset()
	{
		apps_.clear();
		app_stat_map_.clear();
		tier_stat_map_.clear();
	}


//	public: void run(application_type& app/*, uint_type max_num_deps*/)
//	{
//		// Deregister some DES event hooks for tiers
/*FIXME: Does not compile. Why??*/
//		for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
//		{
//			//::dcs::shared_ptr<request_info_map> ptr_req_info_map(req_info_maps[tier_id]);
//
//			app.simulation_model().request_tier_arrival_event_source(tier_id).disconnect(
//					::dcs::functional::bind(
//						&self_type::process_tier_request_arrival_event,
//						this,
//						::dcs::functional::placeholders::_1,
//						::dcs::functional::placeholders::_2,
//						tier_id,
//						app
//					)
//				);
//			app.simulation_model().request_tier_service_event_source(tier_id).disconnect(
//					::dcs::functional::bind(
//						&self_type::process_tier_request_service_event,
//						this,
//						::dcs::functional::placeholders::_1,
//						::dcs::functional::placeholders::_2,
//						tier_id,
//						app
//					)
//				);
//			app.simulation_model().request_tier_departure_event_source(tier_id).disconnect(
//					::dcs::functional::bind(
//						&self_type::process_tier_request_departure_event,
//						this,
//						::dcs::functional::placeholders::_1,
//						::dcs::functional::placeholders::_2,
//						tier_id,
//						app
//					)
//				);
//		}
//		// Deregister some global DES event hooks
//		app.simulation_model().request_arrival_event_source().disconnect(
//				::dcs::functional::bind(
//					&self_type::process_request_arrival_event,
//					this,
//					::dcs::functional::placeholders::_1,
//					::dcs::functional::placeholders::_2,
//					app
//				)
//			);
//		app.simulation_model().request_departure_event_source().disconnect(
//				::dcs::functional::bind(
//					&self_type::process_request_departure_event,
//					this,
//					::dcs::functional::placeholders::_1,
//					::dcs::functional::placeholders::_2,
//					app
//				)
//			);

//		ptr_des_eng->system_initialization_event_source().disconnect(
//				::dcs::functional::bind(
//					&self_type::process_sys_init_event,
//					this,
//					::dcs::functional::placeholders::_1,
//					::dcs::functional::placeholders::_2,
//					app
//				)
//			);
//		ptr_des_eng->system_finalization_event_source().disconnect(
//				::dcs::functional::bind(
//					&self_type::process_sys_finit_event,
//					this,
//					::dcs::functional::placeholders::_1,
//					::dcs::functional::placeholders::_2,
//					app
//				)
//			);
//	}


	private: static uint_type num_quantiles()
	{
		return sizeof(probs_)/sizeof(probs_[0]);
	}


	private: template <typename StatisticT, typename DesEngineT>
		static analyzable_statistic_pointer make_analyzable_statistic(StatisticT const& stat, DesEngineT& eng)
	{
		return ::dcs::des::make_analyzable_statistic(
					stat,
					::dcs::des::null_transient_detector<real_type,uint_type>(),
					::dcs::des::replications::dummy_replication_size_detector<real_type,uint_type>(),
					::dcs::des::replications::dummy_num_replications_detector<real_type,uint_type>(),
					eng,
					::dcs::math::constants::infinity<real_type>::value,
					::dcs::math::constants::infinity<uint_type>::value
			);
	}


/*
	private: ::std::string to_string(statistic_type const& stat)
	{
//					   << to_string(ptr_stat->category()) << ": " << *ptr_stat << ::std::endl;
		::std::ostringstream oss;
		switch (stat.category())
		{
			case ::dcs::des::max_statistic:
				oss << "Max";
				break;
			case ::dcs::des::mean_statistic:
				oss << "Mean";
				break;
			case ::dcs::des::min_statistic:
				oss << "Min";
				break;
			case ::dcs::des::quantile_statistic:
{
::std::cerr << "Before cast: " << stat << ::std::endl;///XXX
				::dcs::des::quantile_estimator<real_type,uint_type> const* ptr_stat = dynamic_cast< ::dcs::des::quantile_estimator<real_type,uint_type> const* >(&stat);
				oss << ptr_stat->probability() << "th Quantile";
::std::cerr << "After cast: " << stat << ::std::endl;///XXX
}
				break;
			case ::dcs::des::variance_statistic:
				oss << "Variance";
				break;
			default:
				throw ::std::runtime_error("[detail::benchmark::to_string] Unknown statistic category.");
		}

		oss << ": " << stat;

		return oss.str();
	}
*/

/*
	//@{ Event Handlers


	private: void process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(evt);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ctx);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(app);

		DCS_DEBUG_TRACE("BEGIN Process SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");

//		num_arrs_ = num_deps_ = uint_type(0);

		DCS_DEBUG_TRACE("END Process SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(evt);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ctx);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(app);

		DCS_DEBUG_TRACE("BEGIN Process SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ")");

//		bool sim_stop(true);
//
//		for (uint_type i = 0; i < num_quantiles(); ++i)
//		{
//			q_stats_[i](::boost::accumulators::extended_p_square(*ptr_acc_)[i]);
//			if (q_stats_[i].relative_precision() > 0.4)
//			{
//				sim_stop = false;
//			}
//		}
//
//		if (sim_stop)
//		{
//			::dcs::des::cloud::registry<traits_type>::instance().des_engine_ptr()->stop_now();
//		}


		DCS_DEBUG_TRACE("END Process SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(evt);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ctx);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(app);

		DCS_DEBUG_TRACE("BEGIN Process REQUEST-ARRIVAL (Clock: " << ctx.simulated_time() << ")");

//		++num_arrs_;

		DCS_DEBUG_TRACE("END Process REQUEST-ARRIVAL (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(evt);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ctx);

		DCS_DEBUG_TRACE("BEGIN Process REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");

		user_request_type req = app.simulation_model().request_state(evt);

		real_type rt(ctx.simulated_time()-req.arrival_time());

//		(*ptr_acc_)(rt);

//		++num_deps_;
//
//		if (num_deps_ == max_num_deps_)
//		{
//			::dcs::des::cloud::registry<traits_type>::instance().des_engine_ptr()->stop_now();
//		}

		DCS_DEBUG_TRACE("END Process REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_tier_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(evt);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(app);

		DCS_DEBUG_TRACE("BEGIN Process TIER-REQUEST-ARRIVAL (Clock: " << ctx.simulated_time() << ")");

		user_request_type req = app.simulation_model().request_state(evt);

		tier_req_info_map_[tier_id][req.id()].arrival_time = ctx.simulated_time();

		DCS_DEBUG_TRACE("END Process TIER-REQUEST-ARRIVAL (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_tier_request_service_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(evt);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ctx);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(tier_id);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(app);

		DCS_DEBUG_TRACE("BEGIN Process TIER-REQUEST-SERVICE (Clock: " << ctx.simulated_time() << ")");

		DCS_DEBUG_TRACE("END Process TIER-REQUEST-SERVICE (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_tier_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(evt);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ctx);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(app);

		DCS_DEBUG_TRACE("BEGIN Process TIER-REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");

		user_request_type req = app.simulation_model().request_state(evt);

		DCS_DEBUG_ASSERT( tier_req_info_map_.count(tier_id) );
		DCS_DEBUG_ASSERT( tier_req_info_map_[tier_id].count(req.id()) );

		real_type rt(ctx.simulated_time()-tier_req_info_map_[tier_id][req.id()].arrival_time);

//		(*(tier_acc_map_.at(tier_id)))(rt);

		tier_req_info_map_[tier_id].erase(req.id());

		DCS_DEBUG_TRACE("END Process TIER-REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");
	}


	//@} Event Handlers
*/


//	private: ::std::map<uint_type,accumulator_pointer> app_acc_map_;
//	private: ::std::map<uint_type,accumulator_pointer> tier_acc_map_;
//	private: uint_type max_num_deps_;
//	private: uint_type num_arrs_;
//	private: uint_type num_deps_;
//	private: tier_request_info_map tier_req_info_map_;
	private: application_container apps_;
	private: application_statistic_map app_stat_map_;
	private: tier_statistic_map tier_stat_map_;
}; // benchmark

template <typename TraitsT>
const typename TraitsT::real_type benchmark<TraitsT>::probs_[] = {0.25, 0.50, 0.75, 0.90, 0.95, 0.99, 0.995, 0.999, 0.9999, 0.99999};


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

}} // Namespace detail::<unnamed>


int main(int argc, char* argv[])
{
//	typedef dcs::shared_ptr< dcs::des::cloud::data_center<traits_type> > data_center_pointer;
//	typedef dcs::shared_ptr< dcs::des::cloud::data_center_manager<traits_type> > data_center_manager_pointer;
	typedef dcs::des::cloud::config::configuration<real_type,uint_type> configuration_type;
	typedef dcs::shared_ptr<configuration_type> configuration_pointer;
//	typedef configuration_type::data_center_config_type data_center_config_type;
//	typedef data_center_config_type::application_config_container::const_iterator app_iterator;
//	typedef dcs::des::cloud::physical_machine<traits_type> physical_machine_type;
//	typedef dcs::shared_ptr<physical_machine_type> physical_machine_pointer;
//	typedef dcs::des::cloud::physical_resource<traits_type> physical_resource_type;
//	typedef dcs::shared_ptr<physical_resource_type> physical_resource_pointer;
//	typedef application_type::reference_physical_resource_type reference_resource_type;


	prog_name = argv[0];

	if (argc < 2)
	{
		detail::usage();
		return -1;
	}


	// Parse command line arguments

//	uint_type num_samples;
	std::string conf_fname;

	try
	{
//		num_samples = detail::get_option<uint_type>(argv, argv+argc, "--ns");
		conf_fname = detail::get_option<std::string>(argv, argv+argc, "--conf");
	}
	catch (std::exception const& e)
	{
		std::cerr << "[Error] Error while parsing command-line options: " << e.what() << std::endl;
		std::abort();
	}

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
								dcs::des::cloud::config::yaml_reader<real_type,uint_type>()
							)
						);
				break;
			default:
				throw std::runtime_error("Unknown configuration category.");
		}
	}
    catch (std::exception const& e)
    {
		std::clog << "[Error] Unable to read configuration: " << e.what() << ::std::endl;
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
    des_engine_pointer ptr_des_eng;
    ptr_des_eng = dcs::des::cloud::config::make_des_engine(*ptr_conf);
    reg.des_engine(ptr_des_eng);
    random_generator_pointer ptr_rng;
    ptr_rng = dcs::des::cloud::config::make_random_number_generator(*ptr_conf);
    reg.uniform_random_generator(ptr_rng);

	random_seeder_type seeder(ptr_conf->rng().seed);
	ptr_des_eng->system_initialization_event_source().connect(
			dcs::functional::bind(
				&detail::process_sys_init_sim_event,
				dcs::functional::placeholders::_1,
				dcs::functional::placeholders::_2,
				seeder
			)
		);


    detail::benchmark<traits_type> bench;

	std::cerr.precision(16);
	std::cout.precision(16);

	bench.run(*ptr_conf);

	// Report statistics
	std::cout << "STATISTICS:" << std::endl;
	bench.report_stats(std::cout);
	std::cout << "--------------------------------------------------------------------------------" << std::endl;

//	app_iterator app_end_it = ptr_conf->data_center().applications().end();
//	for (app_iterator app_it = ptr_conf->data_center().applications().begin(); app_it != app_end_it; ++app_it)
//	{
//		dcs::shared_ptr<application_type> ptr_app;
//		benchmark<traits_type> bench;
//
//		// Build the simulator
//		des_engine_pointer ptr_des_eng;
//		ptr_des_eng = detail::make_des_engine<traits_type>();
//		reg.des_engine(ptr_des_eng);
//
//		// Build the random number generator
//		random_generator_pointer ptr_rng;
//		ptr_rng = dcs::des::cloud::config::make_random_number_generator(*ptr_conf);
//		reg.uniform_random_generator(ptr_rng);
//
//		// Build the application
//		ptr_app = dcs::des::cloud::config::make_multi_tier_application<traits_type>(*app_it, *ptr_conf, ptr_rng, ptr_des_eng);
//
//		bench.run(*ptr_app/*, num_samples*/);
//
//		//// Report statistics
//		//detail::report_stats(::std::cout, ptr_dc);
//	}
}
