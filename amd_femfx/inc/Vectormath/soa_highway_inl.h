// This file is implementation detail included only by soa_highway.h.
#ifndef FEMFX_SOA_HIGHWAY_H
#error "soa_highway_inl.h must be included by soa_highway.h"
#endif

#define FEMFX_SOA_COPY_LANES(To, From, Convert)                \
    do { for (unsigned i = 0; i < Count; ++i)                  \
        (To)[i] = Convert((From)[i]); } while (0)

template <unsigned Count>
SoaHwyBool<Count>::SoaHwyBool(const SoaHwyBool& v)
{ FEMFX_SOA_COPY_LANES(mData, v.mData, static_cast<std::uint8_t>); }
template <unsigned Count>
SoaHwyBool<Count>::SoaHwyBool(bool scalar)
{ for (unsigned i = 0; i < Count; ++i) mData[i] = scalar; }
template <unsigned Count>
template <typename... T>
SoaHwyBool<Count>::SoaHwyBool(bool first, T... rest)
{
    static_assert(sizeof...(T) + 1 == Count, "wrong SoA lane count");
    mData[0] = first;
    set<1>(rest...);
}
template <unsigned Count>
SoaHwyBool<Count>::SoaHwyBool(const SoaHwyFloat<Count>& v)
{ for (unsigned i = 0; i < Count; ++i) mData[i] = v.getSlice(i) != 0.0f; }
template <unsigned Count>
SoaHwyBool<Count>::SoaHwyBool(const SoaHwyInt<Count>& v)
{ for (unsigned i = 0; i < Count; ++i) mData[i] = v.getSlice(i) != 0; }
template <unsigned Count>
SoaHwyBool<Count>::SoaHwyBool(const SoaHwyUint<Count>& v)
{ for (unsigned i = 0; i < Count; ++i) mData[i] = v.getSlice(i) != 0; }
template <unsigned Count> SoaHwyBool<Count>& SoaHwyBool<Count>::operator=(const SoaHwyBool& v)
{ FEMFX_SOA_COPY_LANES(mData, v.mData, static_cast<std::uint8_t>); return *this; }
template <unsigned Count> const SoaHwyBool<Count> SoaHwyBool<Count>::operator!() const
{ SoaHwyBool r; AMD::FmHwy::BoolNot(mData, Count, r.mData); return r; }
template <unsigned Count> SoaHwyBool<Count>& SoaHwyBool<Count>::operator&=(const SoaHwyBool& v)
{ AMD::FmHwy::BoolBinary(mData, v.mData, Count, AMD::FmHwy::BinaryAnd, mData); return *this; }
template <unsigned Count> SoaHwyBool<Count>& SoaHwyBool<Count>::operator^=(const SoaHwyBool& v)
{ AMD::FmHwy::BoolBinary(mData, v.mData, Count, AMD::FmHwy::BinaryXor, mData); return *this; }
template <unsigned Count> SoaHwyBool<Count>& SoaHwyBool<Count>::operator|=(const SoaHwyBool& v)
{ AMD::FmHwy::BoolBinary(mData, v.mData, Count, AMD::FmHwy::BinaryOr, mData); return *this; }

