/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003, 2007, 2008, 2009, 2012 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef JSArray_h
#define JSArray_h

#include "ArrayConventions.h"
#include "ButterflyInlines.h"
#include "JSObject.h"

namespace JSC {

class JSArray;
class LLIntOffsetsExtractor;

class JSArray : public JSNonFinalObject {
    friend class LLIntOffsetsExtractor;
    friend class Walker;
    friend class JIT;

public:
    typedef JSNonFinalObject Base;

protected:
    explicit JSArray(JSGlobalData& globalData, Structure* structure, Butterfly* butterfly)
        : JSNonFinalObject(globalData, structure, butterfly)
    {
    }

public:
    static JSArray* create(JSGlobalData&, Structure*, unsigned initialLength = 0);

    // tryCreateUninitialized is used for fast construction of arrays whose size and
    // contents are known at time of creation. Clients of this interface must:
    //   - null-check the result (indicating out of memory, or otherwise unable to allocate vector).
    //   - call 'initializeIndex' for all properties in sequence, for 0 <= i < initialLength.
    static JSArray* tryCreateUninitialized(JSGlobalData&, Structure*, unsigned initialLength);

    JS_EXPORT_PRIVATE static bool defineOwnProperty(JSObject*, ExecState*, PropertyName, PropertyDescriptor&, bool throwException);

    static bool getOwnPropertySlot(JSCell*, ExecState*, PropertyName, PropertySlot&);
    static bool getOwnPropertyDescriptor(JSObject*, ExecState*, PropertyName, PropertyDescriptor&);

    static JS_EXPORTDATA const ClassInfo s_info;
        
    unsigned length() const { return getArrayLength(); }
    // OK to use on new arrays, but not if it might be a RegExpMatchArray.
    bool setLength(ExecState*, unsigned, bool throwException = false);

    void sort(ExecState*);
    void sort(ExecState*, JSValue compareFunction, CallType, const CallData&);
    void sortNumeric(ExecState*, JSValue compareFunction, CallType, const CallData&);

    void push(ExecState*, JSValue);
    JSValue pop(ExecState*);

    enum ShiftCountMode {
        // This form of shift hints that we're doing queueing. With this assumption in hand,
        // we convert to ArrayStorage, which has queue optimizations.
        ShiftCountForShift,
            
        // This form of shift hints that we're just doing care and feeding on an array that
        // is probably typically used for ordinary accesses. With this assumption in hand,
        // we try to preserve whatever indexing type it has already.
        ShiftCountForSplice
    };

    bool shiftCountForShift(ExecState* exec, unsigned startIndex, unsigned count)
    {
        return shiftCountWithArrayStorage(startIndex, count, ensureArrayStorage(exec->globalData()));
    }
    bool shiftCountForSplice(ExecState* exec, unsigned startIndex, unsigned count)
    {
        return shiftCountWithAnyIndexingType(exec, startIndex, count);
    }
    template<ShiftCountMode shiftCountMode>
    bool shiftCount(ExecState* exec, unsigned startIndex, unsigned count)
    {
        switch (shiftCountMode) {
        case ShiftCountForShift:
            return shiftCountForShift(exec, startIndex, count);
        case ShiftCountForSplice:
            return shiftCountForSplice(exec, startIndex, count);
        default:
            CRASH();
            return false;
        }
    }
        
    bool unshiftCountForShift(ExecState* exec, unsigned startIndex, unsigned count)
    {
        return unshiftCountWithArrayStorage(exec, startIndex, count, ensureArrayStorage(exec->globalData()));
    }
    bool unshiftCountForSplice(ExecState* exec, unsigned startIndex, unsigned count)
    {
        return unshiftCountWithAnyIndexingType(exec, startIndex, count);
    }
    template<ShiftCountMode shiftCountMode>
    bool unshiftCount(ExecState* exec, unsigned startIndex, unsigned count)
    {
        switch (shiftCountMode) {
        case ShiftCountForShift:
            return unshiftCountForShift(exec, startIndex, count);
        case ShiftCountForSplice:
            return unshiftCountForSplice(exec, startIndex, count);
        default:
            CRASH();
            return false;
        }
    }

