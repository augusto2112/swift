//===--- RemoteInspectionEqualsTest.cpp -----------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2026 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#include "swift/RemoteInspection/TypeRef.h"
#include "swift/RemoteInspection/TypeRefBuilder.h"
#include "gtest/gtest.h"

using namespace swift;
using namespace swift::reflection;

// Two independent builders model the two reflection contexts the harness
// compares: structurally equal TypeRefs have distinct pointers across builders.
TEST(TypeRefEquals, BuiltinAcrossBuilders) {
  TypeRefBuilder B1(TypeRefBuilder::ForTesting);
  TypeRefBuilder B2(TypeRefBuilder::ForTesting);

  auto *si1 = B1.createBuiltinType("Si", "Si");
  auto *si2 = B2.createBuiltinType("Si", "Si");
  auto *sd1 = B1.createBuiltinType("Sd", "Sd");

  EXPECT_NE(si1, si2);            // distinct pointers (distinct builders).
  EXPECT_TRUE(si1->Equals(si2));  // but structurally equal.
  EXPECT_FALSE(si1->Equals(sd1)); // different mangled name.
  EXPECT_FALSE(si1->Equals(nullptr));
  EXPECT_TRUE(si1->Equals(si1));  // self.
}

TEST(TypeRefEquals, TupleRecursesAndComparesLabels) {
  TypeRefBuilder B1(TypeRefBuilder::ForTesting);
  TypeRefBuilder B2(TypeRefBuilder::ForTesting);

  auto *si1 = B1.createBuiltinType("Si", "Si");
  auto *sd1 = B1.createBuiltinType("Sd", "Sd");
  auto *si2 = B2.createBuiltinType("Si", "Si");
  auto *sd2 = B2.createBuiltinType("Sd", "Sd");

  auto *t1 = B1.createTupleType({si1, sd1}, {});
  auto *t2 = B2.createTupleType({si2, sd2}, {});
  auto *t3 = B2.createTupleType({sd2, si2}, {}); // element order differs.

  EXPECT_TRUE(t1->Equals(t2));
  EXPECT_FALSE(t1->Equals(t3));
  EXPECT_FALSE(t1->Equals(si1)); // different kind.
}

// Two function TypeRefs that are structurally identical except for
// differentiability kind must not compare equal, since DifferentiabilityKind
// is part of a function type's identity (it is hashed in
// FunctionTypeRef::Profile() alongside Flags/ExtFlags).
TEST(TypeRefEquals, FunctionComparesDifferentiability) {
  TypeRefBuilder B1(TypeRefBuilder::ForTesting);
  TypeRefBuilder B2(TypeRefBuilder::ForTesting);

  auto *result1 = B1.createBuiltinType("Si", "Si");
  auto *result2 = B2.createBuiltinType("Si", "Si");

  auto *fnNonDiff1 = B1.createFunctionType(
      {}, result1, FunctionTypeFlags(), ExtendedFunctionTypeFlags(),
      FunctionMetadataDifferentiabilityKind::NonDifferentiable,
      /*globalActor*/ nullptr, /*thrownError*/ nullptr);
  auto *fnNonDiff2 = B2.createFunctionType(
      {}, result2, FunctionTypeFlags(), ExtendedFunctionTypeFlags(),
      FunctionMetadataDifferentiabilityKind::NonDifferentiable,
      /*globalActor*/ nullptr, /*thrownError*/ nullptr);
  auto *fnReverse2 = B2.createFunctionType(
      {}, result2, FunctionTypeFlags(), ExtendedFunctionTypeFlags(),
      FunctionMetadataDifferentiabilityKind::Reverse,
      /*globalActor*/ nullptr, /*thrownError*/ nullptr);

  // Identical functions (same diffKind) across builders compare equal.
  EXPECT_TRUE(fnNonDiff1->Equals(fnNonDiff2));

  // Functions differing only in diffKind must not compare equal.
  EXPECT_FALSE(fnNonDiff1->Equals(fnReverse2));
}
