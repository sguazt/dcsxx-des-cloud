#include <algorithm>
//#if defined(OFFSYSID_EXP_AGGREGATE_MEASURES)
//# include <boost/accumulators/accumulators.hpp>
//# include <boost/accumulators/statistics/stats.hpp>
//# if defined(OFFSYSID_EXP_AGGREGATE_MEASURES_BY_SIMPLE_MEAN)
//#  include <boost/accumulators/statistics/mean.hpp>
//# elif defined(OFFSYSID_EXP_AGGREGATE_MEASURES_BY_WEIGHTED_MEAN)
//#  include <boost/accumulators/statistics/weighted_mean.hpp>
//# endif // OFFSYSID_EXP_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
//#endif // OFFSYSID_EXP_AGGREGATE_MEASURES
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
	gaussian_white_noise_signal,
	ramp_signal,
	sinusoidal_signal,
	step_signal
};


enum system_identification_category
{
	siso_system_identification,
	miso_system_identification
};


enum filter_category
{
	none_filter_category,
	average_filter_category,
	ewma_filter_category
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
				<< "  --conf <configuration-file>" << ::std::endl
				<< "    Path to the configuration file." << ::std::endl
				<< "  --infilt {'none'|'mean'}" << ::std::endl
				<< "    The type of filter to be applied on input data." << ::std::endl
				<< "  --ns <number-of-samples>" << ::std::endl
				<< "    The number of samples to collect." << ::std::endl
				<< "  --outfilt {'none'|'mean'|'ewma'}" << ::std::endl
				<< "    The type of filter to be applied on output data." << ::std::endl
				<< "  --sys {'siso'|'miso'}" << ::std::endl
				<< "    The type of identification that is to be performed." << ::std::endl
				<< "  --sig {'gaussian'|'ramp'|'sine'|'step'}" << ::std::endl
				<< "    The shape of the input signal used to excite the target system." << ::std::endl
				<< "  --ts <sampling-time>" << ::std::endl
				<< "    The sampling time used to vary the input signal." << ::std::endl
				<< "  --help" << ::std::endl
				<< "    Show this message." << ::std::endl;
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
	if (!str.compare("gaussian"))
	{
		return gaussian_white_noise_signal;
	}
	if (!str.compare("ramp"))
	{
		return ramp_signal;
	}
	if (!str.compare("sine"))
	{
		return sinusoidal_signal;
	}
	if (!str.compare("step"))
	{
		return step_signal;
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


filter_category parse_input_filter_category(::std::string const& str)
{
	if (!str.compare("none"))
	{
		return none_filter_category;
	}
	if (!str.compare("average"))
	{
		return average_filter_category;
	}

	throw ::std::invalid_argument("[detail::parse_input_filter_category] Cannot find a valid input filter category.");
}


filter_category parse_output_filter_category(::std::string const& str)
{
	if (!str.compare("none"))
	{
		return none_filter_category;
	}
	if (!str.compare("ewma"))
	{
		return ewma_filter_category;
	}
	if (!str.compare("average"))
	{
		return average_filter_category;
	}

	throw ::std::invalid_argument("[detail::parse_output_filter_category] Cannot find a valid output filter category.");
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


	public: void reset()
	{
		do_reset();
	}


	public: virtual ~base_signal_generator()
	{
		// empty
	}


	private: virtual vector_type do_generate() = 0;

	private: virtual void do_reset() = 0;
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


	private: void do_reset()
	{
		// do nothing: the signal is constant.
	}


	private: vector_type u_;
};

template <typename ValueT>
class ramp_signal_generator: public base_signal_generator<ValueT>
{
	private: typedef base_signal_generator<ValueT> base_type;
	public: typedef ValueT value_type;
	public: typedef typename base_type::vector_type vector_type;


	public: ramp_signal_generator(vector_type const& u0, vector_type const& incr)
	: u0_(u0),
	  u_(u0),
	  h_(incr)
	{
	}

 
	private: vector_type do_generate()
	{
		u_ += h_;
		return u_;
	}


	private: void do_reset()
	{
		u_ = u0_;
	}


	private: vector_type u0_;
	private: vector_type u_;
	private: vector_type h_;
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


	private: void do_reset()
	{
		// do nothing: the generator is reset by resetting the random number generator, which should be made elsewhere.
	}


	private: normal_distribution_container distrs_;
};


/**
 * \brief Generate a sinusoidal wave according to the sample-based mode.
 *
 * Sample-based mode uses the following formula to compute the output of the sine wave:
 * \f[
 *   y = A\sin(2\pi(k+o)/p) + b
 * \f]
 * where
 * - A is the amplitude of the sine wave.
 * - p is the number of time samples per sine wave period.
 * - k is a repeating integer value that ranges from 0 to p–1.
 * - o is the offset (phase shift) of the signal.
 * - b is the signal bias.
 * .
 */
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
	  h_(ublas::scalar_vector<value_type>(ublasx::size(a),0.1)),
	  x_min_(0),
	  x_max_(1),
	  x0_(ublas::zero_vector<value_type>(ublasx::size(a))),
	  x_(x0_)
	{
		// pre: size(a) == size(f) == ...
	}


	public: sinusoidal_signal_generator(vector_type a, vector_type f, vector_type p, vector_type d)
	: a_(a),
	  f_(f),
	  p_(p),
	  d_(d),
	  w_(::dcs::math::constants::double_pi<value_type>::value*f),
	  h_(ublas::scalar_vector<value_type>(ublasx::size(a),0.1)),
	  x_min_(-1),
	  x_max_(1),
	  x0_(ublas::zero_vector<value_type>(ublasx::size(a))),
	  x_(x0_)
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

			DCS_DEBUG_TRACE("Generated: x(" << i << ")=" << x_(i) << " ==> u(" << i << ")=" << u(i));//XXX

			x_(i) += h_(i);
			if (x_(i) < x_min_ || x_(i) > x_max_)
			{
				x_(i) = x_min_;
			}
		}

