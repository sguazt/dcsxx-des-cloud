#ifndef DCS_EESIM_WORKLOAD_TIMED_STEP_HPP
#define DCS_EESIM_WORKLOAD_TIMED_STEP_HPP


#include <dcs/eesim/registry.hpp>
#include <dcs/math/stats/distribution/any_distribution.hpp>
#include <dcs/math/stats/function/rand.hpp>
#include <list>
#include <sstream>
#include <stdexcept>
#include <utility>


namespace dcs { namespace eesim {

template <typename TraitsT, typename ValueT>
class timed_step_workload_model
{
	public: typedef TraitsT traits_type;
	public: typedef ValueT value_type;
	public: typedef typename traits_type::uint_type uint_type;
	private: typedef typename traits_type::real_type real_type;
	private: typedef ::dcs::math::stats::any_distribution<value_type> distribution_type;
	private: typedef ::std::pair<real_type,distribution_type> phase_type;
	private: typedef ::std::list<phase_type> phase_container;


	public: timed_step_workload_model()
	{
	}


	public: template <typename ForwardIterT>
		timed_step_workload_model(ForwardIterT first, ForwardIterT last)
	{
		phases(first, last);
	}


	public: template <typename ForwardIterT>
		void phase(ForwardIterT first, ForwardIterT last)
	{
::std::cerr << "ADD_PHASES.1" << ::std::endl;//XXX
		phases_.clear();
		while (first != last)
		{
::std::cerr << "ADD_PHASES.2" << ::std::endl;//XXX
			add_phase(first->first, first->second);
::std::cerr << "ADD_PHASES.3" << ::std::endl;//XXX
		}
::std::cerr << "ADD_PHASES.4" << ::std::endl;//XXX
	}


	public: template <typename DistributionT>
		void add_phase(real_type start_time, DistributionT distr)
	{
		typedef typename phase_container::iterator iterator;

		iterator end(phases_.end());
		iterator it(phases_.begin());
		while (it != end && it->first < start_time)
		{
			++it;
		}

		phase_type phase = ::std::make_pair(start_time, ::dcs::math::stats::make_any_distribution(distr));
		if (it != end && it->first == start_time)
		{
			// update
			*it = phase;
		}
		else
		{
			// insert
			phases_.insert(it, phase);
		}
	}


	public: template <typename UniformRandomGeneratorT>
		value_type rand(UniformRandomGeneratorT& rng) const
	{
		typedef typename phase_container::const_iterator iterator;
		typedef registry<traits_type> registry_type;
		typedef typename traits_type::des_engine_type des_engine_type;

		des_engine_type const& eng(registry<traits_type>::instance().des_engine());
		real_type cur_time(eng.simulated_time());

		iterator it(phases_.begin());
		iterator end(phases_.end());
		while (it != end && it->first <= cur_time)
		{
			++it;
		}

		if (it == phases_.begin())
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::timed_step_workload_model::rand] Unable to find a phase for time: " << cur_time << ".";
			throw ::std::runtime_error(oss.str());
		}

		--it;

		return ::dcs::math::stats::rand(it->second, rng);
	}


	private: phase_container phases_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_WORKLOAD_TIMED_STEP_HPP
