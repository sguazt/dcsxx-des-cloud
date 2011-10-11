#ifndef DCS_EESIM_OPTIMAL_SOLVER_INPUT_METHODS_HPP
#define DCS_EESIM_OPTIMAL_SOLVER_INPUT_METHODS_HPP


namespace dcs { namespace eesim {

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

}} // Namespace dcs::eesim


#endif // DCS_EESIM_OPTIMAL_SOLVER_INPUT_METHODS_HPP