		return u;
	}


	private: void do_reset()
	{
		x_ = x0_;
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
	/// The increment
	private: vector_type h_;
	private: value_type x_min_;
	private: value_type x_max_;
	private: vector_type x0_;
	private: vector_type x_;
}; // sinusoidal_signal_generator

//@} Signal generators


/*
//@{ Filter

template <typename ValueT>
class base_filter
{
	public: typedef ValueT value_type;


	public: virtual ~base_filter()
	{
		// empty
	}


	public: filter_category category() const
	{
		return do_category();
	}


	public: void operator()(value_type x)
	{
		do_collect(x);
	}


	public: value_type apply()
	{
		return do_apply();
	}


	public: void reset()
	{
		do_reset();
	}


	private: virtual filter_category do_category() const = 0;


	private: virtual void do_collect(value_type x) = 0;


	private: virtual value_type do_apply() = 0;


	private: virtual void do_reset() = 0;
}; // base_filter


template <typename ValueT>
class none_filter
{
	public: typedef ValueT value_type;


	public: none_filter()
	: x_(0)
	{
	}


	private: filter_category do_category() const
	{
		return none_filter_category;
	}


	private: void do_collect(value_type x)
	{
		x_ = x;
	}


	private: value_type do_apply()
	{
		return x_;
	}


	private: void do_reset()
	{
	}


	private: value_type x_;
}; // none_filter


template <typename ValueT, typename SizeT=::std::size_t>
class avg_filter: public base_filter<ValueT>
{
	public: typedef ValueT value_type;
	public: typedef SizeT size_type;


	public: avg_filter()
	: m_(0),
	  c_(0)
	{
	}


	private: filter_category do_category() const
	{
		return average_filter_category;
	}


	private: void do_collect(value_type x)
	{
		++c_;
		m_ += (x-m_)/c_;
	}


	private: value_type do_apply()
	{
		return m_;
	}


	private: void do_reset()
	{
		m_ = value_type();
		c_ = size_type();
	}


	private: value_type m_;
	private: size_type c_;
}; // avg_filter

//@} Filter
*/

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
	bool done;
};


template <typename TraitsT>
struct sysid_siso_request_info: public sysid_base_request_info<TraitsT>
{
	typedef TraitsT traits_type;
	typedef typename traits_type::uint_type uint_type;
	typedef typename traits_type::real_type real_type;
	typedef ::dcs::eesim::physical_resource_category resource_category_type;
	typedef ::std::map<resource_category_type,real_type> resource_share_map;

	resource_share_map share_map;
//[EXP-20110609]
#if defined(OFFSYSID_EXP_AGGREGATE_SHARES)
# if defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
	uint_type share_count;
# elif defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
	real_type last_share_change_time;
	real_type weigths_sum;
	resource_share_map old_share_map;
# endif // OFFSYSID_EXP_AGGREGATE_SHARES_BY_WEIGHTED_MEAN
#endif // OFFSYSID_EXP_AGGREGATE_SHARES
//[EXP-20110609]
};


template <typename TraitsT>
struct sysid_miso_request_info: public sysid_base_request_info<TraitsT>
{
	typedef TraitsT traits_type;
	typedef typename traits_type::uint_type uint_type;
	typedef typename traits_type::real_type real_type;
	typedef ::dcs::eesim::physical_resource_category resource_category_type;
	typedef ::std::map<resource_category_type, ::std::map<uint_type,real_type> > resource_share_map;

	resource_share_map share_map;
	uint_type share_count;
};


template <typename TraitsT>
struct sysid_state
{
	typedef TraitsT traits_type;
	typedef typename traits_type::uint_type uint_type;
	typedef sysid_base_request_info<traits_type> request_info_type;
	typedef ::dcs::shared_ptr<request_info_type> request_info_pointer;
	typedef ::std::map<uint_type,request_info_pointer> request_info_map;
	typedef ::std::vector<request_info_map> request_info_map_container;

	uint_type num_arrs;
	uint_type num_deps;
	uint_type max_num_deps;
	request_info_map_container req_info_maps; // [tier_id][req_id => request_info*]
};


template <typename ValueT, typename SizeT=::std::size_t>
class mean_statistic
{
	public: typedef ValueT value_type;
	public: typedef SizeT size_type;


	public: mean_statistic()
	: m_(0),
	  n_(0)
	{
	}


	public: void operator()(value_type x)
	{
		m_ += (x-m_)/static_cast<value_type>(n_);
	}


	public: value_type estimate() const
	{
		return m_;
	}


	public: size_type size() const
	{
		return n_;
	}

	private: value_type m_;
	private: size_type n_;
};


template <typename ValueT, typename SizeT=::std::size_t>
class weighted_mean_statistic
{
	public: typedef ValueT value_type;
	public: typedef SizeT size_type;


	public: weighted_mean_statistic()
	: m_(0),
	  sumw_(0),
	  n_(0)
	{
	}


	public: void operator()(value_type x, value_type w)
	{
		++n_;
		value_type q(x-m_);
		sumw_ += w;
		m_ += q*w/sumw_;
	}


	public: value_type estimate() const
	{
		return m_;
	}


	public: size_type size() const
	{
		return n_;
	}


	private: value_type m_;
	private: value_type sumw_;
	private: size_type n_;
};


template <typename ValueT, typename SizeT>
void update_mean(ValueT x, SizeT n, ValueT& m)
{
	typedef ValueT value_type;

	m += (x-m)/static_cast<value_type>(n);
}

