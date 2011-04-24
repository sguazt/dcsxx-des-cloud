#include <algorithm>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublasx/operation/size.hpp>
#include <cmath>
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


enum signal_category
{
	step_signal,
	gaussian_white_noise_signal,
	sinusoidal_signal
};


enum system_identification_category
{
	siso_system_identification,
	miso_system_identification
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
typedef dcs::eesim::virtual_machine<traits_type> virtual_machine_type;
typedef dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;


namespace detail { namespace /*<unnamed>*/ {


void usage()
{
	::std::cerr << "Usage: " << prog_name << " <options>" << ::std::endl
				<< "Options:" << ::std::endl
				<< "  --ts <sampling-time>" << ::std::endl
				<< "  --ns <number-of-samples>" << ::std::endl
				<< "  --sys {'siso'|'miso'}" << ::std::endl
				<< "  --sig {'step'|'gaussian'|'sine'}" << ::std::endl
				<< "  --conf <configuration-file>" << ::std::endl;
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


signal_category parse_signal_category(::std::string const& str)
{
	if (!str.compare("step"))
	{
		return step_signal;
	}
	if (!str.compare("gaussian"))
	{
		return gaussian_white_noise_signal;
	}
	if (!str.compare("sine"))
	{
		return sinusoidal_signal;
	}

	throw ::std::invalid_argument("[detail::parse_signal_category] Cannot find a valid signal category.");
}


system_identification_category parse_system_identification_category(::std::string const& str)
{
	if (!str.compare("siso"))
	{
		return siso_system_identification;
	}
	if (!str.compare("miso"))
	{
		return miso_system_identification;
	}

	throw ::std::invalid_argument("[detail::parse_system_identification_category] Cannot find a valid system identification category.");
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


//@{ Signal generators

template <typename ValueT>
class base_signal_generator
{
	public: typedef ValueT value_type;
	public: typedef ::boost::numeric::ublas::vector<value_type> vector_type;


	public: vector_type operator()()
	{
		return do_generate();
	}


	public: virtual ~base_signal_generator()
	{
		// empty
	}


	private: virtual vector_type do_generate() = 0;
};

template <typename ValueT>
class step_signal_generator: public base_signal_generator<ValueT>
{
	private: typedef base_signal_generator<ValueT> base_type;
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
class gaussian_signal_generator: public base_signal_generator<ValueT>
{
	private: typedef base_signal_generator<ValueT> base_type;
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


template <typename ValueT>
class sinusoidal_signal_generator: public base_signal_generator<ValueT>
{
	private: typedef base_signal_generator<ValueT> base_type;
	public: typedef ValueT value_type;
	public: typedef typename base_type::vector_type vector_type;


	public: sinusoidal_signal_generator(vector_type a, vector_type f)
	: a_(a),
	  f_(f),
	  p_(ublas::zero_vector<value_type>(ublasx::size(a))),
	  d_(ublas::zero_vector<value_type>(ublasx::size(a))),
	  w_(::dcs::math::constants::double_pi<value_type>::value*f),
	  nx_(ublas::scalar_vector<value_type>(ublasx::size(a),100)),
	  x_(ublas::zero_vector<value_type>(ublasx::size(a)))
	{
		// pre: size(a) == size(f) == ...
	}


	public: sinusoidal_signal_generator(vector_type a, vector_type f, vector_type p, vector_type d)
	: a_(a),
	  f_(f),
	  p_(p),
	  d_(d),
	  w_(::dcs::math::constants::double_pi<value_type>::value*f),
	  nx_(ublas::scalar_vector<value_type>(ublasx::size(a),100)),
	  x_(ublas::zero_vector<value_type>(ublasx::size(a)))
	{
		// pre: size(a) == size(f) == ...
	}


	private: vector_type do_generate()
	{
//		random_generator_pointer ptr_rng(::dcs::eesim::registry<traits_type>::instance().uniform_random_generator_ptr());

		::std::size_t n(ublasx::size(a_));
		vector_type u(n);
		for (::std::size_t i = 0; i < n; ++i)
		{
			u(i) = a_(i)*::std::sin(w_(i)*x_(i)+p_(i))+d_(i);
//::std::cerr << "x(" << i << ")=" << x_(i) << " ==> u(" << i << ")=" << u(i) << ::std::endl;//XXX

			x_(i) += value_type(1)/nx_(i);
			//if (x_(i) < 0 || x_(i) > (1.0/f_(i)))
			if (x_(i) < 0 || x_(i) > 1.0)
			{
				x_(i) = 0;
			}
		}

		return u;
	}

	/// The amplitude (the peak deviation of the function from its center position).
	private: vector_type a_;
	/// The signal frequency
	private: vector_type f_;
	/// The phase (specifies where in its cycle the oscillation begins at t = 0).
	private: vector_type p_;
	/// The DC offset (a non-zero center amplitude).
	private: vector_type d_;
	/// The angular frequency (how many oscillations occur in a unit time interval, in radians per second).
	private: vector_type w_;
	private: vector_type nx_;
	private: vector_type x_;
};

//@} Signal generators


template <typename TraitsT>
struct sysid_base_request_info
{
	typedef TraitsT traits_type;
//	typedef typename traits_type::uint_type uint_type;
	typedef typename traits_type::real_type real_type;


	virtual ~sysid_base_request_info() { }


//	uint_type id;
	real_type arr_time;
//	real_type dep_time;
//	::std::map<resource_category_type,real_type> share_map;
};


template <typename TraitsT>
struct sysid_siso_request_info: public sysid_base_request_info<TraitsT>
{
	typedef TraitsT traits_type;
//	typedef typename traits_type::uint_type uint_type;
	typedef typename traits_type::real_type real_type;
	typedef ::dcs::eesim::physical_resource_category resource_category_type;

	::std::map<resource_category_type,real_type> share_map;
};


template <typename TraitsT>
struct sysid_miso_request_info: public sysid_base_request_info<TraitsT>
{
	typedef TraitsT traits_type;
	typedef typename traits_type::uint_type uint_type;
	typedef typename traits_type::real_type real_type;
	typedef ::dcs::eesim::physical_resource_category resource_category_type;

	::std::map<resource_category_type, ::std::map<uint_type,real_type> > share_map;
};


template <typename TraitsT>
struct sysid_state
{
	typedef TraitsT traits_type;
	typedef typename traits_type::uint_type uint_type;
	typedef sysid_base_request_info<traits_type> request_info_type;
	typedef ::dcs::shared_ptr<request_info_type> request_info_pointer;
	typedef std::map<uint_type,request_info_pointer> request_info_map;

	uint_type num_arrs;
	uint_type num_deps;
	uint_type max_num_deps;
	::std::vector<request_info_map> req_info_maps; // [tier_id][req_id => request_info*]
};


}} // Namespace detail::<unnamed>


template <typename TraitsT>
class base_system_identificator
{
	private: typedef base_system_identificator<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
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
	private: typedef detail::base_signal_generator<real_type> signal_generator_type;
	private: typedef ::dcs::shared_ptr<signal_generator_type> signal_generator_pointer;
	private: typedef detail::sysid_state<traits_type> sysid_state_type;
	private: typedef ::dcs::shared_ptr<sysid_state_type> sysid_state_pointer;


	public: base_system_identificator(real_type excite_ts)
	: excite_ts_(excite_ts),
	  ptr_excite_sys_evt_src_(new des_event_source_type())
	{
	}


	public: void identify(application_type& app, signal_generator_pointer const& ptr_sig_gen, uint_type max_num_deps)
	{
		sysid_state_pointer ptr_sysid_state;

		::std::vector<physical_machine_pointer> pms;
		::std::vector<virtual_machine_pointer> vms;

		size_type num_tiers(app.num_tiers());

		ptr_sysid_state = ::dcs::make_shared<sysid_state_type>();
		ptr_sysid_state->num_arrs = uint_type/*zero*/();
		ptr_sysid_state->num_deps = uint_type/*zero*/();
		ptr_sysid_state->req_info_maps.resize(num_tiers);
		ptr_sysid_state->max_num_deps = max_num_deps;

		des_engine_pointer ptr_des_eng(::dcs::eesim::registry<traits_type>::instance().des_engine_ptr());

		ptr_des_eng->system_initialization_event_source().connect(
				::dcs::functional::bind(
					&self_type::process_sys_init_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2,
					app
				)
			);
		ptr_des_eng->system_finalization_event_source().connect(
				::dcs::functional::bind(
					&self_type::process_sys_finit_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2,
					app
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
			app.simulation_model().request_tier_service_event_source(tier_id).connect(
					::dcs::functional::bind(
						&self_type::process_tier_request_service_event,
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
			app.simulation_model().request_tier_service_event_source(tier_id).disconnect(
					::dcs::functional::bind(
						&self_type::process_tier_request_service_event,
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
					&self_type::process_request_arrival_event,
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

/*
		ptr_des_eng->system_initialization_event_source().disconnect(
				::dcs::functional::bind(
					&self_type::process_sys_init_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2,
					app
				)
			);
		ptr_des_eng->system_finalization_event_source().disconnect(
				::dcs::functional::bind(
					&self_type::process_sys_finit_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2,
					app
				)
			);
*/
	}


	private: void schedule_excite_system_event()
	{
		des_engine_pointer ptr_des_eng(::dcs::eesim::registry<traits_type>::instance().des_engine_ptr());
		ptr_des_eng->schedule_event(ptr_excite_sys_evt_src_, ptr_des_eng->simulated_time()+excite_ts_);
	}


	//@{ Event Handlers


	private: void process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app)
	{
		DCS_DEBUG_TRACE("BEGIN Process SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");

		do_process_sys_init_event(evt, ctx, app);

		schedule_excite_system_event();

		DCS_DEBUG_TRACE("END Process SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app)
	{
		DCS_DEBUG_TRACE("BEGIN Process SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ")");

		do_process_sys_finit_event(evt, ctx, app);

		DCS_DEBUG_TRACE("END Process SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_DEBUG_TRACE("BEGIN Process REQUEST-ARRIVAL (Clock: " << ctx.simulated_time() << ")");

		ptr_sysid_state->num_arrs += 1;

		do_process_request_arrival_event(evt, ctx, ptr_sysid_state);

		DCS_DEBUG_TRACE("END Process REQUEST-ARRIVAL (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_DEBUG_TRACE("BEGIN Process REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");

		ptr_sysid_state->num_deps += 1;

		do_process_request_departure_event(evt, ctx, ptr_sysid_state);

//::std::cerr << "# deps -> " << ptr_sysid_state->num_deps << ::std::endl;//XXX
		if (ptr_sysid_state->num_deps == ptr_sysid_state->max_num_deps)
		{
//::std::cerr << "STOP!" << ::std::endl;//XXX
			::dcs::eesim::registry<traits_type>::instance().des_engine_ptr()->stop_now();
		}

		DCS_DEBUG_TRACE("END Process REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_tier_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_DEBUG_TRACE("BEGIN Process TIER-REQUEST-ARRIVAL (Clock: " << ctx.simulated_time() << ")");

		do_process_tier_request_arrival_event(evt, ctx, tier_id, app, ptr_sysid_state);

		DCS_DEBUG_TRACE("END Process TIER-REQUEST-ARRIVAL (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_tier_request_service_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_DEBUG_TRACE("BEGIN Process TIER-REQUEST-SERVICE (Clock: " << ctx.simulated_time() << ")");

		do_process_tier_request_service_event(evt, ctx, tier_id, app, ptr_sysid_state);

		DCS_DEBUG_TRACE("END Process TIER-REQUEST-SERVICE (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_tier_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_DEBUG_TRACE("BEGIN Process TIER-REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");

		do_process_tier_request_departure_event(evt, ctx, tier_id, app, ptr_sysid_state);

		DCS_DEBUG_TRACE("END Process TIER-REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_excite_system_event(des_event_type const& evt, des_engine_context_type& ctx, application_type& app, signal_generator_pointer const& ptr_sig_gen, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		do_process_excite_system_event(evt, ctx, app, ptr_sig_gen, ptr_sysid_state);

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

				new_share = ::std::max(real_type(0.01), ::std::min(ref_share*(u(tier_id)+1), (*res_it)->utilization_threshold()));

				ptr_vm->wanted_resource_share((*res_it)->category(), new_share);
				ptr_vm->resource_share((*res_it)->category(), new_share);
			}
		}

		// Reschedule this event
		schedule_excite_system_event();
	}


	//@} Event Handlers


	//@{ Polymorphic Event Handlers


	private: virtual void do_process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app) = 0;


	private: virtual void do_process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app) = 0;


	private: virtual void do_process_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, sysid_state_pointer const& ptr_sysid_state) = 0;


	private: virtual void do_process_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, sysid_state_pointer const& ptr_sysid_state) = 0;


	private: virtual void do_process_tier_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state) = 0;


	private: virtual void do_process_tier_request_service_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state) = 0;


	private: virtual void do_process_tier_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state) = 0;


	private: virtual void do_process_excite_system_event(des_event_type const& evt, des_engine_context_type& ctx, application_type& app, signal_generator_pointer const& ptr_sig_gen, sysid_state_pointer const& ptr_sysid_state) = 0;


	//@} Polymorphic Event Handlers


	private: real_type excite_ts_;
	private: des_event_source_pointer ptr_excite_sys_evt_src_;
}; // base_system_identificator


template <typename TraitsT>
class siso_system_identificator: public base_system_identificator<TraitsT>
{
	private: typedef base_system_identificator<TraitsT> base_type;
	private: typedef siso_system_identificator<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
//	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	private: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
	private: typedef detail::base_signal_generator<real_type> signal_generator_type;
	private: typedef ::dcs::shared_ptr<signal_generator_type> signal_generator_pointer;
	private: typedef detail::sysid_state<traits_type> sysid_state_type;
	private: typedef ::dcs::shared_ptr<sysid_state_type> sysid_state_pointer;
	private: typedef detail::sysid_base_request_info<traits_type> sysid_request_info_type;
	private: typedef detail::sysid_siso_request_info<traits_type> sysid_request_info_impl_type;


	public: siso_system_identificator(real_type excite_ts)
	: base_type(excite_ts)
	{
		// empty
	}


	private: void do_process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// Output the header
		::std::cout << "\"tid\",\"rid\"";
		typedef ::std::vector<typename application_type::reference_physical_resource_type> resource_container;
		typedef typename resource_container::const_iterator resource_iterator;
		resource_container resources(app.reference_resources());
		resource_iterator end_it(resources.end());
		::std::size_t count(0);
		for (resource_iterator it = resources.begin(); it != end_it; ++it)
		{
			::std::cout << ",\"category_" << count << "\",\"share_" << count << "\",\"delta_share_" << count << "\"";
			++count;
		}
		::std::cout << ",\"rt\",\"delta_rt\"" << ::std::endl;
	}


	private: void do_process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );

		// empty
	}


	private: void do_process_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, sysid_state_pointer const& ptr_sysid_state) 
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		// empty
	}


	private: void do_process_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		// empty
	}


	private: void do_process_tier_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		user_request_type req = app.simulation_model().request_state(evt);

		if (!ptr_sysid_state->req_info_maps[tier_id][req.id()])
		{
			ptr_sysid_state->req_info_maps[tier_id][req.id()] = ::dcs::make_shared<sysid_request_info_impl_type>();
		}
		ptr_sysid_state->req_info_maps[tier_id][req.id()]->arr_time = ctx.simulated_time();
	}


