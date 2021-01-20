#include "gtest/gtest.h"
#include "sim_vector.h"
#include "test_util.test.h"

static float complex_distance(std::complex<float> a, std::complex<float> b) {
    auto d = a - b;
    return sqrtf(d.real()*d.real() + d.imag()*d.imag());
}

#define ASSERT_NEAR_C(a, b) ASSERT_LE(complex_distance(a, b), 1e-4)

TEST(vector_sim, qubit_order) {
    SimVector sim(2);
    sim.apply("H", 0);
    sim.apply("CNOT", 0, 1);
    ASSERT_NEAR_C(sim.state[0], sqrtf(0.5));
    ASSERT_NEAR_C(sim.state[1], 0);
    ASSERT_NEAR_C(sim.state[2], 0);
    ASSERT_NEAR_C(sim.state[3], sqrtf(0.5));
}

TEST(vector_sim, h_squared) {
    SimVector sim(1);
    sim.apply("H", 0);
    sim.apply("H", 0);
    ASSERT_NEAR_C(sim.state[0], 1);
    ASSERT_NEAR_C(sim.state[1], 0);
}

TEST(vector_sim, sqrt_x_squared) {
    SimVector sim(1);
    sim.apply("SQRT_X_DAG", 0);
    sim.apply("SQRT_X_DAG", 0);
    ASSERT_NEAR_C(sim.state[0], 0);
    ASSERT_NEAR_C(sim.state[1], 1);
}

TEST(vector_sim, state_channel_duality_cnot) {
    SimVector sim(4);
    sim.apply("H", 0);
    sim.apply("H", 1);
    sim.apply("CNOT", 0, 2);
    sim.apply("CNOT", 1, 3);
    sim.apply("CNOT", 2, 3);
    for (size_t row = 0; row < 4; row++) {
        for (size_t col = 0; col < 4; col++) {
            ASSERT_NEAR_C(sim.state[row*4 + col], GATE_UNITARIES.at("CNOT")[row][col]*0.5f);
        }
    }
}

TEST(vector_sim, state_channel_duality_y) {
    SimVector sim(2);
    sim.apply("H", 0);
    sim.apply("CNOT", 0, 1);
    sim.apply("Y", 1);
    for (size_t row = 0; row < 2; row++) {
        for (size_t col = 0; col < 2; col++) {
            ASSERT_NEAR_C(sim.state[row*2 + col], GATE_UNITARIES.at("Y")[row][col]*sqrtf(0.5f));
        }
    }
}

TEST(vector_sim, apply_pauli) {
    SimVector sim(2);

    sim.apply(PauliStringVal::from_str("+II").ref(), 0);
    ASSERT_NEAR_C(sim.state[0], 1);
    ASSERT_NEAR_C(sim.state[1], 0);
    ASSERT_NEAR_C(sim.state[2], 0);
    ASSERT_NEAR_C(sim.state[3], 0);

    sim.apply(PauliStringVal::from_str("-II").ref(), 0);
    ASSERT_NEAR_C(sim.state[0], -1);
    ASSERT_NEAR_C(sim.state[1], 0);
    ASSERT_NEAR_C(sim.state[2], 0);
    ASSERT_NEAR_C(sim.state[3], 0);

    sim.apply(PauliStringVal::from_str("+XI").ref(), 0);
    ASSERT_NEAR_C(sim.state[0], 0);
    ASSERT_NEAR_C(sim.state[1], -1);
    ASSERT_NEAR_C(sim.state[2], 0);
    ASSERT_NEAR_C(sim.state[3], 0);

    sim.apply(PauliStringVal::from_str("+IZ").ref(), 0);
    ASSERT_NEAR_C(sim.state[0], 0);
    ASSERT_NEAR_C(sim.state[1], -1);
    ASSERT_NEAR_C(sim.state[2], 0);
    ASSERT_NEAR_C(sim.state[3], 0);

    sim.apply(PauliStringVal::from_str("+ZI").ref(), 0);
    ASSERT_NEAR_C(sim.state[0], 0);
    ASSERT_NEAR_C(sim.state[1], 1);
    ASSERT_NEAR_C(sim.state[2], 0);
    ASSERT_NEAR_C(sim.state[3], 0);

    sim.apply(PauliStringVal::from_str("+IY").ref(), 0);
    ASSERT_NEAR_C(sim.state[0], 0);
    ASSERT_NEAR_C(sim.state[1], 0);
    ASSERT_NEAR_C(sim.state[2], 0);
    ASSERT_NEAR_C(sim.state[3], std::complex<float>(0, 1));

    sim.apply(PauliStringVal::from_str("+XX").ref(), 0);
    ASSERT_NEAR_C(sim.state[0], std::complex<float>(0, 1));
    ASSERT_NEAR_C(sim.state[1], 0);
    ASSERT_NEAR_C(sim.state[2], 0);
    ASSERT_NEAR_C(sim.state[3], 0);

    sim.apply(PauliStringVal::from_str("+X").ref(), 1);
    ASSERT_NEAR_C(sim.state[0], 0);
    ASSERT_NEAR_C(sim.state[1], 0);
    ASSERT_NEAR_C(sim.state[2], std::complex<float>(0, 1));
    ASSERT_NEAR_C(sim.state[3], 0);
}