template <typename ValueT>
void update_weigthed_mean(ValueT x, ValueT w, ValueT sumw, ValueT& m)
{
	typedef ValueT value_type;

	value_type q(x-m);
	sumw += w;
	value_type r(q*w/sumw);
	m += r;
}

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
	protected: typedef ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	protected: typedef ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	protected: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
	private: typedef detail::base_signal_generator<real_type> signal_generator_type;
	protected: typedef ::dcs::shared_ptr<signal_generator_type> signal_generator_pointer;
	private: typedef detail::sysid_state<traits_type> sysid_state_type;
	protected: typedef ::dcs::shared_ptr<sysid_state_type> sysid_state_pointer;


	private: static const real_type min_share;


	public: base_system_identificator(real_type excite_ts)
	: excite_ts_(excite_ts),
	  ptr_excite_sys_evt_src_(new des_event_source_type())
	{
	}


	public: real_type sampling_time() const
	{
		return excite_ts_;
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
		ptr_sysid_state->req_info_maps.resize(num_tiers+1); // num_tiers position for each tier + 1 position for the whole app
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

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_vm );

			ptr_vm->id(vms.size());
			ptr_vm->guest_system(app.tier(tier_id));
			app.simulation_model().tier_virtual_machine(ptr_vm);
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
				share *= 0.5; // initial share is set to middle-capacity

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
					app,
					ptr_sysid_state
				)
			);
		app.simulation_model().request_departure_event_source().connect(
				::dcs::functional::bind(
					&self_type::process_request_departure_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2,
					app,
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

		// Deregister some global DES event hooks
		app.simulation_model().request_arrival_event_source().disconnect(
				::dcs::functional::bind(
					&self_type::process_request_arrival_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2,
					app,
					ptr_sysid_state
				)
			);
		app.simulation_model().request_departure_event_source().disconnect(
				::dcs::functional::bind(
					&self_type::process_request_departure_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2,
					app,
					ptr_sysid_state
				)
			);

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


	private: void process_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_DEBUG_TRACE("BEGIN Process REQUEST-ARRIVAL (Clock: " << ctx.simulated_time() << ")");

		ptr_sysid_state->num_arrs += 1;

		do_process_request_arrival_event(evt, ctx, app, ptr_sysid_state);

		DCS_DEBUG_TRACE("END Process REQUEST-ARRIVAL (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_DEBUG_TRACE("BEGIN Process REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");

		ptr_sysid_state->num_deps += 1;

		do_process_request_departure_event(evt, ctx, app, ptr_sysid_state);

		if (ptr_sysid_state->num_deps == ptr_sysid_state->max_num_deps)
		{
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

//::std::cerr << "BEGIN process-excite_system_event (Clock: " << ctx.simulated_time() << ::std::endl;//XXX
		size_type num_tiers(app.num_tiers());

		ublas::vector<real_type> u((*ptr_sig_gen)());
		for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_vm );

			typedef std::vector<physical_resource_pointer> resource_container;
			typedef typename resource_container::const_iterator resource_iterator;
			resource_container resources(ptr_vm->vmm().hosting_machine().resources());
			resource_iterator res_end_it(resources.end());

//			ublas::vector<real_type> u((*ptr_sig_gen)());
			for (resource_iterator res_it = resources.begin(); res_it != res_end_it; ++res_it)
			{
				real_type ref_share(app.tier(tier_id)->resource_share((*res_it)->category()));
				real_type new_share;

				new_share = ::std::max(real_type(min_share), ::std::min(0.5*ref_share*(u(tier_id)+1), (*res_it)->utilization_threshold()));
				//new_share = ::std::max(real_type(min_share), ::std::min(u(tier_id), (*res_it)->utilization_threshold()));

//::std::cerr << "process-excite_system_event>> tier: " << tier_id << " - old share: " << ptr_vm->resource_share((*res_it)->category()) << " - new share: " << new_share << ::std::endl;//XXX
				ptr_vm->wanted_resource_share((*res_it)->category(), new_share);
				ptr_vm->resource_share((*res_it)->category(), new_share);
			}
		}

		do_process_excite_system_event(evt, ctx, app, ptr_sig_gen, ptr_sysid_state);

		// Reschedule this event
		schedule_excite_system_event();
//::std::cerr << "END process-excite_system_event (Clock: " << ctx.simulated_time() << ::std::endl;//XXX
	}


	//@} Event Handlers


	//@{ Polymorphic Event Handlers


	private: virtual void do_process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app) = 0;


	private: virtual void do_process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app) = 0;


	private: virtual void do_process_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state) = 0;


	private: virtual void do_process_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state) = 0;


	private: virtual void do_process_tier_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state) = 0;


	private: virtual void do_process_tier_request_service_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state) = 0;


	private: virtual void do_process_tier_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state) = 0;


	private: virtual void do_process_excite_system_event(des_event_type const& evt, des_engine_context_type& ctx, application_type& app, signal_generator_pointer const& ptr_sig_gen, sysid_state_pointer const& ptr_sysid_state) = 0;


	//@} Polymorphic Event Handlers


	private: real_type excite_ts_;
	private: des_event_source_pointer ptr_excite_sys_evt_src_;
}; // base_system_identificator


template <typename TraitsT>
const typename TraitsT::real_type base_system_identificator<TraitsT>::min_share = 0.01;


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
	private: typedef ::dcs::shared_ptr<sysid_request_info_type> sysid_request_info_pointer;
	private: typedef ::dcs::shared_ptr<sysid_request_info_impl_type> sysid_request_info_impl_pointer;
#if defined(OFFSYSID_EXP_AGGREGATE_MEASURES)
# if defined(OFFSYSID_EXP_AGGREGATE_MEASURES_BY_SIMPLE_MEAN)
	private: typedef detail::mean_statistic<real_type> measure_statistic_type;
# elif defined(OFFSYSID_EXP_AGGREGATE_MEASURES_BY_WEIGHTED_MEAN)
	private: typedef detail::weighted_mean_statistic<real_type> measure_statistic_type;
# endif // OFFSYSID_EXP_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
	private: typedef ::std::vector<measure_statistic_type> measure_statistic_container;
	private: typedef ::std::vector<real_type> measure_container;