	private: void do_process_tier_request_service_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		user_request_type req = app.simulation_model().request_state(evt);

		virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);
		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;
		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator end_it(resource_shares.end());
		for (resource_share_iterator it = resource_shares.begin(); it != end_it; ++it)
		{
			::dcs::shared_ptr<sysid_request_info_type> ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);
			::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info)->share_map[it->first] = it->second;
		}
	}


	private: void do_process_tier_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		user_request_type req = app.simulation_model().request_state(evt);

		::std::cout << tier_id
					<< "," << req.id();

//		virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);
//		typedef virtual_machine_type::resource_share_container resource_share_container;
//		typedef resource_share_container::const_iterator resource_share_iterator;
//		resource_share_container resource_shares(ptr_vm->resource_shares());
//		resource_share_iterator end_it(resource_shares.end());
//		for (resource_share_iterator it = resource_shares.begin(); it != end_it; ++it)
//		{
//			::std::cout << "," << it->first
//						<< "," << it->second
//						<< "," << detail::relative_deviation(it->second, app.tier(tier_id)->resource_share(it->first));
//		}

		::dcs::shared_ptr<sysid_request_info_impl_type> ptr_req_info;
		ptr_req_info = ::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

		DCS_DEBUG_ASSERT( ptr_req_info );


		virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);
		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;
		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator end_it(resource_shares.end());
		for (resource_share_iterator it = resource_shares.begin(); it != end_it; ++it)
		{
			::std::cout << "," << it->first
						<< "," << ptr_req_info->share_map[it->first]
						<< "," << detail::relative_deviation(it->second, app.tier(tier_id)->resource_share(it->first));
		}

