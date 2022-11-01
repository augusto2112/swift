// RUN: %empty-directory(%t)
// RUN: %target-build-swift -g -lswiftSwiftReflectionTest %s -o %t/reflect_Optional_Bool
// RUN: %target-codesign %t/reflect_Optional_Bool

// RUN: %target-run %target-swift-reflection-test %t/reflect_Optional_Bool | %FileCheck %s 


// REQUIRES: reflection_test_support
// REQUIRES: executable_test
// UNSUPPORTED: use_os_stdlib

import SwiftReflectionTest

let optFalse: Bool? = false
reflect(enum: optFalse)

// CHECK-64: Reflecting an enum.
// CHECK-64: Instance pointer in child address space: 0x{{[0-9a-fA-F]+}}

// CHECK-64: Type reference:
// CHECK-64: (bound_generic_enum Swift.Optional
// CHECK-64:   (struct Swift.Bool))

// CHECK-64: Type info:
// CHECK-64: (single_payload_enum size=1 alignment=1 stride=1 num_extra_inhabitants=253 bitwise_takable=1
// CHECK-64:   (case name=some index=0 offset=0
// CHECK-64:     (struct size=1 alignment=1 stride=1 num_extra_inhabitants=254 bitwise_takable=1
// CHECK-64:       (field name=_value offset=0
// CHECK-64:         (builtin size=1 alignment=1 stride=1 num_extra_inhabitants=254 bitwise_takable=1))))
// CHECK-64:   (case name=none index=1))

let optTrue: Bool? = true
reflect(enum: optTrue)

let optNil: Bool? = .none
reflect(enum: optNil)

  
let optOptFalse: Bool?? = false
reflect(enum: optOptFalse)

let optOptTrue: Bool?? = true
reflect(enum: optOptTrue)

let optOptSomeNil: Bool?? = .some(.none)
reflect(enum: optOptSomeNil)

let optOptNil: Bool?? = .some(.none)
reflect(enum: optOptNil)

doneReflecting()

// CHECK-64: Done.

