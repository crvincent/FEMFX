#ifndef FEMFX_SOA_HIGHWAY_H
#define FEMFX_SOA_HIGHWAY_H

#include <cstdint>
#include <cstdio>

#include "simd_utils.h"

#define SIMD_VECTORMATH_ALIGN16 SIMD_UTILS_ALIGN16
#define SIMD_VECTORMATH_FORCE_INLINE SIMD_UTILS_FORCE_INLINE

// Keep the old register conversion API available while callers migrate away
// from it.  It is intentionally confined to this Vectormath implementation.
#ifndef FEMFX_ENABLE_LEGACY_SOA_INTRINSICS
#define FEMFX_ENABLE_LEGACY_SOA_INTRINSICS 0
#endif

namespace AMD {
namespace FmHwy {

enum FloatOp
{
    FloatNegate,
    FloatSqrt,
    FloatAbs,
    FloatAcos,
    FloatTan,
    FloatExp,
    FloatLog
};

enum BinaryOp
{
    BinaryAdd,
    BinarySub,
    BinaryMul,
    BinaryDiv,
    BinaryMin,
    BinaryMax,
    BinaryAnd,
    BinaryOr,
    BinaryXor,
    BinaryPow
};

enum CompareOp
{
    CompareLT,
    CompareLE,
    CompareGT,
    CompareGE,
    CompareEQ,
    CompareNE
};

void FloatUnary(const float* input, unsigned count, FloatOp op, float* output);
void FloatBinary(const float* lhs, const float* rhs, unsigned count,
                 BinaryOp op, float* output);
void FloatCompare(const float* lhs, const float* rhs, unsigned count,
                  CompareOp op, std::uint8_t* output);
void FloatSelect(const float* lhs, const float* rhs,
                 const std::uint8_t* select, unsigned count, float* output);
void FloatSinCos(const float* input, unsigned count, float* sinOutput,
                 float* cosOutput);

void IntBinary(const std::int32_t* lhs, const std::int32_t* rhs,
               unsigned count, BinaryOp op, std::int32_t* output);
void IntCompare(const std::int32_t* lhs, const std::int32_t* rhs,
                unsigned count, CompareOp op, std::uint8_t* output);
void IntSelect(const std::int32_t* lhs, const std::int32_t* rhs,
               const std::uint8_t* select, unsigned count,
               std::int32_t* output);

void UintBinary(const std::uint32_t* lhs, const std::uint32_t* rhs,
                unsigned count, BinaryOp op, std::uint32_t* output);
void UintCompare(const std::uint32_t* lhs, const std::uint32_t* rhs,
                 unsigned count, CompareOp op, std::uint8_t* output);
void UintSelect(const std::uint32_t* lhs, const std::uint32_t* rhs,
                const std::uint8_t* select, unsigned count,
                std::uint32_t* output);

void BoolBinary(const std::uint8_t* lhs, const std::uint8_t* rhs,
                unsigned count, BinaryOp op, std::uint8_t* output);
void BoolNot(const std::uint8_t* input, unsigned count,
             std::uint8_t* output);
void BoolSelect(const std::uint8_t* lhs, const std::uint8_t* rhs,
                const std::uint8_t* select, unsigned count,
                std::uint8_t* output);
bool BoolAll(const std::uint8_t* input, unsigned count);
bool BoolAny(const std::uint8_t* input, unsigned count);

}  // namespace FmHwy
}  // namespace AMD

namespace FmVectormath {

template <unsigned Count>
class SoaHwyFloat;
template <unsigned Count>
class SoaHwyInt;
template <unsigned Count>
class SoaHwyUint;
template <unsigned Count>
class SoaHwyBool;

#define FEMFX_DECLARE_SOA_ASSOCIATES(FloatType, IntType, UintType, BoolType) \
    typedef BoolType SoaBool;                                                \
    typedef IntType SoaInt;                                                  \
    typedef UintType SoaUint;                                                \
    typedef FloatType SoaFloat

template <unsigned Count>
class SoaHwyBool
{
    std::uint8_t mData[Count];

public:
    static const std::uint32_t width = Count;
    FEMFX_DECLARE_SOA_ASSOCIATES(SoaHwyFloat<Count>, SoaHwyInt<Count>,
                                  SoaHwyUint<Count>, SoaHwyBool<Count>);

    SoaHwyBool() {}
    SoaHwyBool(const SoaHwyBool& other);
    explicit SoaHwyBool(bool scalar);
    explicit SoaHwyBool(const SoaHwyFloat<Count>& value);
    explicit SoaHwyBool(const SoaHwyInt<Count>& value);
    explicit SoaHwyBool(const SoaHwyUint<Count>& value);
    template <typename... T> SoaHwyBool(bool first, T... rest);
    const std::uint8_t* lanes() const { return mData; }
    std::uint8_t* lanes() { return mData; }
    bool getSlice(std::uint32_t i) const { return mData[i % Count] != 0; }
    void setSlice(std::uint32_t i, bool value) { mData[i % Count] = value; }
    const SoaHwyBool operator!() const;
    SoaHwyBool& operator=(const SoaHwyBool& other);
    SoaHwyBool& operator&=(const SoaHwyBool& other);
    SoaHwyBool& operator^=(const SoaHwyBool& other);
    SoaHwyBool& operator|=(const SoaHwyBool& other);

private:
    template <unsigned I, typename... T>
    void set(bool value, T... rest)
    {
        mData[I] = value;
        set<I + 1>(rest...);
    }
    template <unsigned I> void set() { static_assert(I == Count, "lane count"); }
};

template <unsigned Count>
class SoaHwyFloat
{
    float mData[Count];

public:
    static const std::uint32_t width = Count;
    FEMFX_DECLARE_SOA_ASSOCIATES(SoaHwyFloat<Count>, SoaHwyInt<Count>,
                                  SoaHwyUint<Count>, SoaHwyBool<Count>);

