#ifndef DCS_EESIM_DETAIL_AMPL_VM_PLACEMENT_MINLP_SOLVER_HPP
#define DCS_EESIM_DETAIL_AMPL_VM_PLACEMENT_MINLP_SOLVER_HPP


#include <cstddef>
////#include <boost/numeric/ublas/io.hpp>
//#include <boost/numeric/ublas/matrix.hpp>
//#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublasx/operation/num_columns.hpp>
#include <boost/numeric/ublasx/operation/num_rows.hpp>
#include <dcs/assert.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/data_center.hpp>
#include <dcs/eesim/detail/ampl/solver_results.hpp>
#include <dcs/eesim/detail/ampl/utility.hpp>
#include <dcs/eesim/detail/ampl/vm_placement_problem.hpp>
#include <dcs/eesim/detail/ampl/vm_placement_problem_result.hpp>
#include <dcs/eesim/detail/base_initial_vm_placement_optimal_solver.hpp>
#include <dcs/eesim/detail/base_vm_placement_optimal_solver.hpp>
#include <dcs/eesim/optimal_solver_categories.hpp>
#include <dcs/eesim/optimal_solver_ids.hpp>
#include <dcs/eesim/optimal_solver_input_methods.hpp>
#include <dcs/eesim/optimal_solver_proxies.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <iosfwd>
//#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


namespace dcs { namespace eesim { namespace detail { namespace ampl {

namespace detail { namespace /*<unnamed>*/ {

class minlp_input_producer
{
	public: minlp_input_producer(::std::string const& script)
	: script_(script)
	{
	}


	public: template <typename CharT, typename CharTraitsT>
		void operator()(::std::basic_ostream<CharT,CharTraitsT>& os)
	{
		os << script_;
	}


	private: ::std::string script_;
}; // minlp_input_producer


class minlp_output_consumer: public ::dcs::eesim::detail::ampl::vm_placement_problem_result
{
	private: typedef ::dcs::eesim::detail::ampl::vm_placement_problem_result base_type;
	public: typedef typename base_type::int_type int_type;
	public: typedef typename base_type::smallint_type smallint_type;
	public: typedef typename base_type::real_type real_type;
	public: typedef typename base_type::smallint_vector_type smallint_vector_type;
	public: typedef typename base_type::smallint_matrix_type smallint_matrix_type;
	public: typedef typename base_type::real_matrix_type real_matrix_type;
}; // minlp_input_consumer

}} // Namespace detail::<unnamed>


template <typename TraitsT>
class initial_vm_placement_minlp_solver: public base_initial_vm_placement_optimal_solver<TraitsT>
{
	private: typedef base_initial_vm_placement_optimal_solver<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	private: typedef typename base_type::real_type real_type;
	private: typedef typename base_type::data_center_type data_center_type;
	private: typedef typename base_type::virtual_machine_utilization_map virtual_machine_utilization_map;
/*
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
	private: typedef typename data_center_type::physical_machine_type physical_machine_type;
	private: typedef typename data_center_type::physical_machine_pointer physical_machine_pointer;
	private: typedef typename data_center_type::virtual_machine_type virtual_machine_type;
	private: typedef typename data_center_type::virtual_machine_pointer virtual_machine_pointer;
	private: typedef typename virtual_machine_type::application_tier_type application_tier_type;
*/


	public: static const optimal_solver_ids default_solver_id;


	public: explicit initial_vm_placement_minlp_solver(optimal_solver_ids solver_id = default_solver_id)
	: base_type(solver_id, ampl_optimal_solver_input_method)
	{
	}


	private: optimal_solver_categories do_category() const
	{
		return minco_optimal_solver_category;
	}


	private: optimal_solver_proxies do_proxy() const
	{
		return none_optimal_solver_proxy;
	}


