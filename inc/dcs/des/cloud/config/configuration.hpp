/**
 * \file dcs/des/cloud/config/configuration.hpp
 *
 * \brief Main configuration class.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 *
 * <hr/>
 *
 * Copyright (C) 2009-2012  Marco Guazzone (marco.guazzone@gmail.com)
 *                          [Distributed Computing System (DCS) Group,
 *                           Computer Science Institute,
 *                           Department of Science and Technological Innovation,
 *                           University of Piemonte Orientale,
 *                           Alessandria (Italy)]
 *
 * This file is part of dcsxx-des-cloud (also referred to as "this program").
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
 */

#ifndef DCS_DES_CLOUD_CONFIG_CONFIGURATION_HPP
#define DCS_DES_CLOUD_CONFIG_CONFIGURATION_HPP


#include <dcs/des/cloud/config/data_center.hpp>
#include <dcs/des/cloud/config/logging.hpp>
#include <dcs/des/cloud/config/rng.hpp>
#include <dcs/des/cloud/config/simulation.hpp>
#include <iosfwd>


namespace dcs { namespace des { namespace cloud { namespace config {


template <typename RealT, typename UIntT>
class configuration
{
	public: typedef RealT real_type;
	public: typedef UIntT uint_type;
	public: typedef logging_config logging_config_type;
	public: typedef rng_config<uint_type> rng_config_type;
	public: typedef simulation_config<real_type,uint_type> simulation_config_type;
	public: typedef data_center_config<real_type,uint_type> data_center_config_type;


	public: void data_center(data_center_config_type const& dc)
	{
		dc_ = dc;
	}


	public: data_center_config_type const& data_center() const
	{
		return dc_;
	}


	public: void logging(logging_config_type const& lg)
	{
		log_ = lg;
	}


	public: logging_config_type const& logging() const
	{
		return log_;
	}


	public: void rng(rng_config_type const& rng)
	{
		rng_ = rng;
	}


	public: rng_config_type const& rng() const
	{
		return rng_;
	}


	public: void simulation(simulation_config_type const& sim)
	{
		sim_ = sim;
	}


	public: simulation_config_type const& simulation() const
	{
		return sim_;
	}


	private: logging_config_type log_;
	private: rng_config_type rng_;
	private: simulation_config_type sim_;
	private: data_center_config_type dc_;
};


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, configuration<RealT,UIntT> const& conf)
{
	os << "<(configuration)"
	   << " " << conf.logging()
	   << ", " << conf.rng()
	   << ", " << conf.simulation()
	   << ", " << conf.data_center()
	   << ">";

	return os;
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_CONFIGURATION_HPP
