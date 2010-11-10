#include <cstddef>
#include <dcs/debug.hpp>
#include <dcs/perfeval/qn/open_multi_bcmp_network.hpp>
#include <dcs/perfeval/qn/operation/visit_ratios.hpp>
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
	real_vector_type lambda(nq);

	P(0,0) = 0.0; P(0,1) = 0.3; P(0,2) = 0.5;
	P(1,0) = 1.0; P(1,1) = 0.0; P(1,2) = 0.0;
	P(2,0) = 1.0; P(2,1) = 0.0; P(2,2) = 0.0;
	lambda(0) = 0.15;
	lambda(1) = 0.0;
	lambda(2) = 0.0;

	real_vector_type v;
	real_vector_type expect_v(nq);


	expect_v(0) = 5.0;
	expect_v(1) = 1.5;
	expect_v(2) = 2.5;
	v = dcs::perfeval::qn::visit_ratios(P, lambda);
	DCS_DEBUG_TRACE( "Visit ratios: " << v );
	DCS_TEST_CHECK_VECTOR_CLOSE( v, expect_v, nq, tol );
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
	P(1,0) = 1.0; P(1,1) = 0.0; P(1,2) = 0.0;
	P(2,0) = 1.0; P(2,1) = 0.0; P(2,2) = 0.0;

	vector_type v;
	vector_type expect(nq);


	expect(0) = 1.0;
	expect(1) = 0.3;
	expect(2) = 0.7;
	v = dcs::perfeval::qn::visit_ratios<vector_type>(P);
	DCS_DEBUG_TRACE( "Visit ratios: " << v );
	DCS_TEST_CHECK_VECTOR_CLOSE( v, expect, nq, tol );
}


DCS_TEST_DEF( test_open_multi )
{
	DCS_DEBUG_TRACE("Test Case: Open Queue - Multi Class");

	typedef double real_type;
	typedef ::std::size_t size_type;
	typedef boost::numeric::ublas::vector<real_type> vector_type;
	typedef boost::numeric::ublas::matrix<real_type> matrix_type;
	typedef dcs::perfeval::qn::open_multi_bcmp_network<vector_type, matrix_type> queueing_network_type;

	size_type nc = 2; // number of customer classes
	size_type nq = 3; // number of queues (service centers)

	matrix_type P(nq, nq);
	vector_type lambda(nq);

	P(0,0) = 0.0; P(0,1) = 0.3; P(0,2) = 0.5;
	P(1,0) = 1.0; P(1,1) = 0.0; P(1,2) = 0.0;
	P(2,0) = 1.0; P(2,1) = 0.0; P(2,2) = 0.0;
	lambda(0) = 0.15;
	lambda(1) = 0.0;
	lambda(2) = 0.0;

	matrix_type v(nc, nq);
	vector_type expect(nq);


	expect(0) = 5.0;
	expect(1) = 1.5;
	expect(2) = 2.5;
	boost::numeric::ublas::row(v, 0) = dcs::perfeval::qn::visit_ratios(P, lambda);
	DCS_DEBUG_TRACE( "Visit ratios: " << boost::numeric::ublas::row(v, 0) );
	DCS_TEST_CHECK_VECTOR_CLOSE( boost::numeric::ublas::row(v, 0), expect, nq, tol );
}


int main()
{
	DCS_TEST_SUITE( "Visit Ratios for Product-Form Queueing Networks" );

	DCS_TEST_BEGIN();

	DCS_TEST_DO( test_open_single );
	DCS_TEST_DO( test_closed_single );
//	DCS_TEST_DO( test_open_multi );

	DCS_TEST_END();
}
