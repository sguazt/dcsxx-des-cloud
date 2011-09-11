/**
 * \file src/dcs/eesim/minlp_migration_controller.hpp
 *
 * \brief Migration Controller based on Mixed-Integer Nonlinear Programming.
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
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */

#ifndef DCS_EESIM_MINLP_MIGRATION_CONTROLLER_HPP
#define DCS_EESIM_MINLP_MIGRATION_CONTROLLER_HPP


#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/base_migration_controller.hpp>
#include <dcs/eesim/detail/ampl/utility.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/power_status.hpp>
#include <dcs/memory.hpp>
#include <dcs/perfeval/energy/fan2007_model.hpp>
#include <iosfwd>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>


namespace dcs { namespace eesim {

namespace detail { namespace /*<unnamed>*/ {

//template <typename MachinePointerT>
//struct active_machine_predicate: ::std::unary_function<MachinePointerT,bool>
//{
//	bool operator()(MachinePointerT const& ptr_mach) const
//	{
//		return ptr_mach->power_state() == powered_on_power_status
//			   &&
//			   ptr_mach->virtual_machine_monitor().virtual_machines(powered_on_power_status).size() > 0;
//	}
//};


enum ampl_solver_results
{
	ampl_solved_result, // optimal solution found
	ampl_maybe_solved_result, // optimal solution indicated, but error likely
	ampl_infeasible_result, // constraints cannot be satisfied
	ampl_unbounded_result, // objective can be improved without limit
	ampl_limit_result, // stopped by a limit that you set (such as on iterations)
	ampl_failure_result, // stopped by an error condition in the solver
	ampl_unknown_result // fall-back
};


inline
ampl_solver_results from_string(::std::string const& str)
{
	if (!str.compare("solved"))
	{
		return ampl_solved_result;
	}
	if (!str.compare("solved?"))
	{
		return ampl_maybe_solved_result;
	}
	if (!str.compare("infeasible"))
	{
		return ampl_infeasible_result;
	}
	if (!str.compare("unbounded"))
	{
		return ampl_unbounded_result;
	}
	if (!str.compare("limit"))
	{
		return ampl_limit_result;
	}
	if (!str.compare("failure"))
	{
		return ampl_failure_result;
	}

	return ampl_unknown_result;
}


class ampl_minlp_input_producer
{
	public: ampl_minlp_input_producer(::std::string const& script)
	: script_(script)
	{
	}


	public: template <typename CharT, typename CharTraitsT>
		void operator()(::std::basic_ostream<CharT,CharTraitsT>& os)
	{
		os << script_;
	}


	private: ::std::string script_;
}; // ampl_minlp_input_producer


class ampl_minlp_output_consumer
{
	public: typedef int int_type;
	public: typedef char smallint_type;
	public: typedef double real_type;
	public: typedef ::boost::numeric::ublas::vector<smallint_type> smallint_vector_type;
	public: typedef ::boost::numeric::ublas::matrix<smallint_type> smallint_matrix_type;
	public: typedef ::boost::numeric::ublas::matrix<real_type> real_matrix_type;


	public: ampl_minlp_output_consumer()
	: solver_exit_code_(0),
	  solver_result_(ampl_unknown_result),
	  solver_result_code_(0),
	  cost_(::std::numeric_limits<real_type>::infinity()),
	  x_(),
	  y_(),
	  s_()
	{
	}


