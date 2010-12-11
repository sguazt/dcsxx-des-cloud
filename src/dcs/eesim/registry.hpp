/**
 * \file dcs/eesim/registry.hpp
 *
 * \brief Global registry class.
 *
 * Copyright (C) 2009-2010  Distributed Computing System (DCS) Group, Computer
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
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */

#ifndef DCS_EESIM_REGISTRY_HPP
#define DCS_EESIM_REGISTRY_HPP


#define DCS_EESIM_REGISTRY_USE_ABRAHAMS_SINGLETON
//#undef DCS_EESIM_REGISTRY_USE_ABRAHAMS_SINGLETON


#ifdef DCS_EESIM_REGISTRY_USE_ABRAHAMS_SINGLETON


#include <dcs/config/boost.hpp>

#if !DCS_CONFIG_BOOST_CHECK_VERSION(103200) // 1.32
#	error "Required Boost libraries >= 1.32."
#endif

#include <boost/serialization/singleton.hpp>


namespace dcs { namespace eesim {

template <typename TraitsT>
class registry: public ::boost::serialization::singleton< registry<TraitsT> >
{
	private: typedef ::boost::serialization::singleton< registry<TraitsT> > base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::des_engine_type des_engine_type;
	public: typedef ::dcs::shared_ptr<des_engine_type> des_engine_pointer;
	public: typedef typename traits_type::uniform_random_generator_type uniform_random_generator_type;
	public: typedef ::dcs::shared_ptr<uniform_random_generator_type> uniform_random_generator_pointer;


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


	protected: registry() { }


	private: des_engine_pointer ptr_des_eng_;
	private: uniform_random_generator_pointer ptr_rng_;
};

}} // Namespace dcs::eesim


#else // DCS_EESIM_REGISTRY_USE_ABRAHAMS_SINGLETON


#include <dcs/config/boost.hpp>

#if !DCS_CONFIG_BOOST_CHECK_VERSION(102500) // 1.25
#	error "Required Boost libraries >= 1.25."
#endif

#include <boost/utility.hpp>
#include <boost/thread/once.hpp>
#include <boost/scoped_ptr.hpp>
#include <dcs/memory.hpp>
#include <dcs/type_traits/add_const.hpp>
#include <dcs/type_traits/add_reference.hpp>


namespace dcs { namespace eesim {

namespace detail {

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


} // Namespace detail

template <typename TraitsT>
class registry: public detail::singleton< registry<TraitsT> >
{
	friend class detail::singleton< registry<TraitsT> >;


	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::des_engine_type des_engine_type;
//	public: typedef typename ::dcs::type_traits::add_reference<des_engine_type>::type des_engine_reference;
//	public: typedef typename ::dcs::type_traits::add_reference<
//							typename ::dcs::type_traits::add_const<
//								des_engine_type
//							>::type
//						>::type des_engine_const_reference;
	public: typedef ::dcs::shared_ptr<des_engine_type> des_engine_pointer;
	public: typedef typename traits_type::uniform_random_generator_type uniform_random_generator_type;
//	public: typedef typename ::dcs::type_traits::add_reference<uniform_random_generator_type>::type uniform_random_generator_reference;
//	public: typedef typename ::dcs::type_traits::add_reference<
//							typename ::dcs::type_traits::add_const<
//								uniform_random_generator_type
//							>::type
//						>::type uniform_random_generator_const_reference;
	public: typedef ::dcs::shared_ptr<uniform_random_generator_type> uniform_random_generator_pointer;


//	public: void des_engine(des_engine_const_reference engine)
//	{
//		des_eng_ = engine;
//	}


////	public: void des_engine(des_engine_reference engine)
////	{
////		des_eng_ = engine;
////	}


//	public: des_engine_reference des_engine()
//	{
//		return des_eng_;
//	}


//	public: des_engine_const_reference des_engine() const
//	{
//		return des_eng_;
//	}


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


	public: void uniform_random_generator(uniform_random_generator_pointer const& ptr_generator)
	{
		ptr_rng_ = ptr_generator;
	}


//	public: void uniform_random_generator(uniform_random_generator_type const& generator)
//	{
//		rng_ = generator;
//	}


////	public: void uniform_random_generator(uniform_random_generator_reference generator)
////	{
////		rng_ = generator;
////	}


//	public: uniform_random_generator_reference uniform_random_generator()
//	{
//		return rng_;
//	}


//	public: uniform_random_generator_const_reference uniform_random_generator() const
//	{
//		return rng_;
//	}


	public: uniform_random_generator_pointer uniform_random_generator_ptr() const
	{
		return ptr_rng_;
	}


	public: uniform_random_generator_pointer uniform_random_generator_ptr()
	{
		return ptr_rng_;
	}


	protected: registry() { }


	//private: des_engine_type des_eng_;
	private: des_engine_pointer ptr_des_eng_;
	//private: uniform_random_generator_type rng_;
	private: uniform_random_generator_pointer ptr_rng_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_REGISTRY_USE_ABRAHAMS_SINGLETON


#endif // DCS_EESIM_REGISTRY_HPP