TEST(vector_sim, approximate_equals) {
    SimVector s1(2);
    SimVector s2(2);
    ASSERT_TRUE(s1.approximate_equals(s2));
    ASSERT_TRUE(s1.approximate_equals(s2, false));
    ASSERT_TRUE(s1.approximate_equals(s2, true));
    s1.state[0] *= -1;
    ASSERT_FALSE(s1.approximate_equals(s2));
    ASSERT_FALSE(s1.approximate_equals(s2, false));
    ASSERT_TRUE(s1.approximate_equals(s2, true));
    s1.state[0] *= std::complex<float>(0, 1);
    ASSERT_FALSE(s1.approximate_equals(s2));
    ASSERT_FALSE(s2.approximate_equals(s1));
    ASSERT_FALSE(s1.approximate_equals(s2, false));
    ASSERT_TRUE(s1.approximate_equals(s2, true));
    ASSERT_FALSE(s2.approximate_equals(s1, false));
    ASSERT_TRUE(s2.approximate_equals(s1, true));
    s1.state[0] = 0;
    s1.state[1] = 1;
    ASSERT_FALSE(s1.approximate_equals(s2));
    ASSERT_FALSE(s1.approximate_equals(s2, false));
    ASSERT_FALSE(s1.approximate_equals(s2, true));
    s2.state[0] = 0;
    s2.state[1] = 1;
    ASSERT_TRUE(s1.approximate_equals(s2));
    s1.state[0] = sqrtf(0.5);
    s1.state[1] = sqrtf(0.5);
    s2.state[0] = sqrtf(0.5);
    s2.state[1] = sqrtf(0.5);
    ASSERT_TRUE(s1.approximate_equals(s2));
    s1.state[0] *= -1;
    ASSERT_FALSE(s1.approximate_equals(s2));
}

TEST(vector_sim, project) {
    SimVector sim(2);
    SimVector ref(2);

    sim.state = {0.5, 0.5, 0.5, 0.5};
    ASSERT_NEAR_C(sim.project(PauliStringVal::from_str("ZI")), 0.5);
    ref.state = {sqrtf(0.5), 0, sqrtf(0.5), 0};
    ASSERT_TRUE(sim.approximate_equals(ref));
    ASSERT_NEAR_C(sim.project(PauliStringVal::from_str("ZI")), 1);
    ASSERT_TRUE(sim.approximate_equals(ref));

    sim.state = {0.5, 0.5, 0.5, 0.5};
    sim.project(PauliStringVal::from_str("-ZI"));
    ref.state = {0, sqrtf(0.5), 0, sqrtf(0.5)};
    ASSERT_TRUE(sim.approximate_equals(ref));

    sim.state = {0.5, 0.5, 0.5, 0.5};
    sim.project(PauliStringVal::from_str("IZ"));
    ref.state = {sqrtf(0.5), sqrtf(0.5), 0, 0};
    ASSERT_TRUE(sim.approximate_equals(ref));

    sim.state = {0.5, 0.5, 0.5, 0.5};
    sim.project(PauliStringVal::from_str("-IZ"));
    ref.state = {0, 0, sqrtf(0.5), sqrtf(0.5)};
    ASSERT_TRUE(sim.approximate_equals(ref));

    sim.state = {0.5, 0.5, 0.5, 0.5};
    sim.project(PauliStringVal::from_str("ZZ"));
    ref.state = {sqrtf(0.5), 0, 0, sqrtf(0.5)};
    ASSERT_TRUE(sim.approximate_equals(ref));

    sim.state = {0.5, 0.5, 0.5, 0.5};
    sim.project(PauliStringVal::from_str("-ZZ"));
    ref.state = {0, sqrtf(0.5), sqrtf(0.5), 0};
    ASSERT_TRUE(sim.approximate_equals(ref));

    sim.project(PauliStringVal::from_str("ZI"));
    sim.state = {1, 0, 0, 0};
    sim.project(PauliStringVal::from_str("ZZ"));
    ref.state = {1, 0, 0, 0};
    ASSERT_TRUE(sim.approximate_equals(ref));

    sim.project(PauliStringVal::from_str("XX"));
    ref.state = {sqrtf(0.5f), 0, 0, sqrtf(0.5f)};
    ASSERT_TRUE(sim.approximate_equals(ref));

    sim.project(PauliStringVal::from_str("-YZ"));
    ref.state = {0.5, {0, -0.5}, {0, -0.5}, 0.5};
    ASSERT_TRUE(sim.approximate_equals(ref));

    sim.project(PauliStringVal::from_str("-ZI"));
    ref.state = {0, {0, -sqrtf(0.5)}, 0, sqrtf(0.5)};
    ASSERT_TRUE(sim.approximate_equals(ref));
}

