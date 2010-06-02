#include <cstddef>
#include <dcs/debug.hpp>
#include <dcs/des/batch_means/engine.hpp>
//#include <dcs/eesim/any_physical_machine.hpp>
#include <dcs/eesim/application_tier.hpp>
#include <dcs/eesim/multi_tier_application.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/physical_resource.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/eesim/traits.hpp>
#include <dcs/eesim/virtual_machine_monitor.hpp>
#include <dcs/eesim/virtual_machine.hpp>
#include <dcs/math/random.hpp>
#include <dcs/math/stats/distributions.hpp>
#include <dcs/memory.hpp>
#include <dcs/perfeval/energy/fan2007_model.hpp>
#include <dcs/perfeval/sla//step_cost_model.hpp>
#include <dcs/perfeval/workload/enterprise/generator.hpp>
#include <dcs/perfeval/workload/enterprise/tpcw.hpp>
#include <dcs/perfeval/workload/enterprise/user_request.hpp>
#include <dcs/perfeval/workload/enterprise/user_interaction_mix.hpp>
#include <dcs/perfeval/workload/enterprise/user_interaction_mix_model.hpp>
#include <string>
#include <utility>
#include <vector>


static const unsigned long seed = 5489u;


template <typename TraitsT>
dcs::perfeval::workload::enterprise::user_interaction_mix_model<dcs::perfeval::workload::enterprise::tpcw_request_category,dcs::math::stats::exponential_distribution<typename TraitsT::real_type>, typename TraitsT::real_type> create_tpcw_workload()
{
	typedef TraitsT traits_type;
	typedef typename traits_type::real_type real_type;
	typedef dcs::math::stats::exponential_distribution<real_type> distribution_type;
	typedef dcs::perfeval::workload::enterprise::tpcw_request_category request_category_type;
	typedef dcs::perfeval::workload::enterprise::user_interaction_mix<request_category_type,real_type> user_interaction_mix_type;
	typedef dcs::perfeval::workload::enterprise::user_request<request_category_type, distribution_type> user_request_type;
	typedef dcs::perfeval::workload::enterprise::user_interaction_mix_model<request_category_type, distribution_type,real_type> workload_type;

	distribution_type iatime_dist(2.5);
	std::vector<user_request_type> requests;

	requests.push_back(user_request_type(dcs::perfeval::workload::enterprise::tpcw_home_request_category, iatime_dist));
	requests.push_back(user_request_type(dcs::perfeval::workload::enterprise::tpcw_new_products_request_category, iatime_dist));
	requests.push_back(user_request_type(dcs::perfeval::workload::enterprise::tpcw_best_sellers_request_category, iatime_dist));
	requests.push_back(user_request_type(dcs::perfeval::workload::enterprise::tpcw_product_detail_request_category, iatime_dist));
	requests.push_back(user_request_type(dcs::perfeval::workload::enterprise::tpcw_search_request_request_category, iatime_dist));
	requests.push_back(user_request_type(dcs::perfeval::workload::enterprise::tpcw_search_results_request_category, iatime_dist));
	requests.push_back(user_request_type(dcs::perfeval::workload::enterprise::tpcw_shopping_cart_request_category, iatime_dist));
	requests.push_back(user_request_type(dcs::perfeval::workload::enterprise::tpcw_customer_registration_request_category, iatime_dist));
	requests.push_back(user_request_type(dcs::perfeval::workload::enterprise::tpcw_buy_request_request_category, iatime_dist));
	requests.push_back(user_request_type(dcs::perfeval::workload::enterprise::tpcw_buy_confirm_request_category, iatime_dist));
	requests.push_back(user_request_type(dcs::perfeval::workload::enterprise::tpcw_order_inquiry_request_category, iatime_dist));
	requests.push_back(user_request_type(dcs::perfeval::workload::enterprise::tpcw_order_display_request_category, iatime_dist));
	requests.push_back(user_request_type(dcs::perfeval::workload::enterprise::tpcw_admin_request_request_category, iatime_dist));
	requests.push_back(user_request_type(dcs::perfeval::workload::enterprise::tpcw_admin_confirm_request_category, iatime_dist));

	std::vector<user_interaction_mix_type> mixes;

	// Create browsing mix
	mixes.push_back(
		dcs::perfeval::workload::enterprise::tpcw_browsing_mix<real_type>()
	);
	// Create shopping mix
	mixes.push_back(
		dcs::perfeval::workload::enterprise::tpcw_shopping_mix<real_type>()
	);
	// Create ordering mix
	mixes.push_back(
		dcs::perfeval::workload::enterprise::tpcw_ordering_mix<real_type>()
	);

	::std::vector<real_type> weights;

	weights.push_back(1/3);
	weights.push_back(1/3);
	weights.push_back(1/3);

	workload_type workload(
		requests.begin(), requests.end(),
		mixes.begin(), mixes.end(),
		weights.begin(), weights.end()
	);

	//return dcs::perfeval::workload::enterprise::make_any_generator(workload);
	return workload;
}


