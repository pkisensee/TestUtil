///////////////////////////////////////////////////////////////////////////////
//
//  TestUtil.cpp
//
//  Copyright © Pete Isensee (PKIsensee@msn.com).
//  All rights reserved worldwide.
//
//  Permission to copy, modify, reproduce or redistribute this source code is
//  granted provided the above copyright notice is retained in the resulting 
//  source code.
// 
//  This software is provided "as is" and without any express or implied
//  warranties.
//
///////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <limits>
#include <span>
#include <vector>

#include "Log.h"
#include "Util.h"

using namespace PKIsensee;

#ifdef _DEBUG
#define verify(e) assert(e)
#define test(e)   assert(e)
#else
#define verify(e) static_cast<void>(e)
#define test(e)   static_cast<void>( (e) || ( Util::DebugBreak(), 0 ) )
#endif

struct Size1 PK_PACKED_STRUCT
{
PK_START_PACK
  int8_t a;
PK_END_PACK
};

struct Size5 PK_PACKED_STRUCT
{
PK_START_PACK
  int8_t  a;
  int32_t b;
PK_END_PACK
};

class Size15 PK_PACKED_STRUCT
{
PK_START_PACK
  int8_t  a;
  int16_t e;
  int32_t b;
  int64_t c;
PK_END_PACK
};

void TestStructPacking()
{
  test( sizeof( Size1 ) == 1 );
  test( sizeof( Size5 ) == 5 );
  test( sizeof( Size15 ) == 15 );
}

void TestEndian()
{
#if defined(_WIN32)
  test( !Util::IsBigEndian() );
#else
  test( Util::IsBigEndian() );
#endif
  test( Util::ToLittleEndian( 0xAABBCCDD ) == 0xAABBCCDD );
  test( Util::ToBigEndian( 0xAABBCCDD ) == 0xDDCCBBAA );
}

void TestReverseBytes()
{
  uint8_t u8 = 0xAA;
  uint16_t u16 = 0xAABB;
  test( Util::ReverseBytes( u8 ) == 0xAA );
  test( Util::ReverseBytes( u16 ) == 0xBBAA );
  test( Util::ReverseBytes( 0x11223344 ) == 0x44332211 );
  test( Util::ReverseBytes( 0x11111111 ) == 0x11111111 );
  test( Util::ReverseBytes( 1234 ) == 0xD2040000 );
  test( Util::ReverseBytes( 0x11223344AABBCCDD ) == 0xDDCCBBAA44332211 );

  std::array<char, 6> buf { 'a','b','c','x','y','z' };
  std::array<char, 6> rbuf{ 'z','y','x','c','b','a' };
  test( Util::ReverseBytes( buf ) == rbuf );
}

void TestPackBits()
{
  // Packing 1 byte values is not very useful
  test( Util::PackBits<7>( 0x55 ) == 0x55 );

  test( Util::PackBits<7>( 0x1234 ) == 0x0934 );

  test( Util::PackBits<8>( 0x1234ABCD ) == 0x1234ABCD );
  test( Util::PackBits<7>( 0x7F7F7F7F ) == 0x0FFFFFFF );
  test( Util::PackBits<6>( 0x3F3F3F3F ) == 0x00FFFFFF );
  test( Util::PackBits<5>( 0x1F1F1F1F ) == 0x000FFFFF );
  test( Util::PackBits<4>( 0x0F0F0F0F ) == 0x0000FFFF );
  test( Util::PackBits<3>( 0x07070707 ) == 0x00000FFF );
  test( Util::PackBits<2>( 0x03030303 ) == 0x000000FF );
  test( Util::PackBits<1>( 0x01010101 ) == 0x0000000F );

  test( Util::PackBits<5>( 0x1F1F1F1F1F1F1F1F ) == 0x000000FFFFFFFFFF );

  // Error conditions
  test( Util::PackBits<7>( 0xFF7F7F7F ) == 0xFF7F7F7F );
  test( Util::PackBits<6>( 0x12345678 ) == 0x12345678 );
  test( Util::PackBits<1>( 0x02 ) == 0x02 );

  // We can test Unpacking using random numbers up to the limit
  for( uint16_t i = 0; i < 0x4000; ++i )
  {
    uint16_t source = Util::UnpackBits<7>( i );
    test( Util::PackBits<7>( source ) == i);
  }

  for( uint16_t i = 0; i < 0x1000; ++i )
  {
    uint16_t source = Util::UnpackBits<6>( i );
    test( Util::PackBits<6>( source ) == i );
  }

  for(uint32_t i = 0; i < 0x00100000; ++i )
  {
    uint32_t source = Util::UnpackBits<5>( i );
    test( Util::PackBits<5>( source ) == i );
  }

  for( uint64_t i = 0; i < 0x0000000001000000; ++i )
  {
    uint64_t source = Util::UnpackBits<3>( i );
    test( Util::PackBits<3>( source ) == i );
  }
}