template <unsigned Count> SoaHwyFloat<Count>::SoaHwyFloat(const SoaHwyFloat& v)
{ FEMFX_SOA_COPY_LANES(mData, v.mData, float); }
template <unsigned Count> SoaHwyFloat<Count>::SoaHwyFloat(const float* v)
{ for (unsigned i = 0; i < Count; ++i) mData[i] = v[i]; }
template <unsigned Count> SoaHwyFloat<Count>::SoaHwyFloat(float v)
{ for (unsigned i = 0; i < Count; ++i) mData[i] = v; }
template <unsigned Count> template <typename... T>
SoaHwyFloat<Count>::SoaHwyFloat(float first, float second, float third, T... rest)
{
    static_assert(sizeof...(T) + 3 == Count, "wrong SoA lane count");
    const float values[] = {first, second, third, static_cast<float>(rest)...};
    for (unsigned i = 0; i < Count; ++i) mData[i] = values[i];
}
template <unsigned Count> void SoaHwyFloat<Count>::setFromAligned(float* v)
{ for (unsigned i = 0; i < Count; ++i) mData[i] = v[i]; }
template <unsigned Count> SoaHwyFloat<Count>& SoaHwyFloat<Count>::operator=(const SoaHwyFloat& v)
{ FEMFX_SOA_COPY_LANES(mData, v.mData, float); return *this; }
template <unsigned Count> const SoaHwyFloat<Count> SoaHwyFloat<Count>::operator-() const
{ SoaHwyFloat r; AMD::FmHwy::FloatUnary(mData, Count, AMD::FmHwy::FloatNegate, r.mData); return r; }
template <unsigned Count> SoaHwyFloat<Count>& SoaHwyFloat<Count>::operator++()
{ AMD::FmHwy::FloatBinary(mData, SoaHwyFloat(1.0f).mData, Count, AMD::FmHwy::BinaryAdd, mData); return *this; }
template <unsigned Count> SoaHwyFloat<Count>& SoaHwyFloat<Count>::operator--()
{ AMD::FmHwy::FloatBinary(mData, SoaHwyFloat(1.0f).mData, Count, AMD::FmHwy::BinarySub, mData); return *this; }
template <unsigned Count> const SoaHwyFloat<Count> SoaHwyFloat<Count>::operator++(int)
{ SoaHwyFloat r = *this; ++*this; return r; }
template <unsigned Count> const SoaHwyFloat<Count> SoaHwyFloat<Count>::operator--(int)
{ SoaHwyFloat r = *this; --*this; return r; }
#define FEMFX_FLOAT_ASSIGN(Op, HOp) \
template <unsigned Count> SoaHwyFloat<Count>& SoaHwyFloat<Count>::operator Op(const SoaHwyFloat& v) \
{ AMD::FmHwy::FloatBinary(mData, v.mData, Count, AMD::FmHwy::HOp, mData); return *this; }
FEMFX_FLOAT_ASSIGN(*=, BinaryMul)
FEMFX_FLOAT_ASSIGN(/=, BinaryDiv)
FEMFX_FLOAT_ASSIGN(+=, BinaryAdd)
FEMFX_FLOAT_ASSIGN(-=, BinarySub)
#undef FEMFX_FLOAT_ASSIGN

template <unsigned Count> SoaHwyInt<Count>::SoaHwyInt(const SoaHwyInt& v)
{ FEMFX_SOA_COPY_LANES(mData, v.mData, static_cast<std::int32_t>); }
template <unsigned Count> SoaHwyInt<Count>::SoaHwyInt(const std::int32_t* v)
{ for (unsigned i = 0; i < Count; ++i) mData[i] = v[i]; }
template <unsigned Count> SoaHwyInt<Count>::SoaHwyInt(std::int32_t v)
{ for (unsigned i = 0; i < Count; ++i) mData[i] = v; }
template <unsigned Count> template <typename... T>
SoaHwyInt<Count>::SoaHwyInt(std::int32_t first, std::int32_t second,
                            std::int32_t third, T... rest)
{
    static_assert(sizeof...(T) + 3 == Count, "wrong SoA lane count");
    const std::int32_t values[] = {first, second, third, static_cast<std::int32_t>(rest)...};
    for (unsigned i = 0; i < Count; ++i) mData[i] = values[i];
}
template <unsigned Count> SoaHwyInt<Count>::SoaHwyInt(const SoaHwyBool<Count>& v)
{ for (unsigned i = 0; i < Count; ++i) mData[i] = v.getSlice(i) ? -1 : 0; }
template <unsigned Count> SoaHwyInt<Count>::SoaHwyInt(const SoaHwyUint<Count>& v)
{ for (unsigned i = 0; i < Count; ++i) mData[i] = static_cast<std::int32_t>(v.getSlice(i)); }
template <unsigned Count> SoaHwyInt<Count>& SoaHwyInt<Count>::operator=(const SoaHwyInt& v)
{ FEMFX_SOA_COPY_LANES(mData, v.mData, static_cast<std::int32_t>); return *this; }
template <unsigned Count> const SoaHwyInt<Count> SoaHwyInt<Count>::operator-() const
{ SoaHwyInt r; AMD::FmHwy::IntBinary(SoaHwyInt(0).mData, mData, Count, AMD::FmHwy::BinarySub, r.mData); return r; }
template <unsigned Count> SoaHwyInt<Count>& SoaHwyInt<Count>::operator++()
{ AMD::FmHwy::IntBinary(mData, SoaHwyInt(1).mData, Count, AMD::FmHwy::BinaryAdd, mData); return *this; }
template <unsigned Count> SoaHwyInt<Count>& SoaHwyInt<Count>::operator--()
{ AMD::FmHwy::IntBinary(mData, SoaHwyInt(1).mData, Count, AMD::FmHwy::BinarySub, mData); return *this; }
template <unsigned Count> const SoaHwyInt<Count> SoaHwyInt<Count>::operator++(int)
{ SoaHwyInt r = *this; ++*this; return r; }
template <unsigned Count> const SoaHwyInt<Count> SoaHwyInt<Count>::operator--(int)
{ SoaHwyInt r = *this; --*this; return r; }
#define FEMFX_INT_ASSIGN(Op, HOp) \
template <unsigned Count> SoaHwyInt<Count>& SoaHwyInt<Count>::operator Op(const SoaHwyInt& v) \
{ AMD::FmHwy::IntBinary(mData, v.mData, Count, AMD::FmHwy::HOp, mData); return *this; }
FEMFX_INT_ASSIGN(*=, BinaryMul)
FEMFX_INT_ASSIGN(+=, BinaryAdd)
FEMFX_INT_ASSIGN(-=, BinarySub)
FEMFX_INT_ASSIGN(&=, BinaryAnd)
FEMFX_INT_ASSIGN(^=, BinaryXor)
FEMFX_INT_ASSIGN(|=, BinaryOr)
#undef FEMFX_INT_ASSIGN

