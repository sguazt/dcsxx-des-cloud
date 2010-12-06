#include <dcs/des/engine.hpp>
#include <dcs/eesim/config/configuration.hpp>
#include <dcs/eesim/config/operation/make_data_center.hpp>
#include <dcs/eesim/config/operation/make_des_engine.hpp>
#include <dcs/eesim/config/operation/make_random_number_generator.hpp>
#include <dcs/eesim/config/operation/read_file.hpp>
#include <dcs/eesim/config/yaml.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/eesim/traits.hpp>
#include <dcs/math/random/any_generator.hpp>
#include <dcs/memory.hpp>
#include <iostream>
#include <string>


static std::string prog_name;

enum configuration_category
{
	yaml_configuration,
	xml_configuration
};


void usage()
{
	std::cerr << "Usage: " << prog_name << " conf_file" << std::endl;
}


int main(int argc, char* argv[])
{
	typedef double real_type;
	typedef unsigned long uint_type;
	typedef long int_type;
	typedef dcs::des::engine<real_type> des_engine_type;
	typedef dcs::math::random::base_generator<real_type> random_generator_type;
	typedef dcs::eesim::traits<
				des_engine_type,
				random_generator_type,
				real_type,
				uint_type,
				int_type
			> traits_type;
	typedef dcs::eesim::registry<traits_type> registry_type;
	typedef dcs::shared_ptr<des_engine_type> des_engine_pointer;
	typedef dcs::shared_ptr<random_generator_type> random_generator_pointer;
	//typedef dcs::eesim::data_center<traits_type> data_center_type;
	typedef dcs::shared_ptr< dcs::eesim::data_center<traits_type> > data_center_pointer;


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

	// Build the Data Center

	data_center_pointer ptr_dc;
	ptr_dc = dcs::eesim::config::make_data_center<traits_type>(conf, ptr_rng, ptr_des_eng);

	ptr_des_eng->run();
}
