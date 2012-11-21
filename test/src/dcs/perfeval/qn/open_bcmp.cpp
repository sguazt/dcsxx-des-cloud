#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublasx/operation/num_columns.hpp>
#include <boost/numeric/ublasx/operation/num_rows.hpp>
#include <boost/numeric/ublasx/operation/size.hpp>
#include <cstddef>
#include <dcs/debug.hpp>
#include <dcs/perfeval/qn/open_multi_bcmp_network.hpp>
#include <dcs/perfeval/qn/operation/visit_ratios.hpp>
//#include <dcs/math/la/container/dense_matrix.hpp>
//#include <dcs/math/la/container/dense_vector.hpp>
//#include <dcs/math/la/operation/io.hpp>
#include <dcs/test.hpp>
#include <map>


namespace ublas = boost::numeric::ublas;
namespace ublasx = boost::numeric::ublasx;


const double tol = 1.0e-5;


DCS_TEST_DEF( test_multi_open_lazowska_p137 )
{
	DCS_DEBUG_TRACE("Test Case: Multiclass Open Network (Lazowska, Chapter 7, Pag. 137)");

	typedef double real_type;
	typedef ::std::size_t size_type;
	typedef unsigned int uint_type;
	typedef ublas::vector<size_type> size_vector_type;
	typedef ublas::vector<uint_type> uint_vector_type;
	typedef ublas::vector<real_type> real_vector_type;
	typedef ublas::matrix<real_type> real_matrix_type;
	//typedef dcs::perfeval::qn::open_multiclass_product_form_network<real_vector_type, uint_vector_type, real_matrix_type> queueing_network_type;
	typedef dcs::perfeval::qn::open_multi_bcmp_network<real_type, uint_type> queueing_network_type;

	size_type nc = 2; // number of classes
	size_type ns = 2; // number of queues (service centers)

	real_matrix_type S(nc, ns); // Service times
	real_matrix_type V(nc, ns); // Visit ratios
	uint_vector_type m(ns, 1); // Number of servers for each station
	real_vector_type lambda(nc); // Arrival rates

	// Class A
	lambda(0) = 3.0/19.0; // 3/19 job/sec
	V(0,0) = 10; // V_{A,CPU}
	V(0,1) = 9; // V_{A,Disk}
	S(0,0) = 1.0/10.0; // S_{A,CPU}
	S(0,1) = 1.0/3.0; // S_{A,Disk}

	// Class B
	lambda(1) = 2.0/19.0; // 2/19 job/sec
	V(1,0) = 5; // V_{B,CPU}
	V(1,1) = 4; // V_{B,Disk}
	S(1,0) = 2.0/5.0; // S_{B,CPU}
	S(1,1) = 1; // S_{B,Disk}

	// Expected utilizations
	real_matrix_type expect_U(nc, ns);
	expect_U(0,0) = 0.157894736842105; expect_U(0,1) = 0.473684210526316;
	expect_U(1,0) = 0.210526315789474; expect_U(1,1) = 0.421052631578947;

	// Expected response time
	real_matrix_type expect_R(nc, ns);
	expect_R(0,0) = 0.158333333333333; expect_R(0,1) = 3.166666666666663;
	expect_R(1,0) = 0.633333333333333; expect_R(1,1) = 9.499999999999991;

	// Expected customers number
	real_matrix_type expect_K(nc, ns);
	expect_K(0,0) = 0.250000000000000; expect_K(0,1) = 4.499999999999996;
	expect_K(1,0) = 0.333333333333333; expect_K(1,1) = 3.999999999999996;

	// Expected throughputs
	real_matrix_type expect_X(nc, ns);
	expect_X(0,0) = 1.578947368421053; expect_X(0,1) = 1.421052631578947;
	expect_X(1,0) = 0.526315789473684; expect_X(1,1) = 0.421052631578947;

	// Expected residence time
	real_matrix_type expect_RT(nc, ns);
	expect_RT(0,0) =  expect_R(0,0)*V(0,0); expect_RT(0,1) = expect_R(0,1)*V(0,1);
	expect_RT(1,0) =  expect_R(1,0)*V(1,0); expect_RT(1,1) = expect_R(1,1)*V(1,1);

	// Expected waiting time
	real_matrix_type expect_W(nc, ns);
	expect_W(0,0) =  expect_R(0,0)-S(0,0); expect_W(0,1) = expect_R(0,1)-S(0,1);
	expect_W(1,0) =  expect_R(1,0)-S(1,0); expect_W(1,1) = expect_R(1,1)-S(1,1);

	// Expected queue length
	real_matrix_type expect_Q(nc, ns);
	expect_Q(0,0) = expect_X(0,0)*expect_W(0,0); expect_Q(0,1) = expect_X(0,1)*expect_W(0,1);
	expect_Q(1,0) = expect_X(1,0)*expect_W(1,0); expect_Q(1,1) = expect_X(1,1)*expect_W(1,1);

	// Expected per-class utilizations
	real_vector_type expect_U_class(nc);
	expect_U_class(0) =  expect_U(0,0) + expect_U(0,1);
	expect_U_class(1) =  expect_U(1,0) + expect_U(1,1);

	// Expected per-station utilizations
	real_vector_type expect_U_station(nc);
	expect_U_station(0) =  expect_U(0,0) + expect_U(1,0);
	expect_U_station(1) =  expect_U(0,1) + expect_U(1,1);

	// Expected per-class response time
	real_vector_type expect_R_class(nc);
	expect_R_class(0) =  expect_R(0,0) + expect_R(0,1);
	expect_R_class(1) =  expect_R(1,0) + expect_R(1,1);

	// Expected per-station response time defined below...

	// Expected per-class customers number
	real_vector_type expect_K_class(nc);
	expect_K_class(0) =  expect_K(0,0) + expect_K(0,1);
	expect_K_class(1) =  expect_K(1,0) + expect_K(1,1);

	// Expected per-station customers number
	real_vector_type expect_K_station(ns);
	expect_K_station(0) =  expect_K(0,0) + expect_K(1,0);
	expect_K_station(1) =  expect_K(0,1) + expect_K(1,1);

	// Expected per-class throughputs
	real_vector_type expect_X_class(nc);
	expect_X_class(0) = lambda(0); // == expect_X(0,0) + expect_X(0,1)
	expect_X_class(1) = lambda(1); // == expect_X(1,0) + expect_X(1,1)

	// Expected per-station throughputs
	real_vector_type expect_X_station(ns);
	expect_X_station(0) = expect_X(0,0) + expect_X(1,0);
	expect_X_station(1) = expect_X(0,1) + expect_X(1,1);

	// Expected per-class residence time
	real_vector_type expect_RT_class(nc);
	expect_RT_class(0) =  expect_RT(0,0) + expect_RT(0,1);
	expect_RT_class(1) =  expect_RT(1,0) + expect_RT(1,1);

	// Expected per-station residence time defined below...

	// Expected per-class waiting time
	real_vector_type expect_W_class(nc);
	expect_W_class(0) =  expect_W(0,0) + expect_W(0,1);
	expect_W_class(1) =  expect_W(1,0) + expect_W(1,1);

	// Expected per-station waiting time
	real_vector_type expect_W_station(ns);
	expect_W_station(0) =  expect_W(0,0) + expect_W(1,0);
	expect_W_station(1) =  expect_W(0,1) + expect_W(1,1);

	// Expected per-class queue length
	real_vector_type expect_Q_class(nc);
	expect_Q_class(0) =  expect_Q(0,0) + expect_Q(0,1);
	expect_Q_class(1) =  expect_Q(1,0) + expect_Q(1,1);

	// Expected per-station queue length
	real_vector_type expect_Q_station(ns);
	expect_Q_station(0) =  expect_Q(0,0) + expect_Q(1,0);
	expect_Q_station(1) =  expect_Q(0,1) + expect_Q(1,1);

	// Expected system utilization
	real_type expect_U_sys;
	expect_U_sys =  expect_U_class(0) + expect_U_class(1);

	// Expected system throughput
	real_type expect_X_sys;
	expect_X_sys =  expect_X_class(0) + expect_X_class(1);

	// Expected per-station response time
	real_vector_type expect_R_station(ns);
	expect_R_station(0) =  expect_R(0,0)*expect_X_class(0)/expect_X_sys + expect_R(1,0)*expect_X_class(1)/expect_X_sys;
	expect_R_station(1) =  expect_R(0,1)*expect_X_class(0)/expect_X_sys + expect_R(1,1)*expect_X_class(1)/expect_X_sys;

	// Expected per-station residence time
	real_vector_type expect_RT_station(ns);
	expect_RT_station(0) =  expect_RT(0,0)*expect_X_class(0)/expect_X_sys + expect_RT(1,0)*expect_X_class(1)/expect_X_sys;
	expect_RT_station(1) =  expect_RT(0,1)*expect_X_class(0)/expect_X_sys + expect_RT(1,1)*expect_X_class(1)/expect_X_sys;

	// Expected system response time
	real_type expect_R_sys;
	expect_R_sys =  expect_R_station(0) + expect_R_station(1);

	// Expected system customers number
	real_type expect_K_sys;
	expect_K_sys =  expect_K_station(0) + expect_K_station(1);

	// Expected system residence time
	real_type expect_RT_sys;
	expect_RT_sys =  expect_RT_station(0) + expect_RT_station(1);

	// Expected system waiting time
	real_type expect_W_sys;
	expect_W_sys =  expect_W_station(0) + expect_W_station(1);

	// Expected system queue length
	real_type expect_Q_sys;
	expect_Q_sys =  expect_Q_station(0) + expect_Q_station(1);

	// Bottleneck stations
	::std::map<size_type,size_vector_type> expect_bottlenecks;
	expect_bottlenecks[0] = size_vector_type(1,1);
	expect_bottlenecks[1] = size_vector_type(1,1);

	queueing_network_type qn(lambda, S, V, m);

	DCS_DEBUG_TRACE("Processing Capacity: C=" << qn.processing_capacity());
	DCS_TEST_CHECK( qn.processing_capacity() < 1 );

	DCS_DEBUG_TRACE("Utilization Matrix: U=" << qn.utilizations());
	DCS_TEST_CHECK(
		ublasx::num_rows(qn.utilizations()) == ublasx::num_rows(expect_U)
		&& ublasx::num_columns(qn.utilizations()) == ublasx::num_columns(expect_U)
	);
	DCS_TEST_CHECK_MATRIX_CLOSE( qn.utilizations(), expect_U, nc, ns, tol );

	DCS_DEBUG_TRACE("Respone Time Matrix: R=" << qn.response_times());
	DCS_TEST_CHECK(
		ublasx::num_rows(qn.response_times()) == ublasx::num_rows(expect_R)
		&& ublasx::num_columns(qn.response_times()) == ublasx::num_columns(expect_R)
	);
	DCS_TEST_CHECK_MATRIX_CLOSE( qn.response_times(), expect_R, nc, ns, tol );

	DCS_DEBUG_TRACE("Customers Number Matrix: K=" << qn.customers_numbers());
	DCS_TEST_CHECK(
		ublasx::num_rows(qn.customers_numbers()) == ublasx::num_rows(expect_K)
		&& ublasx::num_columns(qn.customers_numbers()) == ublasx::num_columns(expect_K)
	);
	DCS_TEST_CHECK_MATRIX_CLOSE( qn.customers_numbers(), expect_K, nc, ns, tol );

	DCS_DEBUG_TRACE("Throughput Matrix: X=" << qn.throughputs());
	DCS_TEST_CHECK(
		ublasx::num_rows(qn.throughputs()) == ublasx::num_rows(expect_X)
		&& ublasx::num_columns(qn.throughputs()) == ublasx::num_columns(expect_X)
	);
	DCS_TEST_CHECK_MATRIX_CLOSE( qn.throughputs(), expect_X, nc, ns, tol );

	DCS_DEBUG_TRACE("Residence Time Matrix: RT=" << qn.residence_times());
	DCS_TEST_CHECK(
		ublasx::num_rows(qn.residence_times()) == ublasx::num_rows(expect_RT)
		&& ublasx::num_columns(qn.residence_times()) == ublasx::num_columns(expect_RT)
	);
	DCS_TEST_CHECK_MATRIX_CLOSE( qn.residence_times(), expect_RT, nc, ns, tol );

	DCS_DEBUG_TRACE("Waiting Time Matrix: W=" << qn.waiting_times());
	DCS_TEST_CHECK(
		ublasx::num_rows(qn.waiting_times()) == ublasx::num_rows(expect_W)
		&& ublasx::num_columns(qn.waiting_times()) == ublasx::num_columns(expect_W)
	);
	DCS_TEST_CHECK_MATRIX_CLOSE( qn.waiting_times(), expect_W, nc, ns, tol );

	DCS_DEBUG_TRACE("Queue Length Matrix: Q=" << qn.queue_lengths());
	DCS_TEST_CHECK(
		ublasx::num_rows(qn.queue_lengths()) == ublasx::num_rows(expect_Q)
		&& ublasx::num_columns(qn.queue_lengths()) == ublasx::num_columns(expect_Q)
	);
	DCS_TEST_CHECK_MATRIX_CLOSE( qn.queue_lengths(), expect_Q, nc, ns, tol );

//	XXX: Meaningless
//	DCS_DEBUG_TRACE("Per-class Utilization Vector: U_{class}=" << qn.class_response_times());
//	DCS_TEST_CHECK( ublasx::size(qn.class_utilizations()) == ublasx::size(expect_U_class) );
//	DCS_TEST_CHECK_VECTOR_CLOSE( qn.class_utilizations(), expect_U_class, nc, tol );

	DCS_DEBUG_TRACE("Per-station Utilization Vector: U_{station}=" << qn.station_utilizations());
	DCS_TEST_CHECK( ublasx::size(qn.station_utilizations()) == ublasx::size(expect_U_station) );
	DCS_TEST_CHECK_VECTOR_CLOSE( qn.station_utilizations(), expect_U_station, ns, tol );

	DCS_DEBUG_TRACE("Per-class Respone Time Vector: R_{class}=" << qn.class_response_times());
	DCS_TEST_CHECK( ublasx::size(qn.class_response_times()) == ublasx::size(expect_R_class) );
	DCS_TEST_CHECK_VECTOR_CLOSE( qn.class_response_times(), expect_R_class, nc, tol );

	DCS_DEBUG_TRACE("Per-station Respone Time Vector: R_{station}=" << qn.station_response_times());
	DCS_TEST_CHECK( ublasx::size(qn.station_response_times()) == ublasx::size(expect_R_station) );
	DCS_TEST_CHECK_VECTOR_CLOSE( qn.station_response_times(), expect_R_station, ns, tol );

	DCS_DEBUG_TRACE("Per-class Customers Number Vector: K_{class}=" << qn.class_customers_numbers());
	DCS_TEST_CHECK( ublasx::size(qn.class_customers_numbers()) == ublasx::size(expect_K_class) );
	DCS_TEST_CHECK_VECTOR_CLOSE( qn.class_customers_numbers(), expect_K_class, nc, tol );

	DCS_DEBUG_TRACE("Per-station Customers Number Vector: K_{station}=" << qn.station_customers_numbers());
	DCS_TEST_CHECK( ublasx::size(qn.station_customers_numbers()) == ublasx::size(expect_K_station) );
	DCS_TEST_CHECK_VECTOR_CLOSE( qn.station_customers_numbers(), expect_K_station, ns, tol );

	DCS_DEBUG_TRACE("Per-class Throughput Vector: X_{class}=" << qn.class_throughputs());
	DCS_TEST_CHECK( ublasx::size(qn.class_throughputs()) == ublasx::size(expect_X_class) );
	DCS_TEST_CHECK_VECTOR_CLOSE( qn.class_throughputs(), expect_X_class, nc, tol );

	DCS_DEBUG_TRACE("Per-station Throughput Vector: X_{station}=" << qn.station_throughputs());
	DCS_TEST_CHECK( ublasx::size(qn.station_throughputs()) == ublasx::size(expect_X_station) );
	DCS_TEST_CHECK_VECTOR_CLOSE( qn.station_throughputs(), expect_X_station, ns, tol );

	DCS_DEBUG_TRACE("Per-class Residence Time Vector: RT_{class}=" << qn.class_residence_times());
	DCS_TEST_CHECK( ublasx::size(qn.class_residence_times()) == ublasx::size(expect_RT_class) );
	DCS_TEST_CHECK_VECTOR_CLOSE( qn.class_residence_times(), expect_RT_class, nc, tol );

	DCS_DEBUG_TRACE("Per-station Residence Time Vector: RT_{station}=" << qn.station_residence_times());
	DCS_TEST_CHECK( ublasx::size(qn.station_residence_times()) == ublasx::size(expect_RT_station) );
	DCS_TEST_CHECK_VECTOR_CLOSE( qn.station_residence_times(), expect_RT_station, ns, tol );

	DCS_DEBUG_TRACE("Per-class Waiting Time Vector: W_{class}=" << qn.class_waiting_times());
	DCS_TEST_CHECK( ublasx::size(qn.class_waiting_times()) == ublasx::size(expect_W_class) );
	DCS_TEST_CHECK_VECTOR_CLOSE( qn.class_waiting_times(), expect_W_class, nc, tol );

	DCS_DEBUG_TRACE("Per-station Waiting Time Vector: W_{station}=" << qn.station_waiting_times());
	DCS_TEST_CHECK( ublasx::size(qn.station_waiting_times()) == ublasx::size(expect_W_station) );
	DCS_TEST_CHECK_VECTOR_CLOSE( qn.station_waiting_times(), expect_W_station, ns, tol );

	DCS_DEBUG_TRACE("Per-class Queue Length Vector: Q_{class}=" << qn.class_queue_lengths());
	DCS_TEST_CHECK( ublasx::size(qn.class_queue_lengths()) == ublasx::size(expect_Q_class) );
	DCS_TEST_CHECK_VECTOR_CLOSE( qn.class_queue_lengths(), expect_Q_class, nc, tol );

	DCS_DEBUG_TRACE("Per-station Queue Length Vector: Q_{station}=" << qn.station_queue_lengths());
	DCS_TEST_CHECK( ublasx::size(qn.station_queue_lengths()) == ublasx::size(expect_Q_station) );
	DCS_TEST_CHECK_VECTOR_CLOSE( qn.station_queue_lengths(), expect_Q_station, ns, tol );

//	XXX: Meaningless
//	DCS_DEBUG_TRACE("System Utilization: U_{sys}=" << qn.system_utilization());
//	DCS_TEST_CHECK_CLOSE( qn.system_response_time(), expect_R_sys, tol );

	DCS_DEBUG_TRACE("System Respone Time: R_{sys}=" << qn.system_response_time());
	DCS_TEST_CHECK_CLOSE( qn.system_response_time(), expect_R_sys, tol );

	DCS_DEBUG_TRACE("System Throughput: X_{sys}=" << qn.system_throughput());
	DCS_TEST_CHECK_CLOSE( qn.system_throughput(), expect_X_sys, tol );

	DCS_DEBUG_TRACE("System Customers Number: K_{sys}=" << qn.system_customers_number());
	DCS_TEST_CHECK_CLOSE( qn.system_customers_number(), expect_K_sys, tol );

	DCS_DEBUG_TRACE("System Residence Time: RT_{sys}=" << qn.system_residence_time());
	DCS_TEST_CHECK_CLOSE( qn.system_residence_time(), expect_RT_sys, tol );

	DCS_DEBUG_TRACE("System Waiting Time: W_{sys}=" << qn.system_waiting_time());
	DCS_TEST_CHECK_CLOSE( qn.system_waiting_time(), expect_W_sys, tol );

	DCS_DEBUG_TRACE("System Queue Length: Q_{sys}=" << qn.system_queue_length());
	DCS_TEST_CHECK_CLOSE( qn.system_queue_length(), expect_Q_sys, tol );

	DCS_DEBUG_TRACE("Bottleneck Stations for Class 0: b(0)=" << qn.bottleneck_stations(0));
	DCS_TEST_CHECK( ublasx::size(qn.bottleneck_stations(0)) == ublasx::size(expect_bottlenecks[0]) );
	DCS_TEST_CHECK_VECTOR_EQ( qn.bottleneck_stations(0), expect_bottlenecks[0], ublasx::size(expect_bottlenecks[0]) );
	DCS_DEBUG_TRACE("Bottleneck Stations for Class 1: b(1)=" << qn.bottleneck_stations(1));
	DCS_TEST_CHECK( ublasx::size(qn.bottleneck_stations(1)) == ublasx::size(expect_bottlenecks[1]) );
	DCS_TEST_CHECK_VECTOR_EQ( qn.bottleneck_stations(1), expect_bottlenecks[1], ublasx::size(expect_bottlenecks[1]) );
}


int main()
{
	DCS_TEST_SUITE( "Open BCMP Queueing Networks" );

	DCS_TEST_BEGIN();

	DCS_TEST_DO( test_multi_open_lazowska_p137 );

	DCS_TEST_END();
}