void TestNumericConversion()
{
  test( Util::ToNum<uint32_t>( std::string("1234") ) == 1234u );
  test( Util::ToNum<int32_t>( std::string( "-1234" ) ) == -1234 );
  test( Util::ToNum<uint8_t>( std::string( "123" ) ) == 123u );
  test( Util::ToNum<uint8_t>( std::string( "1234" ) ) == 0u ); // overflow

  test( Util::ToNum<uint16_t>( std::vector{ '1', '2', '3' } ) == 123u );
  test( Util::ToNum<uint16_t>( "123" ) == 123u );

  test( Util::ToNum<uint32_t>( std::string( "1234ABCD" ), 16 ) == 0x1234ABCD );
  test( Util::ToNum<int8_t>( std::string( "1010101" ), 2 ) == 0b1010101 );
  test( Util::ToNum<uint64_t>( std::string( "1010101010101010101010101010101010101010101010101010101010101010" ), 2 ) == 
                                            0b1010101010101010101010101010101010101010101010101010101010101010 );

  test( Util::ToNum<float>( std::string( "12.34" ) ) == 12.34f );
  test( Util::ToNum<double>( std::string( "-0.01234" ) ) == -0.01234 );
  test( Util::ToNum<long double>( std::string( "0.00001234" ) ) == 0.00001234 );

  test( Util::ToNum<float>( std::vector{ '1', '2', '.', '3' } ) == 12.3f );
  test( Util::ToNum<double>( "-123.456" ) == -123.456 );

  test( Util::ToStr<std::string>( 1234u ) == "1234" );
  test( Util::ToString( 1234u ) == "1234" );
  test( Util::ToString( -1234 ) == "-1234" );
  test( Util::ToString( 0x1234ABCD, 16 ) == "1234abcd" );
  test( Util::ToString( 0b1010101, 2 ) == "1010101" );

  test( Util::ToString<>( 0b1010101010101010101010101010101010101010101010101010101010101010, 2 ) == 
                          "1010101010101010101010101010101010101010101010101010101010101010" );

  test( Util::ToString( 12.34f ) == "12.34" );
  test( Util::ToString( -0.01234 ) == "-0.01234" );
  test( Util::ToString( 0.00001234 ) == "1.234e-05" );

  test( Util::ToStr< std::wstring >( 1234u ) == L"1234" );
  auto x = Util::ToStr< std::vector<char> >( 1234u );
  auto y = std::vector<char>{ '1', '2', '3', '4' };
  test( x == y );
}

void TestFourCC()
{
  test( Util::FourCC( "abcd" ) == 'a' + ('b'<<8) + ('c'<<16) + ('d'<<24) );
  test( Util::FourCC( "WAVE" ) == 0x45564157 );
  test( Util::FourCC( "WXYZ" ) == 0x5A595857 );
  test( Util::FourCC( "H264" ) == 0x34363248 );
  unsigned char cc[ 4 ] = { 'R', 'I', 'F', 'F' };
  test( Util::FourCC( cc ) == 0x46464952 );

  std::string riff( "RIFF" );
  test( Util::FourCC( riff ) == 0x46464952 );
  [[maybe_unused]] std::string tooLong( "tooLong" );
  //test( Util::FourCC( tooLong ) == 0xDEADBEEF ); // runtime assert
  //test( Util::FourCC( riff.c_str() ) == 0x46464952 ); // compiler error

  std::array<char, 4> arr{ 'R', 'I', 'F', 'F' };
  test( Util::FourCC( arr ) == 0x46464952 );

  std::span<char> span{ riff };
  test( Util::FourCC( span ) == 0x46464952 );

  //test( Util::FourCC( L"wide" ) == 0xDEADBEEF ); // compiler error
  //test( Util::FourCC( nullptr ) == 0 ); // compiler error
  //test( Util::FourCC( "bad" ) == 0xDEADBEEF ); // compiler error
  //test( Util::FourCC( "invalid" ) == 0xDEADBEEF ); // compiler error
}

void TestLog()
{
  PKLOG_ERR( "Testing error logging\n" );
  PKLOG_WARN( "%s %d\n", "Testing warning logging", 42 );
  PKLOG_SCRN( "%s %f\n", "Testing logging to screen", 42.42 );
  PKLOG_NOTE( "Testing logging of notes\r\n" );
  PKLOG_FILE( "%s\n", "Testing logging to file" );
}

int __cdecl main()
{
  TestStructPacking();
  TestEndian();
  TestReverseBytes();
  TestPackBits();
  TestNumericConversion();
  TestFourCC();
  TestLog();
}
