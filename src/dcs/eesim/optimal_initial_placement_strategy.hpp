/**
 * \file src/dcs/eesim/optimal_initial_placement_strategy.hpp
 *
 * \brief Optimal initial placement strategy based on mathematical programming. 
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

#ifndef DCS_EESIM_OPTIMAL_INITIAL_PLACEMENT_STRATEGY_HPP
#define DCS_EESIM_OPTIMAL_INITIAL_PLACEMENT_STRATEGY_HPP


#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublasx/operation/num_columns.hpp>
#include <boost/numeric/ublasx/operation/num_rows.hpp>
#include <dcs/debug.hpp>
#include <dcs/eesim/base_initial_placement_strategy.hpp>
#include <dcs/eesim/data_center.hpp>
#include <dcs/eesim/detail/ampl/solver_results.hpp>
#include <dcs/eesim/detail/ampl/utility.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/virtual_machine.hpp>
#include <dcs/eesim/virtual_machines_placement.hpp>
#include <dcs/memory.hpp>
#include <dcs/perfeval/energy/fan2007_model.hpp>
#include <iosfwd>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <vector>


//FIXME:
// - Fit is based only on the CPU resource (see detail::pm_comparator)
//


namespace dcs { namespace eesim {

namespace detail { namespace /*<unnamed>*/ { namespace initial_placement {

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
	public: typedef short smallint_type; // don't use char since it may fails on some tests
	public: typedef double real_type;
	public: typedef ::boost::numeric::ublas::vector<smallint_type> smallint_vector_type;
	public: typedef ::boost::numeric::ublas::matrix<smallint_type> smallint_matrix_type;
	public: typedef ::boost::numeric::ublas::matrix<real_type> real_matrix_type;