	public: template <typename CharT, typename CharTraitsT>
		void operator()(::std::basic_istream<CharT,CharTraitsT>& is)
	{
		enum parser_states
		{
			skip_state,
			out_analysis_state,
			results_state,
			end_state
		};


		parser_states state(skip_state);
		bool ok(true);
		::std::size_t num_solver_info(0);
		while (is.good() && state != end_state)
		{
			::std::string line;
			::std::getline(is, line);

			::std::size_t pos(0);
::std::cerr << "Read from AMPL>> " << line << ::std::endl;//XXX

			switch (state)
			{
				case skip_state:
					if (line.find("-- [RESULT] --") != ::std::string::npos)
					{
						state = out_analysis_state;
					}
					break;
				case out_analysis_state:
					if ((pos = line.find("solve_exitcode")) != ::std::string::npos)
					{
						while ((pos = line.find('=', pos)) != ::std::string::npos)
						{
							;
						}

						if (pos != ::std::string::npos)
						{
							::dcs::eesim::detail::ampl::parse_str(line.substr(pos+1), solver_exit_code_);

							if (solver_exit_code_ == 0)
							{
//								state = out_analysis_state;
							}
							else
							{
								// Problem in calling the solver
								state = end_state;
								ok = false;
							}
						}
						else
						{
							state = end_state;
							ok = false;
						}
					}
					else if ((pos = line.find("solve_result")) != ::std::string::npos)
					{
						while ((pos = line.find('=', pos)) != ::std::string::npos)
						{
							;
						}

						if (pos != ::std::string::npos)
						{
							::std::string res;
							::dcs::eesim::detail::ampl::parse_str(line.substr(pos+1), res);

							solver_result_ = from_string(res);

//							state = out_analysis_state;
						}
						else
						{
							state = end_state;
							ok = false;
						}
					}
					else if ((pos = line.find("solve_result_num")) != ::std::string::npos)
					{
						while ((pos = line.find('=', pos)) != ::std::string::npos)
						{
							;
						}

						if (pos != ::std::string::npos)
						{
							::dcs::eesim::detail::ampl::parse_str(line.substr(pos+1), solver_result_code_);

							if (solver_result_ == ampl_solved_result && solver_result_code_ >= 0 && solver_result_code_ < 100)
							{
//								state = results_state;
							}
							else
							{
								state = end_state;
								ok = false;
							}
						}
						else
						{
							state = end_state;
							ok = false;
						}
					}

					if (ok)
					{
						state = (num_solver_info < 3) ? out_analysis_state : results_state;
						++num_solver_info;
					}
					break;
				case results_state:
					if (line.find("cost") != ::std::string::npos)
					{
						while ((pos = line.find('=', pos)) != ::std::string::npos)
						{
							;
						}

						if (pos != ::std::string::npos)
						{
							::dcs::eesim::detail::ampl::parse_str(line.substr(pos+1), cost_);
						}
						else
						{
							state = end_state;
							ok = false;
						}
					}
					else if (line.find("x") != ::std::string::npos)
					{
						while ((pos = line.find('=', pos)) != ::std::string::npos)
						{
							;
						}

						if (pos != ::std::string::npos)
						{
							::dcs::eesim::detail::ampl::parse_str(line.substr(pos+1), x_);
						}
						else
						{
							state = end_state;
							ok = false;
						}
					}
					else if (line.find("y") != ::std::string::npos)
					{
						while ((pos = line.find('=', pos)) != ::std::string::npos)
						{
							;
						}

						if (pos != ::std::string::npos)
						{
							::dcs::eesim::detail::ampl::parse_str(line.substr(pos+1), y_);
						}
						else
						{
							state = end_state;
							ok = false;
						}
					}
					else if (line.find("s") != ::std::string::npos)
					{
						while ((pos = line.find('=', pos)) != ::std::string::npos)
						{
							;
						}

						if (pos != ::std::string::npos)
						{
							::dcs::eesim::detail::ampl::parse_str(line.substr(pos+1), s_);
						}
						else
						{
							state = end_state;
							ok = false;
						}
					}
					else if (line.find("-- [/RESULT] --") != ::std::string::npos)
					{
						state = end_state;
					}
					break;
				case end_state:
					break;
			}
		}
	}


	public: int_type solver_exit_code() const
	{
		return solver_exit_code_;
	}


	public: ampl_solver_results solver_result() const
	{
		return solver_result_;
	}


	public: int_type solver_result_code() const
	{
		return solver_result_code_;
	}


	public: real_type cost() const
	{
		return cost_;
	}


	public: smallint_vector_type physical_machine_selection() const
	{
		return x_;
	}


	public: smallint_matrix_type virtual_machine_placement() const
	{
		return y_;
	}


	public: real_matrix_type virtual_machine_shares() const
	{
		return s_;
	}


	private: int_type solver_exit_code_;
	private: ampl_solver_results solver_result_;
	private: int_type solver_result_code_;
	private: real_type cost_;
	private: smallint_vector_type x_;
	private: smallint_matrix_type y_;
	private: real_matrix_type s_;
}; // ampl_minlp_input_consumer


template <typename TraitsT>
class ampl_minlp_solver_impl
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef data_center<traits_type> data_center_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	private: typedef physical_machine<traits_type> physical_machine_type;
	private: typedef ::dcs::shared_ptr<physical_machine_type> physical_machine_pointer;
	private: typedef virtual_machine<traits_type> virtual_machine_type;
	private: typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
	private: typedef typename virtual_machine_type::application_tier_type application_tier_type;


