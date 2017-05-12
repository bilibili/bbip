/**
 * The first thing to know about are types. The available types in Thrift are:
 *
 *  bool        Boolean, one byte
 *  byte        Signed byte
 *  i16         Signed 16-bit integer
 *  i32         Signed 32-bit integer
 *  i64         Signed 64-bit integer
 *  double      64-bit floating point value
 *  string      String
 *  binary      Blob (byte array)
 *  map<t1,t2>  Map from one type to another
 *  list<t1>    Ordered list of one type
 *  set<t1>     Set of unique elements of one type
 *
 * Did you also notice that Thrift supports C style comments?
 */

// Just in case you were wondering... yes. We support simple C comments too.
namespace cpp bilibili
namespace d bilibili
namespace java com.bilibili
namespace perl bilibili
namespace php com.bilibili
namespace haxe bilibili

struct bbip_result {
  1: i64 range_start
  2: i64 range_end
  3: string country
  4: string province
  5: string city
  6: string district
  7: string isp
  8: string type
  9: string desc
  10: optional double lat
  11: optional double lng
  12: i16 cidr
}

exception InvalidOperation {
  1: i32 code,
  2: string msg
}

/**
 * Ahh, now onto the cool part, defining a service. Services just need a name
 * and can optionally inherit from another service using the extends keyword.
 */
service bbip {

  /**
   * A method definition looks like C code. It has a return type, arguments,
   * and optionally a list of exceptions that it may throw. Note that argument
   * lists and exception lists are specified using the exact same syntax as
   * field lists in struct or exception definitions.
   */

   void ping(),

   i32 count(),

   bbip_result query(1:string string_ip) throws (1:InvalidOperation ouch),

   string query_string(1:string string_ip) throws (1:InvalidOperation ouch),

   i16 GetAreaFlag(1:string string_ip) throws (1:InvalidOperation ouch),

   i64 GetZoneID(1:string string_ip) throws (1:InvalidOperation ouch),

   bbip_result query_long(1:i64 int_ip) throws (1:InvalidOperation ouch)

}