	private: void do_solve(data_center_type const& dc,
						   real_type wp,
						   real_type ws,
						   real_type ref_penalty,
						   virtual_machine_utilization_map const& vm_util_map)
	{
		// Reset previous solution
		this->result().reset();

		// Create a new problem
		vm_placement_problem<traits_type> problem_descr;
		problem_descr = make_vm_placement_problem<traits_type>(dc, wp, ws, ref_penalty, vm_util_map);

		//FIXME: solver 'couenne' is hard-coded
		::std::ostringstream oss;
		oss << "reset;"
			<< "model;"
			<< problem_descr.model
			<< "data;"
			<< problem_descr.data
			<< "option solver " << to_ampl_solver(this->solver_id()) << ";"
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
//			<< "print 'x=(', card({I}), ')[', ({i in I} (i, x[i])), '];';"
//			<< "print 'x=[', ({i in I} (i, x[i])), '];';"
			<< "print 'x=[', {i in I} round(x[i]), '];';"
//			<< "print 'y=(', card({I}), ',', card({J}), ')[', ({i in I} (i, ({j in J} (j, y[i,j])), ';')), '];';"
//			<< "print 'y=[', ({i in I} (i, ({j in J} (j, y[i,j])), ';')), '];';"
			<< "print 'y=[', {i in I} (({j in J} round(y[i,j])), ';'), '];';"
//			<< "print 's=(', card({I}), ',', card({J}), ')[', ({i in I} (i, ({j in J} (j, s[i,j])), ';')), '];';"
//			<< "print 's=[', ({i in I} (i, ({j in J} (j, s[i,j])), ';')), '];';"
			<< "print 's=[', {i in I} (({j in J} s[i,j]), ';'), '];';"
			<< "print '-- [/RESULT] --';"
			<< ::std::endl
			<< "end;"
			<< ::std::endl;

::std::cerr << "Create problem: " << oss.str() << ::std::endl;//XXX
		// Solve the new problem
		detail::minlp_input_producer producer(oss.str());
		detail::minlp_output_consumer consumer;
		run_ampl_command(find_ampl_command(),
						 ::std::vector< ::std::string >(),
						 producer,
						 consumer);

		// Build the new solution
		if (consumer.solver_result() == ::dcs::eesim::detail::ampl::solved_result)
		{
			this->result(::dcs::eesim::detail::make_vm_placement_problem_result<traits_type>(problem_descr, problem_res));
			this->result().solved(true);
		}
	}
}; // initial_vm_placement_minlp_solver

template <typename TraitsT>
const optimal_solver_ids initial_vm_placement_minlp_solver<TraitsT>::default_solver_id(couenne_optimal_solver_id);


template <typename TraitsT>
class vm_placement_minlp_solver: public base_vm_placement_optimal_solver<TraitsT>
{
	private: typedef base_vm_placement_optimal_solver<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	private: typedef typename base_type::real_type real_type;
	private: typedef typename base_type::data_center_type data_center_type;
	private: typedef typename base_type::virtual_machine_utilization_map virtual_machine_utilization_map;
/*
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
	private: typedef typename data_center_type::physical_machine_type physical_machine_type;
	private: typedef typename data_center_type::physical_machine_pointer physical_machine_pointer;
	private: typedef typename data_center_type::virtual_machine_type virtual_machine_type;
	private: typedef typename data_center_type::virtual_machine_pointer virtual_machine_pointer;
	private: typedef typename virtual_machine_type::application_tier_type application_tier_type;
*/


    public: static const optimal_solver_ids default_solver_id;


	public: explicit vm_placement_minlp_solver(optimal_solver_ids solver_id = default_optimal_solver_id)
	: base_type(solver_id, ampl_optiml_solver_input_method)
	{
	}


	private: optimal_solver_categories do_category() const
	{
		return minco_optimal_solver_category;
	}


	private: optimal_solver_proxies do_proxy() const
	{
		return none_optimal_solver_proxy;
	}


	private: void do_solve(data_center_type const& dc,
						   real_type wp,
						   real_type wm,
						   real_type ws,
						   virtual_machine_utilization_map const& vm_util_map)
	{
		// Reset previous solution
		this->result().reset();

		// Create a new problem
		vm_placement_problem<traits_type> problem_descr;
		problem_descr = make_vm_placement_problem<traits_type>(dc, wp, wm, ws, vm_util_map);

		::std::ostringstream oss;
		oss << "reset;"
			<< "model;"
			<< problem_descr.model
			<< "data;"
			<< problem_descr.data
			<< "option solver " << to_ampl_solver(this->solver_id()) << ";"
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
//			<< "print 'x=(', card({I}), ')[', ({i in I} (i, x[i])), '];';"
//			<< "print 'x=[', ({i in I} (i, x[i])), '];';"
			<< "print 'x=[', {i in I} round(x[i]), '];';"
//			<< "print 'y=(', card({I}), ',', card({J}), ')[', ({i in I} (i, ({j in J} (j, y[i,j])), ';')), '];';"
//			<< "print 'y=[', ({i in I} (i, ({j in J} (j, y[i,j])), ';')), '];';"
			<< "print 'y=[', {i in I} (({j in J} round(y[i,j])), ';'), '];';"
//			<< "print 's=(', card({I}), ',', card({J}), ')[', ({i in I} (i, ({j in J} (j, s[i,j])), ';')), '];';"
//			<< "print 's=[', ({i in I} (i, ({j in J} (j, s[i,j])), ';')), '];';"
			<< "print 's=[', {i in I} (({j in J} s[i,j]), ';'), '];';"
			<< "print '-- [/RESULT] --';"
			<< ::std::endl
			<< "end;"
			<< ::std::endl;

::std::cerr << "Create problem: " << oss.str() << ::std::endl;//XXX
		// Solve the new problem
		detail::minlp_input_producer producer(oss.str());
		detail::minlp_output_consumer consumer;
		run_ampl_command(find_ampl_command(),
						 ::std::vector< ::std::string >(),
						 producer,
						 consumer);

//::std::cerr << "AMPL terminated" << ::std::endl;//XXX
		// Build the new solution
		if (consumer.solver_result() == solved_result)
		{
			this->result(::dcs::eesim::detail::make_vm_placement_problem_result<traits_type>(problem_descr, problem_res));
			this->result().solved(true);
		}
	}
}; // vm_placement_minlp_solver

template <typename TraitsT>
const optimal_solver_ids vm_placement_minlp_solver<TraitsT>::default_solver_id(couenne_optimal_solver_id);

}}}} // Namespace dcs::eesim::detail::ampl


#endif // DCS_EESIM_DETAIL_AMPL_VM_PLACEMENT_MINLP_SOLVER_HPP
