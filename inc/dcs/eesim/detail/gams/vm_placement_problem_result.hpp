#ifndef DCS_EESIM_DETAIL_GAMS_VM_PLACEMENT_PROBLEM_RESULT_HPP
#define DCS_EESIM_DETAIL_GAMS_VM_PLACEMENT_PROBLEM_RESULT_HPP


#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <cstddef>
#include <dcs/eesim/detail/gams/model_results.hpp>
#include <dcs/eesim/detail/gams/solver_results.hpp>
#include <dcs/eesim/detail/gams/utility.hpp>
#include <iosfwd>
#include <limits>
#include <sstream>
#include <string>


namespace dcs { namespace eesim { namespace detail { namespace gams {

class vm_placement_problem_result
{
	public: typedef int int_type;
	public: typedef short smallint_type; // don't use char since it may fails on some tests
	public: typedef double real_type;
	public: typedef ::boost::numeric::ublas::vector<smallint_type> smallint_vector_type;
	public: typedef ::boost::numeric::ublas::matrix<smallint_type> smallint_matrix_type;
	public: typedef ::boost::numeric::ublas::matrix<real_type> real_matrix_type;


	public: vm_placement_problem_result()
	: solver_result_(unknown_solver_result),
	  model_result_(unknown_model_result),
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
			bool skip(false);

//::std::cerr << "Read-GAMS>> " << line << " (old state: " << state << ")" <<::std::endl;//XXX
			switch (state)
			{
				case skip_state:
					if (line.find("-- [RESULT] --") != ::std::string::npos)
					{
						state = out_analysis_state;
					}
					break;
				case out_analysis_state:
					if ((pos = line.find("solver_status=")) != ::std::string::npos)
					{
						int status;
						parse_str(line.substr(pos+14), status);
						solver_result_ = static_cast<solver_results>(status);
//::std::cerr << "Read-GAMS>> SOLVER STATUS: " << solver_result_ << ::std::endl;//XXX

						if (solver_result_ != normal_completion_solver_result)
						{
							// Problem in calling the solver
							state = end_state;
							ok = false;
						}
					}
					else if ((pos = line.find("model_status=")) != ::std::string::npos)
					{
						int status;
						parse_str(line.substr(pos+13), status);
						model_result_ = static_cast<model_results>(status);
//::std::cerr << "Read-GAMS>> MODEL STATUS: " << model_result_ << ::std::endl;//XXX
					}
					else
					{
						skip = true;
					}

					if (ok && !skip)
					{
						if (num_solver_info < 1)
						{
							state = out_analysis_state;
							++num_solver_info;
						}
						else if (solver_result_ == normal_completion_solver_result
								 &&
								 (model_result_ == optimal_model_result
								  || model_result_ == locally_optimal_model_result
								  || model_result_ == integer_solution_model_result
								  || model_result_ == solved_unique_model_result
								  || model_result_ == solved_model_result
								  || model_result_ == solved_singular_model_result))
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
						parse_str(line.substr(pos+5), cost_);
//::std::cerr << "Read-GAMS>> COST: " << cost_ << ::std::endl;//XXX
					}
					else if (line.find("x=") != ::std::string::npos)
					{
						parse_str(line.substr(pos+2), x_);
//::std::cerr << "Read-GAMS>> X: " << x_ << ::std::endl;//XXX
					}
					else if (line.find("y=") != ::std::string::npos)
					{
						parse_str(line.substr(pos+2), y_);
//::std::cerr << "Read-GAMS>> Y: " << y_ << ::std::endl;//XXX
					}
					else if (line.find("s=") != ::std::string::npos)
					{
						parse_str(line.substr(pos+2), s_);
//::std::cerr << "Read-GAMS>> S: " << s_ << ::std::endl;//XXX
					}
					else if (line.find("-- [/RESULT] --") != ::std::string::npos)
					{
						state = end_state;
					}
					break;
				case end_state:
					break;
			}
//::std::cerr << "Read-GAMS>> " << line << " (new state: " << state << ")" <<::std::endl;//XXX
		}

//		if (ok && (model_result_ != optimal_model_result || model_result_ != locally_optimal_model_result))
//		{
//			// Do not trust the cost value returned by GAMS
//			cost_ = ...
//		}
	}


	public: solver_results solver_result() const
	{
		return solver_result_;
	}


	public: model_results model_result() const
	{
		return model_result_;
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


	private: solver_results solver_result_;
	private: model_results model_result_;
	private: real_type cost_;
	private: smallint_vector_type x_;
	private: smallint_matrix_type y_;
	private: real_matrix_type s_;
}; // vm_placement_problem_result


template <typename CharT, typename CharTraitsT>
inline
vm_placement_problem_result make_vm_placement_problem_result(::std::basic_istream<CharT,CharTraitsT>& is)
{
	vm_placement_problem_result result;

	result(is);

	return result;
}


inline
vm_placement_problem_result make_vm_placement_problem_result(::std::string const& s)
{
	::std::istringstream iss(s);

	return make_vm_placement_problem_result(iss);
}


}}}} // Namespace dcs::eesim::detail::gams


#endif // DCS_EESIM_DETAIL_GAMS_VM_PLACEMENT_PROBLEM_RESULT_HPP
