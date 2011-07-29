#include <algorithm>
//#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
//# include <boost/accumulators/accumulators.hpp>
//# include <boost/accumulators/statistics/stats.hpp>
//# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN)
//#  include <boost/accumulators/statistics/mean.hpp>
//# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_WEIGHTED_MEAN)
//#  include <boost/accumulators/statistics/weighted_mean.hpp>
//# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
//#endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublasx/operation/size.hpp>
#include <boost/variant.hpp>
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
#include <dcs/math/stats/distribution/continuous_uniform.hpp>
#include <dcs/math/stats/distribution/normal.hpp>
//#include <dcs/math/random/any_generator.hpp>
#include <dcs/math/random.hpp>
#include <dcs/memory.hpp>
#if __GNUC__
# include <execinfo.h>
#endif // __GNUC__
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
	step_signal,
	uniform_signal
};


enum system_identification_category
{
	siso_system_identification,
	miso_system_identification
};


enum aggregation_category
{
	none_aggregation_category,
	mean_aggregation_category,
	weighted_mean_aggregation_category
};

enum filter_category
{
	none_filter_category,
//	average_filter_category,
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
				<< "  --inaggr {'none'|'mean'|'wmean'}" << ::std::endl
				<< "    The type of aggregation to be applied to input data." << ::std::endl
//				<< "  --infilt {'none'|'mean'}" << ::std::endl
//				<< "    The type of filter to be applied on input data." << ::std::endl
				<< "  --ns <number-of-samples>" << ::std::endl
				<< "    The number of samples to collect." << ::std::endl
				<< "  --outaggr {'none'|'mean'|'wmean'}" << ::std::endl
				<< "    The type of aggregation to be applied to output data." << ::std::endl
				<< "  --outfilt {'none'|'ewma'}" << ::std::endl
				<< "    The type of filter to be applied on output data." << ::std::endl
				<< "    Only meaningful when some form of output aggregation is used." << ::std::endl
				<< "  --outfilt-ewma-alpha <value>" << ::std::endl
				<< "    The smoothing factor to be used with the EWMA output filter." << ::std::endl
				<< "  --sys {'siso'|'miso'}" << ::std::endl
				<< "    The type of identification that is to be performed." << ::std::endl
				<< "  --sig {'gaussian'|'ramp'|'sine'|'step'|'unif'}" << ::std::endl
				<< "    The shape of the input signal used to excite the target system." << ::std::endl
				<< "  --sig-gaussian-mean <value>" << ::std::endl
				<< "    The value of the mean for the Gaussian white noise." << ::std::endl
				<< "  --sig-gaussian-sd <value>" << ::std::endl
				<< "    The standard deviation value for the value of the mean for the Gaussian white noise." << ::std::endl
				<< "  --sig-sine-amplitutde <value>" << ::std::endl
				<< "    The amplitude of the sine wave (i.e., the peak deviation of the sine)." << ::std::endl
				<< "    wave from its center position)." << ::std::endl
				<< "  --sig-sine-frequency <value>" << ::std::endl
				<< "    The number of time samples per sine wave period." << ::std::endl
				<< "  --sig-sine-phase <value>" << ::std::endl
				<< "    The phase shift (i.e., the offset of the signal in number of sample times." << ::std::endl
				<< "  --sig-sine-bias <value>" << ::std::endl
				<< "    The signal bias (i.e., the constant value added to the sine to produce the output)." << ::std::endl
				<< "  --sig-uniform-min <value>" << ::std::endl
				<< "    The minimum value for the uniform signal." << ::std::endl
				<< "  --sig-uniform-max <value>" << ::std::endl
				<< "    The maximum value for the uniform signal." << ::std::endl
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


template <typename T, typename ForwardIterT>
T get_option(ForwardIterT begin, ForwardIterT end, std::string const& option, T default_value)
{
    ForwardIterT it = find_option(begin, end, option);

	T value(default_value);

    if (it != end && ++it != end)
    {
		::std::istringstream iss(*it);
		iss >> value;
    }

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
	if (!str.compare("uniform"))
	{
		return uniform_signal;
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


aggregation_category parse_input_aggregation_category(::std::string const& str)
{
	if (!str.compare("none"))
	{
		return none_aggregation_category;
	}
	if (!str.compare("mean"))
	{
		return mean_aggregation_category;
	}
	if (!str.compare("wmean"))
	{
		return weighted_mean_aggregation_category;
	}

	throw ::std::invalid_argument("[detail::parse_input_aggregation_category] Cannot find a valid input aggregation category.");
}


aggregation_category parse_output_aggregation_category(::std::string const& str)
{
	if (!str.compare("none"))
	{
		return none_aggregation_category;
	}
	if (!str.compare("mean"))
	{
		return mean_aggregation_category;
	}
	if (!str.compare("wmean"))
	{
		return weighted_mean_aggregation_category;
	}

	throw ::std::invalid_argument("[detail::parse_output_aggregation_category] Cannot find a valid output aggregation category.");
}


//filter_category parse_input_filter_category(::std::string const& str)
//{
//	if (!str.compare("none"))
//	{
//		return none_filter_category;
//	}
//	if (!str.compare("average"))
//	{
//		return average_filter_category;
//	}
//
//	throw ::std::invalid_argument("[detail::parse_input_filter_category] Cannot find a valid input filter category.");
//}


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
//	if (!str.compare("average"))
//	{
//		return average_filter_category;
//	}

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
 * - A is the amplitude of the sine wave (i.e., the peak deviation of the sine
 *   function from its center position).
 * - p is the number of time samples per sine wave period.
 * - k is a repeating integer value that ranges from 0 to p–1.
 * - o is the offset (phase shift) of the signal in number of sample times.
 * - b is the signal bias (i.e., constant value added to the sine to produce the
 *   output).
 * .
 *
 * See:
 * http://www.mathworks.com/help/toolbox/simulink/slref/sinewave.html
 */
template <typename ValueT>
class sinusoidal_signal_generator: public base_signal_generator<ValueT>
{
	private: typedef base_signal_generator<ValueT> base_type;
	public: typedef ValueT value_type;
	public: typedef typename base_type::vector_type vector_type;
	public: typedef ublas::vector<size_type> size_vector_type;


	public: sinusoidal_signal_generator(vector_type const& a, size_vector_type const& p)
	: a_(a),
	  p_(p),
	  o_(ublas::zero_vector<size_type>(ublasx::size(a))),
	  b_(ublas::zero_vector<value_type>(ublasx::size(a))),
	  k_(ublas::zero_vector<size_type>(ublasx::size(a)))
	{
		// pre: size(a) == size(p)
		DCS_ASSERT(
				ublasx::size(a_) == ublasx::size(p_),
				throw ::std::invalid_argument("[sinusoidal_signal_generator::ctor] Invalid vector size.")
			);
	}


	public: sinusoidal_signal_generator(vector_type const& a, size_vector_type const& p, size_vector_type const& o, vector_type const& b)
	: a_(a),
	  p_(p),
	  o_(o),
	  b_(b),
	  k_(ublas::zero_vector<size_type>(ublasx::size(a)))
	{
		// pre: size(a) == size(p)
		DCS_ASSERT(
				ublasx::size(a_) == ublasx::size(p_),
				throw ::std::invalid_argument("[sinusoidal_signal_generator::ctor] Invalid vector size between 'a' and 'p'.")
			);
		// pre: size(a) == size(o)
		DCS_ASSERT(
				ublasx::size(a_) == ublasx::size(o_),
				throw ::std::invalid_argument("[sinusoidal_signal_generator::ctor] Invalid vector size between 'a' and 'o'.")
			);
		// pre: size(a) == size(b)
		DCS_ASSERT(
				ublasx::size(a_) == ublasx::size(b_),
				throw ::std::invalid_argument("[sinusoidal_signal_generator::ctor] Invalid vector size between 'a' and 'b'.")
			);
	}


	public: void offset(size_vector_type o)
	{
		// pre: size(o) == size(a_)
		DCS_ASSERT(
				ublasx::size(o) == ublasx::size(a_),
				throw ::std::invalid_argument("[sinusoidal_signal_generator::offset] Invalid vector size.")
			);

		o_ = o;
	}


	public: void bias(vector_type b)
	{
		// pre: size(b) == size(a_)
		DCS_ASSERT(
				ublasx::size(b) == ublasx::size(a_),
				throw ::std::invalid_argument("[sinusoidal_signal_generator::bias] Invalid vector size.")
			);

		b_ = b;
	}


	private: vector_type do_generate()
	{
		::std::size_t n(ublasx::size(a_));
		vector_type u(n);
		for (::std::size_t i = 0; i < n; ++i)
		{
			u(i) = a_(i)*::std::sin(::dcs::math::constants::double_pi<value_type>::value*(k_(i)+o_(i))/p_(i))+b_(i);

			DCS_DEBUG_TRACE("Generated: k(" << i << ")=" << k_(i) << " ==> u(" << i << ")=" << u(i));//XXX

			k_(i) += 1;
		}

		return u;
	}


	private: void do_reset()
	{
		k_ = ublas::zero_vector<size_type>(ublasx::size(a_));
	}


	/// The amplitude (the peak deviation of the function from its center position).
	private: vector_type a_;
	private: size_vector_type p_;
	/// The phase (specifies where in its cycle the oscillation begins at t = 0).
	private: size_vector_type o_;
	/// The DC offset (a non-zero center amplitude).
	private: vector_type b_;
	private: vector_type k_;
}; // sinusoidal_signal_generator

template <typename ValueT>
class uniform_signal_generator: public base_signal_generator<ValueT>
{
	private: typedef base_signal_generator<ValueT> base_type;
	public: typedef ValueT value_type;
	public: typedef typename base_type::vector_type vector_type;
	private: typedef ::dcs::math::stats::continuous_uniform_distribution<value_type> continuous_uniform_distribution_type;
	private: typedef ::std::vector<continuous_uniform_distribution_type> continuous_uniform_distribution_container;


	public: uniform_signal_generator(vector_type const& u_min, vector_type const& u_max)
	{
		// pre: size(u_min) == size(u_max)
		DCS_ASSERT(
				ublasx::size(u_min) == ublasx::size(u_max),
				throw ::std::invalid_argument("[uniform_signal_generator::ctor] Invalid size.")
			);

		::std::size_t n(ublasx::size(u_min));
		for (::std::size_t i = 0; i < n; ++i)
		{
			distrs_.push_back(continuous_uniform_distribution_type(u_min(i), u_max(i)));
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


	private: continuous_uniform_distribution_container distrs_;
};

//@} Signal generators


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


	public: value_type operator()(value_type x)
	{
		return do_apply(x);
	}


	public: value_type value() const
	{
		return do_value();
	}


	public: void reset()
	{
		do_reset();
	}


	private: virtual filter_category do_category() const = 0;


	private: virtual value_type do_apply(value_type x) = 0;


	private: virtual value_type do_value() const = 0;


	private: virtual void do_reset() = 0;
}; // base_filter


template <typename ValueT>
class none_filter: public base_filter<ValueT>
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


	private: value_type do_apply(value_type x)
	{
		x_ = x;

		return x_;
	}


	private: value_type do_value() const
	{
		return x_;
	}


	private: void do_reset()
	{
		x_ = 0;
	}


	private: value_type x_;
}; // none_filter


template <typename ValueT, typename RealT=ValueT>
class ewma_filter: public base_filter<ValueT>
{
	public: typedef ValueT value_type;
	public: typedef RealT real_type;


	public: ewma_filter(real_type smooth_factor)
	: s_(0),
	  a_(smooth_factor)
	{
	}


	private: filter_category do_category() const
	{
		return ewma_filter_category;
	}


	private: value_type do_apply(value_type x)
	{
		s_ = a_*x + (1-a_)*s_;

		return s_;
	}


	private: value_type do_value() const
	{
		return s_;
	}


	private: void do_reset()
	{
		s_ = 0;
	}


	private: value_type s_;
	private: real_type a_;
}; // none_filter


/*
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
*/


struct none_filter_info
{
};

template <typename RealT>
struct ewma_filter_info
{
	typedef RealT real_type;

	real_type smoothing_factor;
};

template <typename RealT>
struct filter_info
{
	typedef RealT real_type;

	filter_category category;
	::boost::variant<ewma_filter_info<real_type>,
					 none_filter_info> info;
};


template <typename ValueT, typename RealT>
::dcs::shared_ptr< base_filter<ValueT> > make_filter(filter_info<RealT> const& info)
{
	::dcs::shared_ptr< base_filter<ValueT> > ptr_filter;

	switch (info.category)
	{
		case none_filter_category:
			ptr_filter = ::dcs::make_shared< none_filter<ValueT> >();
			break;
		case ewma_filter_category:
			ptr_filter = ::dcs::make_shared< ewma_filter<ValueT,RealT> >(::boost::get< ewma_filter_info<RealT> >(info.info).smoothing_factor);
			break;
	}

	return ptr_filter;
}

//@} Filter

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
		++n_;
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


template <typename TraitsT>
struct sysid_base_request_info
{
	typedef TraitsT traits_type;
//	typedef typename traits_type::uint_type uint_type;
	typedef typename traits_type::real_type real_type;


	virtual ~sysid_base_request_info() { }


//	uint_type id;
	real_type arr_time;
	real_type dep_time;
//	::std::map<resource_category_type,real_type> share_map;
	bool done; //FIXME: not used for NoAggMeasure/NoAggShare
};


template <typename TraitsT>
struct none_share_aggregation_info
{
	typedef TraitsT traits_type;
	typedef typename traits_type::real_type real_type;
	typedef ::dcs::eesim::physical_resource_category resource_category_type;
	typedef ::std::map<resource_category_type,real_type> resource_share_map; // category => share

	resource_share_map share_map;
};

template <typename TraitsT>
struct mean_share_aggregation_info
{
	typedef TraitsT traits_type;
	typedef typename traits_type::real_type real_type;
	typedef ::dcs::eesim::physical_resource_category resource_category_type;
	typedef detail::mean_statistic<real_type> share_statistic_type;
	typedef ::std::map<resource_category_type,share_statistic_type> resource_share_stat_map; // category => share-stat

	resource_share_stat_map share_map;
};

template <typename TraitsT>
struct weighted_mean_share_aggregation_info
{
	typedef TraitsT traits_type;
	typedef typename traits_type::real_type real_type;
	typedef ::dcs::eesim::physical_resource_category resource_category_type;
	typedef detail::weighted_mean_statistic<real_type> share_statistic_type;
	typedef ::std::map<resource_category_type,real_type> resource_share_map; // category => share
	typedef ::std::map<resource_category_type,share_statistic_type> resource_share_stat_map; // category => share-stat

	resource_share_stat_map share_map;
	real_type last_share_change_time;
	resource_share_map last_share_map;
};

template <typename TraitsT>
struct sysid_siso_request_info: public sysid_base_request_info<TraitsT>
{
	typedef TraitsT traits_type;
	typedef none_share_aggregation_info<traits_type> none_aggregation_info_type;
	typedef mean_share_aggregation_info<traits_type> mean_aggregation_info_type;
	typedef weighted_mean_share_aggregation_info<traits_type> weighted_mean_aggregation_info_type;


	template <typename T>
	T const& get_aggregation_info() const
	{
		return ::boost::get<T>(aggregation_info);
	}

	template <typename T>
	T& get_aggregation_info()
	{
		return ::boost::get<T>(aggregation_info);
	}


	aggregation_category aggregation_cat;
	::boost::variant<none_aggregation_info_type,
					 mean_aggregation_info_type,
					 weighted_mean_aggregation_info_type> aggregation_info;
}; // sysid_siso_request_info


//template <typename TraitsT>
//struct sysid_noagg_measure_noagg_share_siso_request_info: public sysid_base_request_info<TraitsT>
//{
//	typedef TraitsT traits_type;
//	typedef typename traits_type::real_type real_type;
//	typedef ::dcs::eesim::physical_resource_category resource_category_type;
//	typedef ::std::map<resource_category_type,real_type> resource_share_map; // category => share
//	typedef resource_share_map resource_share_stat_map;
//
//	resource_share_stat_map share_map;
//}; // sysid_noagg_measure_noagg_share_siso_request_info


//template <typename TraitsT>
//struct sysid_siso_request_info__: public sysid_base_request_info<TraitsT>
//{
//	typedef TraitsT traits_type;
//	typedef typename traits_type::real_type real_type;
//	typedef ::dcs::eesim::physical_resource_category resource_category_type;
//	typedef ::std::map<resource_category_type,real_type> resource_share_map; // category => share
//#if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
//	typedef resource_share_map resource_share_stat_map;
//#else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
//# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
//	typedef detail::mean_statistic<real_type> share_statistic_type;
//# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
//	typedef typename traits_type::uint_type uint_type;
//	typedef detail::weighted_mean_statistic<real_type> share_statistic_type;
//# else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
//#  error "Aggregate share type not yet implemented!"
//# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
//	typedef ::std::map<resource_category_type,share_statistic_type> resource_share_stat_map;
//#endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
//
//	resource_share_stat_map share_map;
//#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
//# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
////	uint_type share_count;
//# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
//	real_type last_share_change_time;
////	real_type weigths_sum;
//	resource_share_map old_share_map;
//# else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
//#  error "Aggregate share type not yet implemented!"
//# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN
//#endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
//}; // sysid_siso_request_info


template <typename TraitsT>
struct sysid_miso_request_info: public sysid_base_request_info<TraitsT>
{
	typedef TraitsT traits_type;
	typedef none_share_aggregation_info<traits_type> none_aggregation_info_type;
	typedef mean_share_aggregation_info<traits_type> mean_aggregation_info_type;
	typedef weighted_mean_share_aggregation_info<traits_type> weighted_mean_aggregation_info_type;


	template <typename T>
	T const& get_aggregation_info() const
	{
		return ::boost::get<T>(aggregation_info);
	}

	template <typename T>
	T& get_aggregation_info()
	{
		return ::boost::get<T>(aggregation_info);
	}


	aggregation_category aggregation_cat;
	::boost::variant<none_aggregation_info_type,
					 mean_aggregation_info_type,
					 weighted_mean_aggregation_info_type> aggregation_info;
}; // sysid_siso_request_info


//template <typename TraitsT>
//struct sysid_miso_request_info__: public sysid_base_request_info<TraitsT>
//{
//	typedef TraitsT traits_type;
//	typedef typename traits_type::uint_type uint_type;
//	typedef typename traits_type::real_type real_type;
//	typedef ::dcs::eesim::physical_resource_category resource_category_type;
//	typedef ::std::map<resource_category_type, ::std::map<uint_type,real_type> > resource_share_map; // category => {tier_id => share}
//#if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
//	typedef resource_share_map resource_share_stat_map;
//#else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
//# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
//	typedef detail::mean_statistic<real_type> share_statistic_type;
//# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
//	typedef detail::weighted_mean_statistic<real_type> share_statistic_type;
//# else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
//#  error "Aggregate share type not yet implemented!"
//# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
//	typedef ::std::map<resource_category_type, ::std::map<uint_type,share_statistic_type> > resource_share_stat_map;
//#endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
//
////	resource_share_map share_map;//OK
//	resource_share_stat_map share_map;
//#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
//# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
////	uint_type share_count;
//# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
//	real_type last_share_change_time;
////	real_type weigths_sum;
//	resource_share_map old_share_map;
//# else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
//#  error "Aggregate share type not yet implemented!"
//# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN
//#endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
//}; // sysid_miso_request_info


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
	filter_info<real_type> out_filter_info;
};


#ifdef DCS_DEBUG
void stack_tracer()
{
#if __GNUC__
    void *trace_elems[20];
    int trace_elem_count(backtrace( trace_elems, 20 ));
    char **stack_syms(backtrace_symbols( trace_elems, trace_elem_count ));
    for ( int i = 0 ; i < trace_elem_count ; ++i )
    {
        std::cerr << stack_syms[i] << "\n";
    }
    free( stack_syms );

    ::std::exit(1);
#endif // __GNUC__
}   
#endif // DCS_DEBUG

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
	public: typedef detail::base_filter<real_type> filter_type;
	public: typedef ::dcs::shared_ptr<filter_type> filter_pointer;
	protected: typedef ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	protected: typedef ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	protected: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
	private: typedef detail::base_signal_generator<real_type> signal_generator_type;
	protected: typedef ::dcs::shared_ptr<signal_generator_type> signal_generator_pointer;
	private: typedef detail::sysid_state<traits_type> sysid_state_type;
	protected: typedef ::dcs::shared_ptr<sysid_state_type> sysid_state_pointer;
	protected: typedef ::dcs::eesim::physical_resource_category physical_resource_category_type;
	protected: typedef ::std::vector<real_type> tier_share_container; // tier => share
	protected: typedef ::std::map<physical_resource_category_type, tier_share_container> resource_tier_share_map; // category => {tier => share}


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


	public: void identify(application_type& app, signal_generator_pointer const& ptr_sig_gen, uint_type max_num_deps, detail::filter_info<real_type> const& out_filter_info, ::std::vector<real_type> const& init_shares)
	{
		// pre: size(init_shares) == number of shares
		DCS_ASSERT(
				init_shares.size() == app.num_tiers(),
				throw ::std::invalid_argument("[base_system_identificator::identify] Wrong size for the initial shares vector.")
			);

		sysid_state_pointer ptr_sysid_state;

		::std::vector<physical_machine_pointer> pms;
		::std::vector<virtual_machine_pointer> vms;

		size_type num_tiers(app.num_tiers());

		ptr_sysid_state = ::dcs::make_shared<sysid_state_type>();
		ptr_sysid_state->num_arrs = uint_type/*zero*/();
		ptr_sysid_state->num_deps = uint_type/*zero*/();
		ptr_sysid_state->req_info_maps.resize(num_tiers+1); // num_tiers position for each tier + 1 position for the whole app
		ptr_sysid_state->max_num_deps = max_num_deps;
		ptr_sysid_state->out_filter_info = out_filter_info;

		des_engine_pointer ptr_des_eng(::dcs::eesim::registry<traits_type>::instance().des_engine_ptr());

		ptr_des_eng->system_initialization_event_source().connect(
				::dcs::functional::bind(
					&self_type::process_sys_init_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2,
					app,
					ptr_sysid_state
				)
			);
		ptr_des_eng->system_finalization_event_source().connect(
				::dcs::functional::bind(
					&self_type::process_sys_finit_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2,
					app,
					ptr_sysid_state
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
				physical_resource_category_type category((*res_it)->category());

//				real_type share(app.tier(tier_id)->resource_share(category));
//
//				share = ::std::min(share, (*res_it)->utilization_threshold());
//				share *= 0.5; // initial share is set to middle-capacity

				real_type share(::std::min(init_shares[tier_id], (*res_it)->utilization_threshold()));

				ptr_vm->wanted_resource_share(category, share);
				ptr_vm->resource_share(category, share);

				if (last_tier_share_map_.count(category) == 0)
				{
					last_tier_share_map_[category] = tier_share_container(num_tiers, 0);
				}
				last_tier_share_map_[category][tier_id] = share;
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


	protected: resource_tier_share_map& last_tier_share_map()
	{
		return last_tier_share_map_;
	}


	protected: resource_tier_share_map const& last_tier_share_map() const
	{
		return last_tier_share_map_;
	}


	private: void schedule_excite_system_event()
	{
		des_engine_pointer ptr_des_eng(::dcs::eesim::registry<traits_type>::instance().des_engine_ptr());
		ptr_des_eng->schedule_event(ptr_excite_sys_evt_src_, ptr_des_eng->simulated_time()+excite_ts_);
	}


	//@{ Event Handlers


	private: void process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_DEBUG_TRACE("BEGIN Process SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");

		do_process_sys_init_event(evt, ctx, app, ptr_sysid_state);

		schedule_excite_system_event();

		DCS_DEBUG_TRACE("END Process SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_DEBUG_TRACE("BEGIN Process SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ")");

		do_process_sys_finit_event(evt, ctx, app, ptr_sysid_state);

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

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Process EXCITE-SYSTEM-EVENT (Clock: " << ctx.simulated_time() << ")");

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
				physical_resource_category_type category((*res_it)->category());
				//real_type ref_share(app.tier(tier_id)->resource_share(category));
				real_type new_share;

				////FIXME: 0.5 is hard-coded
				//new_share = ::std::max(real_type(min_share), ::std::min(0.5*ref_share*(u(tier_id)+1), (*res_it)->utilization_threshold()));
				new_share = ::std::max(real_type(min_share), ::std::min(u(tier_id), (*res_it)->utilization_threshold()));

				last_tier_share_map_[category][tier_id] = ptr_vm->resource_share(category);

				DCS_DEBUG_TRACE_L(3, "Share Change for Tier '" << tier_id << " and Category: " << category << " --> Old Share: '" << last_tier_share_map_[category][tier_id] << "' - New Share: '" << new_share << "'");

				ptr_vm->wanted_resource_share(category, new_share);
				ptr_vm->resource_share(category, new_share);
			}
		}

		do_process_excite_system_event(evt, ctx, app, ptr_sig_gen, ptr_sysid_state);

		// Reschedule this event
		schedule_excite_system_event();

		DCS_DEBUG_TRACE("(" << this << ") END Process EXCITE-SYSTEM-EVENT (Clock: " << ctx.simulated_time() << ")");
	}


	//@} Event Handlers


	//@{ Polymorphic Event Handlers


	private: virtual void do_process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state) = 0;