TEST(VectorSim, from_stabilizers) {
    SimVector ref(2);
    auto sim = SimVector::from_stabilizers(
            {PauliStringVal::from_str("ZI"), PauliStringVal::from_str("IZ")},
            SHARED_TEST_RNG());
    ref.state = {1, 0, 0, 0};
    ASSERT_TRUE(sim.approximate_equals(ref, true));

    sim = SimVector::from_stabilizers(
            {PauliStringVal::from_str("-YX"), PauliStringVal::from_str("ZZ")},
            SHARED_TEST_RNG());
    ref.state = {sqrtf(0.5), 0, 0, {0, -sqrtf(0.5)}};
    ASSERT_TRUE(sim.approximate_equals(ref, true));

    sim = SimVector::from_stabilizers(
            {PauliStringVal::from_str("ZI"), PauliStringVal::from_str("ZZ")},
            SHARED_TEST_RNG());
    ref.state = {1, 0, 0, 0};
    ASSERT_TRUE(sim.approximate_equals(ref, true));

    sim = SimVector::from_stabilizers(
            {PauliStringVal::from_str("ZI"), PauliStringVal::from_str("-ZZ")},
            SHARED_TEST_RNG());
    ref.state = {0, 0, 1, 0};
    ASSERT_TRUE(sim.approximate_equals(ref, true));

    sim = SimVector::from_stabilizers(
            {PauliStringVal::from_str("ZI"), PauliStringVal::from_str("IX")},
            SHARED_TEST_RNG());
    ref.state = {sqrtf(0.5), 0, sqrtf(0.5), 0};
    ASSERT_TRUE(sim.approximate_equals(ref, true));

    sim = SimVector::from_stabilizers(
            {PauliStringVal::from_str("ZZ"), PauliStringVal::from_str("XX")},
            SHARED_TEST_RNG());
    ref.state = {sqrtf(0.5), 0, 0, sqrtf(0.5)};
    ASSERT_TRUE(sim.approximate_equals(ref, true));

    sim = SimVector::from_stabilizers(
            {PauliStringVal::from_str("XXX"), PauliStringVal::from_str("ZZI"), PauliStringVal::from_str("IZZ")},
            SHARED_TEST_RNG());
    ref.state = {sqrtf(0.5), 0, 0, 0, 0, 0, 0, sqrtf(0.5)};
    ASSERT_TRUE(sim.approximate_equals(ref, true));

    sim = SimVector::from_stabilizers(
            {PauliStringVal::from_str("YYY"), PauliStringVal::from_str("ZZI"), PauliStringVal::from_str("IZZ")},
            SHARED_TEST_RNG());
    ref.state = {sqrtf(0.5), 0, 0, 0, 0, 0, 0, {0, -sqrtf(0.5)}};
    ASSERT_TRUE(sim.approximate_equals(ref, true));

    sim = SimVector::from_stabilizers(
            {PauliStringVal::from_str("-YYY"), PauliStringVal::from_str("-ZZI"), PauliStringVal::from_str("IZZ")},
            SHARED_TEST_RNG());
    ref.state = {0, sqrtf(0.5), 0, 0, 0, 0, {0, -sqrtf(0.5)}, 0};
    ASSERT_TRUE(sim.approximate_equals(ref, true));
}