template <typename TraitsT>
//dcs::eesim::any_physical_machine<TraitsT> create_machine(std::string const& name, typename TraitsT::real_type freq, typename TraitsT::real_type mem)
dcs::eesim::physical_machine<TraitsT> create_machine(std::string const& name, typename TraitsT::real_type freq, typename TraitsT::real_type mem)
{
	typedef TraitsT traits_type;
	typedef typename traits_type::real_type real_type;
	typedef dcs::perfeval::energy::fan2007_model<real_type> energy_model_type;
	typedef dcs::eesim::virtual_machine_monitor<traits_type> vmm_type;
	typedef dcs::eesim::virtual_machine<traits_type> vm_type;
	typedef dcs::eesim::physical_resource<traits_type> physical_resource_type;
	typedef dcs::eesim::physical_machine<traits_type> physical_machine_type;
physical_machine_type machine1;

	machine1.name(name);

	// Add resources
	machine1.add_resource(
		physical_resource_type(
			"cpu0",
			::dcs::eesim::cpu_resource_category,
			freq,
			energy_model_type(100, 1.2, 0.3, 2.5)
		)
	);
	machine1.add_resource(
		physical_resource_type(
			"cpu1",
			::dcs::eesim::cpu_resource_category,
			freq,
			energy_model_type(100, 1.2, 0.3, 2.5)
		)
	);
	machine1.add_resource(
		physical_resource_type(
			"ram",
			::dcs::eesim::memory_resouce_category,
			mem,
			energy_model_type(100, 1.2, 0.3, 2.5)
		)
	);

	// Add virtual machine monitor
	machine1.virtual_machine_monitor(
		vmm_type()
	);

//	// Add virtual machines
//	typedef typename vmm_type::virtual_machine_id_type vm_id_type;
//
//	vm_id_type vm_id;
//	vm_id = machine1.virtual_machine_monitor().add_virtual_machine(
//		dcs::make_shared<vm_type>("vm1")
//	);

//	return dcs::eesim::make_any_physical_machine(machine1);
	return machine1;
}


template <typename TraitsT>
dcs::eesim::virtual_machine<TraitsT> create_virtual_machine(std::string const& name)
{
	typedef TraitsT traits_type;

	dcs::eesim::virtual_machine<traits_type> vm(name);

	return vm;
}


template <
	typename TraitsT,
	typename EnterpriseWorkloadModelT,
	typename SlaCostModelT
>
dcs::eesim::multi_tier_application<
		typename EnterpriseWorkloadModelT::request_category_type,
		TraitsT
	> create_application(std::string const& name, EnterpriseWorkloadModelT const& workload, SlaCostModelT const& sla_cost)
{
//	typedef dcs::math::stats::exponential_distribution<real_type> distribution_type;
//	typedef dcs::perfeval::workload::enterprise::user_interaction_mix_model<dcs::perfeval::workload::enterprise::tpcw_request_category,dcs::math::stats::exponential_distribution<real_type>, real_type> workload_model_type;
//	typedef dcs::perfeval::sla::step_cost_model<real_type> sla_cost_model_type;
//
//	workload_model_type workload;
//	workload = create_tpcw_workload<traits_type>();

	typedef dcs::eesim::multi_tier_application<
			typename EnterpriseWorkloadModelT::request_category_type,
			TraitsT
		> application_type;

	application_type app;
	app.name(name);
	app.workload_model(workload);
	app.sla_cost_model(sla_cost);

	return app;
}


