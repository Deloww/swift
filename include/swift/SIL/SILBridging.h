//===--- SILBridging.h - header for the swift SILBridging module ----------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2021 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_SIL_SILBRIDGING_H
#define SWIFT_SIL_SILBRIDGING_H

#include "swift/Basic/BasicBridging.h"
#include "swift/Basic/BridgedSwiftObject.h"
#include <stdbool.h>
#include <stddef.h>
#include <string>

SWIFT_BEGIN_NULLABILITY_ANNOTATIONS

enum {
  BridgedOperandSize = 4 * sizeof(uintptr_t),
  BridgedSuccessorSize = 4 * sizeof(uintptr_t) + sizeof(uint64_t)
};

enum ChangeNotificationKind {
  instructionsChanged,
  callsChanged,
  branchesChanged
};

typedef struct {
  const void * _Nonnull opaqueCtxt;
} BridgedPassContext;

typedef struct {
  void * _Null_unspecified word0;
  void * _Null_unspecified word1;
  void * _Null_unspecified word2;
} BridgedLocation;

typedef struct {
  void * _Nullable typePtr;
} BridgedType;

typedef struct {
  const void * _Nullable data;
  size_t count;
} BridgedValueArray;

typedef struct {
  const void * _Nonnull op;
} BridgedOperand;

typedef struct {
  const void * _Nullable op;
} OptionalBridgedOperand;

typedef struct {
  const void * _Nonnull succ;
} BridgedSuccessor;

typedef struct {
  const void * _Nullable succ;
} OptionalBridgedSuccessor;

typedef struct {
  SwiftObject obj;
} BridgedFunction;

typedef struct {
  OptionalSwiftObject obj;
} OptionalBridgedFunction;

typedef struct {
  SwiftObject obj;
} BridgedGlobalVar;

typedef struct {
  SwiftObject obj;
} BridgedBasicBlock;

typedef struct {
  OptionalSwiftObject obj;
} OptionalBridgedBasicBlock;

typedef struct {
  SwiftObject obj;
} BridgedArgument;

typedef struct {
  OptionalSwiftObject obj;
} OptionalBridgedArgument;

typedef struct {
  SwiftObject obj;
} BridgedNode;

typedef struct {
  SwiftObject obj;
} BridgedValue;

typedef struct {
  OptionalSwiftObject obj;
} OptionalBridgedValue;

typedef struct {
  SwiftObject obj;
} BridgedInstruction;

typedef struct {
  OptionalSwiftObject obj;
} OptionalBridgedInstruction;

typedef struct {
  SwiftObject obj;
} BridgedMultiValueResult;

typedef struct {
  const unsigned char * _Nullable message;
  SwiftInt position;
} BridgedParsingError;

// Must be in sync with SILInstruction::MemoryBehavior
// TODO: do this less hacky.
typedef enum {
  NoneBehavior,
  MayReadBehavior,
  MayWriteBehavior,
  MayReadWriteBehavior,
  MayHaveSideEffectsBehavior
} BridgedMemoryBehavior;

// AST bridging

typedef struct {
  const void * _Nonnull op;
} BridgedSubstitutionMap;

typedef enum {
  UnknownBuiltin = 0,
#define BUILTIN(Id, Name, Attrs) Id##Builtin,
#include "swift/AST/Builtins.def"
} BridgedBuiltinID;

enum {
  EffectsFlagEscape = 0x1,
  EffectsFlagDerived = 0x2
};

void registerBridgedClass(BridgedStringRef className, SwiftMetatype metatype);

typedef void (* _Nonnull FunctionRegisterFn)(BridgedFunction f,
                                        void * _Nonnull data,
                                        SwiftInt size);
typedef void (* _Nonnull FunctionWriteFn)(BridgedFunction,
                                          BridgedOStream, SwiftInt);
typedef BridgedParsingError (* _Nonnull FunctionParseFn)(BridgedFunction,
                         BridgedStringRef, SwiftInt, SwiftInt, BridgedArrayRef);
typedef SwiftInt (* _Nonnull FunctionCopyEffectsFn)(BridgedFunction,
                                                    BridgedFunction);
typedef SwiftInt (* _Nonnull FunctionGetEffectFlagsFn)(BridgedFunction, SwiftInt);

void Function_register(SwiftMetatype metatype,
            FunctionRegisterFn initFn, FunctionRegisterFn destroyFn,
            FunctionWriteFn writeFn, FunctionParseFn parseFn,
            FunctionCopyEffectsFn copyEffectsFn,
            FunctionGetEffectFlagsFn hasEffectsFn);

SwiftInt PassContext_continueWithNextSubpassRun(BridgedPassContext passContext,
                                                OptionalBridgedInstruction inst);
void PassContext_notifyChanges(BridgedPassContext passContext,
                               enum ChangeNotificationKind changeKind);
void PassContext_eraseInstruction(BridgedPassContext passContext,
                                  BridgedInstruction inst);