# if defined(OFFSYSID_EXP_AGGREGATE_SHARES)
#  if defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
	private: typedef detail::mean_statistic<real_type> share_statistic_type;
#  elif defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
	private: typedef detail::weighted_mean_statistic<real_type> share_statistic_type;
#  endif // OFFSYSID_EXP_AGGREGATE_SHARES_BY_SIMPLE_MEAN
	private: typedef ::dcs::eesim::physical_resource_category resource_category_type;
	private: typedef ::std::map<resource_category_type,share_statistic_type> share_statistic_map;
	private: typedef ::std::vector<share_statistic_map> share_statistic_container;
# endif // OFFSYSID_EXP_AGGREGATE_SHARES
#endif //OFFSYSID_EXP_AGGREGATE_MEASURES


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
		::std::cout << "\"tid\",\"rid\",\"arrtime\",\"deptime\"";
		typedef ::std::vector<typename application_type::reference_physical_resource_type> resource_container;
		typedef typename resource_container::const_iterator resource_iterator;
		resource_container resources(app.reference_resources());
		resource_iterator end_it(resources.end());
		::std::size_t count(0);
		for (resource_iterator it = resources.begin(); it != end_it; ++it)
		{
//			::std::cout << ",\"category_" << count << "\",\"share_" << count << "\",\"delta_share_" << count << "\"";
			::std::cout << ",\"category_" << count << "\",\"share_" << count << "\"";
			++count;
		}
//		::std::cout << ",\"rt\",\"delta_rt\"" << ::std::endl;
		::std::cout << ",\"rt\"" << ::std::endl;

#if defined(OFFSYSID_EXP_AGGREGATE_MEASURES)
		avg_rt_ = measure_statistic_container(app.num_tiers()+1);
		aggavg_rt_ = measure_container(app.num_tiers(), 0);
# if defined(OFFSYSID_EXP_AGGREGATE_SHARES)
		avg_shares_ = share_statistic_container(app.num_tiers()+1);
# endif // OFFSYSID_EXP_AGGREGATE_SHARES
#endif //OFFSYSID_EXP_AGGREGATE_MEASURES
	}


	private: void do_process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );

		// empty
	}


	private: void do_process_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state) 
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		// empty
	}


	private: void do_process_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		// Log the response time of the overall application.

		user_request_type req = app.simulation_model().request_state(evt);
//::std::cerr << "do_process_request_departure_event>> BEGIN Req: " << req.id() << " (Clock: " << ctx.simulated_time() << ")" << ::std::endl;//XXX

		uint_type tier_id(0);// We take shares referred to the first tier
#if !defined(OFFSYSID_EXP_AGGREGATE_MEASURES)
		::std::cout << "-1" // Fake tier-id representing the entire application
					<< "," << req.id()
					<< "," << req.arrival_time()
					<< "," << ctx.simulated_time();
#endif //OFFSYSID_EXP_AGGREGATE_MEASURES

		::dcs::shared_ptr<sysid_request_info_type> ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info );

		::dcs::shared_ptr<sysid_request_info_impl_type> ptr_req_info_impl;
		ptr_req_info_impl = ::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info_impl );

#if !defined(OFFSYSID_EXP_AGGREGATE_MEASURES)
		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_vm );

		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator end_it(resource_shares.end());
		for (resource_share_iterator it = resource_shares.begin(); it != end_it; ++it)
		{
			::std::cout << "," << it->first
						<< "," << ptr_req_info_impl->share_map[it->first];
		}
#endif //OFFSYSID_EXP_AGGREGATE_MEASURES

		real_type rt(ctx.simulated_time()-req.arrival_time());

#if !defined(OFFSYSID_EXP_AGGREGATE_MEASURES)
		::std::cout << "," << rt
					<< ::std::endl;
#else //OFFSYSID_EXP_AGGREGATE_MEASURES
# if defined(OFFSYSID_EXP_AGGREGATE_MEASURES_BY_SIMPLE_MEAN)
		(avg_rt_[app.num_tiers()])(rt);
#  if defined(OFFSYSID_EXP_AGGREGATE_SHARES)
		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_vm );

		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator end_it(resource_shares.end());
		for (resource_share_iterator it = resource_shares.begin(); it != end_it; ++it)
		{
#   if defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
			(avg_shares_[app.num_tiers()][it->first])(ptr_req_info_impl->share_map[it->first]); //FIXME: ok!
			//(avg_shares_[app.num_tiers()])(ptr_req_info_impl->share_map[it->first]); //FIXME: ko!
#   elif defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
			(avg_shares_[app.num_tiers()][it->first])(ptr_req_info_impl->share_map[it->first], 1); //FIXME: ok!
			//(avg_shares_[app.num_tiers()])(ptr_req_info_impl->share_map[it->first], 1); //FIXME: ko!
#   endif // OFFSYSID_EXP_AGGREGATE_SHARES_BY_SIMPLE_MEAN
		}
#  endif // OFFSYSID_EXP_AGGREGATE_SHARES
# elif defined(OFFSYSID_EXP_AGGREGATE_MEASURES_BY_WEIGHTED_MEAN)
		real_type w(1);
		real_type excite_start_time(ctx.simulated_time()-this->sampling_time());
		if (req.arrival_time() < excite_start_time)
		{
			w = req.arrival_time()/excite_start_time;
		}
		//(avg_rt_[app.num_tiers()])(rt, ::boost::accumulators::weight = w);
		(avg_rt_[app.num_tiers()])(rt, w);