	private: virtual void do_process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state) = 0;


	private: virtual void do_process_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state) = 0;


	private: virtual void do_process_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state) = 0;


	private: virtual void do_process_tier_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state) = 0;


	private: virtual void do_process_tier_request_service_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state) = 0;


	private: virtual void do_process_tier_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state) = 0;


	private: virtual void do_process_excite_system_event(des_event_type const& evt, des_engine_context_type& ctx, application_type& app, signal_generator_pointer const& ptr_sig_gen, sysid_state_pointer const& ptr_sysid_state) = 0;


	//@} Polymorphic Event Handlers


	private: real_type excite_ts_;
	private: des_event_source_pointer ptr_excite_sys_evt_src_;
	private: filter_pointer ptr_out_filter_;
	private: resource_tier_share_map last_tier_share_map_;
}; // base_system_identificator


template <typename TraitsT>
const typename TraitsT::real_type base_system_identificator<TraitsT>::min_share = 0.01;


/**
 * \brief SISO system identificator (variant: NM-NS).
 *
 * For each request departed from a tier, output the related residence time and
 * the last share assigned to that tier.
 * For each request departed from the whole application, output the related
 * response time and the unweighted average of the shares each tier had
 * when the request departed from it.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class noagg_measure_noagg_share_siso_system_identificator: public base_system_identificator<TraitsT>
{
	private: typedef base_system_identificator<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	private: typedef ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef detail::base_signal_generator<real_type> signal_generator_type;
	private: typedef ::dcs::shared_ptr<signal_generator_type> signal_generator_pointer;
	private: typedef detail::sysid_state<traits_type> sysid_state_type;
	private: typedef ::dcs::shared_ptr<sysid_state_type> sysid_state_pointer;
	private: typedef detail::sysid_base_request_info<traits_type> sysid_request_info_type;
	private: typedef detail::sysid_siso_request_info<traits_type> sysid_request_info_impl_type;
	private: typedef detail::none_share_aggregation_info<traits_type> share_aggregation_info_impl_type;
	private: typedef ::dcs::shared_ptr<sysid_request_info_type> sysid_request_info_pointer;
	private: typedef ::dcs::shared_ptr<sysid_request_info_impl_type> sysid_request_info_impl_pointer;


	private: static const aggregation_category share_aggregation_category = none_aggregation_category;
	private: static const aggregation_category measure_aggregation_category = none_aggregation_category;


	public: noagg_measure_noagg_share_siso_system_identificator(real_type excite_ts)
	: base_type(excite_ts)
	{
		// empty
	}


	private: void do_process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

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
		uint_type count(0);
		for (resource_iterator it = resources.begin(); it != end_it; ++it)
		{
			::std::cout << ",\"category_" << count << "\",\"share_" << count << "\"";
			++count;
		}
		::std::cout << ",\"rt\"" << ::std::endl;
	}


	private: void do_process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

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
		// Compute and output the response time and the application resource
		// share.
		// As application resource share we take the weighted average of the
		// share assigned to each tier, where the weight is proportional to the
		// residence time of the request in each tier.

		user_request_type req = app.simulation_model().request_state(evt);

		// - Compute the application-level share

		//typedef ::std::map< ::dcs::eesim::physical_resource_category, detail::weighted_mean_statistic<real_type> > resource_share_map;
		typedef ::std::map< ::dcs::eesim::physical_resource_category, detail::mean_statistic<real_type> > resource_share_map;

		uint_type num_tiers(app.num_tiers());
		resource_share_map app_share_map;

		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			if (!ptr_sysid_state->req_info_maps[tier_id].count(req.id()))
			{
				// Current request has not traversed the tier 'tier_id'
				continue;
			}

			sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_req_info );

			sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_req_info_impl );

			share_aggregation_info_impl_type const& share_aggr_info(::boost::get<share_aggregation_info_impl_type>(ptr_req_info_impl->aggregation_info));

			typedef virtual_machine_type::resource_share_container resource_share_container;
			typedef resource_share_container::const_iterator resource_share_iterator;

			virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_vm );

			resource_share_container resource_shares(ptr_vm->resource_shares());
			resource_share_iterator res_end_it(resource_shares.end());
			for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
			{
//				// Add the share of this tier weighted by the request residence
//				// time.
//				// Rationale: the higher is the time spent by a request in a
//				//            tier, the higher should be its contribution for
//				//            the computation of the mean share.
//				real_type w(ptr_req_info_impl->dep_time-ptr_req_info_impl->arr_time);
//
//				(app_share_map[res_it->first])(share_aggr_info.share_map.at(res_it->first), w);
				(app_share_map[res_it->first])(share_aggr_info.share_map.at(res_it->first));
			}
		}

		// - Compute the application-level response time

		real_type rt(ctx.simulated_time()-req.arrival_time());

		// - Output shares and response time

		typedef typename resource_share_map::const_iterator resource_share_iterator;

		::std::cout << -1 // Fake tier-id representing the entire application
					<< "," << req.id()
					<< "," << req.arrival_time()
					<< "," << ctx.simulated_time();

		resource_share_iterator res_end_it(app_share_map.end());
		for (resource_share_iterator res_it = app_share_map.begin(); res_it != res_end_it; ++res_it)
		{
			::std::cout << "," << res_it->first << "," << res_it->second.estimate();
		}

		::std::cout << "," << rt << ::std::endl;

		// - Clean-up memory (this request info)

		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			if (!ptr_sysid_state->req_info_maps[tier_id].count(req.id()))
			{
				// Current request has not traversed the tier 'tier_id'
				continue;
			}

			ptr_sysid_state->req_info_maps[tier_id].erase(req.id());
		}
	}


	private: void do_process_tier_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		// Initialize/Update some request info

		user_request_type req = app.simulation_model().request_state(evt);

		if (!ptr_sysid_state->req_info_maps[tier_id][req.id()])
		{
			ptr_sysid_state->req_info_maps[tier_id][req.id()] = ::dcs::make_shared<sysid_request_info_impl_type>();
		}

		sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info );

		ptr_req_info->arr_time = ctx.simulated_time();
		ptr_req_info->done = false;

		sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info_impl );

		ptr_req_info_impl->aggregation_cat = share_aggregation_category;
		ptr_req_info_impl->aggregation_info = share_aggregation_info_impl_type();
	}


	private: void do_process_tier_request_service_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( tier_id );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		// empty
	}


	private: void do_process_tier_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// Update some request info and output

		user_request_type req = app.simulation_model().request_state(evt);

		sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info );

		sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info_impl );

		ptr_req_info_impl->done = true;
		ptr_req_info_impl->dep_time = ctx.simulated_time();

		real_type rt(ctx.simulated_time()-ptr_req_info_impl->arr_time);

		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_vm );

		// Output execution info for this request

		::std::cout << tier_id
					<< "," << req.id()
					<< "," << req.tier_arrival_times(tier_id).back()
					<< "," << ctx.simulated_time();

		share_aggregation_info_impl_type& share_aggr_info(::boost::get<share_aggregation_info_impl_type>(ptr_req_info_impl->aggregation_info));

		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator res_end_it(resource_shares.end());
		for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
		{
			// Assign to this request the current share of this tier
			share_aggr_info.share_map[res_it->first] = res_it->second;

			// Output the current share
			::std::cout << "," << res_it->first
						<< "," << res_it->second;
		}

		::std::cout << "," << rt << ::std::endl;
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
}; // noagg_measure_noagg_share_siso_system_identificator


/**
 * \brief SISO system identificator (variant: NM-MS).
 *
 * For each request departed from a tier, output the related residence time and
 * the average share computed over all the share assigned to that tier from the
 * beginning of the execution of the request on that tier.
 * For each request departed from the whole application, output the related
 * response time and the unweighted average of the average shares computed for
 * each tier.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class noagg_measure_agg_mean_share_siso_system_identificator: public base_system_identificator<TraitsT>
{
	private: typedef base_system_identificator<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	private: typedef ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef detail::base_signal_generator<real_type> signal_generator_type;
	private: typedef ::dcs::shared_ptr<signal_generator_type> signal_generator_pointer;
	private: typedef detail::sysid_state<traits_type> sysid_state_type;
	private: typedef ::dcs::shared_ptr<sysid_state_type> sysid_state_pointer;
	private: typedef detail::sysid_base_request_info<traits_type> sysid_request_info_type;
	private: typedef detail::sysid_siso_request_info<traits_type> sysid_request_info_impl_type;
	private: typedef detail::mean_share_aggregation_info<traits_type> share_aggregation_info_impl_type;
	private: typedef ::dcs::shared_ptr<sysid_request_info_type> sysid_request_info_pointer;
	private: typedef ::dcs::shared_ptr<sysid_request_info_impl_type> sysid_request_info_impl_pointer;


	private: static const aggregation_category share_aggregation_category = mean_aggregation_category;
	private: static const aggregation_category measure_aggregation_category = none_aggregation_category;


	public: noagg_measure_agg_mean_share_siso_system_identificator(real_type excite_ts)
	: base_type(excite_ts)
	{
		// empty
	}


	private: void do_process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

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
		uint_type count(0);
		for (resource_iterator it = resources.begin(); it != end_it; ++it)
		{
			::std::cout << ",\"category_" << count << "\",\"share_" << count << "\"";
			++count;
		}
		::std::cout << ",\"rt\"" << ::std::endl;
	}


	private: void do_process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

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
		// Compute and output the response time and the application resource
		// share.
		// As application resource share we take the weighted average of the
		// share assigned to each tier, where the weight is proportional to the
		// residence time of the request in each tier.

		user_request_type req = app.simulation_model().request_state(evt);

		// - Compute the application-level share

//		typedef ::std::map< ::dcs::eesim::physical_resource_category, detail::weighted_mean_statistic<real_type> > resource_share_map;
		typedef ::std::map< ::dcs::eesim::physical_resource_category, detail::mean_statistic<real_type> > resource_share_map;

		uint_type num_tiers(app.num_tiers());
		resource_share_map app_share_map;

		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			if (!ptr_sysid_state->req_info_maps[tier_id].count(req.id()))
			{
				// Current request has not traversed the tier 'tier_id'
				continue;
			}

			sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_req_info );

			sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_req_info_impl );

			share_aggregation_info_impl_type const& share_aggr_info(::boost::get<share_aggregation_info_impl_type>(ptr_req_info_impl->aggregation_info));

			typedef virtual_machine_type::resource_share_container resource_share_container;
			typedef resource_share_container::const_iterator resource_share_iterator;

			virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_vm );

			resource_share_container resource_shares(ptr_vm->resource_shares());
			resource_share_iterator res_end_it(resource_shares.end());
			for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
			{
//				// Add the share of this tier weighted by the request residence
//				// time.
//				// Rationale: the higher is the time spent by a request in a
//				//            tier, the higher should be its contribution for
//				//            the computation of the mean share.
//				real_type w(ptr_req_info_impl->dep_time-ptr_req_info_impl->arr_time);
//
//				(app_share_map[res_it->first])(share_aggr_info.share_map.at(res_it->first).estimate(), w);
				(app_share_map[res_it->first])(share_aggr_info.share_map.at(res_it->first).estimate());
			}
		}

		// - Compute the application-level response time

		real_type rt(ctx.simulated_time()-req.arrival_time());

		// - Output shares and response time

		typedef typename resource_share_map::const_iterator resource_share_iterator;

		::std::cout << -1 // Fake tier-id representing the entire application
					<< "," << req.id()
					<< "," << req.arrival_time()
					<< "," << ctx.simulated_time();

		resource_share_iterator res_end_it(app_share_map.end());
		for (resource_share_iterator res_it = app_share_map.begin(); res_it != res_end_it; ++res_it)
		{
			::std::cout << "," << res_it->first << "," << res_it->second.estimate();
		}

		::std::cout << "," << rt << ::std::endl;

		// - Clean-up memory (this request info)

		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			if (!ptr_sysid_state->req_info_maps[tier_id].count(req.id()))
			{
				// Current request has not traversed the tier 'tier_id'
				continue;
			}

			ptr_sysid_state->req_info_maps[tier_id].erase(req.id());
		}
	}


	private: void do_process_tier_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		// Initialize/Update some request info

		user_request_type req = app.simulation_model().request_state(evt);

		if (!ptr_sysid_state->req_info_maps[tier_id][req.id()])
		{
			ptr_sysid_state->req_info_maps[tier_id][req.id()] = ::dcs::make_shared<sysid_request_info_impl_type>();
		}

		sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info );

		ptr_req_info->arr_time = ctx.simulated_time();
		ptr_req_info->done = false;

		sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info_impl );

		ptr_req_info_impl->aggregation_cat = share_aggregation_category;
		ptr_req_info_impl->aggregation_info = share_aggregation_info_impl_type();

		// Accumulate the initial share

		share_aggregation_info_impl_type& share_aggr_info(::boost::get<share_aggregation_info_impl_type>(ptr_req_info_impl->aggregation_info));

		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_vm );

		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator end_it(resource_shares.end());
		for (resource_share_iterator it = resource_shares.begin(); it != end_it; ++it)
		{
			(share_aggr_info.share_map[it->first])(it->second);
		}
	}


	private: void do_process_tier_request_service_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( tier_id );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		// empty
	}


	private: void do_process_tier_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// Update some request info and output

		user_request_type req = app.simulation_model().request_state(evt);

		sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info );

		sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info_impl );

		ptr_req_info_impl->done = true;
		ptr_req_info_impl->dep_time = ctx.simulated_time();

		real_type rt(ctx.simulated_time()-ptr_req_info_impl->arr_time);

		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_vm );

		// Output execution info for this request

		::std::cout << tier_id
					<< "," << req.id()
					<< "," << req.tier_arrival_times(tier_id).back()
					<< "," << ctx.simulated_time();

		share_aggregation_info_impl_type const& share_aggr_info(::boost::get<share_aggregation_info_impl_type>(ptr_req_info_impl->aggregation_info));

		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator res_end_it(resource_shares.end());
		for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
		{
			::std::cout << "," << res_it->first
						<< "," << share_aggr_info.share_map.at(res_it->first).estimate();
		}

		::std::cout << "," << rt << ::std::endl;
	}


	private: void do_process_excite_system_event(des_event_type const& evt, des_engine_context_type& ctx, application_type& app, signal_generator_pointer const& ptr_sig_gen, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sig_gen );

		// For each tier, for each in-service request, for each resource category, update the mean share

		typedef typename sysid_state_type::request_info_map request_info_map;
		typedef typename request_info_map::iterator request_info_map_iterator;
		typedef typename sysid_state_type::request_info_pointer sysid_request_info_pointer;
		typedef typename sysid_state_type::request_info_map request_info_map;;
		typedef typename request_info_map::iterator request_info_map_iterator;
		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		uint_type num_tiers(app.num_tiers());
		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

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

				sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

				// check: paranoid check
				DCS_DEBUG_ASSERT( ptr_req_info_impl );

				if (ptr_req_info_impl->done)
				{
					continue;
				}

				share_aggregation_info_impl_type& share_aggr_info(::boost::get<share_aggregation_info_impl_type>(ptr_req_info_impl->aggregation_info));

				for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
				{
					(share_aggr_info.share_map[res_it->first])(res_it->second);
				}
			}
		}
	}
}; // noagg_measure_agg_mean_share_siso_system_identificator


/**
 * \brief SISO system identificator (variant: NM-WMS).
 *
 * For each request departed from a tier, output the related residence time and
 * the weigthed average share computed over all the shares assigned to that tier
 * from the beginning of the execution of the request on that tier, weighted by
 * the length of the time interval that share is remained assigned.
 * For each request departed from the whole application, output the related
 * response time and the unweighted average of the weighted average shares
 * computed for each tier.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class noagg_measure_agg_wmean_share_siso_system_identificator: public base_system_identificator<TraitsT>
{
	private: typedef base_system_identificator<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
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
	private: typedef detail::weighted_mean_share_aggregation_info<traits_type> share_aggregation_info_impl_type;
	private: typedef ::dcs::shared_ptr<sysid_request_info_type> sysid_request_info_pointer;
	private: typedef ::dcs::shared_ptr<sysid_request_info_impl_type> sysid_request_info_impl_pointer;


	private: static const aggregation_category share_aggregation_category = weighted_mean_aggregation_category;
	private: static const aggregation_category measure_aggregation_category = none_aggregation_category;


	public: noagg_measure_agg_wmean_share_siso_system_identificator(real_type excite_ts)
	: base_type(excite_ts)
	{
		// empty
	}


	private: void do_process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

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
		uint_type count(0);
		for (resource_iterator it = resources.begin(); it != end_it; ++it)
		{
			::std::cout << ",\"category_" << count << "\",\"share_" << count << "\"";
			++count;
		}
		::std::cout << ",\"rt\"" << ::std::endl;
	}


	private: void do_process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

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
		// Compute and output the response time and the application resource
		// share.
		// As application resource share we take the weighted average of the
		// share assigned to each tier, where the weight is proportional to the
		// residence time of the request in each tier.

		user_request_type req = app.simulation_model().request_state(evt);

		// - Compute the application-level share

//		typedef ::std::map< ::dcs::eesim::physical_resource_category, detail::weighted_mean_statistic<real_type> > resource_share_map;
		typedef ::std::map< ::dcs::eesim::physical_resource_category, detail::mean_statistic<real_type> > resource_share_map;

		uint_type num_tiers(app.num_tiers());
		resource_share_map app_share_map;

		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			if (!ptr_sysid_state->req_info_maps[tier_id].count(req.id()))
			{
				// Current request has not traversed the tier 'tier_id'
				continue;
			}

			sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_req_info );

			sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_req_info_impl );

			share_aggregation_info_impl_type const& share_aggr_info(::boost::get<share_aggregation_info_impl_type>(ptr_req_info_impl->aggregation_info));

			typedef virtual_machine_type::resource_share_container resource_share_container;
			typedef resource_share_container::const_iterator resource_share_iterator;

			virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_vm );

			resource_share_container resource_shares(ptr_vm->resource_shares());
			resource_share_iterator res_end_it(resource_shares.end());
			for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
			{
//				// Add the share of this tier weighted by the request residence
//				// time.
//				// Rationale: the higher is the time spent by a request in a
//				//            tier, the higher should be its contribution for
//				//            the computation of the mean share.
//				real_type w(ptr_req_info_impl->dep_time-ptr_req_info_impl->arr_time);
//
//				(app_share_map[res_it->first])(share_aggr_info.share_map.at(res_it->first).estimate(), w);
				(app_share_map[res_it->first])(share_aggr_info.share_map.at(res_it->first).estimate());
			}
		}

		// - Compute the application-level response time

		real_type rt(ctx.simulated_time()-req.arrival_time());

		// - Output shares and response time

		typedef typename resource_share_map::const_iterator resource_share_iterator;

		::std::cout << -1 // Fake tier-id representing the entire application
					<< "," << req.id()
					<< "," << req.arrival_time()
					<< "," << ctx.simulated_time();

		resource_share_iterator share_end_it(app_share_map.end());
		for (resource_share_iterator share_it = app_share_map.begin(); share_it != share_end_it; ++share_it)
		{
			::std::cout << "," << share_it->first << "," << share_it->second.estimate();
		}

		::std::cout << "," << rt << ::std::endl;

		// - Clean-up memory (this request info)

		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			if (!ptr_sysid_state->req_info_maps[tier_id].count(req.id()))
			{
				// Current request has not traversed the tier 'tier_id'
				continue;
			}

			ptr_sysid_state->req_info_maps[tier_id].erase(req.id());
		}
	}


	private: void do_process_tier_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		// Initialize/Update some request info

		user_request_type req = app.simulation_model().request_state(evt);

		if (!ptr_sysid_state->req_info_maps[tier_id][req.id()])
		{
			ptr_sysid_state->req_info_maps[tier_id][req.id()] = ::dcs::make_shared<sysid_request_info_impl_type>();
		}

		sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info );

		ptr_req_info->arr_time = ctx.simulated_time();
		ptr_req_info->done = false;

		sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info_impl );

		ptr_req_info_impl->aggregation_cat = share_aggregation_category;
		ptr_req_info_impl->aggregation_info = share_aggregation_info_impl_type();

		// Remember the initial share for later user.
		// NOTE: we can't accumulate it since we still don't know its weight.
		//       We will got to know only once either a new share is assigned or
		//       the request departs from this tier.

		share_aggregation_info_impl_type& share_aggr_info(::boost::get<share_aggregation_info_impl_type>(ptr_req_info_impl->aggregation_info));

		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_vm );

		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator end_it(resource_shares.end());
		for (resource_share_iterator it = resource_shares.begin(); it != end_it; ++it)
		{
			share_aggr_info.last_share_map[it->first] = it->second;
		}

		share_aggr_info.last_share_change_time = ptr_req_info_impl->arr_time;
	}


	private: void do_process_tier_request_service_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( tier_id );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		// empty
	}


	private: void do_process_tier_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// Update some request info and output

		user_request_type req = app.simulation_model().request_state(evt);

		sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info );

		sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info_impl );

		ptr_req_info_impl->done = true;
		ptr_req_info_impl->dep_time = ctx.simulated_time();

		real_type rt(ctx.simulated_time()-ptr_req_info_impl->arr_time);

		share_aggregation_info_impl_type& share_aggr_info(::boost::get<share_aggregation_info_impl_type>(ptr_req_info_impl->aggregation_info));

		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_vm );

		// Output execution info for this request

		::std::cout << tier_id
					<< "," << req.id()
					<< "," << req.tier_arrival_times(tier_id).back()
					<< "," << ctx.simulated_time();

		// Shares are weighted by the amount of time they have influenced this request execution.
		real_type w(ctx.simulated_time()-share_aggr_info.last_share_change_time);

		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator res_end_it(resource_shares.end());
		for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
		{
			// Update the mean share with the last assigned share and its weight
            real_type last_share(share_aggr_info.last_share_map.at(res_it->first));
            (share_aggr_info.share_map[res_it->first])(last_share, w);
            share_aggr_info.last_share_map[res_it->first] = res_it->second;

			// Output
			::std::cout << "," << res_it->first
						<< "," << share_aggr_info.share_map.at(res_it->first).estimate();
		}

		::std::cout << "," << rt << ::std::endl;
	}


	private: void do_process_excite_system_event(des_event_type const& evt, des_engine_context_type& ctx, application_type& app, signal_generator_pointer const& ptr_sig_gen, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sig_gen );

		// For each tier, for each in-service request, for each resource category, update the mean share

		typedef typename sysid_state_type::request_info_map request_info_map;
		typedef typename request_info_map::iterator request_info_map_iterator;

		uint_type num_tiers(app.num_tiers());
		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_vm );

			typedef virtual_machine_type::resource_share_container resource_share_container;
			typedef resource_share_container::const_iterator resource_share_iterator;

			resource_share_container resource_shares(ptr_vm->resource_shares());
			resource_share_iterator res_end_it(resource_shares.end());

			request_info_map_iterator req_end_it(ptr_sysid_state->req_info_maps[tier_id].end());
			for (request_info_map_iterator req_it = ptr_sysid_state->req_info_maps[tier_id].begin(); req_it != req_end_it; ++req_it)
			{
				sysid_request_info_pointer ptr_req_info(req_it->second);

				// check: paranoid check
				DCS_DEBUG_ASSERT( ptr_req_info );

				sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

				// check: paranoid check
				DCS_DEBUG_ASSERT( ptr_req_info_impl );

				if (ptr_req_info_impl->done)
				{
					continue;
				}

				share_aggregation_info_impl_type& share_aggr_info(::boost::get<share_aggregation_info_impl_type>(ptr_req_info_impl->aggregation_info));

				// Compute the weight assigned to the last share
                real_type w(ctx.simulated_time()-share_aggr_info.last_share_change_time);

				for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
				{
					// Compute the weighted mean until the "last" share.
					// We cannot take into consideration the new share here, because we
					// cannot compute its weight.
					(share_aggr_info.share_map[res_it->first])(share_aggr_info.last_share_map[res_it->first], w);
					share_aggr_info.last_share_map[res_it->first] = res_it->second;
				}

                share_aggr_info.last_share_change_time = ctx.simulated_time();
			}
		}
	}
}; // noagg_measure_agg_wmean_share_siso_system_identificator


/**
 * \brief SISO system identificator (variant: MM-NS).
 *
 * For each excitement interval, output the average residence and response time
 * of requests departed (either from a tier or from the whole application)
 * during ths excitement interval, along with the share assigned to each tier
 * or to the whole application..
 * For computing the share of the whole application, we take the unweighted
 * average of the shares assigned to each tier.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class agg_mean_measure_noagg_share_siso_system_identificator: public base_system_identificator<TraitsT>
{
	private: typedef base_system_identificator<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
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
	private: typedef detail::none_share_aggregation_info<traits_type> share_aggregation_info_impl_type;
	private: typedef ::dcs::shared_ptr<sysid_request_info_type> sysid_request_info_pointer;
	private: typedef ::dcs::shared_ptr<sysid_request_info_impl_type> sysid_request_info_impl_pointer;
	private: typedef detail::mean_statistic<real_type> measure_statistic_type;
	private: typedef ::std::vector<measure_statistic_type> measure_statistic_container;
	private: typedef typename base_type::filter_type filter_type;
	private: typedef typename base_type::filter_pointer filter_pointer;
	//private: typedef ::std::vector<real_type> measure_container;
	private: typedef ::std::vector<filter_pointer> filter_container;


	private: static const aggregation_category share_aggregation_category = none_aggregation_category;
	private: static const aggregation_category measure_aggregation_category = mean_aggregation_category;


	public: agg_mean_measure_noagg_share_siso_system_identificator(real_type excite_ts)
	: base_type(excite_ts)
	{
		// empty
	}


	private: void do_process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		uint_type num_tiers(app.num_tiers());

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
		uint_type count(0);
		for (resource_iterator it = resources.begin(); it != end_it; ++it)
		{
			::std::cout << ",\"category_" << count << "\",\"share_" << count << "\"";
			++count;
		}
		::std::cout << ",\"rt\"" << ::std::endl;

        avg_rt_ = measure_statistic_container(num_tiers+1);
        //smooth_avg_rt_ = measure_container(num_tiers+1, 0);
        filter_avg_rt_ = filter_container(num_tiers+1);
		for (uint_type tid = 0; tid <= num_tiers; ++tid)
		{
			filter_avg_rt_[tid] = detail::make_filter<real_type,real_type>(ptr_sysid_state->out_filter_info);
			//filter_avg_rt_[tid]->reset();
		}
	}


	private: void do_process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		//DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		//DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		print_data(ctx, app);
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
		// Compute the response time but don't print it.

		user_request_type req = app.simulation_model().request_state(evt);

		uint_type num_tiers(app.num_tiers());

		// - Compute the application-level response time

		real_type rt(ctx.simulated_time()-req.arrival_time());

		(avg_rt_[num_tiers])(rt);
::std::cerr << "[offline_sysid] APP '" << app.id() << " - Request '" << req.id() << "' --> " << rt << " (" << avg_rt_[num_tiers].estimate() << ")" << ::std::endl;//XXX

		// - Clean-up memory (this request info)

		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			if (!ptr_sysid_state->req_info_maps[tier_id].count(req.id()))
			{
				// Current request has not traversed the tier 'tier_id'
				continue;
			}

			ptr_sysid_state->req_info_maps[tier_id].erase(req.id());
		}
	}


	private: void do_process_tier_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		// Initialize/Update some request info

		user_request_type req = app.simulation_model().request_state(evt);

		if (!ptr_sysid_state->req_info_maps[tier_id][req.id()])
		{
			ptr_sysid_state->req_info_maps[tier_id][req.id()] = ::dcs::make_shared<sysid_request_info_impl_type>();
		}

		sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);
		ptr_req_info->arr_time = ctx.simulated_time();
		ptr_req_info->done = false;

		sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info_impl );

		ptr_req_info_impl->aggregation_cat = share_aggregation_category;
		ptr_req_info_impl->aggregation_info = share_aggregation_info_impl_type();
	}


	private: void do_process_tier_request_service_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( tier_id );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		// empty
	}


	private: void do_process_tier_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// Update some request info and output

		user_request_type req = app.simulation_model().request_state(evt);

		sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info );

		sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info_impl );

		ptr_req_info_impl->done = true;
		ptr_req_info_impl->dep_time = ctx.simulated_time();

		real_type rt(ctx.simulated_time()-ptr_req_info_impl->arr_time);

		(avg_rt_[tier_id])(rt);
::std::cerr << "[offline_sysid] APP '" << app.id() << " - TIER '" << tier_id << "' - Request '" << req.id() << "' --> " << rt << " (" << avg_rt_[tier_id].estimate() << ")" << ::std::endl;//XXX
	}


	private: void do_process_excite_system_event(des_event_type const& evt, des_engine_context_type& ctx, application_type& app, signal_generator_pointer const& ptr_sig_gen, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sig_gen );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		print_data(ctx, app);
/*

		//typedef ::std::map< ::dcs::eesim::physical_resource_category, detail::weighted_mean_statistic<real_type> > resource_share_map;
		typedef ::std::map< ::dcs::eesim::physical_resource_category, detail::mean_statistic<real_type> > resource_share_map;

		//TODO: make-me a class member parameterizable by the user.
		const real_type smooth_factor(1.0);

		uint_type num_tiers(app.num_tiers());

		resource_share_map app_share_map;

		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
//			if (avg_rt_[tier_id].size() == 0)
//			{
//				::std::clog << "[Warning] No observation for tier '" << tier_id << "' at excitement interval [" << (ctx.simulated_time()-this->sampling_time()) << "," << ctx.simulated_time() << ")." << ::std::endl;
//
//				continue;
//			}

			::std::cout << tier_id
						<< "," << -1 // Fake Request ID
						<< "," << -1 // Fake Arrival Time
						<< "," << -1; // Fake Departure Time

			typedef virtual_machine_type::resource_share_container resource_share_container;
			typedef resource_share_container::const_iterator resource_share_iterator;

			virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_vm );

			resource_share_container resource_shares(ptr_vm->resource_shares());
			resource_share_iterator res_end_it(resource_shares.end());
			for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
			{
				::std::cout << "," << res_it->first
							<< "," << res_it->second;

//				// Accumulate tier share for the application-level share.
//				// The weight for this share is the number of request departed
//				// from this tier during the last excitement interval.
//
//				real_type w(avg_rt_[tier_id].size());
//
//				(app_share_map[res_it->first])(res_it->second, w);
				(app_share_map[res_it->first])(res_it->second);
			}

			smooth_avg_rt_[tier_id] = smooth_factor*avg_rt_[tier_id].estimate()+(1.0-smooth_factor)*smooth_avg_rt_[tier_id];

			::std::cout << "," << smooth_avg_rt_[tier_id]
						<< ::std::endl;
		}

		// Output the application-level execution info
		if (avg_rt_[num_tiers].size() > 0)
		{
			::std::cout << -1 // Fake Tier ID
						<< "," << -1 // Fake Request ID
						<< "," << -1 // Fake Arrival Time
						<< "," << -1; // Fake Departure Time

			typedef typename resource_share_map::const_iterator resource_share_iterator;

			resource_share_iterator res_end_it(app_share_map.end());
			for (resource_share_iterator res_it = app_share_map.begin(); res_it != res_end_it; ++res_it)
			{
				::std::cout << "," << res_it->first
							<< "," << res_it->second.estimate();
			}

			smooth_avg_rt_[num_tiers] = smooth_factor*avg_rt_[num_tiers].estimate()+(1.0-smooth_factor)*smooth_avg_rt_[num_tiers];

			::std::cout << "," << smooth_avg_rt_[num_tiers]
						<< ::std::endl;
		}
*/

		avg_rt_ = measure_statistic_container(app.num_tiers()+1);
	}


	private: void print_data(des_engine_context_type const& ctx, application_type const& app)
	{
		//typedef ::std::map< ::dcs::eesim::physical_resource_category, detail::weighted_mean_statistic<real_type> > resource_share_map;
		typedef ::std::map< ::dcs::eesim::physical_resource_category, detail::mean_statistic<real_type> > resource_share_map;
		typedef typename resource_share_map::const_iterator resource_share_iterator;

		uint_type num_tiers(app.num_tiers());

		resource_share_map app_share_map;

		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			if (avg_rt_[tier_id].size() == 0)
			{
				::std::clog << "[Warning] No observation for tier '" << tier_id << "' at excitement interval [" << (ctx.simulated_time()-this->sampling_time()) << "," << ctx.simulated_time() << ")." << ::std::endl;

				//continue;
				(avg_rt_[tier_id])(::std::numeric_limits<real_type>::infinity());
			}

			::std::cout << tier_id
						<< "," << -1 // Fake Request ID
						<< "," << -1 // Fake Arrival Time
						<< "," << -1; // Fake Departure Time

			typedef typename base_type::resource_tier_share_map resource_tier_share_map;
			typedef typename resource_tier_share_map::const_iterator resource_tier_share_iterator;

			resource_tier_share_map const& last_share_map(this->last_tier_share_map());
			resource_tier_share_iterator res_end_it(last_share_map.end());
			for (resource_tier_share_iterator res_it = last_share_map.begin(); res_it != res_end_it; ++res_it)
			{
				::std::cout << "," << res_it->first
							<< "," << res_it->second[tier_id];

//				// Accumulate tier share for the application-level share.
//				// The weight for this share is the number of request departed
//				// from this tier during the last excitement interval.
//
//				real_type w(avg_rt_[tier_id].size());
//
//				(app_share_map[res_it->first])(res_it->second, w);
				(app_share_map[res_it->first])(res_it->second[tier_id]);
			}

			//smooth_avg_rt_[tier_id] = smooth_factor*avg_rt_[tier_id].estimate()+(1.0-smooth_factor)*smooth_avg_rt_[tier_id];
			(*filter_avg_rt_[tier_id])(avg_rt_[tier_id].estimate());

			//::std::cout << "," << smooth_avg_rt_[tier_id] << ::std::endl;
			::std::cout << "," << filter_avg_rt_[tier_id]->value() << ::std::endl;
		}

		// Output the application-level execution info

		if (avg_rt_[num_tiers].size() == 0)
		{
			::std::clog << "[Warning] No observation for application at excitement interval [" << (ctx.simulated_time()-this->sampling_time()) << "," << ctx.simulated_time() << ")." << ::std::endl;

			//continue;
			(avg_rt_[num_tiers])(::std::numeric_limits<real_type>::infinity());
		}

		::std::cout << -1 // Fake Tier ID
					<< "," << -1 // Fake Request ID
					<< "," << -1 // Fake Arrival Time
					<< "," << -1; // Fake Departure Time

		resource_share_iterator res_end_it(app_share_map.end());
		for (resource_share_iterator res_it = app_share_map.begin(); res_it != res_end_it; ++res_it)
		{
			::std::cout << "," << res_it->first
						<< "," << res_it->second.estimate();
		}

		//smooth_avg_rt_[num_tiers] = smooth_factor*avg_rt_[num_tiers].estimate()+(1.0-smooth_factor)*smooth_avg_rt_[num_tiers];
		(*filter_avg_rt_[num_tiers])(avg_rt_[num_tiers].estimate());

		//::std::cout << "," << smooth_avg_rt_[num_tiers] << ::std::endl;
		::std::cout << "," << filter_avg_rt_[num_tiers]->value() << ::std::endl;
	}


	private: measure_statistic_container avg_rt_;
