#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <dcs/math/stats/distributions.hpp>
#include <dcs/math/random/mersenne_twister.hpp>
#include <dcs/des/replications/engine.hpp>
#include <dcs/des/cloud/registry.hpp>
#include <dcs/des/cloud/traits.hpp>
#include <dcs/des/cloud/workload.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <iostream>


typedef double real_type;
typedef unsigned long uint_type;
typedef long int_type;
typedef dcs::des::replications::engine<real_type,uint_type> des_engine_type;
typedef dcs::math::random::mt19937 random_generator_type;
typedef dcs::des::cloud::traits<
            des_engine_type,
            random_generator_type,
            real_type,
            uint_type,
            int_type
        > traits_type;
typedef dcs::des::cloud::registry<traits_type> registry_type;
typedef dcs::shared_ptr<des_engine_type> des_engine_pointer;
typedef dcs::shared_ptr<random_generator_type> random_generator_pointer;
//typedef ::dcs::math::random::minstd_rand1 random_seeder_type;
typedef typename dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
typedef typename dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;


const real_type min_replication_len(5.3);
const real_type min_num_replications(1);
const uint_type seed(5489);


namespace detail { namespace /*<unnamed>*/ {

template <typename WorkloadT>
class system
{
	public: typedef WorkloadT workload_type;
	private: typedef system self_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	private: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;


	public: system(workload_type const& workload, random_generator_pointer const& ptr_rng)
	: wkl_(workload),
	  ptr_evt_src_(new des_event_source_type("System Event")),
	  ptr_rng_(ptr_rng)
	{
	}

	public: ~system()
	{
	}


	public: void start(des_engine_context_type& ctx)
	{
		ptr_evt_src_->connect(
				::dcs::functional::bind(
					&self_type::process_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);

		schedule_event(ctx);
	}

	public: void stop(des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ctx);
	}

	private: void schedule_event(des_engine_context_type& ctx)
	{
		real_type time = wkl_.rand(*ptr_rng_);

		::std::cerr << "New Time: " << time << " (Clock: " << ctx.simulated_time() << ")" << ::std::endl;

		ctx.schedule_event(
				ptr_evt_src_,
				ctx.simulated_time()+time
			);
	}


	private: void process_event(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(evt);

		schedule_event(ctx);
	}


	private: workload_type wkl_;
	private: des_event_source_pointer ptr_evt_src_;
	private: random_generator_pointer ptr_rng_;
};

template <typename WorkloadT>
void process_sys_init(des_event_type const& evt, des_engine_context_type& ctx, ::dcs::shared_ptr<system<WorkloadT> > const& ptr_sys)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(evt);

	ptr_sys->start(ctx);
}

template <typename WorkloadT>
void process_sys_finit(des_event_type const& evt, des_engine_context_type& ctx, ::dcs::shared_ptr<system<WorkloadT> > const& ptr_sys)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(evt);

	ptr_sys->stop(ctx);
}


inline
::dcs::des::cloud::timed_step_workload_model<traits_type,real_type> make_timed_step_workload()
{
	::dcs::des::cloud::timed_step_workload_model<traits_type,real_type> workload;

	workload.add_phase(0, ::dcs::math::stats::exponential_distribution<real_type>(0.6));
	workload.add_phase(min_replication_len/3.0, ::dcs::math::stats::exponential_distribution<real_type>(1.0));
	workload.add_phase(2.0*min_replication_len/3.0, ::dcs::math::stats::exponential_distribution<real_type>(1.6));

	return workload;
}


inline
::dcs::des::cloud::mmpp_interarrivals_workload_model<traits_type,real_type> make_mmpp_interarrivals_workload()
{
	const ::std::size_t n(2);

	::boost::numeric::ublas::matrix<real_type> Q(n,n);
	Q(0,0) = -0.2; Q(0,1) =  0.2;
	Q(1,0) =  0.1; Q(1,1) = -0.1;

	::boost::numeric::ublas::vector<real_type> lambda(n);
	lambda(0) = 5;
	lambda(1) = 1;

	::boost::numeric::ublas::vector<real_type> p0(n);
	p0(0) = 0;
	p0(1) = 1;

	return ::dcs::des::cloud::mmpp_interarrivals_workload_model<traits_type,real_type>(lambda, Q, p0);
}


template <typename WorkloadT>
void simulate_system(WorkloadT const& workload)
{
	typedef detail::system<WorkloadT> system_type;

	registry_type& reg = registry_type::instance();

	des_engine_pointer ptr_eng = dcs::make_shared<des_engine_type>(min_replication_len, min_num_replications);
	reg.des_engine(ptr_eng);
	random_generator_pointer ptr_rng = dcs::make_shared<random_generator_type>(seed);
	reg.uniform_random_generator(ptr_rng);

	::dcs::shared_ptr<system_type> ptr_sys = ::dcs::make_shared<system_type>(workload, ptr_rng);

	ptr_eng->system_initialization_event_source().connect(
			::dcs::functional::bind(
				&detail::process_sys_init<WorkloadT>,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2,
				ptr_sys
			)
		);
	ptr_eng->system_finalization_event_source().connect(
			::dcs::functional::bind(
				&detail::process_sys_finit<WorkloadT>,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2,
				ptr_sys
			)
		);

	ptr_eng->run();

/*
	ptr_eng->system_initialization_event_source().disconnect(
			::dcs::functional::bind(
				&detail::process_sys_init,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2,
				ptr_sys
			)
		);
	ptr_eng->system_finalization_event_source().disconnect(
			::dcs::functional::bind(
				&detail::process_sys_finit,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2,
				ptr_sys
			)
		);
*/
}


}} // Namespace detail::<unnamed>


int main()
{
	//detail::simulate_system(detail::make_timed_step_workload());

	detail::simulate_system(detail::make_mmpp_interarrivals_workload());
}