template <unsigned Count> SoaHwyUint<Count>::SoaHwyUint(const SoaHwyUint& v)
{ FEMFX_SOA_COPY_LANES(mData, v.mData, static_cast<std::uint32_t>); }
template <unsigned Count> SoaHwyUint<Count>::SoaHwyUint(const std::uint32_t* v)
{ for (unsigned i = 0; i < Count; ++i) mData[i] = v[i]; }
template <unsigned Count> SoaHwyUint<Count>::SoaHwyUint(std::uint32_t v)
{ for (unsigned i = 0; i < Count; ++i) mData[i] = v; }
template <unsigned Count> SoaHwyUint<Count>::SoaHwyUint(std::int32_t v)
{ for (unsigned i = 0; i < Count; ++i) mData[i] = static_cast<std::uint32_t>(v); }
template <unsigned Count> template <typename... T>
SoaHwyUint<Count>::SoaHwyUint(std::uint32_t first, std::uint32_t second,
                              std::uint32_t third, T... rest)
{
    static_assert(sizeof...(T) + 3 == Count, "wrong SoA lane count");
    const std::uint32_t values[] = {first, second, third, static_cast<std::uint32_t>(rest)...};
    for (unsigned i = 0; i < Count; ++i) mData[i] = values[i];
}
template <unsigned Count> SoaHwyUint<Count>::SoaHwyUint(const SoaHwyBool<Count>& v)
{ for (unsigned i = 0; i < Count; ++i) mData[i] = v.getSlice(i) ? ~std::uint32_t(0) : 0; }
template <unsigned Count> SoaHwyUint<Count>::SoaHwyUint(const SoaHwyInt<Count>& v)
{ for (unsigned i = 0; i < Count; ++i) mData[i] = static_cast<std::uint32_t>(v.getSlice(i)); }
template <unsigned Count> SoaHwyUint<Count>& SoaHwyUint<Count>::operator=(const SoaHwyUint& v)
{ FEMFX_SOA_COPY_LANES(mData, v.mData, static_cast<std::uint32_t>); return *this; }
template <unsigned Count> const SoaHwyInt<Count> SoaHwyUint<Count>::operator-() const
{ return -SoaHwyInt<Count>(*this); }
template <unsigned Count> SoaHwyUint<Count>& SoaHwyUint<Count>::operator++()
{ AMD::FmHwy::UintBinary(mData, SoaHwyUint(1u).mData, Count, AMD::FmHwy::BinaryAdd, mData); return *this; }
template <unsigned Count> SoaHwyUint<Count>& SoaHwyUint<Count>::operator--()
{ AMD::FmHwy::UintBinary(mData, SoaHwyUint(1u).mData, Count, AMD::FmHwy::BinarySub, mData); return *this; }
template <unsigned Count> const SoaHwyUint<Count> SoaHwyUint<Count>::operator++(int)
{ SoaHwyUint r = *this; ++*this; return r; }
template <unsigned Count> const SoaHwyUint<Count> SoaHwyUint<Count>::operator--(int)
{ SoaHwyUint r = *this; --*this; return r; }
#define FEMFX_UINT_ASSIGN(Op, HOp) \
template <unsigned Count> SoaHwyUint<Count>& SoaHwyUint<Count>::operator Op(const SoaHwyUint& v) \
{ AMD::FmHwy::UintBinary(mData, v.mData, Count, AMD::FmHwy::HOp, mData); return *this; }
FEMFX_UINT_ASSIGN(*=, BinaryMul)
FEMFX_UINT_ASSIGN(+=, BinaryAdd)
FEMFX_UINT_ASSIGN(-=, BinarySub)
FEMFX_UINT_ASSIGN(&=, BinaryAnd)
FEMFX_UINT_ASSIGN(^=, BinaryXor)
FEMFX_UINT_ASSIGN(|=, BinaryOr)
#undef FEMFX_UINT_ASSIGN

