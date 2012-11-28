#ifndef DCS_DES_CLOUD_WORKLOAD_TIMED_STEP_HPP
#define DCS_DES_CLOUD_WORKLOAD_TIMED_STEP_HPP


#include <dcs/assert.hpp>
#include <dcs/des/cloud/registry.hpp>
#include <dcs/math/stats/distribution/any_distribution.hpp>
#include <dcs/math/stats/function/rand.hpp>
#include <iterator>
#include <list>
#include <sstream>
#include <stdexcept>
#include <utility>


namespace dcs { namespace des { namespace cloud {

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
	private: typedef typename phase_container::const_iterator phase_const_iterator;


	public: timed_step_workload_model()
	: phases_(),
	  cur_phase_it_(phases_.begin()),
	  cur_phase_duration_(0),
	  move_up_(true)
	{
	}


	public: template <typename ForwardIterT>
		timed_step_workload_model(ForwardIterT first, ForwardIterT last)
	: phases_(first, last),
	  cur_phase_it_(phases_.begin()),
	  cur_phase_duration_(0),
	  move_up_(true)
	{
	}


	public: timed_step_workload_model(timed_step_workload_model const& that)
	: phases_(that.phases_.begin(), that.phases_.end()),
	  cur_phase_it_(phases_.begin()),
	  cur_phase_duration_(0),
	  move_up_(that.move_up_)
	{
	}


	public: timed_step_workload_model const& operator=(timed_step_workload_model const& rhs)
	{
		if (&rhs != this)
		{
			phases_ = phase_container(rhs.phases_.begin(), rhs.phases_.end()),
			cur_phase_it_ = ::std::advance(rhs.phases_.begin(), ::std::distance(rhs.cur_phase_it_, rhs.phases_.begin()));
			cur_phase_duration_ = rhs.cur_phase_duration_;
			move_up_ = rhs.move_up_;
		}

		return *this;
	}


	public: template <typename ForwardIterT>
		void phase(ForwardIterT first, ForwardIterT last)
	{
		phases_.clear();
		while (first != last)
		{
			add_phase(first->first, first->second);
		}
	}


//	public: template <typename DistributionT>
//		void add_phase(real_type start_time, DistributionT distr)
//	{
//		typedef typename phase_container::iterator iterator;
//
//		iterator end(phases_.end());
//		iterator it(phases_.begin());
//		while (it != end && it->first < start_time)
//		{
//			++it;
//		}
//
//		phase_type phase = ::std::make_pair(duration, ::dcs::math::stats::make_any_distribution(distr));
//		if (it != end && it->first == start_time)
//		{
//			// update
//			*it = phase;
//		}
//		else
//		{
//			// insert
//			phases_.insert(it, phase);
//		}
//	}


	public: template <typename DistributionT>
		void add_phase(real_type duration, DistributionT distr)
	{
		phases_.push_back(::std::make_pair(duration, ::dcs::math::stats::make_any_distribution(distr)));
		cur_phase_it_ = phases_.begin();
	}


//	public: template <typename UniformRandomGeneratorT>
//		value_type rand(UniformRandomGeneratorT& rng) const
//	{
//		typedef typename phase_container::const_iterator iterator;
//		typedef registry<traits_type> registry_type;
//		typedef typename traits_type::des_engine_type des_engine_type;
//
//		des_engine_type const& eng(registry<traits_type>::instance().des_engine());
//		real_type cur_time(eng.simulated_time());
//
//		iterator it(phases_.begin());
//		iterator end(phases_.end());
//		while (it != end && it->first <= cur_time)
//		{
//			++it;
//		}
//
//		if (it == phases_.begin())
//		{
//			::std::ostringstream oss;
//			oss << "[dcs::des::cloud::timed_step_workload_model::rand] Unable to find a phase for time: " << cur_time << ".";
//			throw ::std::runtime_error(oss.str());
//		}
//
//		--it;
//
//		return ::dcs::math::stats::rand(it->second, rng);
//	}


	public: template <typename UniformRandomGeneratorT>
		value_type rand(UniformRandomGeneratorT& rng) const
	{
		// pre: one or more phase must exist
		DCS_ASSERT(
			!phases_.empty(),
			throw ::std::runtime_error("[dcs::des::cloud::timed_step_workload_model::rand] No phase defined.")
		);

		if (cur_phase_duration_ > 0 && cur_phase_duration_ >= cur_phase_it_->first)
		{
			cur_phase_duration_ = 0;
			if (move_up_)
			{
				if (::std::distance(cur_phase_it_, phases_.end()) > 1)
				{
					++cur_phase_it_;
				}
				else
				{
					move_up_ = false;
					--cur_phase_it_;
				}
			}
			else
			{
				if (::std::distance(phases_.begin(), cur_phase_it_) > 0)
				{
					--cur_phase_it_;
				}
				else
				{
					move_up_ = true;
					++cur_phase_it_;
				}
			}
		}

		real_type iatime(0);
		while ((iatime = ::dcs::math::stats::rand(cur_phase_it_->second, rng)) <= 0)
		{
			;
		}

		cur_phase_duration_ += iatime;

		return iatime;
	}


	private: phase_container phases_;
	private: mutable phase_const_iterator cur_phase_it_;
	private: mutable real_type cur_phase_duration_;
	private: mutable bool move_up_;
};

}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_WORKLOAD_TIMED_STEP_HPP