//		real_type rt(ctx.simulated_time()-ptr_sysid_state->req_info_maps[tier_id].at(req.id())->arr_time);
		real_type rt(ctx.simulated_time()-ptr_req_info->arr_time);

//FIXME: Keep M/D/1 or return back to M/M/1?
#if 0
		if (tier_id == 0)
		{
			// Treat the first tier as a M/D/1 queue
			::std::cout << "," << rt
						<< "," << detail::relative_deviation(rt, detail::md1_residence_time(0.15, 0.5))
						<< ::std::endl;
::std::cerr << "Reference residence time: " << detail::md1_residence_time(0.15, 0.5) << ::std::endl;//XXX
		}
		else
		{
			// Treat the other tiers as a M/M/1 queue
			::std::cout << "," << rt
						<< "," << detail::relative_deviation(rt, app.performance_model().tier_measure(tier_id, ::dcs::eesim::response_time_performance_measure))
						<< ::std::endl;
		}
#else
		::std::cout << "," << rt
					<< "," << detail::relative_deviation(rt, app.performance_model().tier_measure(tier_id, ::dcs::eesim::response_time_performance_measure))
					<< ::std::endl;
#endif

		ptr_sysid_state->req_info_maps[tier_id].erase(req.id());
	}


	private: void do_process_excite_system_event(des_event_type const& evt, des_engine_context_type& ctx, application_type& app, signal_generator_pointer const& ptr_sig_gen, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sig_gen );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		// empty
	}
}; // siso_system_identificator