//	private: measure_container smooth_avg_rt_;
	private: filter_container filter_avg_rt_;
}; // agg_mean_measure_noagg_share_siso_system_identificator


/**
 * \brief SISO system identificator (variant: MM-MS).
 *
 * For each excitement interval, output the average residence and response time
 * of requests departed (either from a tier or from the whole application)
 * during this excitement interval, along with the unweighted average of shares
 * assigned to each tier while that request was running.
 * For computing the share of the whole application, we take the unweighted
 * average of the shares assigned to each tier.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class agg_mean_measure_agg_mean_share_siso_system_identificator: public base_system_identificator<TraitsT>
{
	private: typedef base_system_identificator<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
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
	private: typedef detail::mean_share_aggregation_info<traits_type> share_aggregation_info_impl_type;
	private: typedef ::dcs::shared_ptr<sysid_request_info_type> sysid_request_info_pointer;
	private: typedef ::dcs::shared_ptr<sysid_request_info_impl_type> sysid_request_info_impl_pointer;
	private: typedef detail::mean_statistic<real_type> measure_statistic_type;
	private: typedef ::std::vector<measure_statistic_type> measure_statistic_container;
	private: typedef ::std::vector<real_type> measure_container;
	private: typedef detail::mean_statistic<real_type> share_statistic_type;
	private: typedef ::dcs::eesim::physical_resource_category resource_category_type;
	private: typedef ::std::map<resource_category_type,share_statistic_type> share_statistic_map;
	private: typedef ::std::vector<share_statistic_map> share_statistic_container;


	private: static const aggregation_category share_aggregation_category = mean_aggregation_category;
	private: static const aggregation_category measure_aggregation_category = mean_aggregation_category;


	public: agg_mean_measure_agg_mean_share_siso_system_identificator(real_type excite_ts)
	: base_type(excite_ts)
	{
		// empty
	}


	private: void do_process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

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
		uint_type count(0);
		for (resource_iterator it = resources.begin(); it != end_it; ++it)
		{
			::std::cout << ",\"category_" << count << "\",\"share_" << count << "\"";
			++count;
		}
		::std::cout << ",\"rt\"" << ::std::endl;

        avg_rt_ = measure_statistic_container(app.num_tiers()+1);
        smooth_avg_rt_ = measure_container(app.num_tiers()+1, 0);
		avg_shares_ = share_statistic_container(app.num_tiers()+1);
	}


	private: void do_process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

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
		// Compute the response time but don't print it.

		user_request_type req = app.simulation_model().request_state(evt);

		uint_type num_tiers(app.num_tiers());

		// - Compute the application-level response time

		real_type rt(ctx.simulated_time()-req.arrival_time());

		(avg_rt_[num_tiers])(rt);

		// - Compute the application-level share

		//typedef ::std::map< ::dcs::eesim::physical_resource_category, detail::weighted_mean_statistic<real_type> > resource_share_map;
		typedef ::std::map< ::dcs::eesim::physical_resource_category, detail::mean_statistic<real_type> > resource_share_map;
		typedef typename resource_share_map::const_iterator resource_share_iterator;

		resource_share_map app_share_map;

		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			if (!ptr_sysid_state->req_info_maps[tier_id].count(req.id()))
			{
				// Current request has not traversed the tier 'tier_id'
				continue;
			}

			sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_req_info );

			sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_req_info_impl );

			share_aggregation_info_impl_type const& share_aggr_info(::boost::get<share_aggregation_info_impl_type>(ptr_req_info_impl->aggregation_info));

			typedef virtual_machine_type::resource_share_container resource_share_container;
			typedef resource_share_container::const_iterator resource_share_iterator;

			virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_vm );

			resource_share_container resource_shares(ptr_vm->resource_shares());
			resource_share_iterator res_end_it(resource_shares.end());
			for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
			{
//				real_type w(ptr_req_info_impl->dep_time-ptr_req_info_impl->arr_time);
//
//				(app_share_map[res_it->first])(share_aggr_info.share_map.at(res_it->first).estimate(), w);
				(app_share_map[res_it->first])(share_aggr_info.share_map.at(res_it->first).estimate());
			}
		}

		resource_share_iterator share_end_it(app_share_map.end());
		for (resource_share_iterator share_it = app_share_map.begin(); share_it != share_end_it; ++share_it)
		{
			(avg_shares_[num_tiers][share_it->first])(share_it->second.estimate());
		}

		// - Clean-up memory (this request info)

		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			if (!ptr_sysid_state->req_info_maps[tier_id].count(req.id()))
			{
				// Current request has not traversed the tier 'tier_id'
				continue;
			}

			ptr_sysid_state->req_info_maps[tier_id].erase(req.id());
		}
	}


	private: void do_process_tier_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		// Initialize/Update some request info

		user_request_type req = app.simulation_model().request_state(evt);

		if (!ptr_sysid_state->req_info_maps[tier_id][req.id()])
		{
			ptr_sysid_state->req_info_maps[tier_id][req.id()] = ::dcs::make_shared<sysid_request_info_impl_type>();
		}

		sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);
		ptr_req_info->arr_time = ctx.simulated_time();
		ptr_req_info->done = false;

		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info_impl );

		ptr_req_info_impl->aggregation_cat = share_aggregation_category;
		ptr_req_info_impl->aggregation_info = share_aggregation_info_impl_type();

		share_aggregation_info_impl_type& share_aggr_info(::boost::get<share_aggregation_info_impl_type>(ptr_req_info_impl->aggregation_info));

		virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_vm );

		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator res_end_it(resource_shares.end());
		for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
		{
			(share_aggr_info.share_map[res_it->first])(res_it->second);
		}
	}


	private: void do_process_tier_request_service_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( tier_id );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		// empty
	}


	private: void do_process_tier_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// Update some request info and output

		user_request_type req = app.simulation_model().request_state(evt);

		sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info );

		sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info_impl );

		ptr_req_info_impl->done = true;
		ptr_req_info_impl->dep_time = ctx.simulated_time();

		// Compute the tier residence time

		real_type rt(ctx.simulated_time()-ptr_req_info_impl->arr_time);

		(avg_rt_[tier_id])(rt);

		// Compute the tier-level share

		share_aggregation_info_impl_type const& share_aggr_info(::boost::get<share_aggregation_info_impl_type>(ptr_req_info_impl->aggregation_info));

		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_vm );

		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator res_end_it(resource_shares.end());
		for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
		{
			(avg_shares_[tier_id][res_it->first])(share_aggr_info.share_map.at(res_it->first).estimate());
		}
	}


	private: void do_process_excite_system_event(des_event_type const& evt, des_engine_context_type& ctx, application_type& app, signal_generator_pointer const& ptr_sig_gen, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sig_gen );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		//TODO: make-me a class member parameterizable by the user.
		const real_type smooth_factor(1.0);

		// Output tier-level and application-level execution info
		uint_type num_tiers(app.num_tiers());
		for (uint_type tier_id = 0; tier_id <= num_tiers; ++tier_id)
		{
			if (avg_rt_[tier_id].size() == 0)
			{
				continue;
			}

			// Output resource shares

			::std::cout << static_cast<long>(tier_id < num_tiers ? tier_id : -1) // Real of Fake Tier ID
						<< "," << -1 // Fake Request ID
						<< "," << -1 // Fake Arrival-Time
						<< "," << -1; // Fake Departure Time

			typedef typename share_statistic_map::const_iterator share_statistic_iterator;

			share_statistic_iterator share_end_it(avg_shares_[tier_id].end());
			for (share_statistic_iterator share_it = avg_shares_[tier_id].begin(); share_it != share_end_it; ++share_it)
			{
				::std::cout << "," << share_it->first
							<< "," << share_it->second.estimate();
			}

			// Output (smoothed) residence time

			smooth_avg_rt_[tier_id] = smooth_factor*avg_rt_[tier_id].estimate()+(1.0-smooth_factor)*smooth_avg_rt_[tier_id];

			::std::cout << "," << smooth_avg_rt_[tier_id] << ::std::endl;
		}

		avg_rt_ = measure_statistic_container(num_tiers+1);
		avg_shares_ = share_statistic_container(num_tiers+1);
	}


	private: measure_statistic_container avg_rt_;
	private: measure_container smooth_avg_rt_;
	private: share_statistic_container avg_shares_;
}; // agg_mean_measure_agg_mean_share_siso_system_identificator


/**
 * \brief SISO system identificator (variant: MM-WMS).
 *
 * For each excitement interval, output the unweighted average residence and
 * response time of requests departed (either from a tier or from the whole
 * application) during this excitement interval, along with the unweighted
 * average of the weighted average of shares assigned to each tier while that
 * request was running, weighted according to the distance between the
 * beginning of the request in the tier and the beginning of the last
 * excitement interval.
 * For computing the share of the whole application, we take the unweighted
 * average of the shares assigned to each tier, weighted by the number of
 * request departed from that tier.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
//TODO: we need to better define the meaning of the weights for the mean shares.
template <typename TraitsT>
class agg_mean_measure_agg_wmean_share_siso_system_identificator: public base_system_identificator<TraitsT>
{
	private: typedef base_system_identificator<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
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
	private: typedef detail::weighted_mean_share_aggregation_info<traits_type> share_aggregation_info_impl_type;
	private: typedef ::dcs::shared_ptr<sysid_request_info_type> sysid_request_info_pointer;
	private: typedef ::dcs::shared_ptr<sysid_request_info_impl_type> sysid_request_info_impl_pointer;
	private: typedef detail::mean_statistic<real_type> measure_statistic_type;
	private: typedef ::std::vector<measure_statistic_type> measure_statistic_container;
	private: typedef ::std::vector<real_type> measure_container;
	private: typedef detail::weighted_mean_statistic<real_type> share_statistic_type;
	private: typedef ::dcs::eesim::physical_resource_category resource_category_type;
	private: typedef ::std::map<resource_category_type,share_statistic_type> share_statistic_map;
	private: typedef ::std::vector<share_statistic_map> share_statistic_container;


	private: static const aggregation_category share_aggregation_category = weighted_mean_aggregation_category;
	private: static const aggregation_category measure_aggregation_category = mean_aggregation_category;


	public: agg_mean_measure_agg_wmean_share_siso_system_identificator(real_type excite_ts)
	: base_type(excite_ts)
	{
		// empty
	}


	private: void do_process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

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
		uint_type count(0);
		for (resource_iterator it = resources.begin(); it != end_it; ++it)
		{
			::std::cout << ",\"category_" << count << "\",\"share_" << count << "\"";
			++count;
		}
		::std::cout << ",\"rt\"" << ::std::endl;

        avg_rt_ = measure_statistic_container(app.num_tiers()+1);
        smooth_avg_rt_ = measure_container(app.num_tiers()+1, 0);
		avg_shares_ = share_statistic_container(app.num_tiers()+1);
	}


	private: void do_process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

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
		// Compute the response time but don't print it.

		user_request_type req = app.simulation_model().request_state(evt);

		uint_type num_tiers(app.num_tiers());

		// - Compute the application-level response time

		real_type rt(ctx.simulated_time()-req.arrival_time());

		(avg_rt_[num_tiers])(rt);

//		// - Compute the application-level share
//
////	typedef ::std::map< ::dcs::eesim::physical_resource_category, detail::weighted_mean_statistic<real_type> > resource_share_map;
//		typedef ::std::map< ::dcs::eesim::physical_resource_category, detail::mean_statistic<real_type> > resource_share_map;
//		typedef typename resource_share_map::const_iterator resource_share_iterator;
//
//		resource_share_map app_share_map;
//
//		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
//		{
//			if (!ptr_sysid_state->req_info_maps[tier_id].count(req.id()))
//			{
//				// Current request has not traversed the tier 'tier_id'
//				continue;
//			}
//
//			sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);
//
//			// check: paranoid check
//			DCS_DEBUG_ASSERT( ptr_req_info );
//
//			sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));
//
//			// check: paranoid check
//			DCS_DEBUG_ASSERT( ptr_req_info_impl );
//
//			share_aggregation_info_impl_type const& share_aggr_info(::boost::get<share_aggregation_info_impl_type>(ptr_req_info_impl->aggregation_info));
//
//			typedef virtual_machine_type::resource_share_container resource_share_container;
//			typedef resource_share_container::const_iterator resource_share_iterator;
//
//			virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));
//
//			// check: paranoid check
//			DCS_DEBUG_ASSERT( ptr_vm );
//
//			resource_share_container resource_shares(ptr_vm->resource_shares());
//			resource_share_iterator res_end_it(resource_shares.end());
//			for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
//			{
////				real_type w(ptr_req_info_impl->dep_time-ptr_req_info_impl->arr_time);
////
////				(app_share_map[res_it->first])(share_aggr_info.share_map.at(res_it->first).estimate(), w);
//				(app_share_map[res_it->first])(share_aggr_info.share_map.at(res_it->first).estimate());
//			}
//		}
//
//		// The weight of this request is inversely proportionally to the age of
//		// this request; if the request is begun inside the last excitement
//		// interval, use the max weight (i.e., 1).
//
//		real_type w(1);
//		real_type excite_start_time(ctx.simulated_time()-this->sampling_time());
//		if (req.arrival_time() < excite_start_time)
//		{
//			w = req.arrival_time()/excite_start_time;
//		}
//
//		resource_share_iterator share_end_it(app_share_map.end());
//		for (resource_share_iterator share_it = app_share_map.begin(); share_it != share_end_it; ++share_it)
//		{
//			(avg_shares_[num_tiers][share_it->first])(share_it->second.estimate(), w);
//		}

		// - Clean-up memory (this request info)

		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			if (!ptr_sysid_state->req_info_maps[tier_id].count(req.id()))
			{
				// Current request has not traversed the tier 'tier_id'
				continue;
			}

			ptr_sysid_state->req_info_maps[tier_id].erase(req.id());
		}
	}


	private: void do_process_tier_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		// Initialize/Update some request info

		user_request_type req = app.simulation_model().request_state(evt);

		if (!ptr_sysid_state->req_info_maps[tier_id][req.id()])
		{
			ptr_sysid_state->req_info_maps[tier_id][req.id()] = ::dcs::make_shared<sysid_request_info_impl_type>();
		}

		sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);
		ptr_req_info->arr_time = ctx.simulated_time();
		ptr_req_info->done = false;

		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info_impl );

		ptr_req_info_impl->aggregation_cat = share_aggregation_category;
		ptr_req_info_impl->aggregation_info = share_aggregation_info_impl_type();

		// Remember the initial share for later user.
		// NOTE: we can't accumulate it since we still don't know its weight.
		//       We will got to know only once either a new share is assigned or
		//       the request departs from this tier.

		share_aggregation_info_impl_type& share_aggr_info(::boost::get<share_aggregation_info_impl_type>(ptr_req_info_impl->aggregation_info));

		virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_vm );

		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator res_end_it(resource_shares.end());
		for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
		{
			share_aggr_info.last_share_map[res_it->first] = res_it->second;
		}

		share_aggr_info.last_share_change_time = ptr_req_info_impl->arr_time;
	}


	private: void do_process_tier_request_service_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( tier_id );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		// empty
	}


	private: void do_process_tier_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// Update some request info and output

		user_request_type req = app.simulation_model().request_state(evt);

		sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info );

		sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info_impl );

		ptr_req_info_impl->done = true;
		ptr_req_info_impl->dep_time = ctx.simulated_time();

		// Compute the tier residence time

		real_type rt(ctx.simulated_time()-ptr_req_info_impl->arr_time);

		(avg_rt_[tier_id])(rt);

		// Compute the tier-level share

		share_aggregation_info_impl_type& share_aggr_info(::boost::get<share_aggregation_info_impl_type>(ptr_req_info_impl->aggregation_info));

		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_vm );

		real_type w(ctx.simulated_time()-share_aggr_info.last_share_change_time);

		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator res_end_it(resource_shares.end());
		for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
		{
			// Update the mean share with the last assigned share and its weight
			real_type last_share(share_aggr_info.last_share_map.at(res_it->first));
			(share_aggr_info.share_map[res_it->first])(last_share, w);
			share_aggr_info.last_share_map[res_it->first] = res_it->second;
		}
	}


	private: void do_process_excite_system_event(des_event_type const& evt, des_engine_context_type& ctx, application_type& app, signal_generator_pointer const& ptr_sig_gen, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sig_gen );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		typedef typename sysid_state_type::request_info_map request_info_map;
		typedef typename request_info_map::iterator request_info_map_iterator;

		//TODO: make-me a class member parameterizable by the user.
		const real_type smooth_factor(1.0);

////	typedef ::std::map< ::dcs::eesim::physical_resource_category, detail::weighted_mean_statistic<real_type> > resource_share_map;
		typedef ::std::map< ::dcs::eesim::physical_resource_category, detail::mean_statistic<real_type> > resource_share_map;
		typedef typename resource_share_map::const_iterator resource_share_iterator;

		// Output tier-level and application-level execution info
		uint_type num_tiers(app.num_tiers());
		for (uint_type tier_id = 0; tier_id <= num_tiers; ++tier_id)
		{
			if (avg_rt_[tier_id].size() == 0)
			{
				continue;
			}

			// Finalize the computation of weighted average share.

			if (tier_id < num_tiers)
			{
				virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

				// check: paranoid check
				DCS_DEBUG_ASSERT( ptr_vm );

				typedef virtual_machine_type::resource_share_container resource_share_container;
				typedef resource_share_container::const_iterator resource_share_iterator;

				resource_share_container resource_shares(ptr_vm->resource_shares());
				resource_share_iterator res_end_it(resource_shares.end());

				request_info_map_iterator req_end_it(ptr_sysid_state->req_info_maps[tier_id].end());
				for (request_info_map_iterator req_it = ptr_sysid_state->req_info_maps[tier_id].begin(); req_it != req_end_it; ++req_it)
				{
					sysid_request_info_pointer ptr_req_info(req_it->second);

					// check: paranoid check
					DCS_DEBUG_ASSERT( ptr_req_info );

					sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

					// check: paranoid check
					DCS_DEBUG_ASSERT( ptr_req_info_impl );

					if (ptr_req_info_impl->done)
					{
						continue;
					}

					share_aggregation_info_impl_type& share_aggr_info(::boost::get<share_aggregation_info_impl_type>(ptr_req_info_impl->aggregation_info));

					// Compute the weight assigned to the last share
					real_type w(ctx.simulated_time()-share_aggr_info.last_share_change_time);

					for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
					{
						// Compute the weighted mean until the "last" share.
						// We cannot take into consideration the new share here, because we
						// cannot compute its weight.
						(share_aggr_info.share_map[res_it->first])(share_aggr_info.last_share_map[res_it->first], w);
						share_aggr_info.last_share_map[res_it->first] = res_it->second;
					}

					share_aggr_info.last_share_change_time = ctx.simulated_time();
				}
			}

			// Output resource shares

			::std::cout << static_cast<long>(tier_id < num_tiers ? tier_id : -1) // Real of Fake Tier ID
						<< "," << -1 // Fake Request ID
						<< "," << -1 // Fake Arrival-Time
						<< "," << -1; // Fake Departure Time

			typedef typename share_statistic_map::const_iterator share_statistic_iterator;

			share_statistic_iterator share_end_it(avg_shares_[tier_id].end());
			for (share_statistic_iterator share_it = avg_shares_[tier_id].begin(); share_it != share_end_it; ++share_it)
			{
				::std::cout << "," << share_it->first
							<< "," << share_it->second.estimate();
			}

			// Output (smoothed) residence time

			smooth_avg_rt_[tier_id] = smooth_factor*avg_rt_[tier_id].estimate()+(1.0-smooth_factor)*smooth_avg_rt_[tier_id];

			::std::cout << "," << smooth_avg_rt_[tier_id] << ::std::endl;
		}

		avg_rt_ = measure_statistic_container(num_tiers+1);
		avg_shares_ = share_statistic_container(num_tiers+1);
	}


	private: measure_statistic_container avg_rt_;
	private: measure_container smooth_avg_rt_;
	private: share_statistic_container avg_shares_;
}; // agg_mean_measure_agg_wmean_share_siso_system_identificator


/**
 * \brief MISO system identificator (variant: MM-NS).
 *
 * For each excitement interval, output the average residence and response time
 * of requests departed (either from a tier or from the whole application)
 * during ths excitement interval, along with the share assigned to each tier
 * or to the whole application..
 * For computing the share of the whole application, we take the unweighted
 * average of the shares assigned to each tier.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class agg_mean_measure_noagg_share_miso_system_identificator: public base_system_identificator<TraitsT>
{
	private: typedef base_system_identificator<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
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
	private: typedef detail::none_share_aggregation_info<traits_type> share_aggregation_info_impl_type;
	private: typedef ::dcs::shared_ptr<sysid_request_info_type> sysid_request_info_pointer;
	private: typedef ::dcs::shared_ptr<sysid_request_info_impl_type> sysid_request_info_impl_pointer;
	private: typedef detail::mean_statistic<real_type> measure_statistic_type;
	private: typedef ::std::vector<measure_statistic_type> measure_statistic_container;
	private: typedef typename base_type::filter_type filter_type;
	private: typedef typename base_type::filter_pointer filter_pointer;
	//private: typedef ::std::vector<real_type> measure_container;
	private: typedef ::std::vector<filter_pointer> filter_container;


	private: static const aggregation_category share_aggregation_category = none_aggregation_category;
	private: static const aggregation_category measure_aggregation_category = mean_aggregation_category;


	public: agg_mean_measure_noagg_share_miso_system_identificator(real_type excite_ts)
	: base_type(excite_ts)
	{
		// empty
	}


	private: void do_process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		uint_type num_tiers(app.num_tiers());

		// Output the preamble
		::std::cout << "##" << ::std::endl
					<< "## Application: " << app.name() << ::std::endl
					<< "## Nr. Tiers: " << num_tiers << ::std::endl
					<< "##" << ::std::endl;

		// Output the header
		::std::cout << "\"tid\",\"rid\",\"arrtime\",\"deptime\"";
		typedef ::std::vector<typename application_type::reference_physical_resource_type> resource_container;
		typedef typename resource_container::const_iterator resource_iterator;
		resource_container resources(app.reference_resources());
		resource_iterator end_it(resources.end());
		uint_type count(0);
		for (resource_iterator it = resources.begin(); it != end_it; ++it)
		{
			::std::cout << ",\"category_" << count << "\"";

			for (uint_type tid = 0; tid < num_tiers; ++tid)
			{
				::std::cout << ",\"share_" << tid << "_" << count << "\"";
			}

			++count;
		}
		::std::cout << ",\"rt\"" << ::std::endl;

        avg_rt_ = measure_statistic_container(app.num_tiers()+1);
        //smooth_avg_rt_ = measure_container(app.num_tiers()+1, 0);
		filter_avg_rt_ = filter_container(num_tiers+1);
		for (uint_type tid = 0; tid <= num_tiers; ++tid)
		{
			filter_avg_rt_[tid] = detail::make_filter<real_type,real_type>(ptr_sysid_state->out_filter_info);
		}
	}


	private: void do_process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		//DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		//DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		print_data(ctx, app);
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
		// Compute the response time but don't print it.

		user_request_type req = app.simulation_model().request_state(evt);

		uint_type num_tiers(app.num_tiers());

		// - Compute the application-level response time

		real_type rt(ctx.simulated_time()-req.arrival_time());

		(avg_rt_[num_tiers])(rt);
::std::cerr << "[offline_sysid] APP '" << app.id() << " - Request '" << req.id() << "' --> " << rt << " (" << avg_rt_[num_tiers].estimate() << ")" << ::std::endl;//XXX


		// - Clean-up memory (this request info)

		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			if (!ptr_sysid_state->req_info_maps[tier_id].count(req.id()))
			{
				// Current request has not traversed the tier 'tier_id'
				continue;
			}

			ptr_sysid_state->req_info_maps[tier_id].erase(req.id());
		}
	}


	private: void do_process_tier_request_arrival_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		// Initialize/Update some request info

		user_request_type req = app.simulation_model().request_state(evt);

		if (!ptr_sysid_state->req_info_maps[tier_id][req.id()])
		{
			ptr_sysid_state->req_info_maps[tier_id][req.id()] = ::dcs::make_shared<sysid_request_info_impl_type>();
		}

		sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);
		ptr_req_info->arr_time = ctx.simulated_time();
		ptr_req_info->done = false;

		sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info_impl );

		ptr_req_info_impl->aggregation_cat = share_aggregation_category;
		ptr_req_info_impl->aggregation_info = share_aggregation_info_impl_type();
	}


	private: void do_process_tier_request_service_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( tier_id );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		// empty
	}


	private: void do_process_tier_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// Update some request info and output

		user_request_type req = app.simulation_model().request_state(evt);

		sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info );

		sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info_impl );

		ptr_req_info_impl->done = true;
		ptr_req_info_impl->dep_time = ctx.simulated_time();

		real_type rt(ctx.simulated_time()-ptr_req_info_impl->arr_time);

		(avg_rt_[tier_id])(rt);
::std::cerr << "[offline_sysid] APP '" << app.id() << " - TIER '" << tier_id << "' - Request '" << req.id() << "' --> " << rt << " (" << avg_rt_[tier_id].estimate() << ")" << ::std::endl;//XXX
	}


	private: void do_process_excite_system_event(des_event_type const& evt, des_engine_context_type& ctx, application_type& app, signal_generator_pointer const& ptr_sig_gen, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sig_gen );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		print_data(ctx, app);
/*
		typedef ::std::map< ::dcs::eesim::physical_resource_category, ::std::vector<real_type> > resource_share_map;

		//TODO: make-me a class member parameterizable by the user.
		const real_type smooth_factor(1.0);

		uint_type num_tiers(app.num_tiers());

		resource_share_map share_map;

		// Collect resource share from each tier
		for (uint_type tier_id = 0; tier_id <= num_tiers; ++tier_id)
		{
			if (tier_id == num_tiers)
			{
				// Skip when tier_id represents the overall app
				continue;
			}

			typedef virtual_machine_type::resource_share_container resource_share_container;
			typedef resource_share_container::const_iterator resource_share_iterator;

			virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_vm );

			resource_share_container resource_shares(ptr_vm->resource_shares());
			resource_share_iterator res_end_it(resource_shares.end());
			for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
			{
				if (share_map.count(res_it->first) == 0)
				{
					share_map[res_it->first] = ::std::vector<real_type>(num_tiers);

					// Retrieve the share for current resource from all the tiers
					for (uint_type tid = 0; tid < num_tiers; ++tid)
					{
						if (tid == tier_id)
						{
							share_map[res_it->first][tid] = res_it->second;
						}
						else
						{
							virtual_machine_pointer ptr_vm2(app.simulation_model().tier_virtual_machine(tier_id));

							// check: paranoid check
							DCS_DEBUG_ASSERT( ptr_vm2 );

							real_type vm2_share(0);

							try
							{
								vm2_share = ptr_vm2->resource_share(res_it->first);
							}
							catch (...)
							{
								// Do nothing: this tier does not have this kind of resource -> got a zero share
							}

							share_map[res_it->first][tid] = vm2_share;
						}
					}
				}
			}
		}

		// For each tier, output all shares by resource categoyr.
		for (uint_type tier_id = 0; tier_id <= num_tiers; ++tier_id)
		{
			if (avg_rt_[tier_id].size() == 0)
			{
				// No request departed from this tier
				continue;
			}

			::std::cout << static_cast<long>(tier_id < num_tiers ? tier_id : -1) // Real or Fake Tier ID
						<< "," << -1 // Fake Request ID
						<< "," << -1 // Fake Arrival Time
						<< "," << -1; // Fake Departure Time

			typedef typename resource_share_map::const_iterator resource_share_iterator;

			resource_share_iterator share_end_it(share_map.end());
			for (resource_share_iterator share_it = share_map.begin(); share_it != share_end_it; ++share_it)
			{
				::std::cout << "," << share_it->first;
				for (uint_type tid = 0; tid < num_tiers; ++tid)
				{
					::std::cout << "," << share_it->second[tid];
				}
			}

			smooth_avg_rt_[tier_id] = smooth_factor*avg_rt_[tier_id].estimate()+(1.0-smooth_factor)*smooth_avg_rt_[tier_id];

			::std::cout << "," << smooth_avg_rt_[tier_id]
						<< ::std::endl;
		}
*/

