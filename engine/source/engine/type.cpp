DISABLE_WARNING_PUSH
DISABLE_WARNING_TYPE_PUNNING

u32 float_mantissa(float f){
    u32* f_uint = (u32*)(&f);
    return (*f_uint & BEEWAX_INTERNAL::float_mask_mantissa);
}

u32 float_exponent(float f){
    u32* f_uint = (u32*)(&f);
    //return (*f_uint & BEEWAX_INTERNAL::float_mask_exponent) >> 23;
    return (*f_uint >> 23) & 0xFF;
}

u32 float_sign(float f){
    u32* f_uint = (u32*)(&f);
    //return (*f_uint & BEEWAX_INTERNAL::float_mask_sign) >> 31;
    return (*f_uint >> 31) & 0x01;
}

// NOTE(hugo): +inf or -inf
// * mantissa = 0
// * exponent = 0xFF = 255
// * sign = anything
bool is_infinite(float f){
    u32* f_uint = (u32*)(&f);
    return (*f_uint & BEEWAX_INTERNAL::float_mask_exponent_mantissa) == BEEWAX_INTERNAL::float_mask_exponent;
}

// NOTE(hugo): NaN
// * mantissa = non-zero
// * exponent = 0xFF = 255
// * sign = anything
bool is_nan(float f){
    u32* f_uint = (u32*)(&f);
    u32 f_exponent = *f_uint & BEEWAX_INTERNAL::float_mask_exponent;
    u32 f_mantissa = *f_uint & BEEWAX_INTERNAL::float_mask_mantissa;
    return (f_exponent == BEEWAX_INTERNAL::float_mask_exponent && f_mantissa != 0);
}

float measure_precision(float f){
    u32 f_uint = *(u32*)(&f);

    u32 f_uint_no_mantissa = f_uint & BEEWAX_INTERNAL::float_mask_sign_exponent;
    u32 f_uint_no_mantissa_next = f_uint_no_mantissa + 1u;

    float f_no_mantissa = *(float*)&f_uint_no_mantissa;
    float f_no_mantissa_next = *(float*)&f_uint_no_mantissa_next;

    return f_no_mantissa_next - f_no_mantissa;
}

bool almost_equal(float fA, float fB, float delta_absolute, u32 delta_ULP){
    assert(!is_infinite(fA) && !is_infinite(fB));
    assert(!is_nan(fA) && !is_nan(fB));
    assert(delta_absolute > 0.f);

    // NOTE(hugo): almost_equal_absolute
    float df = fabsf(fB - fA);
    if(df <= delta_absolute){
        return true;
    }

    // NOTE(hugo): almost_equal_ULP
    if(float_sign(fA) != float_sign(fB)){
        return false;
    }

    u32 ULP = (u32)(abs(*(s32*)(&fA) - *(s32*)(&fB)));
    if(ULP <= delta_ULP){
        return true;
    }else{
        return false;
    }

}

u64 double_mantissa(double d){
    u64* d_uint = (u64*)(&d);
    return (*d_uint & BEEWAX_INTERNAL::double_mask_mantissa);
}

u64 double_exponent(double d){
    u64* d_uint = (u64*)(&d);
    //return (*d_uint & BEEWAX_INTERNAL::double_mask_exponent) >> 52;
    return (*d_uint >> 52) & 0x7FF;
}

u64 double_sign(double d){
    u64* d_uint = (u64*)(&d);
    //return (*d_uint & BEEWAX_INTERNAL::double_mask_sign) >> 63;
    return (*d_uint >> 63) & 0x01;
}

// NOTE(hugo): +inf or -inf
// * mantissa = 0
// * exponent = 0x7FF = 2047
// * sign = anything
bool is_infinite(double d){
    u64* double_uint = (u64*)(&d);
    return (*double_uint & BEEWAX_INTERNAL::double_mask_exponent_mantissa) == BEEWAX_INTERNAL::double_mask_exponent;
}

// NOTE(hugo): NaN
// * mantissa = non-zero
// * exponent = 0x7doubledouble = 2047
// * sign = anything
bool is_nan(double d){
    u64* double_uint = (u64*)(&d);
    u64 double_exponent = *double_uint & BEEWAX_INTERNAL::double_mask_exponent;
    u64 double_mantissa = *double_uint & BEEWAX_INTERNAL::double_mask_mantissa;
    return (double_exponent == BEEWAX_INTERNAL::double_mask_exponent && double_mantissa != 0);
}

double measure_precision(double d){
    u64 d_uint = *(u64*)(&d);

    u64 d_uint_no_mantissa = d_uint & BEEWAX_INTERNAL::double_mask_sign_exponent;
    u64 d_uint_no_mantissa_next = d_uint_no_mantissa + 1u;

    double d_no_mantissa = *(double*)&d_uint_no_mantissa;
    double d_no_mantissa_next = *(double*)&d_uint_no_mantissa_next;

    return d_no_mantissa_next - d_no_mantissa;
}

bool almost_equal(double dA, double dB, double delta_absolute, u64 delta_ULP){
    assert(!is_infinite(dA) && !is_infinite(dB));
    assert(!is_nan(dA) && !is_nan(dB));
    assert(delta_absolute > 0.f);

    // NOTE(hugo): almost_equal_absolute
    double df = fabs(dB - dA);
    if(df <= delta_absolute){
        return true;
    }

    // NOTE(hugo): almost_equal_ULP
    if(double_sign(dA) != double_sign(dB)){
        return false;
    }

    u64 ULP = (u64)(abs(*(s64*)(&dA) - *(s64*)(&dB)));
    if(ULP <= delta_ULP){
        return true;
    }else{
        return false;
    }

}

DISABLE_WARNING_POP

