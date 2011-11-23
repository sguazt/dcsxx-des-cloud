#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

std::string to_string(YAML::NodeType::value type)
{
	if (type == YAML::NodeType::Null)
	{
		return "NULL";
	}
	if (type == YAML::NodeType::Scalar)
	{
		return "SCALAR";
	}
	if (type == YAML::NodeType::Sequence)
	{
		return "SEQUENCE";
	}
	if (type == YAML::NodeType::Map)
	{
		return "MAP";
	}

	return "UNKNOWN";
}

template <typename RealT>
struct statistic_info
{
	typedef RealT real_type;

	std::string type;
	real_type estimate;
	real_type sd;
};

template <typename UIntT, typename RealT>
struct application_tier_info
{
	typedef UIntT uint_type;
	typedef RealT real_type;
	typedef statistic_info<real_type> statistic_info_type;
	typedef std::vector<statistic_info_type> statistic_info_container;

	uint_type id;
	std::string name;
	statistic_info_type num_arrivals;
	statistic_info_type num_departures;
	statistic_info_container busy_times;
	statistic_info_container queue_lengths;
	statistic_info_container response_times;
	statistic_info_container throughputs;
	statistic_info_container utilizations;
};

template <typename UIntT, typename RealT>
struct application_info
{
	typedef UIntT uint_type;
	typedef RealT real_type;
	typedef application_tier_info<uint_type,real_type> tier_info_type;
	typedef std::vector<tier_info_type> tier_info_container;
	typedef statistic_info<real_type> statistic_info_type;
	typedef std::vector<statistic_info_type> statistic_info_container;

	uint_type id;
	std::string name;
	statistic_info_type num_arrivals;
	statistic_info_type num_departures;
	statistic_info_type num_sla_violations;
	statistic_info_container response_times;
	statistic_info_container throughputs;
	tier_info_container tiers;
};

template <typename UIntT, typename RealT>
struct physical_machine_info
{
	typedef UIntT uint_type;
	typedef RealT real_type;
	typedef statistic_info<real_type> statistic_info_type;

	uint_type id;
	std::string name;
	statistic_info_type uptime;
	statistic_info_type consumed_energy;
	statistic_info_type utilization;
	statistic_info_type share;
};

template <typename UIntT, typename RealT>
struct data_center_info
{
	typedef UIntT uint_type;
	typedef RealT real_type;
	typedef statistic_info<real_type> statistic_info_type;

	statistic_info_type consumed_energy;
	statistic_info_type num_vm_migrations;
	statistic_info_type vm_migration_rate;
};

template <typename RealT>
void operator>>(YAML::Node const& node, statistic_info<RealT>& stat)
{
	node["type"] >> stat.type;
	node["estimate"] >> stat.estimate;
	node["stddev"] >> stat.sd;
}

template <typename UIntT, typename RealT>
void operator>>(YAML::Node const& node, application_tier_info<UIntT,RealT>& tier)
{
	node["id"] >> tier.id;
	node["name"] >> tier.name;
	node["num-arrivals"] >> tier.num_arrivals;
	node["num-departures"] >> tier.num_departures;
	node["busy-time"] >> tier.busy_times;
	node["queue-length"] >> tier.queue_lengths;
	node["response-time"] >> tier.response_times;
	node["throughput"] >> tier.throughputs;
	node["utilization"] >> tier.utilizations;
}

template <typename UIntT, typename RealT>
void operator>>(YAML::Node const& node, application_info<UIntT,RealT>& app)
{
	node["id"] >> app.id;
	node["name"] >> app.name;
	node["overall"]["num-arrivals"] >> app.num_arrivals;
	node["overall"]["num-departures"] >> app.num_departures;
	node["overall"]["num-sla-violations"] >> app.num_sla_violations;
	node["overall"]["response-time"] >> app.response_times;
	node["overall"]["throughput"] >> app.throughputs;
	node["tiers"] >> app.tiers;
}

template <typename UIntT, typename RealT>
void operator>>(YAML::Node const& node, physical_machine_info<UIntT,RealT>& mach)
{
	node["id"] >> mach.id;
	node["name"] >> mach.name;
	node["uptime"] >> mach.uptime;
	node["consumed-energy"] >> mach.consumed_energy;
	node["utilization"] >> mach.utilization;
	node["share"] >> mach.share;
}

template <typename UIntT, typename RealT>
void operator>>(YAML::Node const& node, data_center_info<UIntT,RealT>& dc)
{
	node["consumed-energy"] >> dc.consumed_energy;
	node["num-vm-migrations"] >> dc.num_vm_migrations;
	node["vm-migration-rate"] >> dc.vm_migration_rate;
}


template <
	typename UIntT,
	typename RealT,
	typename IsCharT,
	typename IsCharTraitsT,
	typename OsCharT,
	typename OsCharTraitsT
>
void yaml_to_csv(std::basic_istream<IsCharT,IsCharTraitsT>& is,
				 std::basic_ostream<OsCharT,OsCharTraitsT>& os,
				 std::string const& sep,
				 std::string const& eol,
				 std::string const& quote)