//::std::cerr << "[offline_sysid] Erasing..." << ::std::endl;//XXX
//::std::cerr << "[offline_sysid] APP '" << app.id() << " - TIER '" << 0 << "' --> " << avg_rt_[0].estimate() << ::std::endl;//XXX
//::std::cerr << "[offline_sysid] APP '" << app.id() << " - TIER '" << 1 << "' --> " << avg_rt_[1].estimate() << ::std::endl;//XXX
//::std::cerr << "[offline_sysid] APP '" << app.id() << " - TIER '" << 2 << "' --> " << avg_rt_[2].estimate() << ::std::endl;//XXX
//::std::cerr << "[offline_sysid] APP '" << app.id() << " --> " << avg_rt_[3].estimate() << ::std::endl;//XXX
		avg_rt_ = measure_statistic_container(app.num_tiers()+1);
	}


    private: void print_data(des_engine_context_type const& ctx, application_type const& app)
    {
		uint_type num_tiers(app.num_tiers());

		for (uint_type tier_id = 0; tier_id <= num_tiers; ++tier_id)
		{
			if (avg_rt_[tier_id].size() == 0)
			{
				if (tier_id < num_tiers)
				{
					::std::clog << "[Warning] No observation for tier '" << tier_id << "' at excitement interval [" << (ctx.simulated_time()-this->sampling_time()) << "," << ctx.simulated_time() << ")." << ::std::endl;
				}
				else
				{
					::std::clog << "[Warning] No observation for application at excitement interval [" << (ctx.simulated_time()-this->sampling_time()) << "," << ctx.simulated_time() << ")." << ::std::endl;
				}

				//continue;
				(avg_rt_[tier_id])(::std::numeric_limits<real_type>::infinity());
			}

			// Prevent the implicit cast: (uint_type)-1
			if (tier_id < num_tiers)
			{
				::std::cout << tier_id;
			}
			else
			{
				::std::cout << -1; // Fake Tier ID
			}

			::std::cout	 << "," << -1 // Fake Request ID
						 << "," << -1 // Fake Arrival Time
						 << "," << -1; // Fake Departure Time

			typedef typename base_type::resource_tier_share_map resource_tier_share_map;
			typedef typename resource_tier_share_map::const_iterator resource_tier_share_iterator;

			resource_tier_share_map const& last_share_map(this->last_tier_share_map());
			resource_tier_share_iterator res_end_it(last_share_map.end());
			for (resource_tier_share_iterator res_it = last_share_map.begin(); res_it != res_end_it; ++res_it)
			{
				::std::cout << "," << res_it->first;

				for (uint_type tid = 0; tid < num_tiers; ++tid)
				{
					::std::cout << "," << res_it->second[tid];

				}
			}

			(*filter_avg_rt_[tier_id])(avg_rt_[tier_id].estimate());

			::std::cout << "," << filter_avg_rt_[tier_id]->value() << ::std::endl;
		}
	}

	private: measure_statistic_container avg_rt_;
	private: filter_container filter_avg_rt_;
}; // agg_mean_measure_noagg_share_siso_system_identificator


