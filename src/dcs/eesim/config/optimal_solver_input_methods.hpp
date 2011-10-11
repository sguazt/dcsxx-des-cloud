#ifndef DCS_EESIM_CONFIG_OPTIMAl_SOLVER_INPUT_METHODS_HPP
#define DCS_EESIM_CONFIG_OPTIMAl_SOLVER_INPUT_METHODS_HPP


#include <dcs/eesim/optimal_solver_input_methods.hpp>
#include <iosfwd>


namespace dcs { namespace eesim { namespace config {

typedef ::dcs::eesim::optimal_solver_input_methods optimal_solver_input_methods;

/*
enum optimal_solver_input_methods
{
    ampl_optimal_solver_input_method, ///< AMPL (http://www.ampl.com)
    c_optimal_solver_input_method, ///< C
    cplex_optimal_solver_input_method, ///< CPLEX (http://plato.asu.edu/cplex_lp.pdf)
    dimacs_optimal_solver_input_method, ///< DIMACS (http://www.mcs.anl.gov/otc/Guide/OptWeb/continuous/constrained/network/dimacs_mcf.html)
    fortran_optimal_solver_input_method, ///< Fortran
    gams_optimal_solver_input_method, ///< GAMS (http://www.gams.com)
    lp_optimal_solver_input_method, ///< LP (http://plato.asu.edu/ftp/lp_format.txt)
    matlab_optimal_solver_input_method, ///< MATLAB
    matlabbinary_optimal_solver_input_method, ///< MATLAB_BINARY (http://plato.asu.edu/ftp/usrguide.pdf)
    mps_optimal_solver_input_method, ///< MPS (http://en.wikipedia.org/wiki/MPS_(format))
    netflo_optimal_solver_input_method, ///< NETFLO (http://www.mcs.anl.gov/otc/Guide/SoftwareGuide/Blurbs/netflo.html)
    qps_optimal_solver_input_method, ///< QPS (http://plato.asu.edu/QPS.pdf)
    relax4_optimal_solver_input_method, ///< RELAX4 (http://www.mcs.anl.gov/otc/Guide/OptWeb/continuous/constrained/network/relax4-format.html)
    sdpa_optimal_solver_input_method, ///< SDPA (http://www.nmt.edu/~sdplib/FORMAT)
    sdplr_optimal_solver_input_method, ///< SDPLR (http://dollar.biz.uiowa.edu/~sburer/files/SDPLR/files/SDPLR-1.03-beta-usrguide.pdf)
    smps_optimal_solver_input_method, ///< SMPS (http://myweb.dal.ca/gassmann/smps2.htm)
    sparse_optimal_solver_input_method, ///< SPARSE
    sparsesdpa_optimal_solver_input_method, ///< SPARSE_SDPA (http://plato.asu.edu/ftp/sdpa_format.txt)
    tsp_optimal_solver_input_method, ///< TSP (http://plato.asu.edu/tsplib.pdf)
    zimpl_optimal_solver_input_method ///< ZIMPL (http://www.zib.de/koch/zimpl/download/zimpl.pdf)
};
*/


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, optimal_solver_input_methods method)
{
	switch (method)
	{
		case ampl_optimal_solver_input_method: ///< AMPL (http://www.ampl.com)
			os << "AMPL";
			break;
		case c_optimal_solver_input_method: ///< C
			os << "C";
			break;
		case cplex_optimal_solver_input_method: ///< CPLEX (http://plato.asu.edu/cplex_lp.pdf)
			os << "CPLEX";
			break;
		case dimacs_optimal_solver_input_method: ///< DIMACS (http://www.mcs.anl.gov/otc/Guide/OptWeb/continuous/constrained/network/dimacs_mcf.html)
			os << "DIMACS";
			break;
		case fortran_optimal_solver_input_method: ///< Fortran
			os << "Fortran";
			break;
		case gams_optimal_solver_input_method: ///< GAMS (http://www.gams.com)
			os << "GAMS";
			break;
		case lp_optimal_solver_input_method: ///< LP (http://plato.asu.edu/ftp/lp_format.txt)
			os << "LP";
			break;
		case matlab_optimal_solver_input_method: ///< MATLAB
			os << "MATLAB";
			break;
		case matlabbinary_optimal_solver_input_method: ///< MATLAB_BINARY (http://plato.asu.edu/ftp/usrguide.pdf)
			os << "MATLAB_BINARY";
			break;
		case mps_optimal_solver_input_method: ///< MPS (http://en.wikipedia.org/wiki/MPS_(format))
			os << "MPS";
			break;
		case netflo_optimal_solver_input_method: ///< NETFLO (http://www.mcs.anl.gov/otc/Guide/SoftwareGuide/Blurbs/netflo.html)
			os << "NETFLO";
			break;
		case qps_optimal_solver_input_method: ///< QPS (http://plato.asu.edu/QPS.pdf)
			os << "QPS";
			break;
		case relax4_optimal_solver_input_method: ///< RELAX4 (http://www.mcs.anl.gov/otc/Guide/OptWeb/continuous/constrained/network/relax4-format.html)
			os << "RELAX4";
			break;
		case sdpa_optimal_solver_input_method: ///< SDPA (http://www.nmt.edu/~sdplib/FORMAT)
			os << "SDPA";
			break;
		case sdplr_optimal_solver_input_method: ///< SDPLR (http://dollar.biz.uiowa.edu/~sburer/files/SDPLR/files/SDPLR-1.03-beta-usrguide.pdf)
			os << "SDPLR";
			break;
		case smps_optimal_solver_input_method: ///< SMPS (http://myweb.dal.ca/gassmann/smps2.htm)
			os << "SMPS";
			break;
		case sparse_optimal_solver_input_method: ///< SPARSE
			os << "SPARSE";
			break;
		case sparsesdpa_optimal_solver_input_method: ///< SPARSE_SDPA (http://plato.asu.edu/ftp/sdpa_format.txt)
			os << "SPARSE_SDPA";
			break;
		case tsp_optimal_solver_input_method: ///< TSP (http://plato.asu.edu/tsplib.pdf)
			os << "TSP";
			break;
		case zimpl_optimal_solver_input_method: ///< ZIMPL (http://www.zib.de/koch/zimpl/download/zimpl.pdf)
			os << "ZIMPL";
			break;
	}

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_OPTIMAl_SOLVER_INPUT_METHODS_HPP