	public: ampl_minlp_output_consumer()
	: solver_exit_code_(0),
	  solver_result_(::dcs::eesim::detail::ampl::unknown_result),
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

//::std::cerr << "Read-AMPL>> " << line << " (old state: " << state << ")" <<::std::endl;//XXX
			switch (state)
			{
				case skip_state:
					if (line.find("-- [RESULT] --") != ::std::string::npos)
					{
						state = out_analysis_state;
					}
					break;
				case out_analysis_state:
					if ((pos = line.find("solve_exitcode=")) != ::std::string::npos)
					{
						::dcs::eesim::detail::ampl::parse_str(line.substr(pos+15), solver_exit_code_);

						if (solver_exit_code_ != 0)
						{
							// Problem in calling the solver
							state = end_state;
							ok = false;
						}
					}
					else if ((pos = line.find("solve_result=")) != ::std::string::npos)
					{
						::std::string res;
						::dcs::eesim::detail::ampl::parse_str(line.substr(pos+13), res);

						solver_result_ = ::dcs::eesim::detail::ampl::solver_result_from_string(res);
					}
					else if ((pos = line.find("solve_result_num=")) != ::std::string::npos)
					{
						::dcs::eesim::detail::ampl::parse_str(line.substr(pos+17), solver_result_code_);
					}

					if (ok)
					{

						if (num_solver_info < 3)
						{
							state = out_analysis_state;
							++num_solver_info;
						}
						else if (solver_result_ == ::dcs::eesim::detail::ampl::solved_result && solver_result_code_ >= 0 && solver_result_code_ < 100)
						{
							state = results_state;
						}
						else
						{
							state = end_state;
							ok = false;
						}
					}
					break;
				case results_state:
					if (line.find("cost=") != ::std::string::npos)
					{
						::dcs::eesim::detail::ampl::parse_str(line.substr(pos+5), cost_);
					}
					else if (line.find("x=") != ::std::string::npos)
					{
						::dcs::eesim::detail::ampl::parse_str(line.substr(pos+2), x_);
					}
					else if (line.find("y=") != ::std::string::npos)
					{
						::dcs::eesim::detail::ampl::parse_str(line.substr(pos+2), y_);
					}
					else if (line.find("s=") != ::std::string::npos)
					{
						::dcs::eesim::detail::ampl::parse_str(line.substr(pos+2), s_);
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


	public: ::dcs::eesim::detail::ampl::solver_results solver_result() const
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
	private: ::dcs::eesim::detail::ampl::solver_results solver_result_;
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
	public: typedef typename traits_type::physical_machine_identifier_type physical_machine_identifier_type;
	public: typedef typename traits_type::virtual_machine_identifier_type virtual_machine_identifier_type;
	public: typedef ::std::pair<physical_machine_identifier_type,virtual_machine_identifier_type> physical_virtual_machine_pair_type;
	public: typedef ::std::vector< ::std::pair<physical_resource_category,real_type> > resource_share_container;
	public: typedef ::std::map<physical_virtual_machine_pair_type,resource_share_container> physical_virtual_machine_map;
	public: typedef ::std::map<virtual_machine_identifier_type,real_type> virtual_machine_utilization_map;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	private: typedef physical_machine<traits_type> physical_machine_type;
	private: typedef ::dcs::shared_ptr<physical_machine_type> physical_machine_pointer;
	private: typedef virtual_machine<traits_type> virtual_machine_type;
	private: typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
	private: typedef typename virtual_machine_type::application_tier_type application_tier_type;
	private: typedef ::std::vector<physical_machine_identifier_type> physical_machine_identifier_container;
	private: typedef ::std::vector<virtual_machine_identifier_type> virtual_machine_identifier_container;


	public: ampl_minlp_solver_impl()
	: pm_ids_(),
	  vm_ids_(),
	  solved_(0),
	  cost_(0),
	  placement_()
	{
	}


	public: void solve(data_center_type const& dc, real_type wp, real_type ws, virtual_machine_utilization_map const& vm_util_map)
	{
		// Reset previous solution
		pm_ids_.clear();
		vm_ids_.clear();
		solved_ = false;
		cost_ = 0;
		placement_.clear();

		// Create a new problem
		::std::string problem;
		problem = make_problem(dc, wp, ws, vm_util_map);

		// Solve the new problem
		ampl_minlp_input_producer producer(problem);
		ampl_minlp_output_consumer consumer;
		::dcs::eesim::detail::ampl::run_ampl_command(
				::dcs::eesim::detail::ampl::find_ampl_command(),
				::std::vector< ::std::string >(),
				producer,
				consumer
			);

		// Build the new solution
		if (consumer.solver_result() == ::dcs::eesim::detail::ampl::solved_result)
		{
			solved_ = true;
			cost_ = consumer.cost();

			typename ampl_minlp_output_consumer::smallint_matrix_type placement_flags(consumer.virtual_machine_placement());
			typename ampl_minlp_output_consumer::real_matrix_type placement_shares(consumer.virtual_machine_shares());

			::std::size_t npm(::boost::numeric::ublasx::num_rows(placement_flags));
			::std::size_t nvm(::boost::numeric::ublasx::num_columns(placement_flags));

			for (::std::size_t i = 0; i < npm; ++i)
			{
				physical_machine_identifier_type pm_id(pm_ids_[i]);

				for (::std::size_t j = 0; j < nvm; ++j)
				{
					if (placement_flags(i,j))
					{
						virtual_machine_identifier_type vm_id(vm_ids_[j]);

						//FIXME: CPU resource category is hard-coded.
						resource_share_container shares(1, ::std::make_pair(cpu_resource_category, placement_shares(i,j)));

						placement_[::std::make_pair(pm_id, vm_id)] = shares;
					}
				}
			}
		}
	}


	public: bool solved() const
	{
		return solved_;
	}


	public: real_type cost() const
	{
		return cost_;
	}


	public: physical_virtual_machine_map const& placement() const
	{
		return placement_;
	}


	private: physical_machine_identifier_type physical_machine_id(::std::size_t idx) const
	{
		// pre: 0 < idx <= size(pm_ids_)
		DCS_ASSERT(
				idx > 0 && idx < pm_ids_.size(),
				throw ::std::invalid_argument("[dcs::eesim::detail::ampl_minlp_solver_impl::physical_machine_id] Bad index.")
			);

		return pm_ids_[idx-1];
	}


	private: virtual_machine_identifier_type virtual_machine_id(::std::size_t idx) const
	{
		// pre: 0 < idx <= size(vm_ids_)
		DCS_ASSERT(
				idx > 0 && idx < vm_ids_.size(),
				throw ::std::invalid_argument("[dcs::eesim::detail::ampl_minlp_solver_impl::virtual_machine_id] Bad index.")
			);

		return vm_ids_[idx-1];
	}


	private: ::std::string make_problem(data_center_type const& dc, real_type wp, real_type ws, virtual_machine_utilization_map const& vm_util_map)
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

		// Create the set of all virtual machines
		vm_container vms(dc.virtual_machines());

		::std::size_t n_vms(vms.size());

		pm_ids_ = physical_machine_identifier_container(n_pms);
		vm_ids_ = virtual_machine_identifier_container(n_vms);

		// # of VMs
		oss << "param nj := " << n_vms << ";" << ::std::endl;

		// Power model coefficients, max share and resource capacity
		oss << "param: c0 c1 c2 r Smax C :=" << ::std::endl;
		for (::std::size_t i = 0; i < n_pms; ++i)
		{
			physical_machine_pointer ptr_pm(pms[i]);

			pm_ids_[i] = ptr_pm->id();

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

		// Reference machine capacity, utilization and min share of tiers
		oss << "param: Cr ur Srmin := " << ::std::endl;
		for (::std::size_t j = 0; j < n_vms; ++j)
		{
			virtual_machine_pointer ptr_vm(vms[j]);

			vm_ids_[j] = ptr_vm->id();
			application_type const& app(ptr_vm->guest_system().application());

			//FIXME: CPU resource category is hard-coded
			reference_resource_type const& ref_resource(app.reference_resource(::dcs::eesim::cpu_resource_category));

			oss << " " << (j+1)
				<< " " << (ref_resource.capacity()*ref_resource.utilization_threshold())
				<< " " << vm_util_map.at(ptr_vm->id())
				<< " " << 0.2 //FIXME: Minimum share is hard-coded
				<< ::std::endl;
		}
		oss << ";" << ::std::endl;

		oss << "param wp := " << wp << ";" << ::std::endl;
		oss << "param ws := " << ws << ";" << ::std::endl;

		//FIXME: solver 'couenne' is hard-coded
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
			<< "print 'solve_exitcode=', solve_exitcode, ';';"
			<< "print 'solve_result=', solve_result, ';';"
			<< "print 'solve_result_num=', solve_result_num, ';';"
			<< "print 'cost=', cost, ';';"
			<< "print 'x=[', {i in I} round(x[i]), '];';"
			<< "print 'y=[', {i in I} (({j in J} round(y[i,j])), ';'), '];';"
			<< "print 's=[', {i in I} (({j in J} round(s[i,j], 5)), ';'), '];';"
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
			//<< "param ur{J} >= 0, <= 1;"
			<< "param ur{J} >= 0;"
			<< "param Cr{J} >= 0;"
			//<< "param Smax{I} >= 0, <= 1;"
			<< "param Smax{I} >= 0;"
			//<< "param Srmin{J} >= 0, <= 1;"
			<< "param Srmin{J} >= 0;"
			<< "param C{I} >= 0;"
			<< "param wp >= 0 default 0;"
			<< "param ws >= 0 default 0;"
			<< "param eps := 1.0e-5;"
			<< "param wwp := wp / (ni * max{i in I} (c0[i]+c1[i]+c2[i]));"
			<< "param wws := ws / nj;"
			<< "var x{I} binary;"
			<< "var y{I,J} binary;"
			<< "var s{I,J} >= 0, <= 1;"
			<< "minimize cost: wwp * sum{i in I} x[i]*(c0[i] + c1[i]*(sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i]*(s[i,j]+eps))) + c2[i]*(sum{j in J} (y[i,j]+eps)*ur[j]*Cr[j]/(C[i]*(s[i,j]+eps)))^r[i]) + wws * sum{i in I, j in J} ((s[i,j]*C[i]/Cr[j]-1)^2)*y[i,j];"
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


	private: physical_machine_identifier_container pm_ids_;
	private: virtual_machine_identifier_container vm_ids_;
	private: bool solved_;
	private: real_type cost_;
	private: physical_virtual_machine_map placement_;
}; // ampl_minlp_solver_impl

}}} // Namespace detail::<unnamed>::initial_placement


template <typename TraitsT>
class optimal_initial_placement_strategy: public base_initial_placement_strategy<TraitsT>
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;