#  if defined(OFFSYSID_EXP_AGGREGATE_SHARES)
		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;
		virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_vm );

		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator end_it(resource_shares.end());
		for (resource_share_iterator it = resource_shares.begin(); it != end_it; ++it)
		{
#   if defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
			(avg_shares_[app.num_tiers()][it->first])(ptr_req_info_impl->share_map[it->first]); //FIXME: ok!
			//(avg_shares_[app.num_tiers()])(ptr_req_info_impl->share_map[it->first]); //FIXME: ko!
#   elif defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
			(avg_shares_[app.num_tiers()][it->first])(ptr_req_info_impl->share_map[it->first], w); //FIXME: ok!
			//(avg_shares_[app.num_tiers()])(ptr_req_info_impl->share_map[it->first], w); //FIXME: ko!
#   endif // OFFSYSID_EXP_AGGREGATE_SHARES_BY_SIMPLE_MEAN
//::std::cerr << "do_process_request_departure_event>> END Req: " << req.id() << " (Clock: " << ctx.simulated_time() << ")" << ::std::endl;//XXX
		}
#  endif // OFFSYSID_EXP_AGGREGATE_SHARES
# endif // OFFSYSID_EXP_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
#endif //OFFSYSID_EXP_AGGREGATE_MEASURES

		for (uint_type tier_id = 0; tier_id < app.num_tiers(); ++tier_id)
		{
			ptr_sysid_state->req_info_maps[tier_id].erase(req.id());
		}
//[/EXP-20110609]
	}


	private: void do_process_tier_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		user_request_type req = app.simulation_model().request_state(evt);

		if (!ptr_sysid_state->req_info_maps[tier_id][req.id()])
		{
			ptr_sysid_state->req_info_maps[tier_id][req.id()] = ::dcs::make_shared<sysid_request_info_impl_type>();
		}

		sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);
		ptr_req_info->arr_time = ctx.simulated_time();
#if defined(OFFSYSID_EXP_AGGREGATE_SHARES)
		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));
		virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_vm );

		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator end_it(resource_shares.end());
		for (resource_share_iterator it = resource_shares.begin(); it != end_it; ++it)
		{
//::std::cerr << "do_process_tier_request_arrival_event>> Arrived Request: " << req.id() << " - Tier: " << tier_id << " - share: " <<  it->second << " (Clock: " << ctx.simulated_time() << ::std::endl;//XXX
			ptr_req_info_impl->share_map[it->first] = it->second;
		}
		ptr_req_info_impl->done = false;
# if defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
		ptr_req_info_impl->share_count = 1;
# elif defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
		ptr_req_info_impl->last_share_change_time = ptr_req_info_impl->arr_time;
		ptr_req_info_impl->weigths_sum = 0;
		ptr_req_info_impl->old_share_map = ptr_req_info_impl->share_map;
# endif // OFFSYSID_EXP_AGGREGATE_SHARES_BY_WEIGHTED_MEAN
#endif // OFFSYSID_EXP_AGGREGATE_SHARES
	}


	private: void do_process_tier_request_service_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

#if !defined(OFFSYSID_EXP_AGGREGATE_SHARES)
		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		user_request_type req = app.simulation_model().request_state(evt);

		sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);
		sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));
		virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_vm );

		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator end_it(resource_shares.end());
		for (resource_share_iterator it = resource_shares.begin(); it != end_it; ++it)
		{
			ptr_req_info_impl->share_map[it->first] = it->second;
		}
#else
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( tier_id );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );
#endif // OFFSYSID_EXP_AGGREGATE_SHARES
	}


	private: void do_process_tier_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		user_request_type req = app.simulation_model().request_state(evt);

#if !defined(OFFSYSID_EXP_AGGREGATE_MEASURES)
		::std::cout << tier_id
					<< "," << req.id()
					<< "," << req.tier_arrival_times(tier_id).back()
					<< "," << ctx.simulated_time();
#endif //OFFSYSID_EXP_AGGREGATE_MEASURES

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

		sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);
		sysid_request_info_impl_pointer ptr_req_info_impl;
		ptr_req_info_impl = ::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info);

		DCS_DEBUG_ASSERT( ptr_req_info );

//#if !defined(OFFSYSID_EXP_AGGREGATE_MEASURES)
		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_vm );

		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator res_end_it(resource_shares.end());
#if defined(OFFSYSID_EXP_AGGREGATE_SHARES)
#if defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
		real_type w(ctx.simulated_time()-ptr_req_info_impl->last_share_change_time);
		ptr_req_info_impl->weigths_sum += w;
#endif // OFFSYSID_EXP_AGGREGATE_SHARES_BY_WEIGHTED_MEAN
#endif // OFFSYSID_EXP_AGGREGATE_SHARES
		for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
		{
#if defined(OFFSYSID_EXP_AGGREGATE_SHARES)
			ptr_req_info_impl->done = true;
#if defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
			real_type x(ptr_req_info_impl->old_share_map.at(res_it->first));
			real_type m(ptr_req_info_impl->share_map.at(res_it->first));
			real_type q(x-m);
			real_type r(q*w/ptr_req_info_impl->weigths_sum);
//::std::cerr << "do_process_tier_request_departure_event>> Request: " << req.id() << " - Tier: " << tier_id << " - Cur Mean: " << m << " - New X: " << x << " - New Weight: " << w << " --> New Mean: " << (m+r) << ::std::endl;//XXX
			m += r;
			ptr_req_info_impl->share_map[res_it->first] = m;
//			detail::update_weighted_mean(res_it->second, w, sumw, ptr_req_info_impl->share_map.at[res_it->first]);
			ptr_req_info_impl->old_share_map[res_it->first] = res_it->second;
#endif // OFFSYSID_EXP_AGGREGATE_SHARES_BY_WEIGHTED_MEAN
#endif // OFFSYSID_EXP_AGGREGATE_SHARES
#if !defined(OFFSYSID_EXP_AGGREGATE_MEASURES)
//			::std::cout << "," << res_it->first
//						<< "," << ptr_req_info_impl->share_map[res_it->first]
//						<< "," << detail::relative_deviation(res_it->second, app.tier(tier_id)->resource_share(res_it->first));
			::std::cout << "," << res_it->first
						<< "," << ptr_req_info_impl->share_map[res_it->first];
//::std::cerr << "do_process_tier_request_departure_event>> Departed Request: " << req.id() << " - Tier: " << tier_id << " - share: " << ptr_req_info_impl->share_map[res_it->first] << " (Clock: " << ctx.simulated_time() << ::std::endl;//XXX
#endif //OFFSYSID_EXP_AGGREGATE_MEASURES
		}