	public: ampl_minlp_solver_impl()
	{
	}


	public: void solve(data_center_type const& dc, real_type wp, real_type wm, real_type ws)
	{
		::std::string problem;

		problem = make_problem(dc, wp, wm, ws);

		ampl_minlp_input_producer producer(problem);
		ampl_minlp_output_consumer consumer;

::std::cerr << "Create problem: " << problem << ::std::endl;//XXX
		::dcs::eesim::detail::ampl::run_ampl_command(
				::dcs::eesim::detail::ampl::find_ampl_command(),
				::std::vector< ::std::string >(),
				producer,
				consumer
			);

::std::cerr << "AMPL terminated" << ::std::endl;//XXX
		if (consumer.solver_result() == detail::ampl_solved_result)
		{
::std::cerr << "AMPL has solved" << ::std::endl;//XXX
::std::cerr << "Optimum cost: " << consumer.cost() << ::std::endl;//XXX
::std::cerr << "Optimum x: " << consumer.physical_machine_selection() << ::std::endl;//XXX
::std::cerr << "Optimum y: " << consumer.virtual_machine_placement() << ::std::endl;//XXX
::std::cerr << "Optimum s: " << consumer.virtual_machine_shares() << ::std::endl;//XXX
		}
	}


	private: ::std::string make_problem(data_center_type const& dc, real_type wp, real_type wm, real_type ws)
	{
		typedef ::std::vector<physical_machine_pointer> machine_container;
		typedef typename machine_container::const_iterator machine_iterator;
		typedef ::std::vector<virtual_machine_pointer> vm_container;
		typedef typename vm_container::const_iterator vm_iterator;
		typedef typename physical_machine_type::resource_type resource_type;
		typedef typename physical_machine_type::resource_pointer resource_pointer;
		typedef typename resource_type::energy_model_type energy_model_type;
		typedef typename ::dcs::perfeval::energy::fan2007_model<typename energy_model_type::real_type> fan2007_energy_model_impl_type;
		typedef typename data_center_type::application_type application_type;
		typedef typename application_type::reference_physical_resource_type reference_resource_type;


		::std::ostringstream oss;

		oss << make_problem_model()
			<< "data;"
			<< ::std::endl;

		// Create the set of all physical machines
		machine_container pms(dc.physical_machines());

		::std::size_t n_pms(pms.size());

		// # of machines
		oss << "param ni := " << n_pms << ";" << ::std::endl;

		// Create the set of active virtual machines
		// An active virtual machine is a virtual machine:
		// * that is powered on
		vm_container active_vms;
		for (::std::size_t i = 0; i < n_pms; ++i)
		{
			physical_machine_pointer ptr_pm(pms[i]);

			if (ptr_pm->power_state() != powered_on_power_status)
			{
				continue;
			}

			vm_container on_vms = ptr_pm->vmm().virtual_machines(powered_on_power_status);
			active_vms.insert(active_vms.end(), on_vms.begin(), on_vms.end());
		}

		::std::size_t n_vms(active_vms.size());

		// # of VMs
		oss << "param nj := " << n_vms << ";" << ::std::endl;

		// Power model coefficients, max share and resource capacity
		oss << "param: c0 c1 c2 r Smax C :=" << ::std::endl;
		for (::std::size_t i = 0; i < n_pms; ++i)
		{
			physical_machine_pointer ptr_pm(pms[i]);

			//FIXME: CPU resource category is hard-coded
			resource_pointer ptr_resource(ptr_pm->resource(::dcs::eesim::cpu_resource_category));
			energy_model_type const& energy_model(ptr_resource->energy_model());
			//FIXME: Fan2007 energy model type is hard-coded
			fan2007_energy_model_impl_type const* ptr_energy_model_impl = dynamic_cast<fan2007_energy_model_impl_type const*>(&energy_model);
			if (!ptr_energy_model_impl)
			{
				throw ::std::runtime_error("[dcs::eesim::detail::minlp_migration_controller_impl::make_problem] Unable to retrieve energy model.");
			}

			oss << (i+1)
				<< " " << ptr_energy_model_impl->coefficient(0) // c0
				<< " " << ptr_energy_model_impl->coefficient(1) // c1
				<< " " << ptr_energy_model_impl->coefficient(2) // c2
				<< " " << ptr_energy_model_impl->coefficient(3) // r
				<< " " << 1 // Smax
				<< " " << (ptr_resource->capacity()*ptr_resource->utilization_threshold()) // C
				<< ::std::endl;
		}
		oss << ";" << ::std::endl;

		// Migration costs
		oss << "param mc :=" << ::std::endl;
		for (::std::size_t i = 0; i < n_pms; ++i)
		{
			physical_machine_pointer ptr_pm(pms[i]);

			oss << "[" << (i+1) << ",*]";
			for (::std::size_t j = 0; j < n_vms; ++j)
			{
				virtual_machine_pointer ptr_vm(active_vms[j]);

				real_type cost(0);

				if (ptr_pm->id() != ptr_vm->vmm().hosting_machine().id())
				{
					cost = 1;
				}

				oss << " " << (j+1) << " " << cost;
			}
			oss << ::std::endl;
		}
		oss << ";" << ::std::endl;

		// Reference machine capacity, utilization and min share of tiers
		oss << "param: Cr ur Srmin := " << ::std::endl;
		for (::std::size_t j = 0; j < n_vms; ++j)
		{
			virtual_machine_pointer ptr_vm(active_vms[j]);

			application_type const& app(ptr_vm->guest_system().application());

			//FIXME: CPU resource category is hard-coded
			reference_resource_type const& ref_resource(app.reference_resource(::dcs::eesim::cpu_resource_category));

			oss << " " << (j+1)
				<< " " << (ref_resource.capacity()*ref_resource.utilization_threshold())
				<< " " << app.performance_model().tier_measure(ptr_vm->guest_system().id(), ::dcs::eesim::utilization_performance_measure)
				<< " " << 0.2 //FIXME: Minimum share is hard-coded
				<< ::std::endl;
		}
		oss << ";" << ::std::endl;

		oss << "param wp := " << wp << ";" << ::std::endl;
		oss << "param wm := " << wm << ";" << ::std::endl;
		oss << "param ws := " << ws << ";" << ::std::endl;

		//FIXME: solver is hard-coded
		oss << "option solver couenne;"
			<< "option solution_precision 0;"
			<< "option solver_msg 0;"
			<< "option display_1col 0;"
			<< "option display_transpose 0;"
			<< "option gutter_width 1;"
			<< "option omit_zero_cols 0;"
			<< "option omit_zero_rows 0;"
			<< "option display_precision 0;"
			<< "option print_precision 0;"
			<< "solve;"
			<< "print '-- [RESULT] --';"
			<< "print 'solve_exitcode = ', solve_exitcode, ';';"
			<< "print 'solve_result = ', solve_result, ';';"
			<< "print 'solver_result_num = ', solve_result_num, ';';"
			<< "print 'cost = ', cost, ';';"
//			<< "print 'x = (', card({I}), ')[', ({i in I} (i, x[i])), '];';"
//			<< "print 'x = [', ({i in I} (i, x[i])), '];';"
			<< "print 'x = [', {i in I} x[i], '];';"
//			<< "print 'y = (', card({I}), ',', card({J}), ')[', ({i in I} (i, ({j in J} (j, y[i,j])), ';')), '];';"
//			<< "print 'y = [', ({i in I} (i, ({j in J} (j, y[i,j])), ';')), '];';"
			<< "print 'y = [', {i in I} (({j in J} y[i,j]), ';'), '];';"
//			<< "print 's = (', card({I}), ',', card({J}), ')[', ({i in I} (i, ({j in J} (j, s[i,j])), ';')), '];';"
//			<< "print 's = [', ({i in I} (i, ({j in J} (j, s[i,j])), ';')), '];';"
			<< "print 's = [', {i in I} (({j in J} s[i,j]), ';'), '];';"
			<< "print '-- [/RESULT] --';" << ::std::endl
			<< "end;"
			<< ::std::endl;

			return oss.str();
	}


