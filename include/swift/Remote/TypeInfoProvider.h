//===--- TypeInfoProvider.h - Abstract access to type info ------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2020 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
//  This file declares an abstract interface for reading type layout info.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_REMOTE_TYPEINFOPROVIDER_H
#define SWIFT_REMOTE_TYPEINFOPROVIDER_H

#include <vector>
#include <string>

namespace llvm {
template <typename T>
class Optional;
class StringRef;
}
namespace swift {
namespace reflection {
class TypeInfo;
struct ReflectionInfo;
class FieldDescriptor;
}
namespace remote {
template <typename T>
class RemoteRef;

/// An abstract interface for providing external type layout information.
struct TypeInfoProvider {
  virtual ~TypeInfoProvider() = default;

  /// Attempt to read type information about (Clang)imported types that are not
  /// represented in the metadata. LLDB can read this information from debug
  /// info, for example.
  virtual const reflection::TypeInfo *
  getTypeInfo(llvm::StringRef mangledName) = 0;

  virtual void registerFieldDescriptors(uint64_t InfoID,
      const reflection::ReflectionInfo &Info,
      const std::vector<std::string> &Names) = 0;

  virtual llvm::Optional<std::pair<uint64_t, uint64_t>>
  getFieldDescriptor(const std::string &Name) = 0;
};

} // namespace remote
} // namespace swift
#endif
