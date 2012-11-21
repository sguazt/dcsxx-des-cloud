/**
 * \file dcs/eesim/registry.hpp
 *
 * \brief Global registry class.
 *
 * Copyright (C) 2009-2011  Distributed Computing System (DCS) Group, Computer
 * Science Department - University of Piemonte Orientale, Alessandria (Italy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_EESIM_REGISTRY_HPP
#define DCS_EESIM_REGISTRY_HPP


#define DCS_EESIM_REGISTRY_USE_ABRAHAMS_SINGLETON
//#undef DCS_EESIM_REGISTRY_USE_ABRAHAMS_SINGLETON


#include <dcs/config/boost.hpp>

#if defined(DCS_EESIM_REGISTRY_USE_ABRAHAMS_SINGLETON)

# if !DCS_CONFIG_BOOST_CHECK_VERSION(103200) // 1.32
#  error "Required Boost libraries >= 1.32."
# endif

# include <boost/serialization/singleton.hpp>

#else // DCS_EESIM_REGISTRY_USE_ABRAHAMS_SINGLETON


# if !DCS_CONFIG_BOOST_CHECK_VERSION(102500) // 1.25
#  error "Required Boost libraries >= 1.25."
# endif

# include <boost/scoped_ptr.hpp>
# include <boost/thread/once.hpp>
# include <boost/utility.hpp>
# include <dcs/type_traits/add_const.hpp>
# include <dcs/type_traits/add_reference.hpp>


#endif // DCS_EESIM_REGISTRY_USE_ABRAHAMS_SINGLETON

#include <dcs/eesim/config/configuration.hpp>
#include <dcs/eesim/identifier_generator.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace eesim {

#if !defined(DCS_EESIM_REGISTRY_USE_ABRAHAMS_SINGLETON)

namespace detail { namespace /*<unnamed>*/ {

/**
 * \brief Thread safe lazy singleton template class.
 *
 * This class is a thread-safe lazy singleton template class, which can be used
 * during static initialization or anytime after.
 *
 * Original code found at http://www.boostcookbook.com/Recipe:/1235044
 *
 * \note
 *  - If T's constructor throws, instance() will return a null reference.
 *  - If your singleton class manages resources, you may provide a public
 *    destructor, and it will be called when the instance of your singleton
 *    class is out of scoped (see scoped_ptr docs).
 *  .
 *
 * \author Port4l, http://www.boostcookbook.com/User:/Port4l
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename T>
class singleton: ::boost::noncopyable
{
	public: static T& instance()
	{
//		::boost::call_once(init, flag_);
		::boost::call_once(flag_, init);
		return *ptr_t_;
	}


	public: static T const& const_instance()
	{
//		::boost::call_once(init, flag_);
		::boost::call_once(flag_, init);
		return *ptr_t_;
	}


	protected: singleton() {}


	protected: virtual ~singleton() { }


	private: static void init() // never throws
	{
		DCS_DEBUG_TRACE("Singleton initialization");//XXX

		ptr_t_.reset(new T());
	}


	private: static ::boost::once_flag flag_;
	private: static ::boost::scoped_ptr<T> ptr_t_;
};

template <typename T>
::boost::once_flag singleton<T>::flag_ = BOOST_ONCE_INIT;

template <typename T>
::boost::scoped_ptr<T> singleton<T>::ptr_t_(0);

}} // Namespace detail::<unnamed>

#endif // DCS_EESIM_REGISTRY_USE_ABRAHAMS_SINGLETON


#if defined(DCS_EESIM_REGISTRY_USE_ABRAHAMS_SINGLETON)

template <typename TraitsT>
class registry: public ::boost::serialization::singleton< registry<TraitsT> >
{
	private: typedef ::boost::serialization::singleton< registry<TraitsT> > base_type;
#else // DCS_EESIM_REGISTRY_USE_ABRAHAMS_SINGLETON

template <typename TraitsT>
class registry: public detail::singleton< registry<TraitsT> >
{
	friend class detail::singleton< registry<TraitsT> >;
#endif // DCS_EESIM_REGISTRY_USE_ABRAHAMS_SINGLETON
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::des_engine_type des_engine_type;
	public: typedef ::dcs::shared_ptr<des_engine_type> des_engine_pointer;
	public: typedef typename traits_type::uniform_random_generator_type uniform_random_generator_type;
	public: typedef ::dcs::shared_ptr<uniform_random_generator_type> uniform_random_generator_pointer;
	public: typedef typename traits_type::configuration_type configuration_type;
	public: typedef ::dcs::shared_ptr<configuration_type> configuration_pointer;
	public: typedef typename traits_type::application_identifier_type application_identifier_type;
	public: typedef typename traits_type::physical_machine_identifier_type physical_machine_identifier_type;
	public: typedef typename traits_type::virtual_machine_identifier_type virtual_machine_identifier_type;
	public: typedef identifier_generator<application_identifier_type> application_id_generator_type;
	public: typedef identifier_generator<physical_machine_identifier_type> physical_machine_id_generator_type;
	public: typedef identifier_generator<virtual_machine_identifier_type> virtual_machine_id_generator_type;


	public: static registry& instance()
	{
		return base_type::get_mutable_instance();
	}


	public: static registry const& const_instance()
	{
		return base_type::get_const_instance();
	}


	public: void des_engine(des_engine_pointer const& ptr_engine)
	{
		ptr_des_eng_ = ptr_engine;
	}


	public: des_engine_pointer des_engine_ptr() const
	{
		return ptr_des_eng_;
	}


	public: des_engine_pointer des_engine_ptr()
	{
		return ptr_des_eng_;
	}


	public: des_engine_type const& des_engine() const
	{
		// pre: DES engine must be set
		DCS_DEBUG_ASSERT( ptr_des_eng_ );

		return *ptr_des_eng_;
	}


	public: des_engine_type& des_engine()
	{
		// pre: DES engine must be set
		DCS_DEBUG_ASSERT( ptr_des_eng_ );

		return *ptr_des_eng_;
	}


	public: void uniform_random_generator(uniform_random_generator_pointer const& ptr_generator)
	{
		ptr_rng_ = ptr_generator;
	}


	public: uniform_random_generator_pointer uniform_random_generator_ptr() const
	{
		return ptr_rng_;
	}


	public: uniform_random_generator_pointer uniform_random_generator_ptr()
	{
		return ptr_rng_;
	}


	public: uniform_random_generator_type const& uniform_random_generator() const
	{
		// pre: random number generator must be set
		DCS_DEBUG_ASSERT( ptr_rng_ );

		return *ptr_rng_;
	}


	public: uniform_random_generator_type& uniform_random_generator()
	{
		// pre: random number generator must be set
		DCS_DEBUG_ASSERT( ptr_rng_ );

		return *ptr_rng_;
	}


	public: void configuration(configuration_pointer const& ptr_conf)
	{
		ptr_conf_ = ptr_conf;
	}


	public: configuration_pointer configuration_ptr() const
	{
		return ptr_conf_;
	}


	public: configuration_pointer configuration_ptr()
	{
		return ptr_conf_;
	}


	public: configuration_type const& configuration() const
	{
		// pre: valid pointer
		DCS_DEBUG_ASSERT( ptr_conf_ );

		return *ptr_conf_;
	}


	public: configuration_type& configuration()
	{
		// pre: valid pointer
		DCS_DEBUG_ASSERT( ptr_conf_ );

		return *ptr_conf_;
	}


	public: application_id_generator_type& application_id_generator()
	{
		return app_id_gen_;
	}


	public: application_id_generator_type const& application_id_generator() const
	{
		return app_id_gen_;
	}


	public: physical_machine_id_generator_type& physical_machine_id_generator()
	{
		return pm_id_gen_;
	}


	public: physical_machine_id_generator_type const& physical_machine_id_generator() const
	{
		return pm_id_gen_;
	}


	public: virtual_machine_id_generator_type& virtual_machine_id_generator()
	{
		return vm_id_gen_;
	}


	public: virtual_machine_id_generator_type const& virtual_machine_id_generator() const
	{
		return vm_id_gen_;
	}


	/// Default constructor
	protected: registry()
	: app_id_gen_(0),
	  pm_id_gen_(0),
	  vm_id_gen_(0)
	{
	}


	private: des_engine_pointer ptr_des_eng_;
	private: uniform_random_generator_pointer ptr_rng_;
	private: configuration_pointer ptr_conf_;
	private: application_id_generator_type app_id_gen_;
	private: physical_machine_id_generator_type pm_id_gen_;
	private: virtual_machine_id_generator_type vm_id_gen_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_REGISTRY_HPP
