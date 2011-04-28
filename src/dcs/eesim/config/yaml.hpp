#ifndef DCS_EESIM_CONFIG_YAML_HPP
#define DCS_EESIM_CONFIG_YAML_HPP


#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/eesim/config/application.hpp>
#include <dcs/eesim/config/application_controller.hpp>
#include <dcs/eesim/config/application_performance_model.hpp>
#include <dcs/eesim/config/application_simulation_model.hpp>
#include <dcs/eesim/config/application_sla.hpp>
#include <dcs/eesim/config/application_tier.hpp>
#include <dcs/eesim/config/configuration.hpp>
#include <dcs/eesim/config/data_center.hpp>
#include <dcs/eesim/config/initial_placement_strategy.hpp>
#include <dcs/eesim/config/logging.hpp>
#include <dcs/eesim/config/metric_category.hpp>
#include <dcs/eesim/config/migration_controller.hpp>
#include <dcs/eesim/config/numeric_matrix.hpp>
#include <dcs/eesim/config/numeric_multiarray.hpp>
#include <dcs/eesim/config/physical_machine.hpp>
#include <dcs/eesim/config/physical_machine_controller.hpp>
#include <dcs/eesim/config/physical_resource.hpp>
#include <dcs/eesim/config/probability_distribution.hpp>
#include <dcs/eesim/config/rng.hpp>
#include <dcs/eesim/config/simulation.hpp>
#include <dcs/math/constants.hpp>
#include <dcs/string/algorithm/to_lower.hpp>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>


namespace dcs { namespace eesim { namespace config {

namespace detail { namespace /*<unnamed>*/ {

logging_category text_to_logging_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("minimal"))
	{
		return minimal_logging;
	}
	if (!istr.compare("compact"))
	{
		return compact_logging;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_logging_category] Unknown logging category.");
}


logging_sink_category text_to_logging_sink_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("console"))
	{
		return console_logging_sink;
	}
	if (!istr.compare("file"))
	{
		return file_logging_sink;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_logging_sink_category] Unknown logging sink category.");
}


stream_logging_sink_category text_to_stream_logging_sink_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("stderr"))
	{
		return stderr_stream_logging_sink;
	}
	if (!istr.compare("stdlog"))
	{
		return stdlog_stream_logging_sink;
	}
	if (!istr.compare("stdout"))
	{
		return stdout_stream_logging_sink;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_stream_logging_sink_category] Unknown stream logging sink category.");
}


physical_resource_category text_to_physical_resource_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("cpu"))
	{
		return cpu_resource;
	}
	if (!istr.compare("mem"))
	{
		return mem_resource;
	}
	if (!istr.compare("disk"))
	{
		return disk_resource;
	}
	if (!istr.compare("nic"))
	{
		return nic_resource;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_physical_resource_category] Unknown physical resource category.");
}


metric_category text_to_metric_category(::std::string str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("response-time"))
	{
		return response_time_metric;
	}
	if (!istr.compare("throughput"))
	{
		return throughput_metric;
	}

	throw ::std::runtime_error("[dcs::eesim::config::metric_category] Unknown metric category.");
}


energy_model_category text_to_energy_model_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("constant"))
	{
		return constant_energy_model;
	}
	if (!istr.compare("fan-2007"))
	{
		return fan2007_energy_model;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_energy_model_category] Unknown energy model category.");
}


rng_engine_category text_to_rng_engine_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("minstd-rand0"))
	{
		return minstd_rand0_rng_engine;
	}
	if (!istr.compare("minstd-rand1"))
	{
		return minstd_rand1_rng_engine;
	}
	if (!istr.compare("minstd-rand2"))
	{
		return minstd_rand2_rng_engine;
	}
	if (!istr.compare("rand48"))
	{
		return rand48_rng_engine;
	}
	if (!istr.compare("mt11213b"))
	{
		return mt11213b_rng_engine;
	}
	if (!istr.compare("mt19937"))
	{
		return mt19937_rng_engine;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_rng_engine_category] Unknown random number generation engine category.");
}


rng_seeder_category text_to_rng_seeder_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("lcg"))
	{
		return lcg_rng_seeder;
	}
	if (!istr.compare("none"))
	{
		return none_rng_seeder;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_rng_seeder_category] Unknown random number generation seeder category.");
}


output_analysis_category text_to_output_analysis_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("independent-replications"))
	{
		return independent_replications_output_analysis;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_output_analysis_category] Unknown simulation output analysis category.");
}


statistic_category text_to_statistic_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("mean"))
	{
		return mean_statistic;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_statistic_category] Unknown simulation statistic category.");
}


probability_distribution_category text_to_probability_distribution_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("degenerate"))
	{
		return degenerate_probability_distribution;
	}
	if (!istr.compare("exponential"))
	{
		return exponential_probability_distribution;
	}
	if (!istr.compare("gamma"))
	{
		return gamma_probability_distribution;
	}
	if (!istr.compare("normal"))
	{
		return normal_probability_distribution;
	}
	if (!istr.compare("map"))
	{
		return map_probability_distribution;
	}
	if (!istr.compare("pmpp"))
	{
		return pmpp_probability_distribution;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_probability_distribution_category] Unknown probability distribution category.");
}


qn_node_category text_to_qn_node_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("delay"))
	{
		return qn_delay_node;
	}
	if (!istr.compare("queue"))
	{
		return qn_queue_node;
	}
	if (!istr.compare("source"))
	{
		return qn_source_node;
	}
	if (!istr.compare("sink"))
	{
		return qn_sink_node;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_qn_node_category] Unknown queueing network node category.");
}


qn_customer_class_category text_to_qn_customer_class_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("closed"))
	{
		return qn_closed_customer_class;
	}
	if (!istr.compare("open"))
	{
		return qn_open_customer_class;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_qn_customer_class_category Unknown queueing network customer class category.");
}


qn_routing_strategy_category text_to_qn_routing_strategy_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("probabilistic"))
	{
		return qn_probabilistic_routing_strategy;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_qn_routing_strategy_category Unknown queueing network routing strategy category.");
}


qn_service_strategy_category text_to_qn_service_strategy_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("infinite-server"))
	{
		return qn_infinite_server_service_strategy;
	}
	if (!istr.compare("load-independent"))
	{
		return qn_load_independent_service_strategy;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_qn_service_strategy_category Unknown queueing network service strategy category.");
}


initial_placement_strategy_category text_to_initial_placement_strategy_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("best-fit"))
	{
		return best_fit_initial_placement_strategy;
	}
	if (!istr.compare("first-fit"))
	{
		return first_fit_initial_placement_strategy;
	}
	if (!istr.compare("first-fit-scaleout"))
	{
		return first_fit_scaleout_initial_placement_strategy;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_initial_placement_strategy_category Unknown init VM placement strategy category.");
}


migration_controller_category text_to_migration_controller_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("lp"))
	{
		return lp_migration_controller;
	}
	if (!istr.compare("none") || !istr.compare("dummy"))
	{
		return dummy_migration_controller;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_migration_controller_category Unknown migration controller category.");
}


application_controller_category text_to_application_controller_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("lqi"))
	{
		return lqi_application_controller;
	}
//	if (!istr.compare("lqiy"))
//	{
//		return lqiy_application_controller;
//	}
	if (!istr.compare("lqr"))
	{
		return lqr_application_controller;
	}
	if (!istr.compare("lqry"))
	{
		return lqry_application_controller;
	}
	if (!istr.compare("matlab-lqi"))
	{
		return matlab_lqi_application_controller;
	}
	if (!istr.compare("matlab-lqr"))
	{
		return matlab_lqr_application_controller;
	}
	if (!istr.compare("matlab-lqry"))
	{
		return matlab_lqry_application_controller;
	}
	if (!istr.compare("none") || !istr.compare("dummy"))
	{
		return dummy_application_controller;
	}
	if (!istr.compare("qn"))
	{
		return qn_application_controller;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_application_controller_category Unknown application controller category.");
}


physical_machine_controller_category text_to_physical_machine_controller_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("conservative"))
	{
		return conservative_physical_machine_controller;
	}
	if (!istr.compare("proportional"))
	{
		return proportional_physical_machine_controller;
	}
	if (!istr.compare("none") || !istr.compare("dummy"))
	{
		return dummy_physical_machine_controller;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_physical_machine_controller_category Unknown physical machine controller category.");
}


map_probability_distribution_characterization_category text_to_map_characterization_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("standard"))
	{
		return standard_map_characterization;
	}
	if (!istr.compare("casale-2009"))
	{
		return casale2009_map_characterization;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_map_characterization_category Unknown MAP characterization category.");
}