BridgedStringRef SILFunction_getName(BridgedFunction function);
std::string SILFunction_debugDescription(BridgedFunction function);
OptionalBridgedBasicBlock SILFunction_firstBlock(BridgedFunction function);
OptionalBridgedBasicBlock SILFunction_lastBlock(BridgedFunction function);
SwiftInt SILFunction_numIndirectResultArguments(BridgedFunction function);
SwiftInt SILFunction_getSelfArgumentIndex(BridgedFunction function);
SwiftInt SILFunction_getNumSILArguments(BridgedFunction function);
BridgedType SILFunction_getSILArgumentType(BridgedFunction function, SwiftInt idx);
BridgedType SILFunction_getSILResultType(BridgedFunction function);
SwiftInt SILFunction_isSwift51RuntimeAvailable(BridgedFunction function);

BridgedStringRef SILGlobalVariable_getName(BridgedGlobalVar global);
std::string SILGlobalVariable_debugDescription(BridgedGlobalVar global);

OptionalBridgedBasicBlock SILBasicBlock_next(BridgedBasicBlock block);
OptionalBridgedBasicBlock SILBasicBlock_previous(BridgedBasicBlock block);
BridgedFunction SILBasicBlock_getFunction(BridgedBasicBlock block);
std::string SILBasicBlock_debugDescription(BridgedBasicBlock block);
OptionalBridgedInstruction SILBasicBlock_firstInst(BridgedBasicBlock block);
OptionalBridgedInstruction SILBasicBlock_lastInst(BridgedBasicBlock block);
SwiftInt SILBasicBlock_getNumArguments(BridgedBasicBlock block);
BridgedArgument SILBasicBlock_getArgument(BridgedBasicBlock block, SwiftInt index);
OptionalBridgedSuccessor SILBasicBlock_getFirstPred(BridgedBasicBlock block);
OptionalBridgedSuccessor SILSuccessor_getNext(BridgedSuccessor succ);
BridgedBasicBlock SILSuccessor_getTargetBlock(BridgedSuccessor succ);
BridgedInstruction SILSuccessor_getContainingInst(BridgedSuccessor succ);

BridgedValue Operand_getValue(BridgedOperand);
OptionalBridgedOperand Operand_nextUse(BridgedOperand);
BridgedInstruction Operand_getUser(BridgedOperand);
SwiftInt Operand_isTypeDependent(BridgedOperand);

std::string SILNode_debugDescription(BridgedNode node);
BridgedFunction SILNode_getFunction(BridgedNode node);
OptionalBridgedOperand SILValue_firstUse(BridgedValue value);
BridgedType SILValue_getType(BridgedValue value);

BridgedStringRef SILType_debugDescription(BridgedType);
SwiftInt SILType_isAddress(BridgedType);
SwiftInt SILType_isTrivial(BridgedType, BridgedFunction);
SwiftInt SILType_isReferenceCounted(BridgedType type, BridgedFunction);
SwiftInt SILType_isNonTrivialOrContainsRawPointer(BridgedType, BridgedFunction);
SwiftInt SILType_isNominal(BridgedType type);
SwiftInt SILType_isClass(BridgedType type);
SwiftInt SILType_isStruct(BridgedType type);
SwiftInt SILType_isTuple(BridgedType type);
SwiftInt SILType_isEnum(BridgedType type);
SwiftInt SILType_getNumTupleElements(BridgedType type);
BridgedType SILType_getTupleElementType(BridgedType type, SwiftInt elementIdx);
SwiftInt SILType_getNumNominalFields(BridgedType type);
BridgedType SILType_getNominalFieldType(BridgedType type, SwiftInt index,
                                        BridgedFunction function);
SwiftInt SILType_getFieldIdxOfNominalType(BridgedType type,
                                          BridgedStringRef fieldName);
BridgedSubstitutionMap SILType_getContextSubstitutionMap(BridgedType);

BridgedBasicBlock SILArgument_getParent(BridgedArgument argument);
SwiftInt SILArgument_isExclusiveIndirectParameter(BridgedArgument argument);

OptionalBridgedInstruction SILInstruction_next(BridgedInstruction inst);
OptionalBridgedInstruction SILInstruction_previous(BridgedInstruction inst);
BridgedBasicBlock SILInstruction_getParent(BridgedInstruction inst);
BridgedArrayRef SILInstruction_getOperands(BridgedInstruction inst);
void SILInstruction_setOperand(BridgedInstruction inst, SwiftInt index,
                               BridgedValue value);
BridgedLocation SILInstruction_getLocation(BridgedInstruction inst);
BridgedMemoryBehavior SILInstruction_getMemBehavior(BridgedInstruction inst);
bool SILInstruction_mayRelease(BridgedInstruction inst);

BridgedInstruction MultiValueInstResult_getParent(BridgedMultiValueResult result);
SwiftInt MultiValueInstResult_getIndex(BridgedMultiValueResult result);
SwiftInt MultipleValueInstruction_getNumResults(BridgedInstruction inst);
BridgedMultiValueResult
  MultipleValueInstruction_getResult(BridgedInstruction inst, SwiftInt index);

