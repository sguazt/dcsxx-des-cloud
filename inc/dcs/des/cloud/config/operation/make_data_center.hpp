#ifndef DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_DATA_CENTER_HPP
#define DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_DATA_CENTER_HPP


#include <dcs/des/cloud/base_physical_machine_controller.hpp>
#include <dcs/des/cloud/config/configuration.hpp>
#include <dcs/des/cloud/config/operation/make_physical_machine.hpp>
#include <dcs/des/cloud/config/operation/make_physical_machine_controller.hpp>
#include <dcs/des/cloud/data_center.hpp>
#include <dcs/des/cloud/physical_machine.hpp>
#include <dcs/memory.hpp>
#include <stdexcept>


namespace dcs { namespace des { namespace cloud { namespace config {

template <typename TraitsT, typename RealT, typename UIntT>
::dcs::shared_ptr< ::dcs::des::cloud::data_center<TraitsT> > make_data_center(configuration<RealT,UIntT> const& conf,
																		 ::dcs::shared_ptr<typename TraitsT::uniform_random_generator_type> const& ptr_rng,
																		 ::dcs::shared_ptr<typename TraitsT::des_engine_type> const& ptr_des_eng)
{
	typedef TraitsT traits_type;
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef ::dcs::des::cloud::data_center<traits_type> data_center_type;
	typedef configuration<real_type,uint_type> configuration_type;
	typedef typename configuration_type::data_center_config_type data_center_config_type;

	// pre: random number generator pointer must be a valid pointer
	DCS_ASSERT(
		ptr_rng,
		throw ::std::invalid_argument("[dcs::des::cloud::config::make_data_center] Invalid random number generator.")
	);
	// pre: DES engine pointer must be a valid pointer
	DCS_ASSERT(
		ptr_des_eng,
		throw ::std::invalid_argument("[dcs::des::cloud::config::make_data_center] Invalid DES engine.")
	);

	::dcs::shared_ptr<data_center_type> ptr_dc = ::dcs::make_shared<data_center_type>();

	// Make physical machines
	{
		typedef typename data_center_config_type::physical_machine_config_container::const_iterator iterator;
		iterator end_it = conf.data_center().physical_machines().end();
		for (iterator it = conf.data_center().physical_machines().begin(); it != end_it; ++it)
		{
			::dcs::shared_ptr< ::dcs::des::cloud::physical_machine<traits_type> > ptr_mach;

			ptr_mach = make_physical_machine<traits_type>(*it);

			::dcs::shared_ptr< ::dcs::des::cloud::base_physical_machine_controller<traits_type> > ptr_mach_controller;

			ptr_mach_controller = make_physical_machine_controller<traits_type>(it->controller);
			ptr_mach_controller->machine(ptr_mach);

			ptr_dc->add_physical_machine(ptr_mach, ptr_mach_controller);
		}
	}

//[2011-09-25]: moved in make_data_center_manager
//	// Make applications
//	{
//		typedef typename data_center_config_type::application_config_container::const_iterator iterator;
//		iterator end_it = conf.data_center().applications().end();
//		for (iterator it = conf.data_center().applications().begin(); it != end_it; ++it)
//		{
//			::dcs::shared_ptr< ::dcs::des::cloud::multi_tier_application<traits_type> > ptr_app;
//
//			ptr_app = make_application<traits_type>(*it, conf, sim_info);
//
//			::dcs::shared_ptr< ::dcs::des::cloud::base_application_controller<traits_type> > ptr_app_controller;
//
//			ptr_app_controller = make_application_controller<traits_type>(it->controller, ptr_app);
//			ptr_app_controller->application(ptr_app);
//
//			ptr_dc->add_application(ptr_app, ptr_app_controller);
//		}
//	}

	return ptr_dc;
}


}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_DATA_CENTER_HPP