    SoaHwyFloat() {}
    SoaHwyFloat(const SoaHwyFloat& other);
    explicit SoaHwyFloat(const float* values);
    SoaHwyFloat(float scalar);
    template <typename... T>
    explicit SoaHwyFloat(float first, float second, float third, T... rest);
    const float* lanes() const { return mData; }
    float* lanes() { return mData; }
    float getSlice(std::uint32_t i) const { return mData[i % Count]; }
    void setSlice(std::uint32_t i, float value) { mData[i % Count] = value; }
    void setFromAligned(float* values);
    const SoaHwyFloat operator++(int);
    const SoaHwyFloat operator--(int);
    SoaHwyFloat& operator++();
    SoaHwyFloat& operator--();
    const SoaHwyFloat operator-() const;
    SoaHwyFloat& operator=(const SoaHwyFloat& other);
    SoaHwyFloat& operator*=(const SoaHwyFloat& other);
    SoaHwyFloat& operator/=(const SoaHwyFloat& other);
    SoaHwyFloat& operator+=(const SoaHwyFloat& other);
    SoaHwyFloat& operator-=(const SoaHwyFloat& other);
};

template <unsigned Count>
class SoaHwyInt
{
    std::int32_t mData[Count];

public:
    static const std::uint32_t width = Count;
    FEMFX_DECLARE_SOA_ASSOCIATES(SoaHwyFloat<Count>, SoaHwyInt<Count>,
                                  SoaHwyUint<Count>, SoaHwyBool<Count>);

    SoaHwyInt() {}
    SoaHwyInt(const SoaHwyInt& other);
    explicit SoaHwyInt(const std::int32_t* values);
    SoaHwyInt(std::int32_t scalar);
    template <typename... T>
    explicit SoaHwyInt(std::int32_t first, std::int32_t second,
                       std::int32_t third, T... rest);
    SoaHwyInt(const SoaHwyBool<Count>& value);
    SoaHwyInt(const SoaHwyUint<Count>& value);
    const std::int32_t* lanes() const { return mData; }
    std::int32_t* lanes() { return mData; }
    std::int32_t getSlice(std::uint32_t i) const { return mData[i % Count]; }
    void setSlice(std::uint32_t i, std::int32_t value) { mData[i % Count] = value; }
    const SoaHwyInt operator++(int);
    const SoaHwyInt operator--(int);
    SoaHwyInt& operator++();
    SoaHwyInt& operator--();
    const SoaHwyInt operator-() const;
    SoaHwyInt& operator=(const SoaHwyInt& other);
    SoaHwyInt& operator*=(const SoaHwyInt& other);
    SoaHwyInt& operator+=(const SoaHwyInt& other);
    SoaHwyInt& operator-=(const SoaHwyInt& other);
    SoaHwyInt& operator&=(const SoaHwyInt& other);
    SoaHwyInt& operator^=(const SoaHwyInt& other);
    SoaHwyInt& operator|=(const SoaHwyInt& other);
};

template <unsigned Count>
class SoaHwyUint
{
    std::uint32_t mData[Count];

public:
    static const std::uint32_t width = Count;
    FEMFX_DECLARE_SOA_ASSOCIATES(SoaHwyFloat<Count>, SoaHwyInt<Count>,
                                  SoaHwyUint<Count>, SoaHwyBool<Count>);

    SoaHwyUint() {}
    SoaHwyUint(const SoaHwyUint& other);
    explicit SoaHwyUint(const std::uint32_t* values);
    SoaHwyUint(std::uint32_t scalar);
    SoaHwyUint(std::int32_t scalar);
    template <typename... T>
    explicit SoaHwyUint(std::uint32_t first, std::uint32_t second,
                        std::uint32_t third, T... rest);
    SoaHwyUint(const SoaHwyBool<Count>& value);
    SoaHwyUint(const SoaHwyInt<Count>& value);
    const std::uint32_t* lanes() const { return mData; }
    std::uint32_t* lanes() { return mData; }
    std::uint32_t getSlice(std::uint32_t i) const { return mData[i % Count]; }
    void setSlice(std::uint32_t i, std::uint32_t value) { mData[i % Count] = value; }
    const SoaHwyUint operator++(int);
    const SoaHwyUint operator--(int);
    SoaHwyUint& operator++();
    SoaHwyUint& operator--();
    const SoaHwyInt<Count> operator-() const;
    SoaHwyUint& operator=(const SoaHwyUint& other);
    SoaHwyUint& operator*=(const SoaHwyUint& other);
    SoaHwyUint& operator+=(const SoaHwyUint& other);
    SoaHwyUint& operator-=(const SoaHwyUint& other);
    SoaHwyUint& operator&=(const SoaHwyUint& other);
    SoaHwyUint& operator^=(const SoaHwyUint& other);
    SoaHwyUint& operator|=(const SoaHwyUint& other);
};

#undef FEMFX_DECLARE_SOA_ASSOCIATES

#include "soa_highway_inl.h"

typedef SoaHwyFloat<4> Soa4Float;
typedef SoaHwyInt<4> Soa4Int;
typedef SoaHwyUint<4> Soa4Uint;
typedef SoaHwyBool<4> Soa4Bool;
#if SIMD_UTILS_USE_AVX || defined(FEMFX_SIMD_RUNTIME)
typedef SoaHwyFloat<8> Soa8Float;
typedef SoaHwyInt<8> Soa8Int;
typedef SoaHwyUint<8> Soa8Uint;
typedef SoaHwyBool<8> Soa8Bool;
#endif

}  // namespace FmVectormath

#endif