	private: static ::std::string make_problem_model()
	{
		::std::ostringstream oss;

		oss << "reset;"
		    << "model;"
		    << "param ni > 0 integer;"
		    << "param nj > 0 integer;"
		    << "set I := 1..ni;"
		    << "set J := 1..nj;"
		    << "param c0{I};"
		    << "param c1{I};"
		    << "param c2{I};"
		    << "param r{I};"
		    << "param mc{I,J} >= 0;"
		    << "param ur{J} >= 0, <= 1;"
		    << "param Cr{J} >= 0;"
		    << "param Smax{I} >= 0, <= 1;"
		    << "param Srmin{J} >= 0, <= 1;"
		    << "param C{I} >= 0;"
		    << "param wp >= 0 default 0;"
		    << "param wm >= 0 default 0;"
		    << "param ws >= 0 default 0;"
		    << "param eps := 1.0e-5;"
		    << "param wwp := wp / (ni * max{i in I} (c0[i]+c1[i]+c2[i]));"
		    << "param wwm := if wm > 0 then wm / (nj * max{i in I, j in J} mc[i,j]) else 0;"
		    << "param wws := ws / nj;"
		    << "var x{I} binary;"
		    << "var y{I,J} binary;"
		    << "var s{I,J} >= 0, <= 1;"
		    << "minimize cost: wwp * sum{i in I} x[i]*(c0[i] + c1[i]*(sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i]*(s[i,j]+eps))) + c2[i]*(sum{j in J} (y[i,j]+eps)*ur[j]*Cr[j]/(C[i]*(s[i,j]+eps)))^r[i]) + wwm * sum{i in I, j in J} mc[i,j]*y[i,j] + wws * sum{i in I, j in J} ((s[i,j]*C[i]/Cr[j]-1)^2)*y[i,j];"
		    << "subject to one_vm_per_mach{j in J}: sum{i in I} y[i,j] = 1;"
		    << "subject to vm_on_active_mach1{i in I, j in J}: y[i,j] <= x[i];"
		    << "subject to vm_on_active_mach2{i in I}: sum{j in J} y[i,j] >= x[i];"
		    << "subject to valid_vm_share{i in I, j in J}: s[i,j] <= y[i,j];"
		    << "subject to min_vm_share{i in I, j in J}: s[i,j] >= y[i,j]*Srmin[j]*Cr[j]/C[i];"
		    << "subject to max_aggr_vm_share{i in I}: sum{j in J} s[i,j] <= Smax[i];"
		    << "subject to valid_util{i in I}: sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i]*(s[i,j]+eps)) <= 1;"
			<< ::std::endl;

		return oss.str();
	}


