//===--- StructMetadataVisitor.h - CRTP for struct metadata ------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
// A CRTP class useful for laying out struct metadata.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_IRGEN_STRUCTMETADATALAYOUT_H
#define SWIFT_IRGEN_STRUCTMETADATALAYOUT_H

#include "NominalMetadataVisitor.h"

namespace swift {
namespace irgen {

/// A CRTP class for laying out struct metadata.
///
/// This produces an object corresponding to the StructMetadata type.
/// It does not itself doing anything special for metadata templates.
template <class Impl> class StructMetadataVisitor
       : public NominalMetadataVisitor<Impl> {
  using super = NominalMetadataVisitor<Impl>;

protected:
  using super::IGM;
  using super::asImpl;

  /// The struct.
  StructDecl *const Target;

  StructMetadataVisitor(IRGenModule &IGM, StructDecl *target)
    : super(IGM), Target(target) {}

public:
  void layout() {
    // Metadata header.
    super::layout();

    // StructMetadata header.
    asImpl().addNominalTypeDescriptor();

    // If changing this layout, you must update the magic number in
    // emitParentMetadataRef.

    // Instantiation-specific.
    asImpl().addGenericFields(Target, Target->getDeclaredTypeInContext());

    // Struct field offsets.
    asImpl().noteStartOfFieldOffsets();
    for (VarDecl *prop : Target->getStoredProperties())
      asImpl().addFieldOffset(prop);
  }
  
  // Note the start of the field offset vector.
  void noteStartOfFieldOffsets() {}
};

/// An "implementation" of StructMetadataVisitor that just scans through
/// the metadata layout, maintaining the offset of the next field.
template <class Impl>
class StructMetadataScanner : public StructMetadataVisitor<Impl> {
  typedef StructMetadataVisitor<Impl> super;
protected:
  Size NextOffset = Size(0);

  StructMetadataScanner(IRGenModule &IGM, StructDecl *target)
    : super(IGM, target) {}

public:
  void addMetadataFlags() { addPointer(); }
  void addValueWitnessTable() { addPointer(); }
  void addNominalTypeDescriptor() { addPointer(); }
  void addFieldOffset(VarDecl*) { addPointer(); }
  void addGenericArgument(CanType argument) { addPointer(); }
  void addGenericWitnessTable(CanType argument, ProtocolConformanceRef conf) {
    addPointer();
  }

private:
  void addPointer() {
    NextOffset += super::IGM.getPointerSize();
  }
};

} // end namespace irgen
} // end namespace swift

#endif