sla_model_category text_to_sla_model_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("step"))
	{
		return step_sla_model;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_sla_model_category Unknown SLA model category.");
}


num_replications_detector_category text_to_num_replications_detector_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("constant"))
	{
		return constant_num_replications_detector;
	}
	if (!istr.compare("banks2005"))
	{
		return banks2005_num_replications_detector;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_num_replications_detector_category Unknown number of replications detector category.");
}


replication_size_detector_category text_to_replication_size_detector_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("fixed-duration"))
	{
		return fixed_duration_replication_size_detector;
	}
	if (!istr.compare("fixed-num-observations"))
	{
		return fixed_num_obs_replication_size_detector;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_replication_size_detector_category Unknown replication size detector category.");
}


system_identification_category text_to_system_identification_category(::std::string const& str)
{
	::std::string istr = ::dcs::string::to_lower_copy(str);

	if (!istr.compare("rls-bittanti1990"))
	{
		return rls_bittanti1990_system_identification;
	}
	if (!istr.compare("rls-ff"))
	{
		return rls_ff_system_identification;
	}
	if (!istr.compare("rls-kulhavy1984"))
	{
		return rls_kulhavy1984_system_identification;
	}
	if (!istr.compare("rls-park1991"))
	{
		return rls_park1991_system_identification;
	}
//	if (!istr.compare("none") || !istr.compare("dummy"))
//	{
//		return dummy_system_identification;
//	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_system_identification_category Unknown system identification category.");
}


}} // Namespace detail::<unnamed>


template <typename T>
void operator>>(::YAML::Node const& node, numeric_matrix<T>& m)
{
	::std::size_t nr;
	::std::size_t nc;
	::std::vector<T> data;
	bool byrow;
	node["rows"] >> nr;
	node["cols"] >> nc;
	node["byrow"] >> byrow;
	node["data"] >> data;
	m = numeric_matrix<T>(nr, nc, data.begin(), data.end(), byrow);
}


template <typename T>
void operator>>(::YAML::Node const& node, numeric_multiarray<T>& a)
{
	typedef ::std::size_t size_type;

	::std::vector<size_type> dims;
	::std::vector<size_type> bydims;
	::std::vector<T> data;

	node["dims"] >> dims;
	if (node.FindValue("bydims"))
	{
		node["bydims"] >> bydims;
	}
	else
	{
		bydims = ::std::vector<size_type>(dims.size());
		for (size_type i = 0; i < bydims.size(); ++i)
		{
			bydims[i] = i;
		}
	}
	node["data"] >> data;
	a = numeric_multiarray<T>(dims.begin(), dims.end(), data.begin(), data.end(), bydims.begin(), bydims.end());
}


void operator>>(::YAML::Node const& node, logging_sink_config& sink_conf)
{
	typedef logging_sink_config sink_config_type;

	::std::string label;

	node["type"] >> label;
	sink_conf.category = detail::text_to_logging_sink_category(label);
	switch (sink_conf.category)
	{
		case console_logging_sink:
			{
				typedef sink_config_type::console_logging_sink_type sink_config_impl_type;

				sink_config_impl_type sink_conf_impl;

				if (node.FindValue("stream"))
				{
					node["stream"] >> label;
					sink_conf_impl.stream = detail::text_to_stream_logging_sink_category(label);
				}
				else
				{
					sink_conf_impl.stream = stderr_stream_logging_sink;
				}

				sink_conf.category_conf = sink_conf_impl;
			}
			break;
		case file_logging_sink:
			{
				typedef sink_config_type::file_logging_sink_type sink_config_impl_type;

				sink_config_impl_type sink_conf_impl;

				node["name"] >> sink_conf_impl.name;

				sink_conf.category_conf = sink_conf_impl;
			}
			break;
	}
}


void operator>>(::YAML::Node const& node, logging_config& logging_conf)
{
	typedef logging_config logging_config_type;

	::std::string label;

	node["enabled"] >> logging_conf.enabled;
	node["type"] >> label;
	logging_conf.category = detail::text_to_logging_category(label);
	switch (logging_conf.category)
	{
		case minimal_logging:
			{
				typedef logging_config_type::minimal_logging_type logging_config_impl_type;

				logging_config_impl_type logging_conf_impl;

				node["sink"] >> logging_conf_impl.sink;

				logging_conf.category_conf = logging_conf_impl;
			}
			break;
		case compact_logging:
			{
				typedef logging_config_type::compact_logging_type logging_config_impl_type;

				logging_config_impl_type logging_conf_impl;

				node["sink"] >> logging_conf_impl.sink;

				logging_conf.category_conf = logging_conf_impl;
			}
			break;
	}
}


template <typename UIntT>
void operator>>(::YAML::Node const& node, rng_config<UIntT>& rng)
{
	::std::string label;

	node["engine"] >> label;
	rng.engine = detail::text_to_rng_engine_category(label);
	node["seed"] >> rng.seed;
	if (node.FindValue("seeder"))
	{
		node["seeder"] >> label;
		rng.seeder = detail::text_to_rng_seeder_category(label);
	}
	else
	{
		rng.seeder = none_rng_seeder;
	}
}


