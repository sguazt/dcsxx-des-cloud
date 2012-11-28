/**
 * \file dcs/des/cloud/application_controller_triggers.hpp
 *
 * \brief Class for triggering control action in an application controller.
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

#ifndef DCS_DES_CLOUD_APPLICATION_CONTROLLER_TRIGGERS_HPP
#define DCS_DES_CLOUD_APPLICATION_CONTROLLER_TRIGGERS_HPP


namespace dcs { namespace des { namespace cloud {

template <typename TraitsT>
class application_controller_triggers
{
	public: typedef TraitsT traits_type;


	/// Default constructor
	public: application_controller_triggers()
	: actual_sla_ko_(false),
	  predicted_sla_ko_(false)
	{
	}


	// Compiler-generated copy-constructor/assignment and destructor are fine.


	public: void actual_value_sla_ko(bool value)
	{
		actual_sla_ko_ = value;
	}


	public: bool actual_value_sla_ko() const
	{
		return actual_sla_ko_;
	}


	public: void predicted_value_sla_ko(bool value)
	{
		predicted_sla_ko_ = value;
	}


	public: bool predicted_value_sla_ko() const
	{
		return predicted_sla_ko_;
	}


	private: bool actual_sla_ko_;
	private: bool predicted_sla_ko_;
}; // base_application_controller_triggers

}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_APPLICATION_CONTROLLER_TRIGGERS_HPP