#define FEMFX_DEFINE_FLOAT_BINARY(Name, HOp) \
template <unsigned Count> const SoaHwyFloat<Count> Name(const SoaHwyFloat<Count>& a, const SoaHwyFloat<Count>& b) \
{ SoaHwyFloat<Count> r; AMD::FmHwy::FloatBinary(a.lanes(), b.lanes(), Count, AMD::FmHwy::HOp, r.lanes()); return r; }
FEMFX_DEFINE_FLOAT_BINARY(operator+, BinaryAdd)
FEMFX_DEFINE_FLOAT_BINARY(operator-, BinarySub)
FEMFX_DEFINE_FLOAT_BINARY(operator*, BinaryMul)
FEMFX_DEFINE_FLOAT_BINARY(operator/, BinaryDiv)
FEMFX_DEFINE_FLOAT_BINARY(max, BinaryMax)
FEMFX_DEFINE_FLOAT_BINARY(min, BinaryMin)
FEMFX_DEFINE_FLOAT_BINARY(powf, BinaryPow)
#undef FEMFX_DEFINE_FLOAT_BINARY

template <unsigned Count> const SoaHwyFloat<Count> operator+(
    const SoaHwyFloat<Count>& a, float b) { return a + SoaHwyFloat<Count>(b); }
template <unsigned Count> const SoaHwyFloat<Count> operator+(
    float a, const SoaHwyFloat<Count>& b) { return SoaHwyFloat<Count>(a) + b; }
template <unsigned Count> const SoaHwyFloat<Count> operator-(
    const SoaHwyFloat<Count>& a, float b) { return a - SoaHwyFloat<Count>(b); }
template <unsigned Count> const SoaHwyFloat<Count> operator-(
    float a, const SoaHwyFloat<Count>& b) { return SoaHwyFloat<Count>(a) - b; }
template <unsigned Count> const SoaHwyFloat<Count> operator*(
    const SoaHwyFloat<Count>& a, float b) { return a * SoaHwyFloat<Count>(b); }
template <unsigned Count> const SoaHwyFloat<Count> operator*(
    float a, const SoaHwyFloat<Count>& b) { return SoaHwyFloat<Count>(a) * b; }
template <unsigned Count> const SoaHwyFloat<Count> operator/(
    const SoaHwyFloat<Count>& a, float b) { return a / SoaHwyFloat<Count>(b); }
template <unsigned Count> const SoaHwyFloat<Count> operator/(
    float a, const SoaHwyFloat<Count>& b) { return SoaHwyFloat<Count>(a) / b; }
template <unsigned Count> const SoaHwyFloat<Count> max(
    const SoaHwyFloat<Count>& a, float b) { return max(a, SoaHwyFloat<Count>(b)); }