#if 0
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
#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN)
	private: typedef detail::mean_statistic<real_type> measure_statistic_type;
# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_WEIGHTED_MEAN)
	private: typedef detail::weighted_mean_statistic<real_type> measure_statistic_type;
# else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
#  error "Aggregate measure type not yet implemented!"
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
	private: typedef ::std::vector<measure_statistic_type> measure_statistic_container;
	private: typedef ::std::vector<real_type> measure_container;
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
#  if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
	private: typedef detail::mean_statistic<real_type> share_statistic_type;
#  elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
	private: typedef detail::weighted_mean_statistic<real_type> share_statistic_type;
#  else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
#   error "Aggregate share type not yet implemented!"
#  endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
	private: typedef ::dcs::eesim::physical_resource_category resource_category_type;
	private: typedef ::std::map<resource_category_type,share_statistic_type> share_statistic_map;
	private: typedef ::std::vector<share_statistic_map> share_statistic_container;
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
#endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES


	public: siso_system_identificator(real_type excite_ts)
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
		uint_type count(0);
		for (resource_iterator it = resources.begin(); it != end_it; ++it)
		{
			::std::cout << ",\"category_" << count << "\",\"share_" << count << "\"";
			++count;
		}
		::std::cout << ",\"rt\"" << ::std::endl;