//#endif //OFFSYSID_EXP_AGGREGATE_MEASURES

//		real_type rt(ctx.simulated_time()-ptr_sysid_state->req_info_maps[tier_id].at(req.id())->arr_time);
		real_type rt(ctx.simulated_time()-ptr_req_info_impl->arr_time);

#if !defined(OFFSYSID_EXP_AGGREGATE_MEASURES)
//FIXME: Keep M/D/1 or return back to M/M/1?
//#if 0
//		if (tier_id == 0)
//		{
//			// Treat the first tier as a M/D/1 queue
//			::std::cout << "," << rt
//						<< "," << detail::relative_deviation(rt, detail::md1_residence_time(0.15, 0.5))
//						<< ::std::endl;
//::std::cerr << "Reference residence time: " << detail::md1_residence_time(0.15, 0.5) << ::std::endl;//XXX
//		}
//		else
//		{
//			// Treat the other tiers as a M/M/1 queue
//			::std::cout << "," << rt
//						<< "," << detail::relative_deviation(rt, app.performance_model().tier_measure(tier_id, ::dcs::eesim::response_time_performance_measure))
//						<< ::std::endl;
//		}
//#else
//		::std::cout << "," << rt
//					<< "," << detail::relative_deviation(rt, app.performance_model().tier_measure(tier_id, ::dcs::eesim::response_time_performance_measure))
//					<< ::std::endl;
//#endif
		::std::cout << "," << rt
					<< ::std::endl;
#else //OFFSYSID_EXP_AGGREGATE_MEASURES
# if defined(OFFSYSID_EXP_AGGREGATE_MEASURES_BY_SIMPLE_MEAN)
		(avg_rt_[tier_id])(rt);
#  if defined(OFFSYSID_EXP_AGGREGATE_SHARES)
		for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
		{
#   if defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
			(avg_shares_[tier_id][it->first])(ptr_req_info_impl->share_map[it->first]); //FIXME: ok!
			//(avg_shares_[tier_id])(ptr_req_info_impl->share_map[it->first]); //FIXME: ko!
#   elif defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
			(avg_shares_[tier_id][it->first])(ptr_req_info_impl->share_map[it->first], 1); //FIXME: ok!
			//(avg_shares_[tier_id])(ptr_req_info_impl->share_map[it->first], 1); //FIXME: ko!
#   endif // OFFSYSID_EXP_AGGREGATE_SHARES_BY_SIMPLE_MEAN
		}
#  endif // OFFSYSID_EXP_AGGREGATE_SHARES
# elif defined(OFFSYSID_EXP_AGGREGATE_MEASURES_BY_WEIGHTED_MEAN)
#  if defined(OFFSYSID_EXP_AGGREGATE_SHARES) && defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
		w = 1;
#  else
		real_type w(1);
#  endif // OFFSYSID_EXP_AGGREGATE_SHARES
		real_type excite_start_time(ctx.simulated_time()-this->sampling_time());
		if (ptr_req_info_impl->arr_time < excite_start_time)
		{
			w = ptr_req_info_impl->arr_time/excite_start_time;
		}
		(avg_rt_[tier_id])(rt, w);
#  if defined(OFFSYSID_EXP_AGGREGATE_SHARES)
		for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
		{
#   if defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
			(avg_shares_[tier_id][res_it->first])(ptr_req_info_impl->share_map[res_it->first]); //FIXME: ok!
			//(avg_shares_[tier_id])(ptr_req_info_impl->share_map[res_it->first]); //FIXME: ko!
#   elif defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
			(avg_shares_[tier_id][res_it->first])(ptr_req_info_impl->share_map[res_it->first], w); //FIXME: ok!
			//(avg_shares_[tier_id])(ptr_req_info_impl->share_map[res_it->first], w); //FIXME: ko!
#   endif // OFFSYSID_EXP_AGGREGATE_SHARES_BY_SIMPLE_MEAN
		}
#  endif // OFFSYSID_EXP_AGGREGATE_SHARES
# endif //OFFSYSID_EXP_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
#endif //OFFSYSID_EXP_AGGREGATE_MEASURES

//		ptr_sysid_state->req_info_maps[tier_id].erase(req.id());
	}


	private: void do_process_excite_system_event(des_event_type const& evt, des_engine_context_type& ctx, application_type& app, signal_generator_pointer const& ptr_sig_gen, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sig_gen );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

#if defined(OFFSYSID_EXP_AGGREGATE_SHARES)
		// For each tier, for each in-service request, for each resource category, update the mean share

		typedef typename sysid_state_type::request_info_map request_info_map;
		typedef typename request_info_map::iterator request_info_map_iterator;
		typedef typename sysid_state_type::request_info_pointer sysid_request_info_pointer;
		typedef detail::sysid_siso_request_info<traits_type> sysid_request_info_impl_type;
		typedef ::dcs::shared_ptr<sysid_request_info_impl_type> sysid_request_info_impl_pointer;
		typedef typename sysid_request_info_impl_type::resource_share_map resource_share_map;
		typedef typename resource_share_map::iterator resource_share_map_iterator;
		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		for (::std::size_t tier_id = 0; tier_id < app.num_tiers(); ++tier_id)
		{
			virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_vm );

			resource_share_container resource_shares(ptr_vm->resource_shares());
			resource_share_iterator res_end_it(resource_shares.end());

			request_info_map_iterator req_end_it(ptr_sysid_state->req_info_maps[tier_id].end());
			for (request_info_map_iterator req_it = ptr_sysid_state->req_info_maps[tier_id].begin(); req_it != req_end_it; ++req_it)
			{
				sysid_request_info_pointer ptr_req_info(req_it->second);

				// check: paranoid check
				DCS_DEBUG_ASSERT( ptr_req_info );

				sysid_request_info_impl_pointer ptr_req_info_impl;
				ptr_req_info_impl = ::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info);

				// check: paranoid check
				DCS_DEBUG_ASSERT( ptr_req_info_impl );

				if (ptr_req_info_impl->done)
				{
					continue;
				}