template <typename TraitsT>
class miso_system_identificator: public base_system_identificator<TraitsT>
{
	private: typedef base_system_identificator<TraitsT> base_type;
	private: typedef miso_system_identificator<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
//	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	private: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
	private: typedef detail::base_signal_generator<real_type> signal_generator_type;
	private: typedef ::dcs::shared_ptr<signal_generator_type> signal_generator_pointer;
	private: typedef detail::sysid_state<traits_type> sysid_state_type;
	private: typedef ::dcs::shared_ptr<sysid_state_type> sysid_state_pointer;
	private: typedef detail::sysid_base_request_info<traits_type> sysid_request_info_type;
	private: typedef detail::sysid_miso_request_info<traits_type> sysid_request_info_impl_type;


	public: miso_system_identificator(real_type excite_ts)
	: base_type(excite_ts)
	{
		// empty
	}


	private: void do_process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// Output the preamble
		::std::cout << "##" << ::std::endl
					<< "## Application: " << app.name() << ::std::endl
					<< "## Nr. Tiers: " << app.num_tiers() << ::std::endl
					<< "##" << ::std::endl;

		// Output the header
		::std::cout << "\"tid\",\"rid\"";
		typedef ::std::vector<typename application_type::reference_physical_resource_type> resource_container;
		typedef typename resource_container::const_iterator resource_iterator;
		resource_container resources(app.reference_resources());
		resource_iterator end_it(resources.end());
		::std::size_t count(0);
		::std::size_t num_tiers(app.num_tiers());
		for (resource_iterator it = resources.begin(); it != end_it; ++it)
		{
			::std::cout << ",\"category_" << count << "\"";
			for (::std::size_t tier_id = 0; tier_id < num_tiers; ++tier_id)
			{
				::std::cout << ",\"share_" << tier_id << "_" << count << "\",\"delta_share_" << tier_id << "_" << count << "\"";
			}
			++count;
		}
		::std::cout << ",\"rt\",\"delta_rt\"" << ::std::endl;
	}


	private: void do_process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );

		// empty
	}


	private: void do_process_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, sysid_state_pointer const& ptr_sysid_state) 
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		// empty
	}


	private: void do_process_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		// empty
	}


	private: void do_process_tier_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		user_request_type req = app.simulation_model().request_state(evt);

		if (!ptr_sysid_state->req_info_maps[tier_id][req.id()])
		{
			ptr_sysid_state->req_info_maps[tier_id][req.id()] = ::dcs::make_shared<sysid_request_info_impl_type>();
		}
		ptr_sysid_state->req_info_maps[tier_id][req.id()]->arr_time = ctx.simulated_time();
	}


	private: void do_process_tier_request_service_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		user_request_type req = app.simulation_model().request_state(evt);

		::std::size_t num_tiers(app.num_tiers());
		for (::std::size_t t = 0; t < num_tiers; ++t)
		{
			virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(t);
			typedef virtual_machine_type::resource_share_container resource_share_container;
			typedef resource_share_container::const_iterator resource_share_iterator;
			resource_share_container resource_shares(ptr_vm->resource_shares());
			resource_share_iterator end_it(resource_shares.end());
			for (resource_share_iterator it = resource_shares.begin(); it != end_it; ++it)
			{
				::dcs::shared_ptr<sysid_request_info_type> ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);
				::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info)->share_map[it->first][t] = it->second;
			}
		}
	}


	private: void do_process_tier_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		user_request_type req = app.simulation_model().request_state(evt);

		::std::cout << tier_id
					<< "," << req.id();