#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
		avg_rt_ = measure_statistic_container(app.num_tiers()+1);
		aggavg_rt_ = measure_container(app.num_tiers(), 0);//FIXME: what is the right size: num_tiers or (num_tiers+1)?
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
		avg_shares_ = share_statistic_container(app.num_tiers()+1);
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
#endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES
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
		// Cases:
		// - Non-aggregated Measures / Non-aggregated Shares: output the application-level execution info for this request.
		// - Non-aggregated Measures / Aggregated Shares: output the application-level execution info for this request.
		// - Aggregate Measures / Non-aggregated Shares: update the application-level aggregated measure value.
		// - Aggregate Measures / Aggregated Shares: update the application-level aggregated measure and share values.

		// Log the response time of the overall application.

		user_request_type req = app.simulation_model().request_state(evt);

#if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
		::std::cout << "-1" // Fake tier-id representing the entire application
					<< "," << req.id()
					<< "," << req.arrival_time()
					<< "," << ctx.simulated_time();
#endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES

		typedef ::std::map< ::dcs::eesim::physical_resource_category, detail::mean_statistic<real_type> > resource_share_map;

		uint_type num_tiers(app.num_tiers());
		resource_share_map app_share_map;

		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			::dcs::shared_ptr<sysid_request_info_type> ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_req_info );

			::dcs::shared_ptr<sysid_request_info_impl_type> ptr_req_info_impl;
			ptr_req_info_impl = ::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_req_info_impl );

#if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
			typedef virtual_machine_type::resource_share_container resource_share_container;
			typedef resource_share_container::const_iterator resource_share_iterator;

			virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_vm );

			resource_share_container resource_shares(ptr_vm->resource_shares());
			resource_share_iterator res_end_it(resource_shares.end());
			for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
			{
				//TODO: maybe, instead of using the simple mean we may use the
				//      weighted mean where the weight is the tier residence time

//				::std::cout << "," << res_it->first
# if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
//							<< "," << ptr_req_info_impl->share_map[res_it->first];
				(app_share_map[res_it->first])(ptr_req_info_impl->share_map[res_it->first]);
# else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
//							<< "," << ptr_req_info_impl->share_map[res_it->first].estimate();
				(app_share_map[res_it->first])(ptr_req_info_impl->share_map[res_it->first].estimate());
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
			}
#endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES
		}

		typedef typename resource_share_map::const_iterator resource_share_iterator;

		resource_share_iterator res_end_it(app_share_map.end());
		for (resource_share_iterator res_it = app_share_map.begin(); res_it != res_end_it; ++res_it)
		{
			::std::cout << "," << res_it->first << "," << res_it->second.estimate();
		}

		real_type rt(ctx.simulated_time()-req.arrival_time());

#if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
		::std::cout << "," << rt
					<< ::std::endl;
#else //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES

# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN)
		(avg_rt_[num_tiers])(rt);

#  if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
		for (resource_share_iterator res_it = app_share_map.begin(); res_it != res_end_it; ++res_it)
		{
#   if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
			(avg_shares_[num_tiers][res_it->first])(res_it->second.estimate());
#   elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
			(avg_shares_[num_tiers][res_it->first])(res_it->second.estimate(), 1);
#   else
#    error "Aggregate share type not yet implemented!"
#   endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
		}
#  endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_WEIGHTED_MEAN)
		real_type w(1);
		real_type excite_start_time(ctx.simulated_time()-this->sampling_time());
		if (req.arrival_time() < excite_start_time)
		{
			w = req.arrival_time()/excite_start_time;
		}

		(avg_rt_[num_tiers])(rt, w);

#  if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
		for (resource_share_iterator res_it = app_share_map.begin(); res_it != res_end_it; ++res_it)
		{
#   if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
			(avg_shares_[num_tiers][res_it->first])(res_it->second.estimate());
#   elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
			(avg_shares_[num_tiers][res_it->first])(res_it->second.estimate(), w);
#   else
#    error "Aggregate share type not yet implemented!"
#   endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
		}
#  endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
# else
#  error "Aggregate measure type not yet implemented!"
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
#endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES

		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			ptr_sysid_state->req_info_maps[tier_id].erase(req.id());
		}
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
#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
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
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
			(ptr_req_info_impl->share_map[it->first])(it->second);
# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
			ptr_req_info_impl->old_share_map[it->first] = it->second;
# else
#  error "Aggregate share type not yet implemented!"
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
		}
		ptr_req_info_impl->done = false;
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
		ptr_req_info_impl->last_share_change_time = ptr_req_info_impl->arr_time;
# else
#  error "Aggregate share type not yet implemented!"
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN
#endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
	}


	private: void do_process_tier_request_service_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

#if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
		// To the just finished request assigne the share currently assigned to this tier

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

		// Aggregated shares are updated later (see TIER-REQUEST-DEPARTURE event handler)
#endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
	}


	private: void do_process_tier_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// Cases:
		// - Non-aggregated Measures / Non-aggregated Shares: output the execution info for this request.
		// - Non-aggregated Measures / Aggregated Shares: update aggregated share value.
		// - Aggregate Measures / Non-aggregated Shares: update aggregated measure value.
		// - Aggregate Measures / Aggregated Shares: update aggregated measure and share values.

		user_request_type req = app.simulation_model().request_state(evt);

#if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
		// Output execution info for this request

		::std::cout << tier_id
					<< "," << req.id()
					<< "," << req.tier_arrival_times(tier_id).back()
					<< "," << ctx.simulated_time();
#endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES

		sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info );

		sysid_request_info_impl_pointer ptr_req_info_impl;
		ptr_req_info_impl = ::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info_impl );

		ptr_req_info_impl->done = true;

		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_vm );

		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator res_end_it(resource_shares.end());
#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
		// Shares are weighted by the amount of time they have influenced this request execution.

		real_type w(ctx.simulated_time()-ptr_req_info_impl->last_share_change_time);
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN
#endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
		for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
		{
#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
			// Update aggregated values.
			// (Note: simple mean aggregation need not to be updated; it is only at the beginning of the request and at every excite signal).

//			ptr_req_info_impl->done = true;
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
			real_type last_share(ptr_req_info_impl->old_share_map.at(res_it->first));
			(ptr_req_info_impl->share_map[res_it->first])(last_share, w);
			ptr_req_info_impl->old_share_map[res_it->first] = res_it->second;
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN
#endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES

#if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
			::std::cout << "," << res_it->first
# if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
						<< "," << ptr_req_info_impl->share_map[res_it->first];
# else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
						<< "," << ptr_req_info_impl->share_map[res_it->first].estimate();
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
#endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES
		}

		real_type rt(ctx.simulated_time()-ptr_req_info_impl->arr_time);

#if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
		::std::cout << "," << rt
					<< ::std::endl;
#else //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES
		// Update aggregated measure value

# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN)
		(avg_rt_[tier_id])(rt);

#  if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
		// Update aggregated share value for the aggregated measure value
		for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
		{
#   if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
			(avg_shares_[tier_id][res_it->first])(ptr_req_info_impl->share_map[res_it->first].estimate());
#   elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
			(avg_shares_[tier_id][res_it->first])(ptr_req_info_impl->share_map[res_it->first].estimate(), 1);
#   else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN
#    error "Aggregate share type not yet implemented!"
#   endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
		}
#  endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_WEIGHTED_MEAN)
#  if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES) && defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
		w = 1;
#  else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
		real_type w(1);
#  endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
		real_type excite_start_time(ctx.simulated_time()-this->sampling_time());
		if (ptr_req_info_impl->arr_time < excite_start_time)
		{
			w = ptr_req_info_impl->arr_time/excite_start_time;
		}
		(avg_rt_[tier_id])(rt, w);