#if defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
				ptr_req_info_impl->share_count += 1;
#elif defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
				real_type w(ctx.simulated_time()-ptr_req_info_impl->last_share_change_time);
				ptr_req_info_impl->last_share_change_time = ctx.simulated_time();
				ptr_req_info_impl->weigths_sum += w;
#endif // OFFSYSID_EXP_AGGREGATE_SHARES_BY_SIMPLE_MEAN
				for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
				{
#if defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
					real_type m(ptr_req_info_impl->share_map.at(res_it->first));
					m += (res_it->second-m)/ptr_req_info_impl->share_count;
					ptr_req_info_impl->share_map[res_it->first] = m;
//					detail::update_mean(res_it->second, ptr_req_info_impl->share_count, ptr_req_info_impl->share_map.at[res_it->first]);
#elif defined(OFFSYSID_EXP_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
					// Computer the weighted mean until the "old" share.
					// We cannot take into consideration the new share here, because we
					// cannot compute its weight.
					real_type x(ptr_req_info_impl->old_share_map.at(res_it->first));
					real_type m(ptr_req_info_impl->share_map.at(res_it->first));
					real_type q(x-m);
					real_type r(q*w/ptr_req_info_impl->weigths_sum);
//::std::cerr << "do_process_excite_system_event>> Request: " << req_it->first << " - Tier: " << tier_id << " - Cur Mean: " << m << " - New X: " << x << " - New Weight: " << w << " --> New Mean: " << (m+r) << ::std::endl;//XXX
					m += r;
					ptr_req_info_impl->share_map[res_it->first] = m;
//					detail::update_weighted_mean(res_it->second, w, sumw, ptr_req_info_impl->share_map.at[res_it->first]);
					ptr_req_info_impl->old_share_map[res_it->first] = res_it->second;
#endif // OFFSYSID_EXP_AGGREGATE_SHARES_BY_SIMPLE_MEAN
				}
			}
		}
#endif // OFFSYSID_EXP_AGGREGATE_SHARES
//[/EXP-20110609]

#if defined(OFFSYSID_EXP_AGGREGATE_MEASURES)
		for (::std::size_t tier_id = 0; tier_id < app.num_tiers(); ++tier_id)
		{
			::std::cout << tier_id
						<< "," << "0" //Fake Request ID
						<< "," << ctx.simulated_time() //Fake Arrival-Time
						<< "," << ctx.simulated_time();

			typedef virtual_machine_type::resource_share_container resource_share_container;
			typedef resource_share_container::const_iterator resource_share_iterator;

			virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_vm );

			resource_share_container resource_shares(ptr_vm->resource_shares());
			resource_share_iterator end_it(resource_shares.end());
			for (resource_share_iterator it = resource_shares.begin(); it != end_it; ++it)
			{
# if !defined(OFFSYSID_EXP_AGGREGATE_SHARES)
				::std::cout << "," << it->first
							<< "," << it->second;
# else // OFFSYSID_EXP_AGGREGATE_SHARES
				::std::cout << "," << it->first
							<< "," << avg_shares_[tier_id].at(it->first).estimate(); // FIXME: ok!
//							<< "," << avg_shares_[tier_id].estimate(); // FIXME: ko!
# endif // OFFSYSID_EXP_AGGREGATE_SHARES
			}

			const real_type alpha(1.0);
//# if defined(OFFSYSID_EXP_AGGREGATE_MEASURES_BY_SIMPLE_MEAN)
//			if (::boost::accumulators::count(avg_rt_[tier_id]) > 0)
//			{
//				aggavg_rt_[tier_id] = alpha*::boost::accumulators::mean(avg_rt_[tier_id])+(1-alpha)*aggavg_rt_[tier_id];//[EXP-20110607]
//			}
//# elif defined(OFFSYSID_EXP_AGGREGATE_MEASURES_BY_WEIGHTED_MEAN)
			if (avg_rt_[tier_id].size() > 0)
			{
				aggavg_rt_[tier_id] = alpha*avg_rt_[tier_id].estimate()+(1-alpha)*aggavg_rt_[tier_id];//[EXP-20110607]
			}
//# endif // OFFSYSID_EXP_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
//			else
//			{
//				// use previous value
//			}
			::std::cout << "," << aggavg_rt_[tier_id]
						<< ::std::endl;
		}

		avg_rt_ = measure_statistic_container(app.num_tiers()+1);
# if !defined(OFFSYSID_EXP_AGGREGATE_SHARES)
		avg_shares_ = share_statistic_container(app.num_tiers()+1);
# endif // OFFSYSID_EXP_AGGREGATE_SHARES
#endif//OFFSYSID_EXP_AGGREGATE_MEASURES
	}


#if defined(OFFSYSID_EXP_AGGREGATE_MEASURES)
	private: measure_statistic_container avg_rt_;
	private: measure_container aggavg_rt_;
# if defined(OFFSYSID_EXP_AGGREGATE_SHARES)
	private: share_statistic_container avg_shares_;
# endif // OFFSYSID_EXP_AGGREGATE_SHARES
#endif //OFFSYSID_EXP_AGGREGATE_MEASURES
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
		::std::cout << "\"tid\",\"rid\",\"arrtime\",\"deptime\"";
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
//				::std::cout << ",\"share_" << tier_id << "_" << count << "\",\"delta_share_" << tier_id << "_" << count << "\"";
				::std::cout << ",\"share_" << tier_id << "_" << count << "\"";
			}
			++count;
		}
