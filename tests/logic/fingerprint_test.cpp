// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <vector>

#include "pages/fingerprint/fingers.h"

using namespace settings;
using cutefish::fprint::Finger;

TEST(Fingerprint, NewFingerprintsTakeTheIndexFingersFirst)
{
    std::vector<Finger> enrolled;
    EXPECT_EQ(freeFinger(enrolled), Finger::RightIndex);
    enrolled.push_back(Finger::RightIndex);
    EXPECT_EQ(freeFinger(enrolled), Finger::LeftIndex);
    enrolled.push_back(Finger::LeftIndex);
    EXPECT_EQ(freeFinger(enrolled), Finger::RightMiddle);
}

TEST(Fingerprint, AFreedFingerIsTakenAgain)
{
    const std::vector<Finger> enrolled = {Finger::RightIndex, Finger::RightMiddle};
    EXPECT_EQ(freeFinger(enrolled), Finger::LeftIndex);
}

TEST(Fingerprint, NoFingerIsFreeOnceAllAreStored)
{
    std::vector<Finger> enrolled;
    for (int i = 0; i < cutefish::fprint::kFingerCount; ++i)
        enrolled.push_back(cutefish::fprint::Finger(i));
    EXPECT_FALSE(freeFinger(enrolled));
}

TEST(Fingerprint, TheListFollowsTheOrderOfTheFingers)
{
    const std::vector<Finger> enrolled = {Finger::LeftThumb, Finger::LeftIndex, Finger::RightIndex};
    EXPECT_EQ(inFingerOrder(enrolled), (std::vector<Finger>{Finger::RightIndex, Finger::LeftIndex, Finger::LeftThumb}));
}

TEST(Fingerprint, TheGripIsAskedForInTheLastThird)
{
    EXPECT_EQ(enrollmentStep(0, 21), EnrollmentStep::Place);
    EXPECT_EQ(enrollmentStep(13, 21), EnrollmentStep::Place);
    EXPECT_EQ(enrollmentStep(14, 21), EnrollmentStep::AdjustGrip);
    EXPECT_EQ(enrollmentStep(20, 21), EnrollmentStep::AdjustGrip);
    EXPECT_EQ(enrollmentStep(2, 5), EnrollmentStep::Place);
    EXPECT_EQ(enrollmentStep(3, 5), EnrollmentStep::AdjustGrip);
}

TEST(Fingerprint, ReadersThatWantFewScansAskForNoGrip)
{
    EXPECT_EQ(enrollmentStep(2, 3), EnrollmentStep::Place);
    EXPECT_EQ(enrollmentStep(1, 1), EnrollmentStep::Place);
}
