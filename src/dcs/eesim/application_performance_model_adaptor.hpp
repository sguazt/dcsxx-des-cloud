#ifndef DCS_EESIM_APPLICATION_PERFORMANCE_MODEL_ADAPTOR_HPP
#define DCS_EESIM_APPLICATION_PERFORMANCE_MODEL_ADAPTOR_HPP


#include <dcs/eesim/application_performance_model_traits.hpp>
#include <dcs/eesim/base_application_performance_model.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <dcs/exception.hpp>
#include <stdexcept>


namespace dcs { namespace eesim {

template <
	typename TraitsT,
	typename ModelT,
	typename ModelTraitsT = application_performance_model_traits<TraitsT,ModelT>
>
class application_performance_model_adaptor: public base_application_performance_model<TraitsT>
{
	private: typedef base_application_performance_model<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef ModelT model_type;
	public: typedef ModelTraitsT model_traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
//	public: typedef ::dcs::shared_ptr<model_type> model_pointer;


	public: explicit application_performance_model_adaptor(model_type const& model)
	: base_type(),
	  model_(model)
	{
	}


	/// Copy constructor.
	private: application_performance_model_adaptor(application_performance_model_adaptor const& that)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(that);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy constructor not yet implemented." );
	}


	/// Copy assignment.
	private: application_performance_model_adaptor& operator=(application_performance_model_adaptor const& rhs)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(rhs);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy assignment not yet implemented." );
	}


	private: real_type do_application_measure(performance_measure_category category) const
	{
		return model_traits_type::application_measure(model_, category);
	}


	private: real_type do_tier_measure(uint_type tier_id, performance_measure_category category) const
	{
		return model_traits_type::tier_measure(model_, tier_id, category);
	}


	private: model_type model_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_APPLICATION_PERFORMANCE_MODEL_ADAPTOR_HPP