BridgedArrayRef TermInst_getSuccessors(BridgedInstruction term);

BridgedStringRef CondFailInst_getMessage(BridgedInstruction cfi);
BridgedBuiltinID BuiltinInst_getID(BridgedInstruction bi);
BridgedGlobalVar GlobalAccessInst_getGlobal(BridgedInstruction globalInst);
BridgedFunction FunctionRefInst_getReferencedFunction(BridgedInstruction fri);
BridgedStringRef StringLiteralInst_getValue(BridgedInstruction sli);
SwiftInt TupleExtractInst_fieldIndex(BridgedInstruction tei);
SwiftInt TupleElementAddrInst_fieldIndex(BridgedInstruction teai);
SwiftInt StructExtractInst_fieldIndex(BridgedInstruction sei);
OptionalBridgedValue StructInst_getUniqueNonTrivialFieldValue(BridgedInstruction si);
SwiftInt StructElementAddrInst_fieldIndex(BridgedInstruction seai);
SwiftInt ProjectBoxInst_fieldIndex(BridgedInstruction pbi);
SwiftInt EnumInst_caseIndex(BridgedInstruction ei);
SwiftInt UncheckedEnumDataInst_caseIndex(BridgedInstruction uedi);
SwiftInt InitEnumDataAddrInst_caseIndex(BridgedInstruction idea);
SwiftInt UncheckedTakeEnumDataAddrInst_caseIndex(BridgedInstruction utedi);
SwiftInt InjectEnumAddrInst_caseIndex(BridgedInstruction ieai);
SwiftInt RefElementAddrInst_fieldIndex(BridgedInstruction reai);
SwiftInt PartialApplyInst_numArguments(BridgedInstruction ai);
SwiftInt ApplyInst_numArguments(BridgedInstruction ai);
SwiftInt PartialApply_getCalleeArgIndexOfFirstAppliedArg(BridgedInstruction pai);
SwiftInt PartialApplyInst_isOnStack(BridgedInstruction pai);
SwiftInt AllocRefInstBase_isObjc(BridgedInstruction arb);
SwiftInt AllocRefInstBase_canAllocOnStack(BridgedInstruction arb);
SwiftInt BeginApplyInst_numArguments(BridgedInstruction ai);
SwiftInt TryApplyInst_numArguments(BridgedInstruction ai);
BridgedBasicBlock BranchInst_getTargetBlock(BridgedInstruction bi);
SwiftInt SwitchEnumInst_getNumCases(BridgedInstruction se);
SwiftInt SwitchEnumInst_getCaseIndex(BridgedInstruction se, SwiftInt idx);
SwiftInt StoreInst_getStoreOwnership(BridgedInstruction store);
SwiftInt CopyAddrInst_isTakeOfSrc(BridgedInstruction copyAddr);
SwiftInt CopyAddrInst_isInitializationOfDest(BridgedInstruction copyAddr);
void RefCountingInst_setIsAtomic(BridgedInstruction rc, bool isAtomic);
bool RefCountingInst_getIsAtomic(BridgedInstruction rc);
SwiftInt CondBranchInst_getNumTrueArgs(BridgedInstruction cbr);

BridgedInstruction SILBuilder_createBuiltinBinaryFunction(
          BridgedInstruction insertionPoint,
          BridgedLocation loc, BridgedStringRef name,
          BridgedType operandType, BridgedType resultType, BridgedValueArray arguments);
BridgedInstruction SILBuilder_createCondFail(BridgedInstruction insertionPoint,
          BridgedLocation loc, BridgedValue condition, BridgedStringRef message);
BridgedInstruction SILBuilder_createIntegerLiteral(BridgedInstruction insertionPoint,
          BridgedLocation loc, BridgedType type, SwiftInt value);
BridgedInstruction SILBuilder_createDeallocStackRef(BridgedInstruction insertionPoint,
          BridgedLocation loc, BridgedValue operand);
BridgedInstruction SILBuilder_createUncheckedRefCast(BridgedInstruction insertionPoint,
                                                     BridgedLocation loc,
                                                     BridgedValue op,
                                                     BridgedType type);
BridgedInstruction
SILBuilder_createSetDeallocating(BridgedInstruction insertionPoint,
                                 BridgedLocation loc, BridgedValue op,
                                 bool isAtomic);
BridgedInstruction
SILBuilder_createFunctionRef(BridgedInstruction insertionPoint,
                             BridgedLocation loc, BridgedFunction function);
BridgedInstruction SILBuilder_createApply(BridgedInstruction insertionPoint,
                                          BridgedLocation loc,
                                          BridgedValue function,
                                          BridgedSubstitutionMap subMap,
                                          BridgedValueArray arguments);

SWIFT_END_NULLABILITY_ANNOTATIONS

#endif
