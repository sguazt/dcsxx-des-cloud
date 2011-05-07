#include <cstddef>
#include <dcs/debug.hpp>
#include <dcs/perfeval/qn/open_multi_bcmp_network.hpp>
#include <dcs/perfeval/qn/operation/traffic_rates.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <dcs/test.hpp>


static const double tol = 1.0e-5;


DCS_TEST_DEF( test_open_single )
{
	DCS_DEBUG_TRACE("Test Case: Open Queue - Single Class");

	typedef double real_type;
	typedef ::std::size_t size_type;
	typedef ::std::size_t uint_type;
	typedef boost::numeric::ublas::vector<real_type> real_vector_type;
	typedef boost::numeric::ublas::vector<size_type> uint_vector_type;
	typedef boost::numeric::ublas::matrix<real_type> matrix_type;
	//typedef dcs::perfeval::qn::open_multi_bcmp_network<real_vector_type, uint_vector_type, matrix_type> queueing_network_type;
	typedef dcs::perfeval::qn::open_multi_bcmp_network<real_type, uint_type> queueing_network_type;

	size_type nq = 3; // number of queues (service centers)

	matrix_type P(nq, nq);
	real_vector_type lambda0(nq);

	P(0,0) = 0.00; P(0,1) = 0.50; P(0,2) = 0.00;
	P(1,0) = 0.33; P(1,1) = 0.00; P(1,2) = 0.33;
	P(2,0) = 0.00; P(2,1) = 0.50; P(2,2) = 0.00;
	lambda0(0) = 0.051;
	lambda0(1) = 0.025;
	lambda0(2) = 0.301;

	real_vector_type lambda;
	real_vector_type expect_lambda(nq);


	expect_lambda(0) = 0.15;
	expect_lambda(1) = 0.30;
	expect_lambda(2) = 0.40;
	lambda = dcs::perfeval::qn::traffic_rates(P, lambda0);
	DCS_DEBUG_TRACE( "Traffic rates: " << lambda );
	DCS_TEST_CHECK_VECTOR_CLOSE( lambda, expect_lambda, nq, tol );
}


DCS_TEST_DEF( test_closed_single )
{
	DCS_DEBUG_TRACE("Test Case: Closed Queue - Single Class");

	typedef double real_type;
	typedef ::std::size_t size_type;
	typedef boost::numeric::ublas::vector<real_type> vector_type;
	typedef boost::numeric::ublas::matrix<real_type> matrix_type;
	typedef dcs::perfeval::qn::open_multi_bcmp_network<vector_type, matrix_type> queueing_network_type;

	size_type nq = 3; // number of queues (service centers)

	matrix_type P(nq, nq);

	P(0,0) = 0.0; P(0,1) = 0.3; P(0,2) = 0.7;
	P(1,0) = 0.1; P(1,1) = 0.4; P(1,2) = 0.5;
	P(2,0) = 0.0; P(2,1) = 0.6; P(2,2) = 0.4;

	vector_type lambda;
	vector_type expect_lambda(nq);


	expect_lambda(0) =  1.0;
	expect_lambda(1) = 10.0;
	expect_lambda(2) =  9.5;
	lambda = dcs::perfeval::qn::traffic_rates(P);
	DCS_DEBUG_TRACE( "Traffic rates: " << lambda );
	DCS_TEST_CHECK_VECTOR_CLOSE( lambda, expect_lambda, nq, tol );
}


int main()
{
	DCS_TEST_SUITE( "Traffic Rates for Product-Form Queueing Networks" );

	DCS_TEST_BEGIN();

	DCS_TEST_DO( test_open_single );
	DCS_TEST_DO( test_closed_single );

	DCS_TEST_END();
}
