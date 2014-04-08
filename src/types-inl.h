// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_TYPES_INL_H_
#define V8_TYPES_INL_H_

#include "types.h"

#include "factory.h"
#include "handles-inl.h"

namespace v8 {
namespace internal {

// static
Type* ZoneTypeConfig::handle(Type* type) {
  return type;
}


// static
bool ZoneTypeConfig::is_bitset(Type* type) {
  return reinterpret_cast<intptr_t>(type) & 1;
}


// static
bool ZoneTypeConfig::is_struct(Type* type) {
  return !is_bitset(type);
}


// static
bool ZoneTypeConfig::is_class(Type* type) {
  return is_struct(type) && struct_tag(as_struct(type)) == Type::kClassTag;
}


// static
bool ZoneTypeConfig::is_constant(Type* type) {
  return is_struct(type) && struct_tag(as_struct(type)) == Type::kConstantTag;
}


// static
int ZoneTypeConfig::as_bitset(Type* type) {
  ASSERT(is_bitset(type));
  return static_cast<int>(reinterpret_cast<intptr_t>(type) >> 1);
}


// static
ZoneTypeConfig::Struct* ZoneTypeConfig::as_struct(Type* type) {
  ASSERT(is_struct(type));
  return reinterpret_cast<Struct*>(type);
}


// static
i::Handle<i::Map> ZoneTypeConfig::as_class(Type* type) {
  ASSERT(is_class(type));
  return i::Handle<i::Map>(static_cast<i::Map**>(as_struct(type)->args[1]));
}


// static
i::Handle<i::Object> ZoneTypeConfig::as_constant(Type* type) {
  ASSERT(is_constant(type));
  return i::Handle<i::Object>(
      static_cast<i::Object**>(as_struct(type)->args[1]));
}


// static
ZoneTypeConfig::Type* ZoneTypeConfig::from_bitset(int bitset) {
  return reinterpret_cast<Type*>((bitset << 1) | 1);
}


// static
ZoneTypeConfig::Type* ZoneTypeConfig::from_bitset(int bitset, Zone* Zone) {
  return from_bitset(bitset);
}


// static
ZoneTypeConfig::Type* ZoneTypeConfig::from_struct(Struct* structured) {
  return reinterpret_cast<Type*>(structured);
}


// static
ZoneTypeConfig::Type* ZoneTypeConfig::from_class(
    i::Handle<i::Map> map, int lub, Zone* zone) {
  Struct* structured = struct_create(Type::kClassTag, 2, zone);
  structured->args[0] = from_bitset(lub);
  structured->args[1] = map.location();
  return from_struct(structured);
}


// static
ZoneTypeConfig::Type* ZoneTypeConfig::from_constant(
    i::Handle<i::Object> value, int lub, Zone* zone) {
  Struct* structured = struct_create(Type::kConstantTag, 2, zone);
  structured->args[0] = from_bitset(lub);
  structured->args[1] = value.location();
  return from_struct(structured);
}


// static
ZoneTypeConfig::Struct* ZoneTypeConfig::struct_create(
    int tag, int length, Zone* zone) {
  Struct* structured = reinterpret_cast<Struct*>(
      zone->New(sizeof(Struct) + sizeof(void*) * (length - 1)));  // NOLINT
  structured->tag = tag;
  structured->length = length;
  return structured;
}


// static
void ZoneTypeConfig::struct_shrink(Struct* structured, int length) {
  ASSERT(0 <= length && length <= structured->length);
  structured->length = length;
}


// static
int ZoneTypeConfig::struct_tag(Struct* structured) {
  return structured->tag;
}


// static
Type* ZoneTypeConfig::struct_get(Struct* structured, int i) {
  ASSERT(0 <= i && i < structured->length);
  return static_cast<Type*>(structured->args[i]);
}


// static
void ZoneTypeConfig::struct_set(Struct* structured, int i, Type* type) {
  ASSERT(0 <= i && i < structured->length);
  structured->args[i] = type;
}


// static
int ZoneTypeConfig::struct_length(Struct* structured) {
  return structured->length;
}


// static
int ZoneTypeConfig::lub_bitset(Type* type) {
  ASSERT(is_class(type) || is_constant(type));
  return as_bitset(struct_get(as_struct(type), 0));
}

// -------------------------------------------------------------------------- //

// static
i::Handle<HeapTypeConfig::Type> HeapTypeConfig::handle(Type* type) {
  return i::handle(type, i::HeapObject::cast(type)->GetIsolate());
}


// static
bool HeapTypeConfig::is_bitset(Type* type) {
  return type->IsSmi();
}


// static
bool HeapTypeConfig::is_class(Type* type) {
  return type->IsMap();
}


// static
bool HeapTypeConfig::is_constant(Type* type) {
  return type->IsBox();
}


// static
bool HeapTypeConfig::is_struct(Type* type) {
  return type->IsFixedArray();
}


// static
int HeapTypeConfig::as_bitset(Type* type) {
  return Smi::cast(type)->value();
}


// static
i::Handle<i::Map> HeapTypeConfig::as_class(Type* type) {
  return i::handle(i::Map::cast(type));
}


// static
i::Handle<i::Object> HeapTypeConfig::as_constant(Type* type) {
  i::Box* box = i::Box::cast(type);
  return i::handle(box->value(), box->GetIsolate());
}


// static
i::Handle<HeapTypeConfig::Struct> HeapTypeConfig::as_struct(Type* type) {
  return i::handle(Struct::cast(type));
}


// static
HeapTypeConfig::Type* HeapTypeConfig::from_bitset(int bitset) {
  return Type::cast(i::Smi::FromInt(bitset));
}


// static
i::Handle<HeapTypeConfig::Type> HeapTypeConfig::from_bitset(
    int bitset, Isolate* isolate) {
  return i::handle(from_bitset(bitset), isolate);
}


// static
i::Handle<HeapTypeConfig::Type> HeapTypeConfig::from_class(
    i::Handle<i::Map> map, int lub, Isolate* isolate) {
  return i::Handle<Type>::cast(i::Handle<Object>::cast(map));
}


// static
i::Handle<HeapTypeConfig::Type> HeapTypeConfig::from_constant(
    i::Handle<i::Object> value, int lub, Isolate* isolate) {
  i::Handle<Box> box = isolate->factory()->NewBox(value);
  return i::Handle<Type>::cast(i::Handle<Object>::cast(box));
}


// static
i::Handle<HeapTypeConfig::Type> HeapTypeConfig::from_struct(
    i::Handle<Struct> structured) {
  return i::Handle<Type>::cast(i::Handle<Object>::cast(structured));
}


// static
i::Handle<HeapTypeConfig::Struct> HeapTypeConfig::struct_create(
    int tag, int length, Isolate* isolate) {
  i::Handle<Struct> structured = isolate->factory()->NewFixedArray(length + 1);
  structured->set(0, i::Smi::FromInt(tag));
  return structured;
}


// static
void HeapTypeConfig::struct_shrink(i::Handle<Struct> structured, int length) {
  structured->Shrink(length + 1);
}


// static
int HeapTypeConfig::struct_tag(i::Handle<Struct> structured) {
  return static_cast<i::Smi*>(structured->get(0))->value();
}


// static
i::Handle<HeapTypeConfig::Type> HeapTypeConfig::struct_get(
    i::Handle<Struct> structured, int i) {
  Type* type = static_cast<Type*>(structured->get(i + 1));
  return i::handle(type, structured->GetIsolate());
}


// static
void HeapTypeConfig::struct_set(
    i::Handle<Struct> structured, int i, i::Handle<Type> type) {
  structured->set(i + 1, *type);
}


// static
int HeapTypeConfig::struct_length(i::Handle<Struct> structured) {
  return structured->length() - 1;
}


// static
int HeapTypeConfig::lub_bitset(Type* type) {
  return 0;  // kNone, which causes recomputation.
}

} }  // namespace v8::internal

#endif  // V8_TYPES_INL_H_