template <unsigned Count> const SoaHwyFloat<Count> max(
    float a, const SoaHwyFloat<Count>& b) { return max(SoaHwyFloat<Count>(a), b); }
template <unsigned Count> const SoaHwyFloat<Count> min(
    const SoaHwyFloat<Count>& a, float b) { return min(a, SoaHwyFloat<Count>(b)); }
template <unsigned Count> const SoaHwyFloat<Count> min(
    float a, const SoaHwyFloat<Count>& b) { return min(SoaHwyFloat<Count>(a), b); }
template <unsigned Count> const SoaHwyFloat<Count> powf(
    const SoaHwyFloat<Count>& a, float b) { return powf(a, SoaHwyFloat<Count>(b)); }

#define FEMFX_DEFINE_FLOAT_UNARY(Name, HOp) \
template <unsigned Count> const SoaHwyFloat<Count> Name(const SoaHwyFloat<Count>& v) \
{ SoaHwyFloat<Count> r; AMD::FmHwy::FloatUnary(v.lanes(), Count, AMD::FmHwy::HOp, r.lanes()); return r; }
FEMFX_DEFINE_FLOAT_UNARY(sqrtf, FloatSqrt)
FEMFX_DEFINE_FLOAT_UNARY(fabsf, FloatAbs)
FEMFX_DEFINE_FLOAT_UNARY(acosf, FloatAcos)
FEMFX_DEFINE_FLOAT_UNARY(tanf, FloatTan)
FEMFX_DEFINE_FLOAT_UNARY(expf, FloatExp)
FEMFX_DEFINE_FLOAT_UNARY(logf, FloatLog)
#undef FEMFX_DEFINE_FLOAT_UNARY

template <unsigned Count> void sincosf(const SoaHwyFloat<Count>& v,
                                       SoaHwyFloat<Count>* s,
                                       SoaHwyFloat<Count>* c)
{ AMD::FmHwy::FloatSinCos(v.lanes(), Count, s->lanes(), c->lanes()); }

#define FEMFX_DEFINE_COMPARE(Type, HCompareFn)                                  \
template <unsigned Count> const SoaHwyBool<Count> operator<(const Type& a, const Type& b)  \
{ SoaHwyBool<Count> r; AMD::FmHwy::HCompareFn(a.lanes(), b.lanes(), Count, AMD::FmHwy::CompareLT, r.lanes()); return r; } \
template <unsigned Count> const SoaHwyBool<Count> operator<=(const Type& a, const Type& b) \
{ SoaHwyBool<Count> r; AMD::FmHwy::HCompareFn(a.lanes(), b.lanes(), Count, AMD::FmHwy::CompareLE, r.lanes()); return r; } \
template <unsigned Count> const SoaHwyBool<Count> operator>(const Type& a, const Type& b)  \
{ SoaHwyBool<Count> r; AMD::FmHwy::HCompareFn(a.lanes(), b.lanes(), Count, AMD::FmHwy::CompareGT, r.lanes()); return r; } \
template <unsigned Count> const SoaHwyBool<Count> operator>=(const Type& a, const Type& b) \
{ SoaHwyBool<Count> r; AMD::FmHwy::HCompareFn(a.lanes(), b.lanes(), Count, AMD::FmHwy::CompareGE, r.lanes()); return r; } \
template <unsigned Count> const SoaHwyBool<Count> operator==(const Type& a, const Type& b) \
{ SoaHwyBool<Count> r; AMD::FmHwy::HCompareFn(a.lanes(), b.lanes(), Count, AMD::FmHwy::CompareEQ, r.lanes()); return r; } \
template <unsigned Count> const SoaHwyBool<Count> operator!=(const Type& a, const Type& b) \
{ SoaHwyBool<Count> r; AMD::FmHwy::HCompareFn(a.lanes(), b.lanes(), Count, AMD::FmHwy::CompareNE, r.lanes()); return r; }
FEMFX_DEFINE_COMPARE(SoaHwyFloat<Count>, FloatCompare)
FEMFX_DEFINE_COMPARE(SoaHwyInt<Count>, IntCompare)
FEMFX_DEFINE_COMPARE(SoaHwyUint<Count>, UintCompare)
#undef FEMFX_DEFINE_COMPARE