	private: virtual_machines_placement<traits_type> do_placement(data_center<traits_type> const& dc)
	{
DCS_DEBUG_TRACE("BEGIN Initial Placement");//XXX
		typedef typename traits_type::virtual_machine_identifier_type virtual_machine_identifier_type;
		typedef ::std::map<virtual_machine_identifier_type,real_type> virtual_machine_utilization_map;
		typedef detail::initial_placement::ampl_minlp_solver_impl<traits_type> optimal_solver_type;
		typedef typename optimal_solver_type::physical_virtual_machine_map physical_virtual_machine_map;
		typedef typename physical_virtual_machine_map::const_iterator physical_virtual_machine_iterator;
		typedef typename optimal_solver_type::resource_share_container resource_share_container;
		typedef physical_machine<traits_type> physical_machine_type;
		typedef ::dcs::shared_ptr<physical_machine_type> physical_machine_pointer;
		typedef virtual_machine<traits_type> virtual_machine_type;
		typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;


		virtual_machine_utilization_map vm_util_map;

		// Retrieve application performance info
        {
			typedef virtual_machine<traits_type> virtual_machine_type;
			typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
			typedef typename traits_type::physical_machine_identifier_type physical_machine_identifier_type;
            typedef ::std::vector<virtual_machine_pointer> virtual_machine_container;
            typedef typename virtual_machine_container::const_iterator virtual_machine_iterator;

            virtual_machine_container vms(dc.virtual_machines());
            virtual_machine_iterator vm_end_it(vms.end());
            for (virtual_machine_iterator vm_it = vms.begin(); vm_it != vm_end_it; ++vm_it)
            {
                virtual_machine_pointer ptr_vm(*vm_it);

				vm_util_map[ptr_vm->id()] = ptr_vm->guest_system().application().performance_model().tier_measure(
												ptr_vm->guest_system().id(),
												::dcs::eesim::utilization_performance_measure
					);
			}
		}

		// Solve the optimization problem

		optimal_solver_type solver;

		solver.solve(dc, wp_, ws_, vm_util_map);

		// Check solution and act accordingly

		//FIXME: handle ref_penalty

		if (!solver.solved())
		{
			//TODO: throw an error
		}

		virtual_machines_placement<traits_type> deployment;

		physical_virtual_machine_map pm_vm_map(solver.placement());
::std::cerr << "CHECK SOLVER INITIAL PLACEMENT" << ::std::endl;//XXX
for (typename physical_virtual_machine_map::const_iterator it = solver.placement().begin(); it != solver.placement().end(); ++it)//XXX
{//XXX
::std::cerr << "VM ID: " << (it->first.second) << " placed on PM ID: " << (it->first.first) << " with SHARE: " << ((it->second)[0].second) << ::std::endl;//XXX
}//XXX
		physical_virtual_machine_iterator pm_vm_end_it(pm_vm_map.end());
		for (physical_virtual_machine_iterator pm_vm_it = pm_vm_map.begin(); pm_vm_it != pm_vm_end_it; ++pm_vm_it)
		{
			physical_machine_pointer ptr_pm(dc.physical_machine_ptr(pm_vm_it->first.first));
			virtual_machine_pointer ptr_vm(dc.virtual_machine_ptr(pm_vm_it->first.second));
			resource_share_container shares(pm_vm_it->second);

//::std::cerr << "Going to migrate VM (" << pm_vm_it->first.second << "): " << *ptr_vm << " into PM (" << pm_vm_it->first.first << "): " << *ptr_pm << ::std::endl; //XXX
			deployment.place(*ptr_vm,
							 *ptr_pm,
							 shares.begin(),
							 shares.end());
DCS_DEBUG_TRACE("Placed: VM(" << ptr_vm->id() << ") -> PM(" << ptr_pm->id() << ")");//XXX
		}

DCS_DEBUG_TRACE("END Initial Placement ==> " << deployment);///XXX
		return deployment;
	}


	/// Weight for power consumption cost.
	private: real_type wp_;
	/// Weight for resource share cost.
	private: real_type ws_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_OPTIMAL_INITIAL_PLACEMENT_STRATEGY_HPP