//		::std::cout << ",\"rt\",\"delta_rt\"" << ::std::endl;
		::std::cout << ",\"rt\"" << ::std::endl;
	}


	private: void do_process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );

		// empty
	}


	private: void do_process_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state) 
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		// empty
	}


	private: void do_process_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		// empty
//[EXP-20110609]
		user_request_type req = app.simulation_model().request_state(evt);

		uint_type tier_id(0);// We take shares and arrival time referred to the first tier
		::std::cout << "-1"
					<< "," << req.id()
					<< "," << req.tier_arrival_times(tier_id).front()
					<< "," << ctx.simulated_time();

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

		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_vm );

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

//				::std::cout << "," << share
//							<< "," << detail::relative_deviation(share, app.tier(t)->resource_share(it->first));
				::std::cout << "," << share;
			}
		}

//		real_type rt(ctx.simulated_time()-ptr_sysid_state->req_info_maps[tier_id].at(req.id())->arr_time);
		real_type rt(ctx.simulated_time()-ptr_req_info->arr_time);

//FIXME: Keep M/D/1 or return back to M/M/1?
//#if 0
//		if (tier_id == 0)
//		{
//			// Treat the first tier as a M/D/1 queue
//			::std::cout << "," << rt
//						<< "," << detail::relative_deviation(rt, detail::md1_residence_time(0.15, 0.5))
//						<< ::std::endl;
//::std::cerr << "Reference residence time: " << detail::md1_residence_time(0.15, 0.5) << ::std::endl;//XXX
//		}
//		else
//		{
//			// Treat the other tiers as a M/M/1 queue
//			::std::cout << "," << rt
//						<< "," << detail::relative_deviation(rt, app.performance_model().tier_measure(tier_id, ::dcs::eesim::response_time_performance_measure))
//						<< ::std::endl;
//		}
//#else
//		::std::cout << "," << rt
//					<< "," << detail::relative_deviation(rt, app.performance_model().tier_measure(tier_id, ::dcs::eesim::response_time_performance_measure))
//					<< ::std::endl;
//#endif
		::std::cout << "," << rt
					<< ::std::endl;

		for (uint_type tier_id = 0; tier_id < app.num_tiers(); ++tier_id)
		{
			ptr_sysid_state->req_info_maps[tier_id].erase(req.id());
		}
//[/EXP-20110609]
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
			typedef virtual_machine_type::resource_share_container resource_share_container;
			typedef resource_share_container::const_iterator resource_share_iterator;

			virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(t);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_vm );

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
					<< "," << req.id()
					<< "," << req.tier_arrival_times(tier_id).back()
					<< "," << ctx.simulated_time();

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

		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_vm );

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

//				::std::cout << "," << share
//							<< "," << detail::relative_deviation(share, app.tier(t)->resource_share(it->first));
				::std::cout << "," << share;
			}
		}

//		real_type rt(ctx.simulated_time()-ptr_sysid_state->req_info_maps[tier_id].at(req.id())->arr_time);
		real_type rt(ctx.simulated_time()-ptr_req_info->arr_time);

//FIXME: Keep M/D/1 or return back to M/M/1?
//#if 0
//		if (tier_id == 0)
//		{
//			// Treat the first tier as a M/D/1 queue
//			::std::cout << "," << rt
//						<< "," << detail::relative_deviation(rt, detail::md1_residence_time(0.15, 0.5))
//						<< ::std::endl;
//::std::cerr << "Reference residence time: " << detail::md1_residence_time(0.15, 0.5) << ::std::endl;//XXX
//		}
//		else
//		{
//			// Treat the other tiers as a M/M/1 queue
//			::std::cout << "," << rt
//						<< "," << detail::relative_deviation(rt, app.performance_model().tier_measure(tier_id, ::dcs::eesim::response_time_performance_measure))
//						<< ::std::endl;
//		}
//#else
//		::std::cout << "," << rt
//					<< "," << detail::relative_deviation(rt, app.performance_model().tier_measure(tier_id, ::dcs::eesim::response_time_performance_measure))
//					<< ::std::endl;
//#endif
		::std::cout << "," << rt
					<< ::std::endl;

//		ptr_sysid_state->req_info_maps[tier_id].erase(req.id());
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


	// Check if the 'help' message is requested.
	if (detail::find_option(argv, argv+argc, "--help") != (argv+argc))
	{
		detail::usage();
		return 0;
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
//		in_filter_category = detail::parse_input_filter_category(detail::get_option<std::string>(argv, argv+argc, "--infilt"));
//		out_filter_category = detail::parse_output_filter_category(detail::get_option<std::string>(argv, argv+argc, "--outfilt"));
		conf_fname = detail::get_option<std::string>(argv, argv+argc, "--conf");
	}
	catch (std::exception const& e)
	{
		std::cerr << "[Error] Error while parsing command-line options: " << e.what() << std::endl;
		detail::usage();
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
		// - Create DES engine from configuration
		ptr_des_eng = detail::make_des_engine<traits_type>();
		// - Remove all statistics (we don't care of statistics)
		ptr_des_eng->ignore_statistics();
		// - Register to the registry
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
			case ramp_signal:
				ptr_sig_gen = dcs::make_shared< detail::ramp_signal_generator<real_type> >(
								ublas::scalar_vector<real_type>(ptr_app->num_tiers(), 0),
								ublas::scalar_vector<real_type>(ptr_app->num_tiers(), 0.1)
					);
				break;
			case gaussian_white_noise_signal:
				ptr_sig_gen = dcs::make_shared< detail::gaussian_signal_generator<real_type> >(
								ublas::zero_vector<real_type>(ptr_app->num_tiers()),
								ublas::scalar_vector<real_type>(ptr_app->num_tiers(), 1)
					);
				break;
//			case parabolic_signal: // p(t)=0.5*t^2*u(t), where u(t) is the unit step function
//				//TODO
//				break;
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