/*
template <
	typename UIntT,
	typename RealT
>
void yaml_to_csv(std::istream& is,
				 std::ostream& os,
				 std::string const& sep, std::string const& eol,
				 std::string const& quote)
*/
{
	// Avoid unused variable warnings
	(void)quote;

	typedef UIntT uint_type;
	typedef RealT real_type;
	typedef application_info<uint_type,real_type> application_info_type;
	typedef typename application_info_type::tier_info_type application_tier_info_type;
	typedef typename application_info_type::tier_info_container application_tier_info_container;
	typedef typename application_tier_info_container::const_iterator application_tier_info_iterator;
	typedef physical_machine_info<uint_type,real_type> physical_machine_info_type;
	typedef data_center_info<uint_type,real_type> data_center_info_type;

	YAML::Parser yaml(is);
	YAML::Node doc;
	while (yaml.GetNextDocument(doc))
	{
		// Applications
		{
			YAML::Node const& node = doc["applications"];
			YAML::Iterator app_end_it(node.end());
			for (YAML::Iterator app_it = node.begin(); app_it != app_end_it; ++app_it)
			{
				YAML::Node const& app_node((*app_it));
//::std::cerr << to_string(app_node.Type()) << ::std::endl;///XXX

				application_info_type app;

				app_node >> app;

				if (app_it != node.begin())
				{
					os << sep;
				}

				os        << app.num_arrivals.estimate
				   << sep << app.num_arrivals.sd
				   << sep << app.num_departures.estimate
				   << sep << app.num_departures.sd
				   << sep << app.num_sla_violations.estimate
				   << sep << app.num_sla_violations.sd
				   << sep << app.num_sla_violations.estimate/app.num_departures.estimate
				   << sep << app.response_times.back().estimate
				   << sep << app.response_times.back().sd
				   << sep << app.throughputs.back().estimate
				   << sep << app.throughputs.back().sd;

				// Tier utilizations
				application_tier_info_iterator tier_end_it(app.tiers.end());
				for (application_tier_info_iterator tier_it = app.tiers.begin(); tier_it != tier_end_it; ++tier_it)
				{
					application_tier_info_type const& tier(*tier_it);

					os << sep << tier.utilizations.back().estimate
					   << sep << tier.utilizations.back().sd;
				}

				// Tier wanted resource shares
				for (application_tier_info_iterator tier_it = app.tiers.begin(); tier_it != tier_end_it; ++tier_it)
				{
					//TODO
					os << sep << "" << sep << ""; // Mean and SD resource share
				}

				// Tier assigned resource shares
				for (application_tier_info_iterator tier_it = app.tiers.begin(); tier_it != tier_end_it; ++tier_it)
				{
					//TODO
					os << sep << "" << sep << ""; // Mean and SD resource share
				}
			}
		}

		// Physical Machines
		{
			YAML::Node const& node = doc["physical-machines"];
			YAML::Iterator mach_end_it(node.end());
			for (YAML::Iterator mach_it = node.begin(); mach_it != mach_end_it; ++mach_it)
			{
				YAML::Node const& mach_node((*mach_it));

				physical_machine_info_type mach;

				mach_node >> mach;

				os << sep << mach.uptime.estimate
				   << sep << mach.uptime.sd
				   << sep << mach.utilization.estimate
				   << sep << mach.utilization.sd
				   << sep << mach.share.estimate
				   << sep << mach.share.sd
				   << sep << mach.consumed_energy.estimate
				   << sep << mach.consumed_energy.sd
				   << sep << (mach.consumed_energy.estimate ? mach.consumed_energy.estimate/mach.uptime.estimate : static_cast<real_type>(0));
			}
		}

//		// Data Center
//		{
//			data_center_info_type dc;
//
//			doc["data-center"] >> dc;
//
//			os << sep << dc.consumed_energy.estimate
//			   << sep << dc.consumed_energy.sd
//			   << sep << dc.num_vm_migrations.estimate
//			   << sep << dc.vm_migration_rate.sd
//			   << sep << dc.vm_migration_rate.estimate
//			   << sep << dc.num_vm_migrations.sd;
//		}
//
		os << eol;
	}
}

int main(int argc, char* argv[])
{
	typedef unsigned long uint_type;
	typedef double real_type;


	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " <yaml-file> [<csv-file>]" << std::endl;
		return -1;
	}

	const std::string sep(",");
	const std::string eol("\n");
	const std::string quote("\"");

	std::string yaml_fname;
	std::string csv_fname;

	yaml_fname = argv[1];
	if (argc >= 3)
	{
		csv_fname = argv[2];
	}

	std::ifstream ifs(yaml_fname.c_str());

	if (csv_fname.empty())
	{
		yaml_to_csv<uint_type,real_type>(ifs, std::cout, sep, eol, quote);
	}
	else
	{
		std::ofstream ofs(csv_fname.c_str());

		yaml_to_csv<uint_type,real_type>(ifs, ofs, sep, eol, quote);

		ofs.close();
	}
}