#define FEMFX_FLOAT_SCALAR_COMPARE(Op) \
template <unsigned Count> const SoaHwyBool<Count> operator Op( \
    const SoaHwyFloat<Count>& a, float b) { return a Op SoaHwyFloat<Count>(b); } \
template <unsigned Count> const SoaHwyBool<Count> operator Op( \
    float a, const SoaHwyFloat<Count>& b) { return SoaHwyFloat<Count>(a) Op b; }
FEMFX_FLOAT_SCALAR_COMPARE(<)
FEMFX_FLOAT_SCALAR_COMPARE(<=)
FEMFX_FLOAT_SCALAR_COMPARE(>)
FEMFX_FLOAT_SCALAR_COMPARE(>=)
FEMFX_FLOAT_SCALAR_COMPARE(==)
FEMFX_FLOAT_SCALAR_COMPARE(!=)
#undef FEMFX_FLOAT_SCALAR_COMPARE

#define FEMFX_DEFINE_INT_BINARY(Type, HFn, Name, HOp) \
template <unsigned Count> const Type Name(const Type& a, const Type& b) \
{ Type r; AMD::FmHwy::HFn(a.lanes(), b.lanes(), Count, AMD::FmHwy::HOp, r.lanes()); return r; }
FEMFX_DEFINE_INT_BINARY(SoaHwyInt<Count>, IntBinary, operator+, BinaryAdd)
FEMFX_DEFINE_INT_BINARY(SoaHwyInt<Count>, IntBinary, operator-, BinarySub)
FEMFX_DEFINE_INT_BINARY(SoaHwyInt<Count>, IntBinary, operator*, BinaryMul)
FEMFX_DEFINE_INT_BINARY(SoaHwyInt<Count>, IntBinary, operator&, BinaryAnd)
FEMFX_DEFINE_INT_BINARY(SoaHwyInt<Count>, IntBinary, operator^, BinaryXor)
FEMFX_DEFINE_INT_BINARY(SoaHwyInt<Count>, IntBinary, operator|, BinaryOr)
FEMFX_DEFINE_INT_BINARY(SoaHwyUint<Count>, UintBinary, operator+, BinaryAdd)
FEMFX_DEFINE_INT_BINARY(SoaHwyUint<Count>, UintBinary, operator-, BinarySub)
FEMFX_DEFINE_INT_BINARY(SoaHwyUint<Count>, UintBinary, operator*, BinaryMul)
FEMFX_DEFINE_INT_BINARY(SoaHwyUint<Count>, UintBinary, operator&, BinaryAnd)
FEMFX_DEFINE_INT_BINARY(SoaHwyUint<Count>, UintBinary, operator^, BinaryXor)
FEMFX_DEFINE_INT_BINARY(SoaHwyUint<Count>, UintBinary, operator|, BinaryOr)
#undef FEMFX_DEFINE_INT_BINARY

#define FEMFX_INT_SCALAR_BINARY(Type, Lane, Op) \
template <unsigned Count> const Type operator Op(const Type& a, Lane b) \
{ return a Op Type(b); } \
template <unsigned Count> const Type operator Op(Lane a, const Type& b) \
{ return Type(a) Op b; }
FEMFX_INT_SCALAR_BINARY(SoaHwyInt<Count>, std::int32_t, +)
FEMFX_INT_SCALAR_BINARY(SoaHwyInt<Count>, std::int32_t, -)
FEMFX_INT_SCALAR_BINARY(SoaHwyInt<Count>, std::int32_t, *)
FEMFX_INT_SCALAR_BINARY(SoaHwyUint<Count>, std::uint32_t, +)
FEMFX_INT_SCALAR_BINARY(SoaHwyUint<Count>, std::uint32_t, -)
FEMFX_INT_SCALAR_BINARY(SoaHwyUint<Count>, std::uint32_t, *)
#undef FEMFX_INT_SCALAR_BINARY

