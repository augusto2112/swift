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

#include "swift/RemoteInspection/TypeLowering.h"
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

TEST(TypeInfoComparisonParse, PresetsAndTokens) {
  EXPECT_EQ(parseTypeInfoComparison(""), TypeInfoComparison::None);
  EXPECT_EQ(parseTypeInfoComparison("off"), TypeInfoComparison::None);
  EXPECT_EQ(parseTypeInfoComparison("layout"), TypeInfoComparison::Layout);
  EXPECT_EQ(parseTypeInfoComparison("names"), TypeInfoComparison::Names);
  EXPECT_EQ(parseTypeInfoComparison("strict"), TypeInfoComparison::Strict);

  // Explicit token list ORs the dimensions; unknown tokens are ignored.
  auto f = parseTypeInfoComparison("size, alignment , bogus");
  EXPECT_TRUE(contains(f, TypeInfoComparison::Size));
  EXPECT_TRUE(contains(f, TypeInfoComparison::Alignment));
  EXPECT_FALSE(contains(f, TypeInfoComparison::Stride));
}

TEST(TypeInfoEquals, BaseScalarsAreFlagGated) {
  using B = BitwiseBorrowability;
  // Same kind, different size.
  TypeInfo a(TypeInfoKind::Builtin, /*Size*/8, /*Align*/8, /*Stride*/8,
             /*NumXI*/0, B::TakableAndBorrowable, /*AFD*/false);
  TypeInfo b(TypeInfoKind::Builtin, /*Size*/16, /*Align*/8, /*Stride*/16,
             /*NumXI*/0, B::TakableAndBorrowable, /*AFD*/false);

  // With no dimensions selected, only Kind is compared -> equal.
  EXPECT_TRUE(a.Equals(b, TypeInfoComparison::None));
  // Selecting Size surfaces the difference.
  EXPECT_FALSE(a.Equals(b, TypeInfoComparison::Size));
  // Different kind is always unequal, even at None.
  TypeInfo c(TypeInfoKind::Reference, 8, 8, 8, 0, B::TakableAndBorrowable, false);
  EXPECT_FALSE(a.Equals(c, TypeInfoComparison::None));
  // Identical -> equal at Strict.
  TypeInfo a2(TypeInfoKind::Builtin, 8, 8, 8, 0, B::TakableAndBorrowable, false);
  EXPECT_TRUE(a.Equals(a2, TypeInfoComparison::Strict));
}

namespace {
// EnumTypeInfo is abstract (projectEnumValue is pure virtual); a trivial
// concrete subclass lets us exercise EnumTypeInfo::Equals directly.
class TestEnumTypeInfo : public EnumTypeInfo {
public:
  TestEnumTypeInfo(unsigned Size, unsigned Align, unsigned Stride,
                   unsigned NumXI, BitwiseBorrowability B, bool AFD,
                   EnumKind K, const std::vector<FieldInfo> &Cases)
      : EnumTypeInfo(Size, Align, Stride, NumXI, B, AFD, K, Cases) {}
  bool projectEnumValue(remote::MemoryReader &, remote::RemoteAddress,
                        int *) const override {
    return false;
  }
};
} // end anonymous namespace

TEST(TypeInfoEquals, RecordFieldsRecurseAndGate) {
  using B = BitwiseBorrowability;
  BuiltinTypeInfo i8(8, 8, 8, 0, B::TakableAndBorrowable, false);
  BuiltinTypeInfo i8b(8, 8, 8, 0, B::TakableAndBorrowable, false);

  // Same shape, field names differ only.
  std::vector<FieldInfo> fa{FieldInfo("x", /*Offset*/0, /*Value*/-1,
                                      /*TR*/nullptr, i8)};
  std::vector<FieldInfo> fb{FieldInfo("y", 0, -1, nullptr, i8b)};
  RecordTypeInfo ra(16, 8, 16, 0, B::TakableAndBorrowable, false,
                    RecordKind::Struct, fa);
  RecordTypeInfo rb(16, 8, 16, 0, B::TakableAndBorrowable, false,
                    RecordKind::Struct, fb);

  EXPECT_TRUE(ra.Equals(rb, TypeInfoComparison::Layout));  // names not compared.
  EXPECT_FALSE(ra.Equals(rb, TypeInfoComparison::Names));  // names now compared.

  // Field count mismatch is always unequal (structural invariant).
  std::vector<FieldInfo> fc{FieldInfo("x", 0, -1, nullptr, i8),
                            FieldInfo("z", 8, -1, nullptr, i8)};
  RecordTypeInfo rc(16, 8, 16, 0, B::TakableAndBorrowable, false,
                    RecordKind::Struct, fc);
  EXPECT_FALSE(ra.Equals(rc, TypeInfoComparison::None));

  // Different record sub-kind is always unequal.
  RecordTypeInfo rt(16, 8, 16, 0, B::TakableAndBorrowable, false,
                    RecordKind::Tuple, fa);
  EXPECT_FALSE(ra.Equals(rt, TypeInfoComparison::None));
}

TEST(TypeInfoEquals, EnumKindAndReference) {
  using B = BitwiseBorrowability;
  std::vector<FieldInfo> none;
  TestEnumTypeInfo e1(1, 1, 1, 0, B::TakableAndBorrowable, false,
                      EnumKind::NoPayloadEnum, none);
  TestEnumTypeInfo e2(1, 1, 1, 0, B::TakableAndBorrowable, false,
                      EnumKind::NoPayloadEnum, none);
  TestEnumTypeInfo e3(1, 1, 1, 0, B::TakableAndBorrowable, false,
                      EnumKind::SinglePayloadEnum, none);
  EXPECT_TRUE(e1.Equals(e2, TypeInfoComparison::Strict));
  EXPECT_FALSE(e1.Equals(e3, TypeInfoComparison::None)); // EnumKind always compared.

  ReferenceTypeInfo r1(8, 8, 8, 1, B::TakableAndBorrowable,
                       ReferenceKind::Strong, ReferenceCounting::Native);
  ReferenceTypeInfo r2(8, 8, 8, 1, B::TakableAndBorrowable,
                       ReferenceKind::Strong, ReferenceCounting::Unknown);
  EXPECT_FALSE(r1.Equals(r2, TypeInfoComparison::None)); // refcounting always compared.
}