#  if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
		for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
		{
#   if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
			(avg_shares_[tier_id][res_it->first])(ptr_req_info_impl->share_map[res_it->first].estimate());
#   elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
			(avg_shares_[tier_id][res_it->first])(ptr_req_info_impl->share_map[res_it->first].estimate(), w);
#   else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN
#    error "Aggregate share type not yet implemented!"
#   endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
		}
#  endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
# else
#  error "Aggregate measure type not yet implemented!"
# endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
#endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES
	}


	private: void do_process_excite_system_event(des_event_type const& evt, des_engine_context_type& ctx, application_type& app, signal_generator_pointer const& ptr_sig_gen, sysid_state_pointer const& ptr_sysid_state)
	{
		// Cases:
		// - Non-aggregated Measures / Non-aggregated Shares: do nothing
		// - Non-aggregated Measures / Aggregated Shares: update state information of each running request
		// - Aggregated Measures / Non-aggregated Shares: do nothing
		// - Aggregated Measures / Aggregated Shares: update state information of each running request

#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
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

		uint_type num_tiers(app.num_tiers());
		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
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

# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
				// empty
# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
				real_type w(ctx.simulated_time()-ptr_req_info_impl->last_share_change_time);
				ptr_req_info_impl->last_share_change_time = ctx.simulated_time();
# else
#  error "Aggregate share type not yet implemented!"
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
				for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
				{
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
					(ptr_req_info_impl->share_map[res_it->first])(res_it->second);
# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
					// Computer the weighted mean until the "old" share.
					// We cannot take into consideration the new share here, because we
					// cannot compute its weight.
					(ptr_req_info_impl->share_map[res_it->first])(ptr_req_info_impl->old_share_map[res_it->first], w);
					ptr_req_info_impl->old_share_map[res_it->first] = res_it->second;
# else
#  error "Aggregate share type not yet implemented!"
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
				}
			}
		}
#endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES

#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
		for (::std::size_t tier_id = 0; tier_id < app.num_tiers(); ++tier_id)
		{
			if (avg_rt_[tier_id].size() == 0)
			{
				continue;
			}

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
# if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
				::std::cout << "," << it->first
							<< "," << it->second;
# else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
				::std::cout << "," << it->first
							<< "," << avg_shares_[tier_id].at(it->first).estimate(); // FIXME: ok!
//							<< "," << avg_shares_[tier_id].estimate(); // FIXME: ko!
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
			}

			const real_type alpha(1.0);

			aggavg_rt_[tier_id] = alpha*avg_rt_[tier_id].estimate()+(1-alpha)*aggavg_rt_[tier_id];

			::std::cout << "," << aggavg_rt_[tier_id]
						<< ::std::endl;
		}

		avg_rt_ = measure_statistic_container(app.num_tiers()+1);
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
		avg_shares_ = share_statistic_container(app.num_tiers()+1);
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
#else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sig_gen );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );
#endif// DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES
	}


#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
	private: measure_statistic_container avg_rt_;
	private: measure_container aggavg_rt_;
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
	private: share_statistic_container avg_shares_;
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
#endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES
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
	private: typedef ::dcs::shared_ptr<sysid_request_info_type> sysid_request_info_pointer;
	private: typedef ::dcs::shared_ptr<sysid_request_info_impl_type> sysid_request_info_impl_pointer;
#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN)
	private: typedef detail::mean_statistic<real_type> measure_statistic_type;
# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_WEIGHTED_MEAN)
	private: typedef detail::weighted_mean_statistic<real_type> measure_statistic_type;
# else //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
#  error "Aggregate measure type not yet implemented!"
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
	private: typedef ::std::vector<measure_statistic_type> measure_statistic_container;
	private: typedef ::std::vector<real_type> measure_container;
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
#  if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
	private: typedef detail::mean_statistic<real_type> share_statistic_type;
#  elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
	private: typedef detail::weighted_mean_statistic<real_type> share_statistic_type;
#  else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
#   error "Aggregate share type not yet implemented!"
#  endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
	private: typedef ::dcs::eesim::physical_resource_category resource_category_type;
	private: typedef ::std::map<resource_category_type, ::std::map<uint_type,share_statistic_type> > share_statistic_map;
	private: typedef ::std::vector<share_statistic_map> share_statistic_container;
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
#endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES


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
		uint_type count(0);
		uint_type num_tiers(app.num_tiers());
		for (resource_iterator it = resources.begin(); it != end_it; ++it)
		{
			::std::cout << ",\"category_" << count << "\"";
			for (uint_type tid = 0; tid < num_tiers; ++tid)
			{
				::std::cout << ",\"share_" << tid << "_" << count << "\"";
			}
			++count;
		}
		::std::cout << ",\"rt\"" << ::std::endl;

#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
		avg_rt_ = measure_statistic_container(num_tiers+1);
		aggavg_rt_ = measure_container(num_tiers, 0);//FIXME: what is the right size: num_tiers or (num_tiers+1)?
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
		avg_shares_ = share_statistic_container(num_tiers+1);
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
#endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES
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

		// Cases:
		// - Non-aggregated Measures / Non-aggregated Shares: output the application-level execution info for this request.
		// - Non-aggregated Measures / Aggregated Shares: output the application-level execution info for this request.
		// - Aggregate Measures / Non-aggregated Shares: update the application-level aggregated measure value.
		// - Aggregate Measures / Aggregated Shares: update the application-level aggregated measure and share values.

		// Log the response time of the overall application.

		user_request_type req = app.simulation_model().request_state(evt);

//		uint_type front_tid(0);// We take shares and arrival time referred to the first tier
#if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
		::std::cout << "-1" // Fake tier-id representing the entire application
					<< "," << req.id()
					<< "," << req.arrival_time()
					<< "," << ctx.simulated_time();
#endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES

		typedef ::std::map<uint_type,real_type> tier_share_map; // tier => share
		typedef ::std::map< ::dcs::eesim::physical_resource_category, tier_share_map > resource_tier_share_map; // category => {tier => share}

		uint_type num_tiers(app.num_tiers());
		resource_tier_share_map app_share_map;

		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			::dcs::shared_ptr<sysid_request_info_type> ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_req_info );

			::dcs::shared_ptr<sysid_request_info_impl_type> ptr_req_info_impl;
			ptr_req_info_impl = ::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_req_info_impl );

#if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
			typedef virtual_machine_type::resource_share_container resource_share_container;
			typedef resource_share_container::const_iterator resource_share_iterator;

			virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_vm );

			resource_share_container resource_shares(ptr_vm->resource_shares());
			resource_share_iterator res_end_it(resource_shares.end());
			for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
			{
				real_type share(0);

				if (ptr_req_info_impl->share_map[res_it->first].count(tier_id))
				{
# if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
					share = ptr_req_info_impl->share_map[res_it->first].at(tier_id);
# else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
					share = ptr_req_info_impl->share_map[res_it->first].at(tier_id).estimate();
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
				}
				app_share_map[res_it->first][tier_id] = share;
			}
#endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES
		}

		typedef typename resource_tier_share_map::const_iterator resource_tier_share_iterator;
		typedef typename tier_share_map::const_iterator tier_share_iterator;

		resource_tier_share_iterator res_end_it(app_share_map.end());
		for (resource_tier_share_iterator res_it = app_share_map.begin(); res_it != res_end_it; ++res_it)
		{
			::std::cout << "," << res_it->first;

			tier_share_iterator share_end_it(res_it->second.end());
			for (tier_share_iterator share_it = res_it->second.begin(); share_it != share_end_it; ++share_it)
			{
				::std::cout << "," << share_it->second;
			}
		}

		real_type rt(ctx.simulated_time()-req.arrival_time());

#if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
		::std::cout << "," << rt << ::std::endl;
#else //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES

# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN)
        (avg_rt_[num_tiers])(rt);

#  if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
		for (resource_tier_share_iterator res_it = app_share_map.begin(); res_it != res_end_it; ++res_it)
		{
			tier_share_iterator share_end_it(res_it->second.end());
			for (tier_share_iterator share_it = res_it.second.begin(); share_it != share_end_it; ++share_it)
			{
#   if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
            	(avg_shares_[num_tiers][res_it->first][share_it->first])(share_it->second);
#   elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
            	(avg_shares_[num_tiers][res_it->first][share_it->first])(share_it->second, 1);
#   else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
#    error "Aggregate share type not yet implemented!"
#   endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
			}
        }
#  endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_WEIGHTED_MEAN)
		real_type w(1);
        real_type excite_start_time(ctx.simulated_time()-this->sampling_time());
        if (req.arrival_time() < excite_start_time)
        {
            w = req.arrival_time()/excite_start_time;
        }
        (avg_rt_[num_tiers])(rt, w);
#  if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
		for (resource_tier_share_iterator res_it = app_share_map.begin(); res_it != res_end_it; ++res_it)
		{
			tier_share_iterator share_end_it(res_it->second.end());
			for (tier_share_iterator share_it = res_it.second.begin(); share_it != share_end_it; ++share_it)
			{
#   if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
            	(avg_shares_[num_tiers][res_it->first][share_it->first])(share_it->second);
#   elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
            	(avg_shares_[num_tiers][res_it->first][share_it->first])(share_it->second, w);
#   else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
#    error "Aggregate share type not yet implemented!"
#   endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
			}
        }
#  endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
# else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
#  error "Aggregate measure type not yet implemented!"
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
#endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES





//		typedef virtual_machine_type::resource_share_container resource_share_container;
//		typedef resource_share_container::const_iterator resource_share_iterator;
//
//		eirtual_machine_p inter ptr_vm = app.simulation_model().tier_virtual_machine(front_tid);
//
//		// check: paranoid check
//		DCS_DEBUG_ASSERT( ptr_vm );
//
//		resource_share_container resource_shares(ptr_vm->resource_shares());
//		resource_share_iterator end_it(resource_shares.end());
//		for (resource_share_iterator it = resource_shares.begin(); it != end_it; ++it)
//		{
//			for (uint_type tid = 0; tid < num_tiers; ++tid)
//			{
//#   if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
//				(avg_shares_[num_tiers][it->first][tid])(ptr_req_info_impl->share_map[it->first].at(tid).estimate());
//#   elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
//				(avg_shares_[num_tiers][it->first][tid])(ptr_req_info_impl->share_map[it->first].at(tid).estimate(), 1);
//#   else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
//#    error "Aggregate share type not yet implemented!"
//#   endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
//			}
//		}
//#  endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
//# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_WEIGHTED_MEAN)
//		real_type w(1);
//		real_type excite_start_time(ctx.simulated_time()-this->sampling_time());
//		if (req.arrival_time() < excite_start_time)
//		{
//			w = req.arrival_time()/excite_start_time;
//		}
//		(avg_rt_[num_tiers])(rt, w);
//#  if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
//		typedef virtual_machine_type::resource_share_container resource_share_container;
//		typedef resource_share_container::const_iterator resource_share_iterator;
//
//		virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(front_tid);
//
//		// check: paranoid check
//		DCS_DEBUG_ASSERT( ptr_vm );
//
//		resource_share_container resource_shares(ptr_vm->resource_shares());
//		resource_share_iterator end_it(resource_shares.end());
//		for (resource_share_iterator it = resource_shares.begin(); it != end_it; ++it)
//		{
//			for (uint_type tid = 0; tid < num_tiers; ++tid)
//			{
//#   if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
//				(avg_shares_[num_tiers][it->first][tid])(ptr_req_info_impl->share_map[it->first].at(tid).estimate());
//#   elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
//				(avg_shares_[num_tiers][it->first][tid])(ptr_req_info_impl->share_map[it->first].at(tid).estimate(), w);
//#   else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
//#    error "Aggregate share type not yet implemented!"
//#   endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
//			}
//		}
//#  endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
//# else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
//#  error "Aggregate measure type not yet implemented!"
//# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
//#endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES

		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			ptr_sysid_state->req_info_maps[tier_id].erase(req.id());
		}
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

#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		sysid_request_info_impl_pointer ptr_req_info_impl(::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info));

		// Update the share stats of each tier for this request
		uint_type num_tiers(app.num_tiers());
		for (uint_type tid = 0; tid < num_tiers; ++tid)
		{
			virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tid);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_vm );

			resource_share_container resource_shares(ptr_vm->resource_shares());
			resource_share_iterator end_it(resource_shares.end());
			for (resource_share_iterator it = resource_shares.begin(); it != end_it; ++it)
			{
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
				(ptr_req_info_impl->share_map[it->first][tid])(it->second);
# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
				ptr_req_info_impl->old_share_map[it->first][tid] = it->second;
# else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
#  error "Aggregate share type not yet implemented!"
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
			}
		}
		ptr_req_info_impl->done = false;
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
//		ptr_req_info_impl->share_count = 1;
# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
		ptr_req_info_impl->last_share_change_time = ptr_req_info_impl->arr_time;
# else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
#  error "Aggregate share type not yet implemented!"
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN
#endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
	}


	private: void do_process_tier_request_service_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

#if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		user_request_type req = app.simulation_model().request_state(evt);

		uint_type num_tiers(app.num_tiers());
		for (uint_type tid = 0; tid < num_tiers; ++tid)
		{
			typedef virtual_machine_type::resource_share_container resource_share_container;
			typedef resource_share_container::const_iterator resource_share_iterator;

			virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tid);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_vm );

			resource_share_container resource_shares(ptr_vm->resource_shares());
			resource_share_iterator end_it(resource_shares.end());
			for (resource_share_iterator it = resource_shares.begin(); it != end_it; ++it)
			{
				::dcs::shared_ptr<sysid_request_info_type> ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);
				::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info)->share_map[it->first][tid] = it->second;
			}
		}
#else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( tier_id );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );

		// Aggregated shares are updated later (see TIER-REQUEST-DEPARTURE event handler)
#endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
	}


	private: void do_process_tier_request_departure_event(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id, application_type const& app, sysid_state_pointer const& ptr_sysid_state)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

        // Cases:
		// - Non-aggregated Measures / Non-aggregated Shares: output the execution info for this request.
		// - Non-aggregated Measures / Aggregated Shares: update aggregated share value.
		// - Aggregate Measures / Non-aggregated Shares: update aggregated measure value.
		// - Aggregate Measures / Aggregated Shares: update aggregated measure and share values.

		user_request_type req = app.simulation_model().request_state(evt);

#if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
		::std::cout << tier_id
					<< "," << req.id()
					<< "," << req.tier_arrival_times(tier_id).back()
					<< "," << ctx.simulated_time();
#endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES

		sysid_request_info_pointer ptr_req_info(ptr_sysid_state->req_info_maps[tier_id][req.id()]);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info );

		sysid_request_info_impl_pointer ptr_req_info_impl;
		ptr_req_info_impl = ::dcs::dynamic_pointer_cast<sysid_request_info_impl_type>(ptr_req_info);

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_req_info_impl );

		ptr_req_info_impl->done = true;

		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		virtual_machine_pointer ptr_vm(app.simulation_model().tier_virtual_machine(tier_id));

		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_vm );

		resource_share_container resource_shares(ptr_vm->resource_shares());
		resource_share_iterator res_end_it(resource_shares.end());
#if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES) || defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
		uint_type num_tiers(app.num_tiers());
#endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES
#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
        real_type w(ctx.simulated_time()-ptr_req_info_impl->last_share_change_time);
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN
#endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
		for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
		{
#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
			for (uint_type tid = 0; tid < num_tiers; ++tid)
			{
				virtual_machine_pointer ptr_vm2(app.simulation_model().tier_virtual_machine(tid));

				// check: paranoid check
				DCS_DEBUG_ASSERT( ptr_vm2 );

				real_type last_share(ptr_req_info_impl->old_share_map.at(res_it->first).at(tid));
				(ptr_req_info_impl->share_map[res_it->first][tid])(last_share, w);
				ptr_req_info_impl->old_share_map[res_it->first][tid] = ptr_vm2->resource_share(res_it->first);
			}
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN
#endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
#if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
			::std::cout << "," << res_it->first;
			for (uint_type tid = 0; tid < num_tiers; ++tid)
			{
				real_type share(0);

				if (ptr_req_info_impl->share_map[res_it->first].count(tid))
				{
# if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
					share = ptr_req_info_impl->share_map[res_it->first].at(tid);
# else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
                    share = ptr_req_info_impl->share_map[res_it->first].at(tid).estimate();
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
				}
				::std::cout << "," << share;
			}
#endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES
		}

		real_type rt(ctx.simulated_time()-ptr_req_info->arr_time);

#if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
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
		::std::cout << "," << rt << ::std::endl;
#else //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN)
		// Update aggregated measure value
        (avg_rt_[tier_id])(rt);

#  if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
		// Update aggregated share values for the aggregated measure value
        for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
        {
			for (uint_type tid = 0; tid < num_tiers; ++tid)
			{
#   if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
            	(avg_shares_[tier_id][res_it->first][tid])(ptr_req_info_impl->share_map[res_it->first].at(tid).estimate());
#   elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
            	(avg_shares_[tier_id][res_it->first][tid])(ptr_req_info_impl->share_map[res_it->first].at(tid).estimate(), 1);
#   else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
#    error "Aggregate share type not yet implemented!"
#   endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
			}
        }
