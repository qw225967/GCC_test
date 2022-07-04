///**
// * @file   rapijson.hpp
// * @author wangmj
// *
// */
//
//#pragma once
//
//#include <rapidjson/document.h>
//#include <rapidjson/istreamwrapper.h>
//#include <rapidjson/error/en.h>
//#include <rapidjson/prettywriter.h>
//#include <rapidjson/writer.h>
//
//namespace cls {
//
//#define JSON_HAS_INT(val, key)      (val.HasMember(key) && val[key].IsInt())
//#define JSON_HAS_UINT(val, key)     (val.HasMember(key) && val[key].IsUint())
//#define JSON_HAS_UINT64(val, key)   (val.HasMember(key) && val[key].IsUint64())
//#define JSON_HAS_OBJECT(val, key)   (val.HasMember(key) && val[key].IsObject())
//#define JSON_HAS_STRING(val, key)   (val.HasMember(key) && val[key].IsString())
//#define JSON_HAS_ARRAY(val, key)    (val.HasMember(key) && val[key].IsArray())
//#define JSON_HAS_BOOL(val, key)    (val.HasMember(key) && val[key].IsBool())
//
//
//#define JSON_ADD_OBJECT(obj, key, o, allocator)          obj.AddMember(key, o, allocator);
//#define JSON_ADD_BOOL(obj, key, val, allocator)          obj.AddMember(key, Value().SetBool(val), allocator);
//#define JSON_ADD_C_STR(obj, key, c_str, allocator)       obj.AddMember(key, c_str, allocator);
//#define JSON_ADD_STRING(obj, key, str, allocator)        obj.AddMember(key, Value().SetString(str.c_str(), allocator), allocator);
//
//#define JSON_ADD_INT(obj, key, i, allocator)             obj.AddMember(key, Value(i), allocator);
//#define JSON_ADD_UINT(obj, key, i, allocator)            JSON_ADD_INT(obj, key, (uint32_t)i, allocator)
//#define JSON_ADD_UINT64(obj, key, i, allocator)          JSON_ADD_INT(obj, key, (uint64_t)i, allocator)
//
//#define JSON_ADD_ARRAY_INT(obj, key, val, allocator) {  \
//  Value v(kArrayType);  \
//  for (auto i : val) {  \
//    v.PushBack(i, allocator); \
//  } \
//  obj.AddMember(key, v, allocator);  \
//}
//#define JSON_ADD_ARRAY_UINT(obj, key, val, allocator)    JSON_ADD_ARRAY_INT(obj, key, val, allocator)
//#define JSON_ADD_ARRAY_UINT64(obj, key, val, allocator)  JSON_ADD_ARRAY_INT(obj, key, val, allocator)
//
//} // namespace cls