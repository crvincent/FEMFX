/*
MIT License

Copyright (c) 2019 Advanced Micro Devices, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef _FEMFX_SOA_SCALAR_H
#define _FEMFX_SOA_SCALAR_H

#include <cmath>
#include <cstdint>
#include <cstdio>
#include "simd_utils.h"

#ifndef SIMD_VECTORMATH_FORCE_INLINE
#if defined(_WIN32) && defined(_MSC_VER)
#define SIMD_VECTORMATH_FORCE_INLINE __forceinline
#else
#define SIMD_VECTORMATH_FORCE_INLINE inline
#endif
#endif

namespace FmVectormath {

class Soa4Float;
class Soa4Int;
class Soa4Uint;
class Soa4Bool;

class Soa4Float
{
    float mData[4];
public:
    static const uint32_t width = 4;
    typedef Soa4Bool SoaBool; typedef Soa4Int SoaInt; typedef Soa4Uint SoaUint;
    SIMD_VECTORMATH_FORCE_INLINE Soa4Float() {}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Float(const Soa4Float& v) { for(int i=0;i<4;++i)mData[i]=v.mData[i]; }
    SIMD_VECTORMATH_FORCE_INLINE explicit Soa4Float(FmSimdFloat128 v) { for(int i=0;i<4;++i)mData[i]=v.f[i]; }
    SIMD_VECTORMATH_FORCE_INLINE explicit Soa4Float(const float* v) { for(int i=0;i<4;++i)mData[i]=v[i]; }
    SIMD_VECTORMATH_FORCE_INLINE Soa4Float(float v) { for(int i=0;i<4;++i)mData[i]=v; }
    SIMD_VECTORMATH_FORCE_INLINE Soa4Float(float a,float b,float c,float d) { mData[0]=a;mData[1]=b;mData[2]=c;mData[3]=d; }
    SIMD_VECTORMATH_FORCE_INLINE FmSimdFloat128 get128() const { return FmSimdFloat128(mData[0],mData[1],mData[2],mData[3]); }
    SIMD_VECTORMATH_FORCE_INLINE float getSlice(uint32_t i) const { return mData[i&3]; }
    SIMD_VECTORMATH_FORCE_INLINE void setSlice(uint32_t i,float v) { mData[i&3]=v; }
    SIMD_VECTORMATH_FORCE_INLINE void setFromAligned(float* p) { for(int i=0;i<4;++i)mData[i]=p[i]; }
    SIMD_VECTORMATH_FORCE_INLINE const Soa4Float operator++(int) { Soa4Float r=*this; for(int i=0;i<4;++i)mData[i]+=1.0f; return r; }
    SIMD_VECTORMATH_FORCE_INLINE const Soa4Float operator--(int) { Soa4Float r=*this; for(int i=0;i<4;++i)mData[i]-=1.0f; return r; }
    SIMD_VECTORMATH_FORCE_INLINE Soa4Float& operator++(){for(int i=0;i<4;++i)mData[i]+=1.0f;return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Float& operator--(){for(int i=0;i<4;++i)mData[i]-=1.0f;return *this;}
    SIMD_VECTORMATH_FORCE_INLINE const Soa4Float operator-()const{Soa4Float r;for(int i=0;i<4;++i)r.mData[i]=-mData[i];return r;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Float& operator=(const Soa4Float& v){for(int i=0;i<4;++i)mData[i]=v.mData[i];return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Float& operator*=(const Soa4Float& v){for(int i=0;i<4;++i)mData[i]*=v.mData[i];return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Float& operator/=(const Soa4Float& v){for(int i=0;i<4;++i)mData[i]/=v.mData[i];return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Float& operator+=(const Soa4Float& v){for(int i=0;i<4;++i)mData[i]+=v.mData[i];return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Float& operator-=(const Soa4Float& v){for(int i=0;i<4;++i)mData[i]-=v.mData[i];return *this;}
};

SIMD_VECTORMATH_FORCE_INLINE const Soa4Float operator*(const Soa4Float&a,const Soa4Float&b){Soa4Float r;r=a;return r*=b;}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Float operator/(const Soa4Float&a,const Soa4Float&b){Soa4Float r;r=a;return r/=b;}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Float operator+(const Soa4Float&a,const Soa4Float&b){Soa4Float r=a; return r+=b;}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Float operator-(const Soa4Float&a,const Soa4Float&b){Soa4Float r=a; return r-=b;}

class Soa4Bool
{
    bool mData[4];
public:
    static const uint32_t width=4;
    typedef Soa4Float SoaFloat; typedef Soa4Int SoaInt; typedef Soa4Uint SoaUint;
    SIMD_VECTORMATH_FORCE_INLINE Soa4Bool(){}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Bool(const Soa4Bool& v){for(int i=0;i<4;++i)mData[i]=v.mData[i];}
    SIMD_VECTORMATH_FORCE_INLINE explicit Soa4Bool(bool v){for(int i=0;i<4;++i)mData[i]=v;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Bool(bool a,bool b,bool c,bool d){mData[0]=a;mData[1]=b;mData[2]=c;mData[3]=d;}
    SIMD_VECTORMATH_FORCE_INLINE explicit Soa4Bool(const Soa4Float&);
    SIMD_VECTORMATH_FORCE_INLINE explicit Soa4Bool(const Soa4Int&);
    SIMD_VECTORMATH_FORCE_INLINE explicit Soa4Bool(const Soa4Uint&);
    SIMD_VECTORMATH_FORCE_INLINE explicit Soa4Bool(FmSimdFloat128 v){for(int i=0;i<4;++i)mData[i]=(v.f[i]!=0);}
    SIMD_VECTORMATH_FORCE_INLINE explicit Soa4Bool(FmSimdInt128 v){for(int i=0;i<4;++i)mData[i]=(v.i[i]!=0);}
    SIMD_VECTORMATH_FORCE_INLINE FmSimdFloat128 get128() const {return FmSimdFloat128(mData[0]?1.0f:0,mData[1]?1.0f:0,mData[2]?1.0f:0,mData[3]?1.0f:0);}
    SIMD_VECTORMATH_FORCE_INLINE FmSimdInt128 get128i() const {return FmSimdInt128(mData[0]?-1:0,mData[1]?-1:0,mData[2]?-1:0,mData[3]?-1:0);}
    SIMD_VECTORMATH_FORCE_INLINE bool getSlice(uint32_t i) const{return mData[i&3];}
    SIMD_VECTORMATH_FORCE_INLINE void setSlice(uint32_t i,bool v){mData[i&3]=v;}
    SIMD_VECTORMATH_FORCE_INLINE const Soa4Bool operator!()const{Soa4Bool r;for(int i=0;i<4;++i)r.mData[i]=!mData[i];return r;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Bool& operator=(const Soa4Bool&v){for(int i=0;i<4;++i)mData[i]=v.mData[i];return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Bool& operator&=(const Soa4Bool&v){for(int i=0;i<4;++i)mData[i]&=v.mData[i];return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Bool& operator^=(const Soa4Bool&v){for(int i=0;i<4;++i)mData[i]^=v.mData[i];return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Bool& operator|=(const Soa4Bool&v){for(int i=0;i<4;++i)mData[i]|=v.mData[i];return *this;}
};

class Soa4Int
{
    int32_t mData[4];
public:
    static const uint32_t width=4;
    typedef Soa4Float SoaFloat; typedef Soa4Uint SoaUint; typedef Soa4Bool SoaBool;
    SIMD_VECTORMATH_FORCE_INLINE Soa4Int(){}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Int(const Soa4Int&v){for(int i=0;i<4;++i)mData[i]=v.mData[i];}
    SIMD_VECTORMATH_FORCE_INLINE explicit Soa4Int(FmSimdInt128 v){for(int i=0;i<4;++i)mData[i]=v.i[i];}
    SIMD_VECTORMATH_FORCE_INLINE explicit Soa4Int(const int32_t*v){for(int i=0;i<4;++i)mData[i]=v[i];}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Int(int32_t v){for(int i=0;i<4;++i)mData[i]=v;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Int(int32_t a,int32_t b,int32_t c,int32_t d){mData[0]=a;mData[1]=b;mData[2]=c;mData[3]=d;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Int(const Soa4Bool&v);
    SIMD_VECTORMATH_FORCE_INLINE Soa4Int(const Soa4Uint&v);
    SIMD_VECTORMATH_FORCE_INLINE FmSimdInt128 get128i()const{return FmSimdInt128(mData[0],mData[1],mData[2],mData[3]);}
    SIMD_VECTORMATH_FORCE_INLINE int32_t getSlice(uint32_t i)const{return mData[i&3];}
    SIMD_VECTORMATH_FORCE_INLINE void setSlice(uint32_t i,int32_t v){mData[i&3]=v;}
    SIMD_VECTORMATH_FORCE_INLINE const Soa4Int operator++(int){Soa4Int r=*this;for(int i=0;i<4;++i)mData[i]++;return r;}
    SIMD_VECTORMATH_FORCE_INLINE const Soa4Int operator--(int){Soa4Int r=*this;for(int i=0;i<4;++i)mData[i]--;return r;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Int&operator++(){for(int i=0;i<4;++i)mData[i]++;return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Int&operator--(){for(int i=0;i<4;++i)mData[i]--;return *this;}
    SIMD_VECTORMATH_FORCE_INLINE const Soa4Int operator-()const{Soa4Int r;for(int i=0;i<4;++i)r.mData[i]=-mData[i];return r;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Int&operator=(const Soa4Int&v){for(int i=0;i<4;++i)mData[i]=v.mData[i];return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Int&operator*=(const Soa4Int&v){for(int i=0;i<4;++i)mData[i]*=v.mData[i];return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Int&operator+=(const Soa4Int&v){for(int i=0;i<4;++i)mData[i]+=v.mData[i];return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Int&operator-=(const Soa4Int&v){for(int i=0;i<4;++i)mData[i]-=v.mData[i];return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Int&operator&=(const Soa4Int&v){for(int i=0;i<4;++i)mData[i]&=v.mData[i];return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Int&operator^=(const Soa4Int&v){for(int i=0;i<4;++i)mData[i]^=v.mData[i];return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Int&operator|=(const Soa4Int&v){for(int i=0;i<4;++i)mData[i]|=v.mData[i];return *this;}
};

class Soa4Uint
{
    uint32_t mData[4];
public:
    static const uint32_t width=4;
    typedef Soa4Float SoaFloat; typedef Soa4Int SoaInt; typedef Soa4Bool SoaBool;
    SIMD_VECTORMATH_FORCE_INLINE Soa4Uint(){}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Uint(const Soa4Uint&v){for(int i=0;i<4;++i)mData[i]=v.mData[i];}
    SIMD_VECTORMATH_FORCE_INLINE explicit Soa4Uint(FmSimdInt128 v){for(int i=0;i<4;++i)mData[i]=(uint32_t)v.i[i];}
    SIMD_VECTORMATH_FORCE_INLINE explicit Soa4Uint(const uint32_t*v){for(int i=0;i<4;++i)mData[i]=v[i];}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Uint(uint32_t v){for(int i=0;i<4;++i)mData[i]=v;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Uint(int32_t v){for(int i=0;i<4;++i)mData[i]=(uint32_t)v;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Uint(uint32_t a,uint32_t b,uint32_t c,uint32_t d){mData[0]=a;mData[1]=b;mData[2]=c;mData[3]=d;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Uint(const Soa4Int&v);
    SIMD_VECTORMATH_FORCE_INLINE Soa4Uint(const Soa4Bool&v);
    SIMD_VECTORMATH_FORCE_INLINE FmSimdInt128 get128i()const{return FmSimdInt128((int32_t)mData[0],(int32_t)mData[1],(int32_t)mData[2],(int32_t)mData[3]);}
    SIMD_VECTORMATH_FORCE_INLINE uint32_t getSlice(uint32_t i)const{return mData[i&3];}
    SIMD_VECTORMATH_FORCE_INLINE void setSlice(uint32_t i,uint32_t v){mData[i&3]=v;}
    SIMD_VECTORMATH_FORCE_INLINE const Soa4Uint operator++(int){Soa4Uint r=*this;for(int i=0;i<4;++i)mData[i]++;return r;}
    SIMD_VECTORMATH_FORCE_INLINE const Soa4Uint operator--(int){Soa4Uint r=*this;for(int i=0;i<4;++i)mData[i]--;return r;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Uint&operator++(){for(int i=0;i<4;++i)mData[i]++;return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Uint&operator--(){for(int i=0;i<4;++i)mData[i]--;return *this;}
    SIMD_VECTORMATH_FORCE_INLINE const Soa4Int operator-()const;
    SIMD_VECTORMATH_FORCE_INLINE Soa4Uint&operator=(const Soa4Uint&v){for(int i=0;i<4;++i)mData[i]=v.mData[i];return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Uint&operator*=(const Soa4Uint&v){for(int i=0;i<4;++i)mData[i]*=v.mData[i];return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Uint&operator+=(const Soa4Uint&v){for(int i=0;i<4;++i)mData[i]+=v.mData[i];return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Uint&operator-=(const Soa4Uint&v){for(int i=0;i<4;++i)mData[i]-=v.mData[i];return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Uint&operator&=(const Soa4Uint&v){for(int i=0;i<4;++i)mData[i]&=v.mData[i];return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Uint&operator^=(const Soa4Uint&v){for(int i=0;i<4;++i)mData[i]^=v.mData[i];return *this;}
    SIMD_VECTORMATH_FORCE_INLINE Soa4Uint&operator|=(const Soa4Uint&v){for(int i=0;i<4;++i)mData[i]|=v.mData[i];return *this;}
};

// Scalar lane implementations retain the same public SoA API.
SIMD_VECTORMATH_FORCE_INLINE Soa4Bool::Soa4Bool(const Soa4Float&v){for(int i=0;i<4;++i)mData[i]=(v.getSlice(i)!=0.0f);}
SIMD_VECTORMATH_FORCE_INLINE Soa4Bool::Soa4Bool(const Soa4Int&v){for(int i=0;i<4;++i)mData[i]=(v.getSlice(i)!=0);}
SIMD_VECTORMATH_FORCE_INLINE Soa4Bool::Soa4Bool(const Soa4Uint&v){for(int i=0;i<4;++i)mData[i]=(v.getSlice(i)!=0);}
SIMD_VECTORMATH_FORCE_INLINE Soa4Int::Soa4Int(const Soa4Bool&v){for(int i=0;i<4;++i)mData[i]=v.getSlice(i)?-1:0;}
SIMD_VECTORMATH_FORCE_INLINE Soa4Int::Soa4Int(const Soa4Uint&v){for(int i=0;i<4;++i)mData[i]=(int32_t)v.getSlice(i);}
SIMD_VECTORMATH_FORCE_INLINE Soa4Uint::Soa4Uint(const Soa4Int&v){for(int i=0;i<4;++i)mData[i]=(uint32_t)v.getSlice(i);}
SIMD_VECTORMATH_FORCE_INLINE Soa4Uint::Soa4Uint(const Soa4Bool&v){for(int i=0;i<4;++i)mData[i]=v.getSlice(i)?uint32_t(-1):0;}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Int Soa4Uint::operator-()const{Soa4Int r;for(int i=0;i<4;++i)r.setSlice(i,-(int32_t)getSlice(i));return r;}

#define FEMFX_SOA_COMPARE_RESULT(Type, Op) \
    SIMD_VECTORMATH_FORCE_INLINE const Soa4Bool operator Op (const Type&a,const Type&b){return Soa4Bool(a.getSlice(0) Op b.getSlice(0),a.getSlice(1) Op b.getSlice(1),a.getSlice(2) Op b.getSlice(2),a.getSlice(3) Op b.getSlice(3));}
FEMFX_SOA_COMPARE_RESULT(Soa4Float,<) FEMFX_SOA_COMPARE_RESULT(Soa4Float,<=)
FEMFX_SOA_COMPARE_RESULT(Soa4Float,>) FEMFX_SOA_COMPARE_RESULT(Soa4Float,>=)
FEMFX_SOA_COMPARE_RESULT(Soa4Float,==) FEMFX_SOA_COMPARE_RESULT(Soa4Float,!=)
FEMFX_SOA_COMPARE_RESULT(Soa4Int,<) FEMFX_SOA_COMPARE_RESULT(Soa4Int,<=)
FEMFX_SOA_COMPARE_RESULT(Soa4Int,>) FEMFX_SOA_COMPARE_RESULT(Soa4Int,>=)
FEMFX_SOA_COMPARE_RESULT(Soa4Int,==) FEMFX_SOA_COMPARE_RESULT(Soa4Int,!=)
FEMFX_SOA_COMPARE_RESULT(Soa4Uint,<) FEMFX_SOA_COMPARE_RESULT(Soa4Uint,<=)
FEMFX_SOA_COMPARE_RESULT(Soa4Uint,>) FEMFX_SOA_COMPARE_RESULT(Soa4Uint,>=)
FEMFX_SOA_COMPARE_RESULT(Soa4Uint,==) FEMFX_SOA_COMPARE_RESULT(Soa4Uint,!=)
#undef FEMFX_SOA_COMPARE_RESULT

SIMD_VECTORMATH_FORCE_INLINE const Soa4Bool operator==(const Soa4Bool&a,const Soa4Bool&b){return Soa4Bool(a.getSlice(0)==b.getSlice(0),a.getSlice(1)==b.getSlice(1),a.getSlice(2)==b.getSlice(2),a.getSlice(3)==b.getSlice(3));}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Bool operator!=(const Soa4Bool&a,const Soa4Bool&b){return !(a==b);}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Bool operator&(const Soa4Bool&a,const Soa4Bool&b){return Soa4Bool(a.getSlice(0)&&b.getSlice(0),a.getSlice(1)&&b.getSlice(1),a.getSlice(2)&&b.getSlice(2),a.getSlice(3)&&b.getSlice(3));}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Bool operator^(const Soa4Bool&a,const Soa4Bool&b){return Soa4Bool(a.getSlice(0)!=b.getSlice(0),a.getSlice(1)!=b.getSlice(1),a.getSlice(2)!=b.getSlice(2),a.getSlice(3)!=b.getSlice(3));}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Bool operator|(const Soa4Bool&a,const Soa4Bool&b){return Soa4Bool(a.getSlice(0)||b.getSlice(0),a.getSlice(1)||b.getSlice(1),a.getSlice(2)||b.getSlice(2),a.getSlice(3)||b.getSlice(3));}

SIMD_VECTORMATH_FORCE_INLINE bool all(const Soa4Bool&v){return v.getSlice(0)&&v.getSlice(1)&&v.getSlice(2)&&v.getSlice(3);}
SIMD_VECTORMATH_FORCE_INLINE bool any(const Soa4Bool&v){return v.getSlice(0)||v.getSlice(1)||v.getSlice(2)||v.getSlice(3);}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Bool select(const Soa4Bool&a,const Soa4Bool&b,const Soa4Bool&sel){return Soa4Bool(sel.getSlice(0)?b.getSlice(0):a.getSlice(0),sel.getSlice(1)?b.getSlice(1):a.getSlice(1),sel.getSlice(2)?b.getSlice(2):a.getSlice(2),sel.getSlice(3)?b.getSlice(3):a.getSlice(3));}

#define FEMFX_FLOAT_SCALAR_BINARY(name,fn) \
 SIMD_VECTORMATH_FORCE_INLINE const Soa4Float name(const Soa4Float&a,const Soa4Float&b){return Soa4Float(fn(a.getSlice(0),b.getSlice(0)),fn(a.getSlice(1),b.getSlice(1)),fn(a.getSlice(2),b.getSlice(2)),fn(a.getSlice(3),b.getSlice(3))); }
SIMD_VECTORMATH_FORCE_INLINE const Soa4Float max(const Soa4Float&a,const Soa4Float&b){return Soa4Float(std::fmax(a.getSlice(0),b.getSlice(0)),std::fmax(a.getSlice(1),b.getSlice(1)),std::fmax(a.getSlice(2),b.getSlice(2)),std::fmax(a.getSlice(3),b.getSlice(3)));}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Float min(const Soa4Float&a,const Soa4Float&b){return Soa4Float(std::fmin(a.getSlice(0),b.getSlice(0)),std::fmin(a.getSlice(1),b.getSlice(1)),std::fmin(a.getSlice(2),b.getSlice(2)),std::fmin(a.getSlice(3),b.getSlice(3)));}
FEMFX_FLOAT_SCALAR_BINARY(powf,std::pow)
SIMD_VECTORMATH_FORCE_INLINE const Soa4Float sqrtf(const Soa4Float&v){return Soa4Float(std::sqrt(v.getSlice(0)),std::sqrt(v.getSlice(1)),std::sqrt(v.getSlice(2)),std::sqrt(v.getSlice(3)));}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Float fabsf(const Soa4Float&v){return Soa4Float(std::fabs(v.getSlice(0)),std::fabs(v.getSlice(1)),std::fabs(v.getSlice(2)),std::fabs(v.getSlice(3)));}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Float acosf(const Soa4Float&v){return Soa4Float(std::acos(v.getSlice(0)),std::acos(v.getSlice(1)),std::acos(v.getSlice(2)),std::acos(v.getSlice(3)));}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Float tanf(const Soa4Float&v){return Soa4Float(std::tan(v.getSlice(0)),std::tan(v.getSlice(1)),std::tan(v.getSlice(2)),std::tan(v.getSlice(3)));}
SIMD_VECTORMATH_FORCE_INLINE void sincosf(const Soa4Float&v,Soa4Float*s,Soa4Float*c){for(int i=0;i<4;++i){float si,ci;si=std::sin(v.getSlice(i));ci=std::cos(v.getSlice(i));s->setSlice(i,si);c->setSlice(i,ci);}}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Float expf(const Soa4Float&v){return Soa4Float(std::exp(v.getSlice(0)),std::exp(v.getSlice(1)),std::exp(v.getSlice(2)),std::exp(v.getSlice(3)));}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Float logf(const Soa4Float&v){return Soa4Float(std::log(v.getSlice(0)),std::log(v.getSlice(1)),std::log(v.getSlice(2)),std::log(v.getSlice(3)));}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Float select(const Soa4Float&a,const Soa4Float&b,const Soa4Bool&sel){return Soa4Float(sel.getSlice(0)?b.getSlice(0):a.getSlice(0),sel.getSlice(1)?b.getSlice(1):a.getSlice(1),sel.getSlice(2)?b.getSlice(2):a.getSlice(2),sel.getSlice(3)?b.getSlice(3):a.getSlice(3));}
#undef FEMFX_FLOAT_SCALAR_BINARY

SIMD_VECTORMATH_FORCE_INLINE const Soa4Int operator*(const Soa4Int&a,const Soa4Int&b){Soa4Int r=a;return r*=b;}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Int operator+(const Soa4Int&a,const Soa4Int&b){Soa4Int r=a;return r+=b;}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Int operator-(const Soa4Int&a,const Soa4Int&b){Soa4Int r=a;return r-=b;}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Int operator&(const Soa4Int&a,const Soa4Int&b){Soa4Int r=a;return r&=b;}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Int operator^(const Soa4Int&a,const Soa4Int&b){Soa4Int r=a;return r^=b;}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Int operator|(const Soa4Int&a,const Soa4Int&b){Soa4Int r=a;return r|=b;}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Int select(const Soa4Int&a,const Soa4Int&b,const Soa4Bool&sel){return Soa4Int(sel.getSlice(0)?b.getSlice(0):a.getSlice(0),sel.getSlice(1)?b.getSlice(1):a.getSlice(1),sel.getSlice(2)?b.getSlice(2):a.getSlice(2),sel.getSlice(3)?b.getSlice(3):a.getSlice(3));}

SIMD_VECTORMATH_FORCE_INLINE const Soa4Uint operator*(const Soa4Uint&a,const Soa4Uint&b){Soa4Uint r=a;for(int i=0;i<4;++i)r.setSlice(i,a.getSlice(i)*b.getSlice(i));return r;}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Uint operator+(const Soa4Uint&a,const Soa4Uint&b){Soa4Uint r=a;return r+=b;}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Uint operator-(const Soa4Uint&a,const Soa4Uint&b){Soa4Uint r=a;return r-=b;}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Uint operator&(const Soa4Uint&a,const Soa4Uint&b){Soa4Uint r=a;return r&=b;}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Uint operator^(const Soa4Uint&a,const Soa4Uint&b){Soa4Uint r=a;return r^=b;}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Uint operator|(const Soa4Uint&a,const Soa4Uint&b){Soa4Uint r=a;return r|=b;}
SIMD_VECTORMATH_FORCE_INLINE const Soa4Uint select(const Soa4Uint&a,const Soa4Uint&b,const Soa4Bool&sel){return Soa4Uint(sel.getSlice(0)?b.getSlice(0):a.getSlice(0),sel.getSlice(1)?b.getSlice(1):a.getSlice(1),sel.getSlice(2)?b.getSlice(2):a.getSlice(2),sel.getSlice(3)?b.getSlice(3):a.getSlice(3));}

} // namespace
#endif