#  endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_WEIGHTED_MEAN)
#  if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES) && defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
        w = 1;
#  else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
        real_type w(1);
#  endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
        real_type excite_start_time(ctx.simulated_time()-this->sampling_time());
        if (ptr_req_info_impl->arr_time < excite_start_time)
        {
            w = ptr_req_info_impl->arr_time/excite_start_time;
        }
        (avg_rt_[tier_id])(rt, w);
#  if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
        for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
        {
			for (uint_type tid = 0; tid < num_tiers; ++tid)
			{
#   if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
            	(avg_shares_[tier_id][res_it->first][tid])(ptr_req_info_impl->share_map[res_it->first].at(tid).estimate());
#   elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
            	(avg_shares_[tier_id][res_it->first][tid])(ptr_req_info_impl->share_map[res_it->first].at(tid).estimate(), w);
#   else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
#    error "Aggregate share type not yet implemented!"
#   endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
			}
        }
#  endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
# else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
#  error "Aggregate measure type not yet implemented!"
# endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES_BY_SIMPLE_MEAN
#endif //DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES

//		ptr_sysid_state->req_info_maps[tier_id].erase(req.id());
	}


	private: void do_process_excite_system_event(des_event_type const& evt, des_engine_context_type& ctx, application_type& app, signal_generator_pointer const& ptr_sig_gen, sysid_state_pointer const& ptr_sysid_state)
	{
		// Cases:
		// - Non-aggregated Measures / Non-aggregated Shares: do nothing
		// - Non-aggregated Measures / Aggregated Shares: update state information of each running request
		// - Aggregated Measures / Non-aggregated Shares: do nothing
		// - Aggregated Measures / Aggregated Shares: update state information of each running request

#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES) || defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
		uint_type num_tiers(app.num_tiers());
#endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES

#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
		// For each tier, for each in-service request, for each resource category, update the mean share

		typedef typename sysid_state_type::request_info_map request_info_map;
		typedef typename request_info_map::iterator request_info_map_iterator;
		typedef typename sysid_state_type::request_info_pointer sysid_request_info_pointer;
		typedef detail::sysid_miso_request_info<traits_type> sysid_request_info_impl_type;
		typedef ::dcs::shared_ptr<sysid_request_info_impl_type> sysid_request_info_impl_pointer;
		typedef typename sysid_request_info_impl_type::resource_share_map resource_share_map;
		typedef typename resource_share_map::iterator resource_share_map_iterator;
		typedef virtual_machine_type::resource_share_container resource_share_container;
		typedef resource_share_container::const_iterator resource_share_iterator;

		// Iterate over all tiers
		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			virtual_machine_pointer ptr_vm = app.simulation_model().tier_virtual_machine(tier_id);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_vm );

			resource_share_container resource_shares(ptr_vm->resource_shares());
			resource_share_iterator res_end_it(resource_shares.end());

			// Iterate over all requests
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

				// Check if this request still is in-service or is done
				if (ptr_req_info_impl->done)
				{
					continue;
				}

				// Update resource share stats for this request
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
				// empty
# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
				// Weights current shares by the time this request has seen changed these shares
				real_type w(ctx.simulated_time()-ptr_req_info_impl->last_share_change_time);
				ptr_req_info_impl->last_share_change_time = ctx.simulated_time();
# else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
#  error "Aggregate share type not yet implemented!"
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
				for (resource_share_iterator res_it = resource_shares.begin(); res_it != res_end_it; ++res_it)
				{
					// Since we have a MISO system, we need to take care of share from all tiers
					for (uint_type tid = 0; tid < num_tiers; ++tid)
					{
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN)
						(ptr_req_info_impl->share_map[res_it->first][tid])(res_it->second);
# elif defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_WEIGHTED_MEAN)
						// Computer the weighted mean until the "old" share.
						// We cannot take into consideration the new share here, because we
						// cannot compute its weight.
						(ptr_req_info_impl->share_map[res_it->first][tid])(ptr_req_info_impl->old_share_map[res_it->first][tid], w);
						ptr_req_info_impl->old_share_map[res_it->first][tid] = res_it->second;
# else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
#  error "Aggregate share type not yet implemented!"
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES_BY_SIMPLE_MEAN
					}
				}
			}
		}
#endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES

#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
		for (uint_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			if (avg_rt_[tier_id].size() == 0)
			{
				continue;
			}

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
				::std::cout << "," << it->first;
				for (uint_type tid = 0; tid < num_tiers; ++tid)
				{
# if !defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
					::std::cout << "," << it->second;
# else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
					::std::cout << "," << avg_shares_[tier_id].at(it->first).at(tid).estimate(); // FIXME: ok!
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
				}
			}

			const real_type alpha(1.0);

			aggavg_rt_[tier_id] = alpha*avg_rt_[tier_id].estimate()+(1-alpha)*aggavg_rt_[tier_id];

			::std::cout << "," << aggavg_rt_[tier_id] << ::std::endl;
		}

		avg_rt_ = measure_statistic_container(num_tiers+1);
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
		avg_shares_ = share_statistic_container(num_tiers+1);
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
#else // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( app );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sig_gen );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_sysid_state );
#endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES
	}


#if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES)
	private: measure_statistic_container avg_rt_;
	private: measure_container aggavg_rt_;
# if defined(DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES)
	private: share_statistic_container avg_shares_;
# endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_SHARES
#endif // DCS_EESIM_EXP_OFFSYSID_AGGREGATE_MEASURES
}; // miso_system_identificator
#endif // if 0


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


#ifdef DCS_DEBUG
	::std::set_terminate(detail::stack_tracer);
#endif // DCS_DEBUG

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
	real_type sig_sine_ampl;
	uint_type sig_sine_freq;
	uint_type sig_sine_phase;
	real_type sig_sine_bias;
	real_type sig_unif_min;
	real_type sig_unif_max;
	real_type sig_gauss_mean;
	real_type sig_gauss_sd;
	system_identification_category sysid_category;
	aggregation_category in_aggr_category;
	aggregation_category out_aggr_category;
	filter_category out_filt_category;
	real_type out_filt_ewma_smooth_factor;
	std::string conf_fname;

	try
	{
		excite_sampling_time = detail::get_option<real_type>(argv, argv+argc, "--ts");
		num_samples = detail::get_option<uint_type>(argv, argv+argc, "--ns");
		sysid_category = detail::parse_system_identification_category(detail::get_option<std::string>(argv, argv+argc, "--sys"));
		sig_category = detail::parse_signal_category(detail::get_option<std::string>(argv, argv+argc, "--sig"));
		sig_sine_ampl =  detail::get_option<real_type>(argv, argv+argc, "--sig-sine-amplitude", 0.5);
		sig_sine_freq =  detail::get_option<uint_type>(argv, argv+argc, "--sig-sine-frequency", 8);
		sig_sine_phase =  detail::get_option<uint_type>(argv, argv+argc, "--sig-sine-phase", sig_sine_freq/4);
		sig_sine_bias =  detail::get_option<real_type>(argv, argv+argc, "--sig-sine-bias", 0.5);
		sig_unif_min =  detail::get_option<real_type>(argv, argv+argc, "--sig-uniform-min", 0.0);
		sig_unif_max =  detail::get_option<real_type>(argv, argv+argc, "--sig-uniform-max", 1.0);
		sig_gauss_mean =  detail::get_option<real_type>(argv, argv+argc, "--sig-gaussian-mean", 0.0);
		sig_gauss_sd =  detail::get_option<real_type>(argv, argv+argc, "--sig-gaussian-sd", 1.0);
		in_aggr_category = detail::parse_input_aggregation_category(detail::get_option<std::string>(argv, argv+argc, "--inaggr"));
		out_aggr_category = detail::parse_output_aggregation_category(detail::get_option<std::string>(argv, argv+argc, "--outaggr", "none"));
//		in_filter_category = detail::parse_input_filter_category(detail::get_option<std::string>(argv, argv+argc, "--infilt"));
		out_filt_category = detail::parse_output_filter_category(detail::get_option<std::string>(argv, argv+argc, "--outfilt", "none"));
		out_filt_ewma_smooth_factor = detail::get_option<real_type>(argv, argv+argc, "--outfilt-ewma-alpha", 0.5);
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
			case gaussian_white_noise_signal:
				ptr_sig_gen = dcs::make_shared< detail::gaussian_signal_generator<real_type> >(
								ublas::scalar_vector<real_type>(ptr_app->num_tiers(), sig_gauss_mean),
								ublas::scalar_vector<real_type>(ptr_app->num_tiers(), sig_gauss_sd)
					);
				break;
//			case triangular_signal:
//				//TODO
//				break;
//			case parabolic_signal: // p(t)=0.5*t^2*u(t), where u(t) is the unit step function
//				//TODO
//				break;
			case ramp_signal:
				ptr_sig_gen = dcs::make_shared< detail::ramp_signal_generator<real_type> >(
								ublas::scalar_vector<real_type>(ptr_app->num_tiers(), 0),
								ublas::scalar_vector<real_type>(ptr_app->num_tiers(), 0.1)
					);
				break;
			case sinusoidal_signal:
				ptr_sig_gen = dcs::make_shared< detail::sinusoidal_signal_generator<real_type> >(
//								ublas::scalar_vector<real_type>(ptr_app->num_tiers(), 0.3), // amplitude
//								ublas::scalar_vector<uint_type>(ptr_app->num_tiers(), 8), // # of samples per cycle
//								ublas::scalar_vector<uint_type>(ptr_app->num_tiers(), 2), // phase-shift (offset in # of samples)
//								ublas::scalar_vector<real_type>(ptr_app->num_tiers(), 0.5) // bias
								ublas::scalar_vector<real_type>(ptr_app->num_tiers(), sig_sine_ampl), // amplitude
								ublas::scalar_vector<uint_type>(ptr_app->num_tiers(), sig_sine_freq), // # of samples per cycle
								ublas::scalar_vector<uint_type>(ptr_app->num_tiers(), sig_sine_phase), // phase-shift (offset in # of samples)
								ublas::scalar_vector<real_type>(ptr_app->num_tiers(), sig_sine_bias) // bias
					);
				break;
			case step_signal:
				ptr_sig_gen = dcs::make_shared< detail::step_signal_generator<real_type> >(ublas::scalar_vector<real_type>(ptr_app->num_tiers(), 0));
				break;
			case uniform_signal:
				ptr_sig_gen = dcs::make_shared< detail::uniform_signal_generator<real_type> >(
								ublas::scalar_vector<real_type>(ptr_app->num_tiers(), sig_unif_min), // min
								ublas::scalar_vector<real_type>(ptr_app->num_tiers(), sig_unif_max) // max
					);
				break;
		}

		::dcs::shared_ptr<system_identificator_type> ptr_sysid;
		switch (sysid_category)
		{
			case siso_system_identification:
				////ptr_sysid = ::dcs::make_shared< siso_system_identificator<traits_type> >(excite_sampling_time);
				switch (in_aggr_category)
				{
					case none_aggregation_category:
						switch (out_aggr_category)
						{
							case none_aggregation_category:
								ptr_sysid = ::dcs::make_shared< noagg_measure_noagg_share_siso_system_identificator<traits_type> >(excite_sampling_time);
								break;
							case mean_aggregation_category:
								ptr_sysid = ::dcs::make_shared< agg_mean_measure_noagg_share_siso_system_identificator<traits_type> >(excite_sampling_time);
								break;
							case weighted_mean_aggregation_category:
								throw ::std::runtime_error("Not yet implemented.");
								//ptr_sysid = ::dcs::make_shared< agg_wmean_measure_noagg_share_siso_system_identificator<traits_type> >(excite_sampling_time);
								break;
						}
						break;
					case mean_aggregation_category:
						switch (out_aggr_category)
						{
							case none_aggregation_category:
								ptr_sysid = ::dcs::make_shared< noagg_measure_agg_mean_share_siso_system_identificator<traits_type> >(excite_sampling_time);
								break;
							case mean_aggregation_category:
								ptr_sysid = ::dcs::make_shared< agg_mean_measure_agg_mean_share_siso_system_identificator<traits_type> >(excite_sampling_time);
								break;
							case weighted_mean_aggregation_category:
								throw ::std::runtime_error("Not yet implemented.");
								//ptr_sysid = ::dcs::make_shared< agg_wmean_measure_agg_mean_share_siso_system_identificator<traits_type> >(excite_sampling_time);
								break;
						}
						break;
					case weighted_mean_aggregation_category:
						switch (out_aggr_category)
						{
							case none_aggregation_category:
								ptr_sysid = ::dcs::make_shared< noagg_measure_agg_wmean_share_siso_system_identificator<traits_type> >(excite_sampling_time);
								break;
							case mean_aggregation_category:
								throw ::std::runtime_error("Not yet implemented.");
								//ptr_sysid = ::dcs::make_shared< agg_mean_measure_agg_wmean_share_siso_system_identificator<traits_type> >(excite_sampling_time);
								break;
							case weighted_mean_aggregation_category:
								throw ::std::runtime_error("Not yet implemented.");
								//ptr_sysid = ::dcs::make_shared< agg_wmean_measure_agg_wmean_share_siso_system_identificator<traits_type> >(excite_sampling_time);
								break;
						}
						break;
				}
				break;
			case miso_system_identification:
				//ptr_sysid = ::dcs::make_shared< miso_system_identificator<traits_type> >(excite_sampling_time);
				switch (in_aggr_category)
				{
					case none_aggregation_category:
						switch (out_aggr_category)
						{
							case none_aggregation_category:
								throw ::std::runtime_error("Not yet implemented.");
								//ptr_sysid = ::dcs::make_shared< noagg_measure_noagg_share_miso_system_identificator<traits_type> >(excite_sampling_time);
								break;
							case mean_aggregation_category:
								ptr_sysid = ::dcs::make_shared< agg_mean_measure_noagg_share_miso_system_identificator<traits_type> >(excite_sampling_time);
								break;
							case weighted_mean_aggregation_category:
								throw ::std::runtime_error("Not yet implemented.");
								//ptr_sysid = ::dcs::make_shared< agg_wmean_measure_noagg_share_miso_system_identificator<traits_type> >(excite_sampling_time);
								break;
						}
						break;
					case mean_aggregation_category:
						switch (out_aggr_category)
						{
							case none_aggregation_category:
								throw ::std::runtime_error("Not yet implemented.");
								//ptr_sysid = ::dcs::make_shared< noagg_measure_agg_mean_share_miso_system_identificator<traits_type> >(excite_sampling_time);
								break;
							case mean_aggregation_category:
								throw ::std::runtime_error("Not yet implemented.");
								//ptr_sysid = ::dcs::make_shared< agg_mean_measure_agg_mean_share_miso_system_identificator<traits_type> >(excite_sampling_time);
								break;
							case weighted_mean_aggregation_category:
								throw ::std::runtime_error("Not yet implemented.");
								//ptr_sysid = ::dcs::make_shared< agg_wmean_measure_agg_mean_share_miso_system_identificator<traits_type> >(excite_sampling_time);
								break;
						}
						break;
					case weighted_mean_aggregation_category:
						switch (out_aggr_category)
						{
							case none_aggregation_category:
								throw ::std::runtime_error("Not yet implemented.");
								//ptr_sysid = ::dcs::make_shared< noagg_measure_agg_wmean_share_miso_system_identificator<traits_type> >(excite_sampling_time);
								break;
							case mean_aggregation_category:
								throw ::std::runtime_error("Not yet implemented.");
								//ptr_sysid = ::dcs::make_shared< agg_mean_measure_agg_wmean_share_miso_system_identificator<traits_type> >(excite_sampling_time);
								break;
							case weighted_mean_aggregation_category:
								throw ::std::runtime_error("Not yet implemented.");
								//ptr_sysid = ::dcs::make_shared< agg_wmean_measure_agg_wmean_share_miso_system_identificator<traits_type> >(excite_sampling_time);
								break;
						}
						break;
				}
				break;
		}

		// Build the output filter info
		detail::filter_info<real_type> out_filter_info;
		out_filter_info.category = out_filt_category;
		switch (out_filt_category)
		{
			case none_filter_category:
				out_filter_info.info = detail::none_filter_info();
				break;
			case ewma_filter_category:
				{
					detail::ewma_filter_info<real_type> filter_info_impl;
					filter_info_impl.smoothing_factor = out_filt_ewma_smooth_factor;
					out_filter_info.info = filter_info_impl;
				}
				break;
		}

		::std::vector<real_type> init_shares(ptr_app->num_tiers(), 1);

//		sysid->des_engine(ptr_des_eng);
//		sysid->uniform_random_generator(ptr_rng);
		ptr_sysid->identify(*ptr_app, ptr_sig_gen, num_samples, out_filter_info, init_shares);

		//// Report statistics
		//detail::report_stats(::std::cout, ptr_dc);
	}
}