template <typename RealT>
void operator>>(::YAML::Node const& node, statistic_config<RealT>& stat)
{
	// Read category
	if (node.FindValue("type"))
	{
		::std::string label;
		node["type"] >> label;
		stat.category = detail::text_to_statistic_category(label);
	}
	else
	{
		stat.category = mean_statistic;
	}
	// Read precision
	if (node.FindValue("precision"))
	{
		node["precision"] >> stat.precision;
	}
	else
	{
		stat.precision = ::std::numeric_limits<RealT>::infinity();
	}
	// Read confidence level
	if (node.FindValue("confidence-level"))
	{
		node["confidence-level"] >> stat.confidence_level;
	}
	else
	{
		stat.confidence_level = 0.95;
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, independent_replications_output_analysis_config<RealT,UIntT>& analysis)
{
/*old
	// Read number of replications
	if (node.FindValue("num-replications"))
	{
		node["num-replications"] >> analysis.num_replications;
	}
	else
	{
		analysis.num_replications = 1;
	}
	// Read length of each replication
	if (node.FindValue("replication-duration"))
	{
		node["replication-duration"] >> analysis.replication_duration;
	}
	else
	{
		analysis.replication_duration = 0;
	}
	// Read length of each replication
	if (node.FindValue("num-observations"))
	{
		node["num-observations"] >> analysis.num_observations;
	}
	else
	{
		analysis.num_observations = 0;
	}
*/

	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef independent_replications_output_analysis_config<real_type,uint_type> output_analysis_config_type;

	// Read Number of Replications
	{
		::YAML::Node const& subnode = node["num-replications"];
		::std::string label;

		subnode["type"] >> label;
		analysis.num_replications_category = detail::text_to_num_replications_detector_category(label);

		switch (analysis.num_replications_category)
		{
			case constant_num_replications_detector:
				{
					typedef typename output_analysis_config_type::constant_num_replications_detector_type num_replications_detector_type;

					num_replications_detector_type num_replications_detector;

					if (subnode.FindValue("value"))
					{
						subnode["value"] >> num_replications_detector.num_replications;
					}
					else
					{
						// See Banks et al "Discrete-Event System Simulation" (2005) for the
						// reason of why of choosing 25 as the number of replications
						num_replications_detector.num_replications = 25;
					}

					analysis.num_replications_category_conf = num_replications_detector;
				}
				break;
			case banks2005_num_replications_detector:
				{
					typedef typename output_analysis_config_type::banks2005_num_replications_detector_type num_replications_detector_type;

					num_replications_detector_type num_replications_detector;

//					if (subnode.FindValue("relative-precision"))
//					{
//						subnode["relative-precision"] >> num_replications_detector.relative_precision;
//					}
//					else
//					{
//						// Default to 4%
//						num_replications_detector.relative_precision = 0.04;
//					}
//					if (subnode.FindValue("confidence-level"))
//					{
//						subnode["confidence-level"] >> num_replications_detector.confidence_level;
//					}
//					else
//					{
//						// Default to 95%
//						num_replications_detector.confidence_level = 0.95;
//					}
					if (subnode.FindValue("min-value"))
					{
						subnode["min-value"] >> num_replications_detector.min_num_replications;
					}
					else
					{
						num_replications_detector.min_num_replications = 2;
					}
					if (subnode.FindValue("max-value"))
					{
						subnode["max-value"] >> num_replications_detector.max_num_replications;
					}
					else
					{
						num_replications_detector.max_num_replications = ::dcs::math::constants::infinity<uint_type>::value;
					}

					analysis.num_replications_category_conf = num_replications_detector;
				}
				break;
		}
	}
	// Read Number of Replications
	{
		::YAML::Node const& subnode = node["replication-size"];
		::std::string label;

		subnode["type"] >> label;
		analysis.replication_size_category = detail::text_to_replication_size_detector_category(label);

		switch (analysis.replication_size_category)
		{
			case fixed_duration_replication_size_detector:
				{
					typedef typename output_analysis_config_type::fixed_duration_replication_size_detector_type replication_size_detector_type;

					replication_size_detector_type replication_size_detector;

					if (subnode.FindValue("duration"))
					{
						subnode["duration"] >> replication_size_detector.replication_duration;
					}
					else
					{
						replication_size_detector.replication_duration = 1000;
					}

					analysis.replication_size_category_conf = replication_size_detector;
				}
				break;
			case fixed_num_obs_replication_size_detector:
				{
					typedef typename output_analysis_config_type::fixed_num_obs_replication_size_detector_type replication_size_detector_type;

					replication_size_detector_type replication_size_detector;

					if (subnode.FindValue("num-observations"))
					{
						subnode["num-observations"] >> replication_size_detector.num_observations;
					}
					else
					{
						replication_size_detector.num_observations = ::dcs::math::constants::infinity<uint_type>::value;
					}

					analysis.replication_size_category_conf = replication_size_detector;
				}
				break;
		}
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, simulation_config<RealT,UIntT>& sim)
{
	typedef simulation_config<RealT,UIntT> simulation_config_type;

	// Read Output Analysis
	{
		typedef typename simulation_config_type::output_analysis_config_type output_analysis_config_type;

		output_analysis_config_type output_analysis_conf;

		::YAML::Node const& subnode = node["output-analysis"];

		::std::string label;

		// Read Output Analysis method
		subnode["type"] >> label;
		output_analysis_conf.category = detail::text_to_output_analysis_category(label);
		switch (output_analysis_conf.category)
		{
			case independent_replications_output_analysis:
				{
					independent_replications_output_analysis_config<RealT,UIntT> conf;
					subnode >> conf;

					output_analysis_conf.category_conf = conf;
				}
				break;
		}

		// Read Confidence Level
		if (subnode.FindValue("confidence-level"))
		{
			subnode["confidence-level"] >> output_analysis_conf.confidence_level;
		}
		else
		{
			// Default to 95%
			output_analysis_conf.confidence_level = 0.95;
		}

		// Read Relative Precision
		if (subnode.FindValue("relative-precision"))
		{
			subnode["relative-precision"] >> output_analysis_conf.relative_precision;
		}
		else
		{
			//// Default to 4%
			//output_analysis_conf.relative_precision = 0.04;
			output_analysis_conf.relative_precision = ::std::numeric_limits<RealT>::infinity();
		}

		sim.output_analysis = output_analysis_conf;
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, application_performance_model_config<RealT,UIntT>& model)
{
	application_performance_model_category type;
	::std::string type_lbl;
	node["type"] >> type_lbl;
	if (!type_lbl.compare("open-multi-bcmp-qn"))
	{
		type = open_multi_bcmp_qn_model;
	}
	else
	{
		throw ::std::runtime_error("[dcs::eesim::config::>>] Unknown application performance model.");
	}

	model.type = type;

	switch (type)
	{
		case open_multi_bcmp_qn_model:
			{
				typename application_performance_model_config<RealT,UIntT>::open_multi_bcmp_qn_config conf;
				node["arrival-rates"] >> conf.arrival_rates;
				if (node.FindValue("visit-ratios"))
				{
					node["visit-ratios"] >> conf.visit_ratios;
				}
				else if (node.FindValue("routing-probabilities"))
				{
					node["routing-probabilities"] >> conf.routing_probabilities;
				}
				else
				{
					throw ::std::runtime_error("[dcs::eesim::config::>>] Missing both visit ratios and routing probabilities.");
				}
				node["service-times"] >> conf.service_times;
				node["num-servers"] >> conf.num_servers;

				model.type_conf = conf;
			}
			break;
	}
}


template <typename RealT>
void operator>>(::YAML::Node const& node, probability_distribution_config<RealT>& distr_conf)
{
	typedef probability_distribution_config<RealT> distribution_config_type;

	::std::string label;

	node["type"] >> label;
	distr_conf.category = detail::text_to_probability_distribution_category(label);

	switch (distr_conf.category)
	{
		case degenerate_probability_distribution:
			{
				typedef typename distribution_config_type::degenerate_distribution_config_type distribution_config_impl_type;

				distribution_config_impl_type distr_conf_impl;

				node["k"] >> distr_conf_impl.k;

				distr_conf.category_conf = distr_conf_impl;
			}
			break;
		case exponential_probability_distribution:
			{
				typedef typename distribution_config_type::exponential_distribution_config_type distribution_config_impl_type;

				distribution_config_impl_type distr_conf_impl;

				node["rate"] >> distr_conf_impl.rate;

				distr_conf.category_conf = distr_conf_impl;
			}
			break;
		case gamma_probability_distribution:
			{
				typedef typename distribution_config_type::gamma_distribution_config_type distribution_config_impl_type;

				distribution_config_impl_type distr_conf_impl;

				node["scale"] >> distr_conf_impl.scale;
				node["shape"] >> distr_conf_impl.shape;

				distr_conf.category_conf = distr_conf_impl;
			}
			break;
		case map_probability_distribution:
			{
				typedef typename distribution_config_type::map_distribution_config_type distribution_config_impl_type;

				distribution_config_impl_type distr_conf_impl;

				if (node.FindValue("characterization"))
				{
					node["characterization"] >> label;
				}
				else
				{
					label = "standard";
				}
				distr_conf_impl.characterization_category = detail::text_to_map_characterization_category(label);

				switch (distr_conf_impl.characterization_category)
				{
					case standard_map_characterization:
						{
							typedef typename distribution_config_impl_type::standard_characterization_config_type characterization_type;

							characterization_type characterization;

							node["D0"] >> characterization.D0;
							node["D1"] >> characterization.D1;

							distr_conf_impl.characterization_conf = characterization;
						}
						break;
					case casale2009_map_characterization:
						{
							typedef typename distribution_config_impl_type::casale2009_characterization_config_type characterization_type;

							characterization_type characterization;

							node["order"] >> characterization.order;
							node["m"] >> characterization.mean;
							node["id"] >> characterization.id;

							distr_conf_impl.characterization_conf = characterization;
						}
						break;
				}

				distr_conf.category_conf = distr_conf_impl;
			}
			break;
		case normal_probability_distribution:
			{
				typedef typename distribution_config_type::normal_distribution_config_type distribution_config_impl_type;

				distribution_config_impl_type distr_conf_impl;

				node["mean"] >> distr_conf_impl.mean;
				node["sd"] >> distr_conf_impl.sd;

				distr_conf.category_conf = distr_conf_impl;
			}
			break;
		case pmpp_probability_distribution:
			{
				typedef typename distribution_config_type::pmpp_distribution_config_type distribution_config_impl_type;

				distribution_config_impl_type distr_conf_impl;

				node["rates"] >> distr_conf_impl.rates;
				node["shape"] >> distr_conf_impl.shape;
				node["min"] >> distr_conf_impl.min;

				distr_conf.category_conf = distr_conf_impl;
			}
			break;
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, qn_node_config<RealT,UIntT>& node_conf)
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef qn_node_config<RealT,UIntT> node_config_type;

	::std::string label;

	if (node.FindValue("name"))
	{
		node["name"] >> node_conf.name;
	}
//	if (node.FindValue("id"))
//	{
//		node["id"] >> node_conf.id;
//	}
//	else
//	{
//		node_conf.id = i;
//	}
	node["type"] >> label;
	node_conf.category = detail::text_to_qn_node_category(label);

	switch (node_conf.category)
	{
		case qn_delay_node:
			{
				typedef typename node_config_type::delay_node_config_type node_impl_config_type;

				node_impl_config_type node_impl_conf;

				// Read routing strategy
				{
					::YAML::Node const& routing_node = node["routing-strategy"];
					routing_node["type"] >> label;
					node_impl_conf.routing_category = detail::text_to_qn_routing_strategy_category(label);
					switch (node_impl_conf.routing_category)
					{
						case qn_probabilistic_routing_strategy:
							{
								typedef typename node_impl_config_type::probabilistic_routing_strategy_config_type routing_config_impl_type;

								routing_config_impl_type routing_impl_conf;

								routing_node["probabilities"] >> routing_impl_conf.probabilities;

								node_impl_conf.routing_conf = routing_impl_conf;
							}
							break;
					}
				}
				// Read service strategy
				{
					::YAML::Node const& service_node = node["service-strategy"];
					qn_service_strategy_category service_category;
					if (service_node.FindValue("type"))
					{
						service_node["type"] >> label;
						service_category = detail::text_to_qn_service_strategy_category(label);
					}
					else
					{
						service_category = qn_infinite_server_service_strategy;
					}
					switch (service_category)
					{
						case qn_infinite_server_service_strategy:
							{
								typedef typename node_impl_config_type::probability_distribution_config_type distribution_config_type;

								::YAML::Node const& distributions_node = service_node["distributions"];
								for (::std::size_t i = 0; i < distributions_node.size(); ++i)
								{
									::YAML::Node const& distribution_node = distributions_node[i];
									distribution_config_type distribution_conf;

									distribution_node["distribution"] >> distribution_conf;

									node_impl_conf.distributions.push_back(distribution_conf);
								}
							}
							break;
						default:
							throw ::std::runtime_error("[dcs::eesim::config::>>] The service strategy of a delay node can only be 'infinite-server'.");
					}
				}

				node_conf.category = qn_delay_node;
				node_conf.category_conf = node_impl_conf;
			}
			break;
		case qn_queue_node:
			{
				typedef typename node_config_type::queue_node_config_type node_impl_config_type;

				node_impl_config_type node_impl_conf;

				if (node.FindValue("num-servers"))
				{
					node["num-servers"] >> node_impl_conf.num_servers;
				}
				else
				{
					node_impl_conf.num_servers = 1;
				}
				if (node.FindValue("policy"))
				{
					node["policy"] >> label;
					if (!label.compare("fcfs"))
					{
						node_impl_conf.policy_category = qn_fcfs_scheduling_policy;
					}
					else
					{
						throw ::std::runtime_error("[dcs::eesim::config::>>] Unknown queueing network scheduling policy.");
					}
				}
				else
				{
					node_impl_conf.policy_category = qn_fcfs_scheduling_policy;
				}
				if (node.FindValue("capacity"))
				{
					node["capacity"] >> node_impl_conf.capacity;
					node_impl_conf.is_infinite = false;
				}
				else
				{
					node_impl_conf.is_infinite = true;
					node_impl_conf.capacity = 0;
				}
//				::YAML::Node const& routing_node = node["routing-strategy"];
//				routing_node["type"] >> label;
//				if (!label.compare("probabilistic"))
//				{
//					typedef typename node_impl_config_type::probabilistic_routing_strategy_config_type routing_config_impl_type;
//
//					routing_config_impl_type routing_impl_conf;
//
//					routing_node["probabilities"] >> routing_impl_conf.probabilities;
//
//					node_impl_conf.routing_category = qn_probabilistic_routing_strategy;
//					node_impl_conf.routing_conf = routing_impl_conf;
//				}
//				else
//				{
//					throw ::std::runtime_error("[dcs::eesim::config::>>] Unknown queueing network routing strategy.");
//				}
				// Read routing strategy
				{
					::YAML::Node const& routing_node = node["routing-strategy"];
					routing_node["type"] >> label;
					node_impl_conf.routing_category = detail::text_to_qn_routing_strategy_category(label);
					switch (node_impl_conf.routing_category)
					{
						case qn_probabilistic_routing_strategy:
							{
								typedef typename node_impl_config_type::probabilistic_routing_strategy_config_type routing_config_impl_type;

								routing_config_impl_type routing_impl_conf;

								routing_node["probabilities"] >> routing_impl_conf.probabilities;

								node_impl_conf.routing_conf = routing_impl_conf;
							}
							break;
					}
				}
//				::YAML::Node const& service_node = node["service-strategy"];
//				service_node["type"] >> label;
//				if (!label.compare("load-independent"))
//				{
//					typedef typename node_impl_config_type::load_independent_service_strategy_config_type service_config_impl_type;
//					typedef typename service_config_impl_type::probability_distribution_config_type distribution_config_type;
//
//					service_config_impl_type service_impl_conf;
//
//					::YAML::Node const& distributions_node = service_node["distributions"];
//					for (::std::size_t i = 0; i < distributions_node.size(); ++i)
//					{
//						::YAML::Node const& distribution_node = distributions_node[i];
//						distribution_config_type distribution_conf;
//
//						distribution_node["distribution"] >> distribution_conf;
//
//						service_impl_conf.distributions.push_back(distribution_conf);
//					}
//
//					node_impl_conf.service_category = qn_load_independent_service_strategy;
//					node_impl_conf.service_conf = service_impl_conf;
//				}
//				else
//				{
//					throw ::std::runtime_error("[dcs::eesim::config::>>] Unknown queueing network routing strategy.");
//				}
				// Read service strategy
				{
					::YAML::Node const& service_node = node["service-strategy"];
					if (service_node.FindValue("type"))
					{
						service_node["type"] >> label;
						node_impl_conf.service_category = detail::text_to_qn_service_strategy_category(label);
					}
					else
					{
						node_impl_conf.service_category = qn_load_independent_service_strategy;
					}

					switch (node_impl_conf.service_category)
					{
						case qn_infinite_server_service_strategy:
							throw ::std::runtime_error("[dcs::eesim::config::>>] The service strategy of a queue node cannot be 'infinite-server'.");
						case qn_load_independent_service_strategy:
							{
								typedef typename node_impl_config_type::load_independent_service_strategy_config_type service_config_impl_type;
								typedef typename service_config_impl_type::probability_distribution_config_type distribution_config_type;

								service_config_impl_type service_impl_conf;

								::YAML::Node const& distributions_node = service_node["distributions"];
								for (::std::size_t i = 0; i < distributions_node.size(); ++i)
								{
									::YAML::Node const& distribution_node = distributions_node[i];
									distribution_config_type distribution_conf;

									distribution_node["distribution"] >> distribution_conf;

									service_impl_conf.distributions.push_back(distribution_conf);
								}

								node_impl_conf.service_conf = service_impl_conf;
							}
							break;
					}
				}

				node_conf.category = qn_queue_node;
				node_conf.category_conf = node_impl_conf;
			}
			break;
		case qn_sink_node:
			{
				typedef typename node_config_type::sink_node_config_type node_impl_config_type;

				node_impl_config_type node_impl_conf;

				node_conf.category = qn_sink_node;

				node_conf.category_conf = node_impl_conf;
			}
			break;
		case qn_source_node:
			{
				typedef typename node_config_type::source_node_config_type node_impl_config_type;

				node_impl_config_type node_impl_conf;

//				::YAML::Node const& routing_node = node["routing-strategy"];
//				routing_node["type"] >> label;
//				if (!label.compare("probabilistic"))
//				{
//					typedef typename node_impl_config_type::probabilistic_routing_strategy_config_type routing_config_impl_type;
//
//					routing_config_impl_type routing_impl_conf;
//
//					routing_node["probabilities"] >> routing_impl_conf.probabilities;
//
//					node_impl_conf.routing_category = qn_probabilistic_routing_strategy;
//					node_impl_conf.routing_conf = routing_impl_conf;
//				}
//				else
//				{
//					throw ::std::runtime_error("[dcs::eesim::config::>>] Unknown queueing network routing strategy.");
//				}
				// Read routing strategy
				{
					::YAML::Node const& routing_node = node["routing-strategy"];
					routing_node["type"] >> label;
					node_impl_conf.routing_category = detail::text_to_qn_routing_strategy_category(label);
					switch (node_impl_conf.routing_category)
					{
						case qn_probabilistic_routing_strategy:
							{
								typedef typename node_impl_config_type::probabilistic_routing_strategy_config_type routing_config_impl_type;

								routing_config_impl_type routing_impl_conf;

								routing_node["probabilities"] >> routing_impl_conf.probabilities;

								node_impl_conf.routing_conf = routing_impl_conf;
							}
							break;
					}
				}

				node_conf.category = qn_source_node;
				node_conf.category_conf = node_impl_conf;
			}
			break;
	}

	if (node.FindValue("reference-tier"))
	{
		::YAML::Node const& ref_node = node["reference-tier"];
		ref_node["name"] >> node_conf.ref_tier;
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, qn_customer_class_config<RealT,UIntT>& customer_class_conf)
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef qn_customer_class_config<RealT,UIntT> customer_class_config_type;

	::std::string label;

	if (node.FindValue("name"))
	{
		node["name"] >> customer_class_conf.name;
	}

	node["type"] >> label;
	customer_class_conf.category = detail::text_to_qn_customer_class_category(label);

//	node["reference-node"] >> node_id;
	::YAML::Node const& ref_node = node["reference-node"];
	if (customer_class_conf.category == qn_open_customer_class)
	{
		ref_node["type"] >> label;

		qn_node_category node_cat = detail::text_to_qn_node_category(label);

		if (node_cat != qn_source_node)
		{
			throw ::std::runtime_error("[dcs::eesim::config::>>] Reference nodes of an open customer class can only be source nodes.");
		}
	}
	else
	{
		ref_node["type"] >> label;

		qn_node_category node_cat = detail::text_to_qn_node_category(label);

		if (node_cat != qn_queue_node && node_cat != qn_delay_node)
		{
			throw ::std::runtime_error("[dcs::eesim::config::>>] Reference nodes of an closed customer class can only be queueing or delay nodes.");
		}
	}
	ref_node["name"] >> customer_class_conf.ref_node;

	switch (customer_class_conf.category)
	{
		case qn_open_customer_class:
			{
				typedef typename customer_class_config_type::open_class_config_type customer_class_config_impl_type;

				customer_class_config_impl_type customer_class_conf_impl;

				node["distribution"] >> customer_class_conf_impl.distribution;

				customer_class_conf.category_conf = customer_class_conf_impl;
			}
			break;
		case qn_closed_customer_class:
			{
				typedef typename customer_class_config_type::closed_class_config_type customer_class_config_impl_type;

				customer_class_config_impl_type customer_class_conf_impl;

				node["size"] >> customer_class_conf_impl.size;

				customer_class_conf.category_conf = customer_class_conf_impl;
			}
			break;
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, application_simulation_model_config<RealT,UIntT>& model)
{
	typedef application_simulation_model_config<RealT,UIntT> sim_model_type;
	typedef RealT real_type;
	typedef UIntT uint_type;

	application_simulation_model_category model_category;
	::std::string label;
	node["type"] >> label;
	if (!label.compare("qn"))
	{
		model_category = qn_model;
	}
	else
	{
		throw ::std::runtime_error("[dcs::eesim::config::>>] Unknown application simulation model.");
	}

	model.category = model_category;

	switch (model_category)
	{
		case qn_model:
			{
				typedef typename sim_model_type::qn_model_config_type qn_config_type;
				qn_config_type conf;

//TODO: allow the user to specify a unique (global) routing probability matrix
//				// Read (optional) global routing probability table
//				if (node.FindValue("routing-strategy"))
//				{
//					::YAML::Node const& subnode = node["routing-strategy"];
//
//					subnode["type"] >> label;
//
//					conf.routing_category = detail::text_to_routing_strategy_category(label);
//
//					switch (conf.routing_category)
//					{
//						case qn_probabilistic_routing_strategy:
//							{
//								typedef typename qn_config_type::probabilistic_routing_strategy_confg_type routing_config_impl_type;
//
//								routing_config_impl_type routing_conf_impl;
//
//								numeric_multiarray<real_type> a;
//								subnode["probabilities"] >> routing_conf_impl.probabilities;
//							}
//							break;
//					}
//				}

				// Read nodes
				{
					typedef typename qn_config_type::node_config_type node_config_type;

					::YAML::Node const& subnode = node["nodes"];
					for (::std::size_t i = 0; i < subnode.size(); ++i)
					{
						node_config_type node_conf;

						::YAML::Node const& node_node = subnode[i]["node"];

						node_node >> node_conf;

						node_conf.id = i;

						conf.nodes.push_back(node_conf);
					}
				}

				// Read customer classes
				{
					typedef typename qn_config_type::customer_class_config_type customer_class_config_type;

					::YAML::Node const& subnode = node["customer-classes"];
					for (::std::size_t i = 0; i < subnode.size(); ++i)
					{
						customer_class_config_type customer_class_conf;

						::YAML::Node const& customer_class_node = subnode[i]["customer-class"];

						customer_class_node >> customer_class_conf;

						customer_class_conf.id = i;

						conf.customer_classes.push_back(customer_class_conf);
					}
				}

				model.category_conf = conf;
			}
			break;
	}

//	// Read statistics
//	{
//		typedef statistic_config<real_type> statistic_config_type;
//
//		::YAML::Node const& subnode = node["statistics"];
//		for (::YAML::Iterator it = subnode.begin(); it != subnode.end(); ++it)
//		{
//			::YAML::Node const& key_node = it.first();
//			::YAML::Node const& value_node = it.second();
//
//			::std::string label;
//			metric_category category;
//			statistic_config_type stat_conf;
//
//			key_node >> label;
//			category = detail::text_to_metric_category(label);
//
//			value_node >> stat_conf;
//
//			model.statistics[category] = stat_conf;
//		}
//	}
}


template <typename RealT>
void operator>>(::YAML::Node const& node, sla_metric_config<RealT>& metric)
{
	node["value"] >> metric.value;
	if (node.FindValue("tolerance"))
	{
		node["tolerance"] >> metric.tolerance;
	}
	else
	{
		metric.tolerance = 0;
	}
}


template <typename RealT>
void operator>>(::YAML::Node const& node, application_sla_config<RealT>& sla_conf)
{
	// Read cost model
	{
		::YAML::Node const& subnode = node["cost-model"];
		::std::string label;
		subnode["type"] >> label;
		sla_conf.category = detail::text_to_sla_model_category(label);
		switch (sla_conf.category)
		{
			case step_sla_model:
				{
					typedef step_sla_model_config<RealT> model_config_impl_type;

					model_config_impl_type model_conf_impl;

					subnode["penalty"] >> model_conf_impl.penalty;
					subnode["revenue"] >> model_conf_impl.revenue;

					sla_conf.category_conf = model_conf_impl;
				}
				break;
		}
	}

	// Read performance metrics
	{
		::YAML::Node const& subnode = node["performance-metrics"];
		for(::YAML::Iterator it = subnode.begin(); it != subnode.end(); ++it)
		{
			::YAML::Node const& key_node = it.first();
			::YAML::Node const& value_node = it.second();

			::std::string label;
			metric_category category;
			key_node >> label;
			category = detail::text_to_metric_category(label);

			sla_metric_config<RealT> metric_conf;
			value_node >> metric_conf;
			sla_conf.metrics[category] = metric_conf;
		}
	}
}


template <typename RealT>
void operator>>(::YAML::Node const& node, application_tier_config<RealT>& tier)
{
	if (node.FindValue("name"))
	{
		node["name"] >> tier.name;
	}
	if (node.FindValue("shares"))
	{
		::YAML::Node const& subnode = node["shares"];
		for (::YAML::Iterator it = subnode.begin(); it != subnode.end(); ++it)
		{
			::YAML::Node const& key_node = it.first();
			::YAML::Node const& value_node = it.second();

			physical_resource_category category;
			RealT share;
			::std::string resource;

			key_node >> resource;
			value_node >> share;

			category = detail::text_to_physical_resource_category(resource);

			tier.shares[category] = share;
		}
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, base_rls_system_identification_config<RealT,UIntT>& ident_conf)
{
	typedef RealT real_type;
	typedef UIntT uint_type;

	if (node.FindValue("miso"))
	{
		node["miso"] >> ident_conf.mimo_as_miso;
	}
	else
	{
		ident_conf.mimo_as_miso = false;
	}
//	if (node.FindValue("ewma-smoothing-filter"))
//	{
//		node["ewma-smoothing-filter"] >> ident_conf.enable_ewma_smoothing_filter;
//		if (ident_conf.enable_ewma_smoothing_filter)
//		{
//			node["ewma-smoothing-factor"] >> ident_conf.ewma_smoothing_factor;
//		}
//	}
//	else
//	{
//		ident_conf.enable_ewma_smoothing_filter = false;
//	}
	if (node.FindValue("max-covariance-heuristic"))
	{
		node["max-covariance-heuristic"] >> ident_conf.enable_max_cov_heuristic;
		if (ident_conf.enable_max_cov_heuristic)
		{
			node["max-covariance-heuristic-max-value"] >> ident_conf.max_cov_heuristic_value;
		}
	}
	else
	{
		ident_conf.enable_max_cov_heuristic = false;
	}
	if (node.FindValue("cond-covariance-heuristic"))
	{
		node["cond-covariance-heuristic"] >> ident_conf.enable_cond_cov_heuristic;
		if (ident_conf.enable_cond_cov_heuristic)
		{
			node["cond-covariance-heuristic-trusted-digits"] >> ident_conf.cond_cov_heuristic_trust_digits;
		}
	}
	else
	{
		ident_conf.enable_cond_cov_heuristic = false;
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, rls_bittanti1990_system_identification_config<RealT,UIntT>& ident_conf)
{
	typedef RealT real_type;
	typedef UIntT uint_type;

	// Read common properties
	base_rls_system_identification_config<real_type,uint_type>* ptr_base_ident_conf = &ident_conf;
	node >> *ptr_base_ident_conf;

	// Read specialized properties
	if (node.FindValue("forgetting-factor"))
	{
		node["forgetting-factor"] >> ident_conf.forgetting_factor;
	}
	else
	{
		ident_conf.forgetting_factor = 1.0;
	}
	if (node.FindValue("correction-factor"))
	{
		node["correction-factor"] >> ident_conf.delta;
	}
	else
	{
		ident_conf.delta = 0.001;
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, rls_ff_system_identification_config<RealT,UIntT>& ident_conf)
{
	typedef RealT real_type;
	typedef UIntT uint_type;

	// Read common properties
	base_rls_system_identification_config<real_type,uint_type>* ptr_base_ident_conf = &ident_conf;
	node >> *ptr_base_ident_conf;

	// Read specialized properties
	if (node.FindValue("forgetting-factor"))
	{
		node["forgetting-factor"] >> ident_conf.forgetting_factor;
	}
	else
	{
		ident_conf.forgetting_factor = 1.0;
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, rls_kulhavy1984_system_identification_config<RealT,UIntT>& ident_conf)
{
	typedef RealT real_type;
	typedef UIntT uint_type;

	// Read common properties
	base_rls_system_identification_config<real_type,uint_type>* ptr_base_ident_conf = &ident_conf;
	node >> *ptr_base_ident_conf;

	// Read specialized properties
	if (node.FindValue("forgetting-factor"))
	{
		node["forgetting-factor"] >> ident_conf.forgetting_factor;
	}
	else
	{
		ident_conf.forgetting_factor = 1.0;
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, rls_park1991_system_identification_config<RealT,UIntT>& ident_conf)
{
	typedef RealT real_type;
	typedef UIntT uint_type;

	// Read common properties
	base_rls_system_identification_config<real_type,uint_type>* ptr_base_ident_conf = &ident_conf;
	node >> *ptr_base_ident_conf;

	// Read specialized properties
	if (node.FindValue("forgetting-factor"))
	{
		node["forgetting-factor"] >> ident_conf.forgetting_factor;
	}
	else
	{
		ident_conf.forgetting_factor = 1.0;
	}
	if (node.FindValue("sensitivity-gain"))
	{
		node["sensitivity-gain"] >> ident_conf.rho;
	}
	else
	{
		ident_conf.rho = 1.0;
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, lq_application_controller_config<RealT,UIntT>& controller_conf)
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	::std::string label;

	node["sys-ident"]["type"] >> label;
	controller_conf.ident_category = detail::text_to_system_identification_category(label);
	switch (controller_conf.ident_category)
	{
		case rls_bittanti1990_system_identification:
			{
				typedef rls_bittanti1990_system_identification_config<real_type,uint_type> ident_config_impl_type;

				ident_config_impl_type ident_config_impl;
				node["sys-ident"] >> ident_config_impl;
				controller_conf.ident_category_conf = ident_config_impl;
			}
			break;
		case rls_ff_system_identification:
			{
				typedef rls_ff_system_identification_config<real_type,uint_type> ident_config_impl_type;

				ident_config_impl_type ident_config_impl;
				node["sys-ident"] >> ident_config_impl;
				controller_conf.ident_category_conf = ident_config_impl;
			}
			break;
		case rls_kulhavy1984_system_identification:
			{
				typedef rls_kulhavy1984_system_identification_config<real_type,uint_type> ident_config_impl_type;

				ident_config_impl_type ident_config_impl;
				node["sys-ident"] >> ident_config_impl;
				controller_conf.ident_category_conf = ident_config_impl;
			}
			break;
		case rls_park1991_system_identification:
			{
				typedef rls_park1991_system_identification_config<real_type,uint_type> ident_config_impl_type;

				ident_config_impl_type ident_config_impl;
				node["sys-ident"] >> ident_config_impl;
				controller_conf.ident_category_conf = ident_config_impl;
			}
			break;
	}
	if (node.FindValue("ewma-smoothing-factor"))
	{
		node["ewma-smoothing-factor"] >> controller_conf.ewma_smoothing_factor;
	}
	else
	{
		controller_conf.ewma_smoothing_factor = 0;
	}
//	if (node.FindValue("rls-forgetting-factor"))
//	{
//		node["rls-forgetting-factor"] >> controller_conf.rls_forgetting_factor;
//	}
//	else
//	{
//		controller_conf_impl.rls_forgetting_factor = 1.0;
//	}
	if (node.FindValue("output-order"))
	{
		node["output-order"] >> controller_conf.n_a;
	}
	else
	{
		controller_conf.n_a = 2;
	}
	if (node.FindValue("input-order"))
	{
		node["input-order"] >> controller_conf.n_b;
	}
	else
	{
		controller_conf.n_b = 1;
	}
	if (node.FindValue("input-delay"))
	{
		node["input-delay"] >> controller_conf.d;
	}
	else
	{
		controller_conf.d = 1;
	}
	node["Q"] >> controller_conf.Q;
	node["R"] >> controller_conf.R;
	if (node.FindValue("N"))
	{
		node["N"] >> controller_conf.N;
	}
	else
	{
		// default to N=[0...0;...;0...0]
		controller_conf.N = numeric_matrix<real_type>(
				controller_conf.Q.num_rows(),
				controller_conf.R.num_rows(),
				real_type/*zero*/()
			);
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, application_controller_config<RealT,UIntT>& controller_conf)
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef application_controller_config<real_type,uint_type> controller_config_type;

	::std::string label;

	// Read controller category
	node["type"] >> label;
	controller_conf.category = detail::text_to_application_controller_category(label);
	switch (controller_conf.category)
	{
		case dummy_application_controller:
			{
				typedef typename controller_config_type::dummy_controller_config_type controller_config_impl_type;

				controller_config_impl_type controller_conf_impl;

				controller_conf.category_conf = controller_conf_impl;
			}
			break;
		case lqi_application_controller:
			{
				typedef typename controller_config_type::lqi_controller_config_type controller_config_impl_type;
				typedef typename controller_config_impl_type::base_type base_controller_config_impl_type;;

				controller_config_impl_type controller_conf_impl;

				node >> static_cast<base_controller_config_impl_type&>(controller_conf_impl);

//				if (node.FindValue("integral-weight"))
//				{
//					node["integral-weight"] >> controller_conf_impl.integral_weight;
//				}
//				else
//				{
//					controller_conf_impl.integral_weight = 0.98;
//				}

				controller_conf.category_conf = controller_conf_impl;
			}
			break;
		case lqr_application_controller:
			{
				typedef typename controller_config_type::lqr_controller_config_type controller_config_impl_type;
				typedef typename controller_config_impl_type::base_type base_controller_config_impl_type;;

				controller_config_impl_type controller_conf_impl;

				node >> static_cast<base_controller_config_impl_type&>(controller_conf_impl);

				controller_conf.category_conf = controller_conf_impl;
			}
			break;
		case lqry_application_controller:
			{
				typedef typename controller_config_type::lqry_controller_config_type controller_config_impl_type;
				typedef typename controller_config_impl_type::base_type base_controller_config_impl_type;;

				controller_config_impl_type controller_conf_impl;

				node >> static_cast<base_controller_config_impl_type&>(controller_conf_impl);

				controller_conf.category_conf = controller_conf_impl;
			}
			break;
		case matlab_lqi_application_controller:
			{
				typedef typename controller_config_type::matlab_lqi_controller_config_type controller_config_impl_type;
				typedef typename controller_config_impl_type::base_type base_controller_config_impl_type;;

				controller_config_impl_type controller_conf_impl;

				node >> static_cast<base_controller_config_impl_type&>(controller_conf_impl);

//				if (node.FindValue("integral-weight"))
//				{
//					node["integral-weight"] >> controller_conf_impl.integral_weight;
//				}
//				else
//				{
//					controller_conf_impl.integral_weight = 0.98;
//				}

				controller_conf.category_conf = controller_conf_impl;
			}
			break;
		case matlab_lqr_application_controller:
			{
				typedef typename controller_config_type::matlab_lqr_controller_config_type controller_config_impl_type;
				typedef typename controller_config_impl_type::base_type base_controller_config_impl_type;;

				controller_config_impl_type controller_conf_impl;

				node >> static_cast<base_controller_config_impl_type&>(controller_conf_impl);

//				if (node.FindValue("integral-weight"))
//				{
//					node["integral-weight"] >> controller_conf_impl.integral_weight;
//				}
//				else
//				{
//					controller_conf_impl.integral_weight = 0.98;
//				}

				controller_conf.category_conf = controller_conf_impl;
			}
			break;
		case matlab_lqry_application_controller:
			{
				typedef typename controller_config_type::matlab_lqry_controller_config_type controller_config_impl_type;
				typedef typename controller_config_impl_type::base_type base_controller_config_impl_type;;

				controller_config_impl_type controller_conf_impl;

				node >> static_cast<base_controller_config_impl_type&>(controller_conf_impl);

//				if (node.FindValue("integral-weight"))
//				{
//					node["integral-weight"] >> controller_conf_impl.integral_weight;
//				}
//				else
//				{
//					controller_conf_impl.integral_weight = 0.98;
//				}

				controller_conf.category_conf = controller_conf_impl;
			}
			break;
		case qn_application_controller:
			{
				typedef typename controller_config_type::qn_controller_config_type controller_config_impl_type;

				controller_config_impl_type controller_conf_impl;

				controller_conf.category_conf = controller_conf_impl;
			}
			break;
	}

	// Read sampling time
	if (controller_conf.category != dummy_application_controller)
	{
		node["sampling-time"] >> controller_conf.sampling_time;
		if (controller_conf.sampling_time <= 0)
		{
			throw ::std::runtime_error("[dcs::eesim::config::>>] Invalid sampling time for application controller: non-positive value.");
		}
	}
	else
	{
		controller_conf.sampling_time = 0;
	}

	// Read triggers
	controller_conf.triggers.actual_value_sla_ko_enabled = false; // default value
	controller_conf.triggers.predicted_value_sla_ko_enabled = false; // default value
	if (controller_conf.category != dummy_application_controller)
	{
		if (node.FindValue("triggers"))
		{
			::YAML::Node const& subnode = node["triggers"];
			if (subnode.FindValue("actual-value-sla-ko"))
			{
				subnode["actual-value-sla-ko"] >> controller_conf.triggers.actual_value_sla_ko_enabled;
			}
			if (subnode.FindValue("predicted-value-sla-ko"))
			{
				subnode["predicted-value-sla-ko"] >> controller_conf.triggers.predicted_value_sla_ko_enabled;
			}
		}
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, application_config<RealT,UIntT>& app)
{
	// Read (optional) name
	if (node.FindValue("name"))
	{
		node["name"] >> app.name;
	}
	// Read performance model
	node["perf-model"] >> app.perf_model;
	// Read simulation model
	node["sim-model"] >> app.sim_model;
	// Read reference resources
	{
		::YAML::Node const& subnode = node["reference-resources"];

		for (::YAML::Iterator it = subnode.begin(); it != subnode.end(); ++it)
		{
			::YAML::Node const& key_node = it.first();
			::YAML::Node const& value_node = it.second();

			physical_resource_category category;
			RealT capacity;
			::std::string resource;

			key_node >> resource;
			value_node >> capacity;

			category = detail::text_to_physical_resource_category(resource);

			app.reference_resources[category] = capacity;
		}
	}
	// Read SLA model
	node["sla"] >> app.sla;
	// Read tiers
	{
		::YAML::Node const& subnode = node["tiers"];
		for (::std::size_t i = 0; i < subnode.size(); ++i)
		{
			application_tier_config<RealT> tier;

			subnode[i]["tier"] >> tier;

			app.tiers.push_back(tier);
		}
	}
	// Read controller
	node["controller"] >> app.controller;
}


template <typename RealT>
void operator>>(::YAML::Node const& node, physical_resource_config<RealT>& res)
{
	// Read the (optional) name
	if (node.FindValue("name"))
	{
		node["name"] >> res.name;
	}
	// Read resource capacity
	node["capacity"] >> res.capacity;
	// Read (optonal) resource utilization threshold
	if (node.FindValue("threshold"))
	{
		node["threshold"] >> res.threshold;
	}
	else
	{
		res.threshold = 1;
	}
	// Read energy model
	{
		::std::string category_lbl;
		::YAML::Node const& subnode = node["energy-model"];
		subnode["type"] >> category_lbl;

		energy_model_category category;
		category = detail::text_to_energy_model_category(category_lbl);
		res.energy_model_type = category;

		switch (category)
		{
			case constant_energy_model:
				{
					constant_energy_model_config<RealT> model_conf;

					subnode["c0"] >> model_conf.c0;

					res.energy_model_conf = model_conf;
				}
				break;
			case fan2007_energy_model:
				{
					fan2007_energy_model_config<RealT> model_conf;

					subnode["c0"] >> model_conf.c0;
					subnode["c1"] >> model_conf.c1;
					subnode["c2"] >> model_conf.c2;
					subnode["r"] >> model_conf.r;

					res.energy_model_conf = model_conf;
				}
				break;
		}
	}
}


template <typename RealT>
void operator>>(::YAML::Node const& node, physical_machine_controller_config<RealT>& controller_conf)
{
	typedef physical_machine_controller_config<RealT> controller_config_type;

	::std::string label;

	// Read controller category
	node["type"] >> label;
	controller_conf.category = detail::text_to_physical_machine_controller_category(label);
	switch (controller_conf.category)
	{
		case conservative_physical_machine_controller:
			{
				typedef typename controller_config_type::conservative_controller_config_type controller_config_impl_type;

				controller_config_impl_type controller_conf_impl;

				controller_conf.category_conf = controller_conf_impl;
			}
			break;
		case proportional_physical_machine_controller:
			{
				typedef typename controller_config_type::proportional_controller_config_type controller_config_impl_type;

				controller_config_impl_type controller_conf_impl;

				controller_conf.category_conf = controller_conf_impl;
			}
			break;
		case dummy_physical_machine_controller:
			{
				typedef typename controller_config_type::dummy_controller_config_type controller_config_impl_type;

				controller_config_impl_type controller_conf_impl;

				controller_conf.category_conf = controller_conf_impl;
			}
			break;
	}

	// Read sampling time
	if (controller_conf.category != dummy_physical_machine_controller)
	{
		if (node.FindValue("sampling-time"))
		{
			node["sampling-time"] >> controller_conf.sampling_time;
			if (controller_conf.sampling_time <= 0)
			{
				throw ::std::runtime_error("[dcs::eesim::config::>>] Invalid sampling time for physical machine controller: non-positive value.");
			}
		}
		else
		{
			controller_conf.sampling_time = 0;
		}
	}
	else
	{
		controller_conf.sampling_time = 0;
	}
}


template <typename RealT>
void operator>>(::YAML::Node const& node, physical_machine_config<RealT>& mach)
{
	// Read (optional) name
	if (node.FindValue("name"))
	{
		node["name"] >> mach.name;
	}
	// Read resources
	{
		::YAML::Node const& subnode = node["resources"];
		for (::YAML::Iterator it = subnode.begin(); it != subnode.end(); ++it)
		{
			::YAML::Node const& key_node = it.first();
			::YAML::Node const& value_node = it.second();

			physical_resource_category category;
			physical_resource_config<RealT> resource_conf;
			::std::string resource;

			key_node >> resource;
			value_node >> resource_conf;

			category = detail::text_to_physical_resource_category(resource);

			resource_conf.type = category;
			mach.resources[category] = resource_conf;
		}
	}
	// Read controller
	{
		node["controller"] >> mach.controller;
	}
}


template <typename RealT>
void operator>>(::YAML::Node const& node, initial_placement_strategy_config<RealT>& strategy_conf)
{
	typedef initial_placement_strategy_config<RealT> strategy_config_type;

	::std::string label;

	node["type"] >> label;

	strategy_conf.category = detail::text_to_initial_placement_strategy_category(label);

	switch (strategy_conf.category)
	{
		case best_fit_initial_placement_strategy:
			{
				typedef typename strategy_config_type::best_fit_initial_placement_strategy_config_type strategy_config_impl_type;

				strategy_config_impl_type strategy_conf_impl;

				strategy_conf.category_conf = strategy_conf_impl;
			}
			break;
		case first_fit_initial_placement_strategy:
			{
				typedef typename strategy_config_type::first_fit_initial_placement_strategy_config_type strategy_config_impl_type;

				strategy_config_impl_type strategy_conf_impl;

				strategy_conf.category_conf = strategy_conf_impl;
			}
			break;
		case first_fit_scaleout_initial_placement_strategy:
			{
				typedef typename strategy_config_type::first_fit_scaleout_initial_placement_strategy_config_type strategy_config_impl_type;

				strategy_config_impl_type strategy_conf_impl;

				strategy_conf.category_conf = strategy_conf_impl;
			}
			break;
	}

	if (node.FindValue("ref-penalty"))
	{
		node["ref-penalty"] >> strategy_conf.ref_penalty;
	}
}


template <typename RealT>
void operator>>(::YAML::Node const& node, migration_controller_config<RealT>& controller_conf)
{
	typedef migration_controller_config<RealT> controller_config_type;

	::std::string label;

	node["type"] >> label;

	controller_conf.category = detail::text_to_migration_controller_category(label);

	switch (controller_conf.category)
	{
		case lp_migration_controller:
			{
				typedef typename controller_config_type::lp_migration_controller_config_type controller_config_impl_type;

				controller_config_impl_type controller_conf_impl;

				controller_conf.category_conf = controller_conf_impl;
			}
			break;
		case dummy_migration_controller:
			{
				typedef typename controller_config_type::dummy_migration_controller_config_type controller_config_impl_type;

				controller_config_impl_type controller_conf_impl;

				controller_conf.category_conf = controller_conf_impl;
			}
			break;
	}

	// Read sampling time
	if (controller_conf.category != dummy_migration_controller)
	{
		node["sampling-time"] >> controller_conf.sampling_time;
		if (controller_conf.sampling_time <= 0)
		{
			throw ::std::runtime_error("[dcs::eesim::config::>>] Invalid sampling time for migration controller: non-positive value.");
		}
	}
	else
	{
		controller_conf.sampling_time = 0;
	}
}


template <typename RealT, typename UIntT>
class yaml_reader
{
	public: typedef RealT real_type;
	public: typedef UIntT uint_type;
	public: typedef configuration<RealT,UIntT> configuration_type;
	private: static const ::std::string tag_dcs_eesim;
	private: static const ::std::string tag_dcs_eesim_app;
	private: static const ::std::string tag_dcs_eesim_matrix;
//	private: static const ::std::string tag_dcs_eesim_perf_qn;
//	private: static const ::std::string tag_dcs_eesim_sim_qn;


	public: yaml_reader()
	{
	}


	public: yaml_reader(::std::string const& fname)
	{
		read(fname);
	}


	public: configuration_type read(::std::string const& fname)
	{
		::std::ifstream ifs(fname.c_str());

		configuration_type conf;

		conf = read(ifs);

		ifs.close();

		return conf;
	}


	public: template <typename CharT, typename CharTraitsT>
		configuration_type read(::std::basic_istream<CharT,CharTraitsT>& is)
	{
		configuration_type conf;

		::YAML::Parser parser(is);
		::YAML::Node doc;
		while(parser.GetNextDocument(doc))
		{
			conf = read(doc);
		}

		return conf;
	}


	public: configuration_type read(YAML::Node const& doc)
	{
		configuration_type conf;
		::std::string label;

		// Read logging settings
		if (doc.FindValue("logging"))
		{
			logging_config logging;

			doc["logging"] >> logging;

			conf.logging(logging);
		}
		// Read random number generation settings
		{
			rng_config<uint_type> rng;

			doc["random-number-generation"] >> rng;

			conf.rng(rng);
		}
		// Read simulation settings
		{
			simulation_config<real_type,uint_type> sim;

			doc["simulation"] >> sim;

			conf.simulation(sim);
		}
		// Read data center
		{
			::YAML::Node const& node = doc["data-center"];

			typedef typename configuration_type::data_center_config_type data_center_config_type;

			data_center_config_type dc;

			// Read applications
			{
				::YAML::Node const& subnode = node["applications"];
				::std::size_t n = subnode.size();
				for (::std::size_t i = 0; i < n; ++i)
				{
					application_config<real_type,uint_type> app;

					::YAML::Node const& app_node = subnode[i]["application"];
					app_node >> app;

					dc.add_application(app);
				}
			}
			// Read physical machines
			{
				::YAML::Node const& subnode = node["physical-machines"];
				::std::size_t n = subnode.size();
				for (::std::size_t i = 0; i < n; ++i)
				{
					physical_machine_config<real_type> mach;

					::YAML::Node const& mach_node = subnode[i]["physical-machine"];
					mach_node >> mach;

					dc.add_physical_machine(mach);
				}
			}
			// Initial placement
			{
				typedef typename data_center_config_type::initial_placement_strategy_config_type initial_placement_strategy_config_type;

				::YAML::Node const& subnode = node["initial-placement-strategy"];

				initial_placement_strategy_config_type strategy;

				subnode >> strategy;

				dc.initial_placement_strategy(strategy);
			}
			// Migration controller
			{
				typedef typename data_center_config_type::migration_controller_config_type migration_controller_config_type;

				::YAML::Node const& subnode = node["migration-controller"];

				migration_controller_config_type controller;

				subnode >> controller;

				dc.migration_controller(controller);
			}

			conf.data_center(dc);
		}

		return conf;
	}


	private: ::std::string type_to_string(::YAML::NodeType::value type)
	{
			switch (type)
			{
				case ::YAML::NodeType::Scalar:
					return "SCALAR";
				case ::YAML::NodeType::Sequence:
					return "SEQUENCE";
				case ::YAML::NodeType::Map:
					return "MAP";
				case ::YAML::NodeType::Null:
					return "(empty)";
				default:
					return "(unknown)";
			}
	}


/*
	private: void traverse(YAML::Node const& node, unsigned int depth = 0)
	{
		::std::string out;
		::YAML::NodeType::value type = node.Type();
		::std::string indent((size_t)depth, '\t');

		::std::string tag = node.Tag();

		if (tag.empty())
		{
			switch (type)
			{
				case ::YAML::NodeType::Scalar:
					node >> out;
					::std::cout << indent << "SCALAR: " << out << ::std::endl;
					break;
				case ::YAML::NodeType::Sequence:
					::std::cout << indent << "SEQUENCE:" << ::std::endl;
					for (unsigned int i = 0; i < node.size(); i++) {
						const ::YAML::Node & subnode = node[i];
						::std::cout << indent << "[" << i << "]:" << ::std::endl;
						traverse(subnode, depth + 1);
					}
					break;
				case ::YAML::NodeType::Map:
					::std::cout << indent << "MAP:" << ::std::endl;
					for (::YAML::Iterator i = node.begin(); i != node.end(); ++i) {
						const ::YAML::Node & key   = i.first();
						const ::YAML::Node & value = i.second();
						key >> out;
						::std::cout << indent << "KEY: " << out << ::std::endl;
						::std::cout << indent << "VALUE:" << ::std::endl;
						traverse(value, depth + 1);
					}
					break;
				case ::YAML::NodeType::Null:
					::std::cout << indent << "(empty)" << ::std::endl;
					break;
				default:
					::std::cerr << "Warning: traverse: unknown/unsupported node type" << ::std::endl;
			}
		}
//		else if (!tag.compare(0, tag_dcs_eesim.length(), tag_dcs_eesim))
//		{
//			::std::cout << indent << "(tag: " << node.Tag() << ")" << ::std::endl;
//		}
		else if (!tag.compare(tag_dcs_eesim_sim_qn_node))
		{
			::std::cout << indent << "(tag: " << node.Tag() << ")" << ::std::endl;
			detail::matrix<double> m;
			node >> m;
			::std::cout << "Matrix: " << m << std::endl;
		}
		else if (!tag.compare(tag_dcs_eesim_matrix))
		{
			::std::cout << indent << "(tag: " << node.Tag() << ")" << ::std::endl;
			detail::matrix<double> m;
			node >> m;
			::std::cout << "Matrix: " << m << std::endl;
		}
		else
		{
			::std::cerr << "Warning: traverse: unknown/unsupported node tag '" << tag << "'" << ::std::endl;
		}
	}
*/
};

template <typename RealT, typename UIntT>
const ::std::string yaml_reader<RealT,UIntT>::tag_dcs_eesim = ::std::string("tag:dcs.di.unipmn.it,2010:eesim/");

template <typename RealT, typename UIntT>
const ::std::string yaml_reader<RealT,UIntT>::tag_dcs_eesim_app = yaml_reader::tag_dcs_eesim + ::std::string("app");

template <typename RealT, typename UIntT>
const ::std::string yaml_reader<RealT,UIntT>::tag_dcs_eesim_matrix = ::std::string("tag:dcs.di.unipmn.it,2010:eesim/matrix");


}}} // Namespace dcs::eesim::config

#endif // DCS_EESIM_CONFIG_YAML_HPP
