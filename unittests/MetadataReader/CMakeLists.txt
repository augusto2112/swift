add_swift_unittest(SwiftMetadataReaderTests
  MetadataReaderTest.cpp
  ObjectFileContextTest.cpp
  RemoteInspectionEqualsTest.cpp
)

target_link_libraries(SwiftMetadataReaderTests
  PRIVATE
  swiftBasic
  swiftDemangling
  swiftStaticMirror
  swiftRemoteInspection
  LLVMObject
  LLVMObjectYAML
)