    void fillArgList(ExecState*, MarkedArgumentBuffer&);
    void copyToArguments(ExecState*, CallFrame*, uint32_t length);

    static Structure* createStructure(JSGlobalData& globalData, JSGlobalObject* globalObject, JSValue prototype, IndexingType indexingType)
    {
        return Structure::create(globalData, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), &s_info, indexingType);
    }
        
protected:
    static const unsigned StructureFlags = OverridesGetOwnPropertySlot | OverridesGetPropertyNames | JSObject::StructureFlags;
    static void put(JSCell*, ExecState*, PropertyName, JSValue, PutPropertySlot&);

    static bool deleteProperty(JSCell*, ExecState*, PropertyName);
    JS_EXPORT_PRIVATE static void getOwnNonIndexPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);

private:
    bool isLengthWritable()
    {
        ArrayStorage* storage = arrayStorageOrNull();
        if (!storage)
            return true;
        SparseArrayValueMap* map = storage->m_sparseMap.get();
        return !map || !map->lengthIsReadOnly();
    }
        
    bool shiftCountWithAnyIndexingType(ExecState*, unsigned startIndex, unsigned count);
    bool shiftCountWithArrayStorage(unsigned startIndex, unsigned count, ArrayStorage*);

    bool unshiftCountWithAnyIndexingType(ExecState*, unsigned startIndex, unsigned count);
    bool unshiftCountWithArrayStorage(ExecState*, unsigned startIndex, unsigned count, ArrayStorage*);
    bool unshiftCountSlowCase(JSGlobalData&, bool, unsigned);

    template<IndexingType indexingType>
    void sortNumericVector(ExecState*, JSValue compareFunction, CallType, const CallData&);
        
    template<IndexingType indexingType>
    void sortCompactedVector(ExecState*, void* begin, unsigned relevantLength);
        
    template<IndexingType indexingType>
    void sortVector(ExecState*, JSValue compareFunction, CallType, const CallData&);

    bool setLengthWithArrayStorage(ExecState*, unsigned newLength, bool throwException, ArrayStorage*);
    void setLengthWritable(ExecState*, bool writable);
        
    template<IndexingType indexingType>
    void compactForSorting(unsigned& numDefined, unsigned& newRelevantLength);
};

inline Butterfly* createContiguousArrayButterfly(JSGlobalData& globalData, unsigned length, unsigned& vectorLength)
{
    IndexingHeader header;
    vectorLength = std::max(length, BASE_VECTOR_LEN);
    header.setVectorLength(vectorLength);
    header.setPublicLength(length);
    Butterfly* result = Butterfly::create(
        globalData, 0, 0, true, header, vectorLength * sizeof(EncodedJSValue));
    return result;
}

inline Butterfly* createArrayButterfly(JSGlobalData& globalData, unsigned initialLength)
{
    Butterfly* butterfly = Butterfly::create(
        globalData, 0, 0, true, baseIndexingHeaderForArray(initialLength), ArrayStorage::sizeFor(BASE_VECTOR_LEN));
    ArrayStorage* storage = butterfly->arrayStorage();
    storage->m_indexBias = 0;
    storage->m_sparseMap.clear();
    storage->m_numValuesInVector = 0;
    return butterfly;
}

Butterfly* createArrayButterflyInDictionaryIndexingMode(JSGlobalData&, unsigned initialLength);

inline JSArray* JSArray::create(JSGlobalData& globalData, Structure* structure, unsigned initialLength)
{
    Butterfly* butterfly;
    if (LIKELY(!hasArrayStorage(structure->indexingType()))) {
        ASSERT(
            hasUndecided(structure->indexingType())
            || hasInt32(structure->indexingType())
            || hasDouble(structure->indexingType())
            || hasContiguous(structure->indexingType()));
        unsigned vectorLength;
        butterfly = createContiguousArrayButterfly(globalData, initialLength, vectorLength);
        ASSERT(initialLength < MIN_SPARSE_ARRAY_INDEX);
        if (hasDouble(structure->indexingType())) {
            for (unsigned i = 0; i < vectorLength; ++i)
                butterfly->contiguousDouble()[i] = QNaN;
        }
    } else {
        ASSERT(
            structure->indexingType() == ArrayWithSlowPutArrayStorage
            || structure->indexingType() == ArrayWithArrayStorage);
        butterfly = createArrayButterfly(globalData, initialLength);
    }
    JSArray* array = new (NotNull, allocateCell<JSArray>(globalData.heap)) JSArray(globalData, structure, butterfly);
    array->finishCreation(globalData);
    return array;
}

