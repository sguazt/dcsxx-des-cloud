#ifndef DCS_EESIM_DETAIL_AMPL_VM_PLACEMENT_PROBLEM_RESULT_HPP
#define DCS_EESIM_DETAIL_AMPL_VM_PLACEMENT_PROBLEM_RESULT_HPP


#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <cstddef>
#include <dcs/eesim/detail/ampl/solver_results.hpp>
#include <dcs/eesim/detail/ampl/utility.hpp>
#include <iosfwd>
#include <limits>
#include <sstream>
#include <string>


namespace dcs { namespace eesim { namespace detail { namespace ampl {

class vm_placement_problem_result
{
	public: typedef int int_type;
	public: typedef short smallint_type; // don't use char since it may fails on some tests
	public: typedef double real_type;
	public: typedef ::boost::numeric::ublas::vector<smallint_type> smallint_vector_type;
	public: typedef ::boost::numeric::ublas::matrix<smallint_type> smallint_matrix_type;
	public: typedef ::boost::numeric::ublas::matrix<real_type> real_matrix_type;


	public: vm_placement_problem_result()
	: solver_exit_code_(0),
	  solver_result_(unknown_result),
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
						parse_str(line.substr(pos+15), solver_exit_code_);

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
						parse_str(line.substr(pos+13), res);

						solver_result_ = solver_result_from_string(res);
					}
					else if ((pos = line.find("solve_result_num=")) != ::std::string::npos)
					{
						parse_str(line.substr(pos+17), solver_result_code_);
					}

					if (ok)
					{

						if (num_solver_info < 3)
						{
							state = out_analysis_state;
							++num_solver_info;
						}
						else if (solver_result_ == solved_result && solver_result_code_ >= 0 && solver_result_code_ < 100)
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
//::std::cerr << "Read-AMPL>> COST: " << cost_ << ::std::endl;//XXX
					}
					else if (line.find("x=") != ::std::string::npos)
					{
						parse_str(line.substr(pos+2), x_);
//::std::cerr << "Read-AMPL>> X: " << x_ << ::std::endl;//XXX
					}
					else if (line.find("y=") != ::std::string::npos)
					{
						parse_str(line.substr(pos+2), y_);
//::std::cerr << "Read-AMPL>> Y: " << y_ << ::std::endl;//XXX
					}
					else if (line.find("s=") != ::std::string::npos)
					{
						parse_str(line.substr(pos+2), s_);
//::std::cerr << "Read-AMPL>> S: " << s_ << ::std::endl;//XXX
					}
					else if (line.find("-- [/RESULT] --") != ::std::string::npos)
					{
						state = end_state;
					}
					break;
				case end_state:
					break;
			}
//::std::cerr << "Read-AMPL>> " << line << " (new state: " << state << ")" <<::std::endl;//XXX
		}
	}


	public: int_type solver_exit_code() const
	{
		return solver_exit_code_;
	}


	public: solver_results solver_result() const
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
	private: solver_results solver_result_;
	private: int_type solver_result_code_;
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


}}}} // Namespace dcs::eesim::detail::ampl


#endif // DCS_EESIM_DETAIL_AMPL_VM_PLACEMENT_PROBLEM_RESULT_HPP