int main()
{
	typedef double real_type;
	typedef std::size_t uint_type;
	//typedef dcs::math::random::uniform_01_adaptor<dcs::math::random::mt19937,real_type> rng_type;
	typedef dcs::math::random::uniform_01_adaptor<dcs::math::random::mt19937,real_type> rng_type;
	typedef dcs::des::batch_means::engine<real_type> des_engine_type;
	typedef dcs::eesim::traits<des_engine_type,rng_type,real_type,uint_type> traits_type;
	typedef dcs::eesim::registry<traits_type> registry_type;
	//typedef std::vector< dcs::eesim::any_physical_machine<traits_type> > machine_container;
	typedef std::vector< dcs::eesim::physical_machine<traits_type> > machine_container;
	typedef dcs::eesim::virtual_machine<traits_type> virtual_machine_type;
	typedef dcs::perfeval::workload::enterprise::tpcw_request_category tpcw_request_category_type;
	typedef dcs::math::stats::exponential_distribution<real_type> exponential_distribution_type;
	typedef dcs::perfeval::workload::enterprise::user_interaction_mix_model<tpcw_request_category_type,exponential_distribution_type,real_type> workload_model_type;
	typedef dcs::perfeval::sla::step_cost_model<real_type> sla_cost_model_type;
	typedef dcs::eesim::multi_tier_application<tpcw_request_category_type,traits_type> application_type;

	//dcs::shared_ptr<rng_type> ptr_rng(dcs::make_shared<rng_type>(seed));
	//rng_type rng(seed);

	registry_type& reg(registry_type::instance());
	reg.des_engine(des_engine_type());
	reg.uniform_random_generator(rng_type(seed));

	workload_model_type workload_model;
	workload_model = create_tpcw_workload<traits_type>();

	dcs::perfeval::workload::enterprise::generator<workload_model_type> workload_gen(workload_model);

	rng_type& rng = reg.uniform_random_generator();
	for (std::size_t i = 0; i < 100; ++i)
	{
		typedef dcs::perfeval::workload::enterprise::generator<workload_model_type>::request_category_type request_category_type;

		std::pair<request_category_type,real_type> wkl =  workload_gen(rng);

		DCS_DEBUG_TRACE("Request type: " << wkl.first);
		DCS_DEBUG_TRACE("Request interarrival time: " << wkl.second);
	}

	machine_container machines;
	machines.push_back(
		create_machine<traits_type>("machine1", 800, 2048)
	);
	machines.push_back(
		create_machine<traits_type>("machine2", 1600, 4196)
	);

	for (
		machine_container::iterator it = machines.begin();
		it != machines.end();
		++it
	) {
		DCS_DEBUG_TRACE( "Machine: " << *it );

		uint_type vm_id;

		vm_id = it->virtual_machine_monitor().add_virtual_machine(
			::dcs::make_shared<virtual_machine_type>(
				create_virtual_machine<traits_type>(
					"vm1"
				)
			)
		);
		DCS_DEBUG_TRACE( "Created VM: " << vm_id );

		vm_id = it->virtual_machine_monitor().add_virtual_machine(
			::dcs::make_shared<virtual_machine_type>(
				create_virtual_machine<traits_type>(
					"vm2"
				)
			)
		);
		DCS_DEBUG_TRACE( "Created VM: " << vm_id );

		vm_id = it->virtual_machine_monitor().add_virtual_machine(
			::dcs::make_shared<virtual_machine_type>(
				create_virtual_machine<traits_type>(
					"vm3"
				)
			)
		);
		DCS_DEBUG_TRACE( "Created VM: " << vm_id );
	}


	std::vector<real_type> ref_measures;
	ref_measures.push_back(10);
	sla_cost_model_type sla_cost_model(
			real_type(10),
			real_type(10),
			ref_measures
	);

	application_type app;
	app = create_application<traits_type>(
		"app1",
		workload_model,
		sla_cost_model
	);

	DCS_DEBUG_TRACE( "Created APP: " << app );


	reg.des_engine().run();
}