//		virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);
//		typedef virtual_machine_type::resource_share_container resource_share_container;
//		typedef resource_share_container::const_iterator resource_share_iterator;
//		resource_share_container resource_shares(ptr_vm->resource_shares());
//		resource_share_iterator end_it(resource_shares.end());
//		for (resource_share_iterator it = resource_shares.begin(); it != end_it; ++it)
//		{
//			::std::cout << "," << it->first;
//			for (::std::size_t t = 0; tier_id < num_tiers; ++t)
//			{
//						<< "," << it->second
//						<< "," << detail::relative_deviation(it->second, app.tier(t)->resource_share(it->first));
//			}
//		}

		::dcs::shared_ptr<sysid_request_info_impl_type> ptr_req_info;
		ptr_req_info = ::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

		DCS_DEBUG_ASSERT( ptr_req_info );


		virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);
		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;
		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator end_it(resource_shares.end());
		::std::size_t num_tiers(app.num_tiers());
		for (resource_share_iterator it = resource_shares.begin(); it != end_it; ++it)
		{
			::std::cout << "," << it->first;
			for (::std::size_t t = 0; t < num_tiers; ++t)
			{
				real_type share(0);

				if (ptr_req_info->share_map[it->first].count(t))
				{
					share = ptr_req_info->share_map[it->first].at(t);
				}

				::std::cout << "," << share
							<< "," << detail::relative_deviation(share, app.tier(t)->resource_share(it->first));
			}
		}