inline JSArray* JSArray::tryCreateUninitialized(JSGlobalData& globalData, Structure* structure, unsigned initialLength)
{
    unsigned vectorLength = std::max(BASE_VECTOR_LEN, initialLength);
    if (vectorLength > MAX_STORAGE_VECTOR_LENGTH)
        return 0;
        
    Butterfly* butterfly;
    if (LIKELY(!hasArrayStorage(structure->indexingType()))) {
        ASSERT(
            hasUndecided(structure->indexingType())
            || hasInt32(structure->indexingType())
            || hasDouble(structure->indexingType())
            || hasContiguous(structure->indexingType()));

        void* temp;
        if (!globalData.heap.tryAllocateStorage(Butterfly::totalSize(0, 0, true, vectorLength * sizeof(EncodedJSValue)), &temp))
            return 0;
        butterfly = Butterfly::fromBase(temp, 0, 0);
        butterfly->setVectorLength(vectorLength);
        butterfly->setPublicLength(initialLength);
    } else {
        void* temp;
        if (!globalData.heap.tryAllocateStorage(Butterfly::totalSize(0, 0, true, ArrayStorage::sizeFor(vectorLength)), &temp))
            return 0;
        butterfly = Butterfly::fromBase(temp, 0, 0);
        *butterfly->indexingHeader() = indexingHeaderForArray(initialLength, vectorLength);
        ArrayStorage* storage = butterfly->arrayStorage();
        storage->m_indexBias = 0;
        storage->m_sparseMap.clear();
        storage->m_numValuesInVector = initialLength;
    }
        
    JSArray* array = new (NotNull, allocateCell<JSArray>(globalData.heap)) JSArray(globalData, structure, butterfly);
    array->finishCreation(globalData);
    return array;
}

JSArray* asArray(JSValue);

inline JSArray* asArray(JSCell* cell)
{
    ASSERT(cell->inherits(&JSArray::s_info));
    return jsCast<JSArray*>(cell);
}

inline JSArray* asArray(JSValue value)
{
    return asArray(value.asCell());
}

inline bool isJSArray(JSCell* cell) { return cell->classInfo() == &JSArray::s_info; }
inline bool isJSArray(JSValue v) { return v.isCell() && isJSArray(v.asCell()); }

inline JSArray* constructArray(ExecState* exec, Structure* arrayStructure, const ArgList& values)
{
    JSGlobalData& globalData = exec->globalData();
    unsigned length = values.size();
    JSArray* array = JSArray::tryCreateUninitialized(globalData, arrayStructure, length);

    // FIXME: we should probably throw an out of memory error here, but
    // when making this change we should check that all clients of this
    // function will correctly handle an exception being thrown from here.
    if (!array)
        CRASH();

    for (unsigned i = 0; i < length; ++i)
        array->initializeIndex(globalData, i, values.at(i));
    return array;
}
    
inline JSArray* constructArray(ExecState* exec, Structure* arrayStructure, const JSValue* values, unsigned length)
{
    JSGlobalData& globalData = exec->globalData();
    JSArray* array = JSArray::tryCreateUninitialized(globalData, arrayStructure, length);

    // FIXME: we should probably throw an out of memory error here, but
    // when making this change we should check that all clients of this
    // function will correctly handle an exception being thrown from here.
    if (!array)
        CRASH();

    for (unsigned i = 0; i < length; ++i)
        array->initializeIndex(globalData, i, values[i]);
    return array;
}

} // namespace JSC

#endif // JSArray_h