#define FEMFX_DEFINE_BOOL_BINARY(Name, HOp) \
template <unsigned Count> const SoaHwyBool<Count> Name(const SoaHwyBool<Count>& a, const SoaHwyBool<Count>& b) \
{ SoaHwyBool<Count> r; AMD::FmHwy::BoolBinary(a.lanes(), b.lanes(), Count, AMD::FmHwy::HOp, r.lanes()); return r; }
FEMFX_DEFINE_BOOL_BINARY(operator&, BinaryAnd)
FEMFX_DEFINE_BOOL_BINARY(operator^, BinaryXor)
FEMFX_DEFINE_BOOL_BINARY(operator|, BinaryOr)
#undef FEMFX_DEFINE_BOOL_BINARY
template <unsigned Count> const SoaHwyBool<Count> operator==(const SoaHwyBool<Count>& a,
                                                            const SoaHwyBool<Count>& b)
{ return !(a ^ b); }
template <unsigned Count> const SoaHwyBool<Count> operator!=(const SoaHwyBool<Count>& a,
                                                            const SoaHwyBool<Count>& b)
{ return a ^ b; }

template <unsigned Count> const SoaHwyBool<Count> select(const SoaHwyBool<Count>& a,
                                                         const SoaHwyBool<Count>& b,
                                                         const SoaHwyBool<Count>& mask)
{ SoaHwyBool<Count> r; AMD::FmHwy::BoolSelect(a.lanes(), b.lanes(), mask.lanes(), Count, r.lanes()); return r; }
template <unsigned Count> const SoaHwyFloat<Count> select(const SoaHwyFloat<Count>& a,
                                                          const SoaHwyFloat<Count>& b,
                                                          const SoaHwyBool<Count>& mask)
{ SoaHwyFloat<Count> r; AMD::FmHwy::FloatSelect(a.lanes(), b.lanes(), mask.lanes(), Count, r.lanes()); return r; }
template <unsigned Count> const SoaHwyFloat<Count> select(const SoaHwyFloat<Count>& a,
                                                          float b,
                                                          const SoaHwyBool<Count>& mask)
{ return select(a, SoaHwyFloat<Count>(b), mask); }
template <unsigned Count> const SoaHwyFloat<Count> select(float a,
                                                          const SoaHwyFloat<Count>& b,
                                                          const SoaHwyBool<Count>& mask)
{ return select(SoaHwyFloat<Count>(a), b, mask); }
template <unsigned Count> const SoaHwyInt<Count> select(const SoaHwyInt<Count>& a,
                                                        const SoaHwyInt<Count>& b,
                                                        const SoaHwyBool<Count>& mask)
{ SoaHwyInt<Count> r; AMD::FmHwy::IntSelect(a.lanes(), b.lanes(), mask.lanes(), Count, r.lanes()); return r; }
template <unsigned Count> const SoaHwyUint<Count> select(const SoaHwyUint<Count>& a,
                                                         const SoaHwyUint<Count>& b,
                                                         const SoaHwyBool<Count>& mask)
{ SoaHwyUint<Count> r; AMD::FmHwy::UintSelect(a.lanes(), b.lanes(), mask.lanes(), Count, r.lanes()); return r; }
template <unsigned Count> bool all(const SoaHwyBool<Count>& v)
{ return AMD::FmHwy::BoolAll(v.lanes(), Count); }
template <unsigned Count> bool any(const SoaHwyBool<Count>& v)
{ return AMD::FmHwy::BoolAny(v.lanes(), Count); }

#define FEMFX_DEFINE_PRINT(Type, Format, Access) \
template <unsigned Count> void print(const Type& v) \
{ std::printf("["); for (unsigned i=0; i<Count; ++i) std::printf(Format, static_cast<int>(v.Access(i))); std::printf("]\n"); } \
template <unsigned Count> void print(const Type& v, const char* name) \
{ std::printf("%s ", name); print(v); }
FEMFX_DEFINE_PRINT(SoaHwyFloat<Count>, " %g", getSlice)
FEMFX_DEFINE_PRINT(SoaHwyInt<Count>, " %d", getSlice)
FEMFX_DEFINE_PRINT(SoaHwyUint<Count>, " %u", getSlice)
FEMFX_DEFINE_PRINT(SoaHwyBool<Count>, " %d", getSlice)
#undef FEMFX_DEFINE_PRINT
#undef FEMFX_SOA_COPY_LANES