//		real_type rt(ctx.simulated_time()-ptr_sysid_state->req_info_maps[tier_id].at(req.id())->arr_time);
		real_type rt(ctx.simulated_time()-ptr_req_info->arr_time);

//FIXME: Keep M/D/1 or return back to M/M/1?
#if 0
		if (tier_id == 0)
		{
			// Treat the first tier as a M/D/1 queue
			::std::cout << "," << rt
						<< "," << detail::relative_deviation(rt, detail::md1_residence_time(0.15, 0.5))
						<< ::std::endl;
::std::cerr << "Reference residence time: " << detail::md1_residence_time(0.15, 0.5) << ::std::endl;//XXX
		}
		else
		{
			// Treat the other tiers as a M/M/1 queue
			::std::cout << "," << rt
						<< "," << detail::relative_deviation(rt, app.performance_model().tier_measure(tier_id, ::dcs::eesim::response_time_performance_measure))
						<< ::std::endl;
		}
#else
		::std::cout << "," << rt
					<< "," << detail::relative_deviation(rt, app.performance_model().tier_measure(tier_id, ::dcs::eesim::response_time_performance_measure))
					<< ::std::endl;
#endif

		ptr_sysid_state->req_info_maps[tier_id].erase(req.id());
	}


	private: void do_process_excite_system_event(des_event_type const& evt, des_engine_context_type& ctx, application_type& app, signal_generator_pointer const& ptr_sig_gen, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sig_gen );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		// empty
	}
}; // miso_system_identificator