	/// Weight for power consumption.
	private: real_type wp_;
	/// Weight for VM migration.
	private: real_type wm_;
	/// Weight for SLA satisfaction.
	private: real_type ws_;
}; // ampl_minlp_solver_impl

}} // Namespace detail::<unnamed>


template <typename TraitsT>
class minlp_migration_controller: public base_migration_controller<TraitsT>
{
	private: typedef base_migration_controller<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename base_type::data_center_pointer data_center_pointer;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	private: typedef physical_machine<traits_type> physical_machine_type;
	private: typedef ::dcs::shared_ptr<physical_machine_type> physical_machine_pointer;
	private: typedef virtual_machine<traits_type> virtual_machine_type;
	private: typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
	private: typedef typename virtual_machine_type::application_tier_type application_tier_type;
	private: typedef detail::ampl_minlp_solver_impl<traits_type> minlp_solver_type;


	public: minlp_migration_controller()
	: base_type(),
	  wp_(1),
	  wm_(1),
	  ws_(1)
	{
//		init();
	}


	public: minlp_migration_controller(data_center_pointer const& ptr_dc, real_type ts)
	: base_type(ptr_dc, ts),
	  wp_(1),
	  wm_(1),
	  ws_(1)
	{
//		init();
	}


//	public: ~minlp_migration_controller()
//	{
//	}


	//@{ Interface Member Functions

//	protected: void do_controlled_data_center(data_center_pointer const& ptr_data_center)
//	{
//	}


//	protected: void do_process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
//	{
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
//
//		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Process SYSTEM-INITIALIZATION event (Clock: " << ctx.simulated_time() << ")");
//
//		DCS_DEBUG_TRACE("(" << this << ") END Do Process SYSTEM-INITIALIZATION event (Clock: " << ctx.simulated_time() << ")");
//	}


	private: void do_process_control(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Process CONTROL event (Clock: " << ctx.simulated_time() << ")");

		minlp_solver_type solver;

		solver.solve(this->controlled_data_center(), wp_, wm_, ws_);

		DCS_DEBUG_TRACE("(" << this << ") END Do Process CONTROL event (Clock: " << ctx.simulated_time() << ")");
	}

	//@} Interface Member Functions



	private: real_type wp_;
	private: real_type wm_;
	private: real_type ws_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_MINLP_MIGRATION_CONTROLLER_HPP