int main(int argc, char* argv[])
{
	typedef dcs::shared_ptr<des_engine_type> des_engine_pointer;
	typedef dcs::shared_ptr<random_generator_type> random_generator_pointer;
	typedef dcs::shared_ptr< dcs::eesim::data_center<traits_type> > data_center_pointer;
	typedef dcs::shared_ptr< dcs::eesim::data_center_manager<traits_type> > data_center_manager_pointer;
	typedef dcs::eesim::config::configuration<real_type,uint_type> configuration_type;
	typedef base_system_identificator<traits_type> system_identificator_type;
	//typedef siso_system_identificator<traits_type> system_identificator_type;
	//typedef miso_system_identificator<traits_type> system_identificator_type;
	typedef configuration_type::data_center_config_type data_center_config_type;
	typedef data_center_config_type::application_config_container::const_iterator app_iterator;
//	typedef dcs::eesim::multi_tier_application<traits_type> application_type;
	typedef dcs::eesim::physical_machine<traits_type> physical_machine_type;
	typedef dcs::shared_ptr<physical_machine_type> physical_machine_pointer;
	typedef dcs::eesim::physical_resource<traits_type> physical_resource_type;
	typedef dcs::shared_ptr<physical_resource_type> physical_resource_pointer;
	typedef application_type::reference_physical_resource_type reference_resource_type;


	prog_name = argv[0];

	if (argc < 6)
	{
		detail::usage();
		return -1;
	}


	// Parse command line arguments

	real_type excite_sampling_time;
	uint_type num_samples;
	signal_category sig_category;
	system_identification_category sysid_category;
	std::string conf_fname;

	try
	{
		excite_sampling_time = detail::get_option<real_type>(argv, argv+argc, "--ts");
		num_samples = detail::get_option<uint_type>(argv, argv+argc, "--ns");
		sysid_category = detail::parse_system_identification_category(detail::get_option<std::string>(argv, argv+argc, "--sys"));
		sig_category = detail::parse_signal_category(detail::get_option<std::string>(argv, argv+argc, "--sig"));
		conf_fname = detail::get_option<std::string>(argv, argv+argc, "--conf");
	}
	catch (std::exception const& e)
	{
		std::cerr << "[Error] Error while parsing command-line options: " << e.what() << std::endl;
		std::abort();
	}

	// Read configuration

	configuration_category conf_cat = yaml_configuration;

	dcs::eesim::config::configuration<real_type,uint_type> conf;

	try
	{
		switch (conf_cat)
		{
			case yaml_configuration:

				conf = dcs::eesim::config::read_file(
					conf_fname,
					::dcs::eesim::config::yaml_reader<real_type,uint_type>()
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

	DCS_DEBUG_TRACE("Configuration: " << conf); //XXX

	// Build the registry

	registry_type& reg(registry_type::instance());
//	des_engine_pointer ptr_des_eng;
//	ptr_des_eng = detail::make_des_engine<traits_type>();
//	reg.des_engine(ptr_des_eng);
//	random_generator_pointer ptr_rng;
//	ptr_rng = dcs::eesim::config::make_random_number_generator(conf);
//	reg.uniform_random_generator(ptr_rng);

//	// Build the Data Center
//	data_center_pointer ptr_dc;
//	data_center_manager_pointer ptr_dc_mngr;
//	ptr_dc = dcs::eesim::config::make_data_center<traits_type>(conf, ptr_rng, ptr_des_eng);
//	ptr_dc_mngr = dcs::eesim::config::make_data_center_manager<traits_type>(conf, ptr_dc);

//	excite_sampling_time = 5;
//	//sig_category = step_signal;
//	//sig_category = gaussian_white_noise_signal;
//	sig_category = sinusoidal_signal;
//	sysid_category = siso_system_identification;
//	//sysid_category = miso_system_identification;

	app_iterator app_end_it = conf.data_center().applications().end();
	for (app_iterator app_it = conf.data_center().applications().begin(); app_it != app_end_it; ++app_it)
	{
		dcs::shared_ptr<application_type> ptr_app;
		dcs::shared_ptr< detail::base_signal_generator<real_type> > ptr_sig_gen;

		// Build the simulator
		des_engine_pointer ptr_des_eng;
		ptr_des_eng = detail::make_des_engine<traits_type>();
		reg.des_engine(ptr_des_eng);

		// Build the random number generator
		random_generator_pointer ptr_rng;
		ptr_rng = dcs::eesim::config::make_random_number_generator(conf);
		reg.uniform_random_generator(ptr_rng);

		// Build the application
		ptr_app = dcs::eesim::config::make_multi_tier_application<traits_type>(*app_it, conf, ptr_rng, ptr_des_eng);

		// Build the signal generator
		switch (sig_category)
		{
			case step_signal:
				ptr_sig_gen = dcs::make_shared< detail::step_signal_generator<real_type> >(ublas::scalar_vector<real_type>(ptr_app->num_tiers(), 0));
				break;
			case gaussian_white_noise_signal:
				ptr_sig_gen = dcs::make_shared< detail::gaussian_signal_generator<real_type> >(
								ublas::zero_vector<real_type>(ptr_app->num_tiers()),
								ublas::scalar_vector<real_type>(ptr_app->num_tiers(), 1)
					);
				break;
			case sinusoidal_signal:
				ptr_sig_gen = dcs::make_shared< detail::sinusoidal_signal_generator<real_type> >(
								ublas::scalar_vector<real_type>(ptr_app->num_tiers(), 1),
								ublas::scalar_vector<real_type>(ptr_app->num_tiers(), 1)
					);
				break;
		}

		::dcs::shared_ptr<system_identificator_type> ptr_sysid;
		switch (sysid_category)
		{
			case siso_system_identification:
				ptr_sysid = ::dcs::make_shared< siso_system_identificator<traits_type> >(excite_sampling_time);
				break;
			case miso_system_identification:
				ptr_sysid = ::dcs::make_shared< miso_system_identificator<traits_type> >(excite_sampling_time);
				break;
		}

//		sysid->des_engine(ptr_des_eng);
//		sysid->uniform_random_generator(ptr_rng);
		ptr_sysid->identify(*ptr_app, ptr_sig_gen, num_samples);

		//// Report statistics
		//detail::report_stats(::std::cout, ptr_dc);
	}
}
