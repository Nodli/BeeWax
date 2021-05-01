#ifndef H_TYPE
#define H_TYPE

u32 float_mantissa(float f);
u32 float_exponent(float f);
u32 float_sign(float f);

bool is_infinite(float f);
bool is_nan(float f);
float measure_precision(float f);

// NOTE(hugo): ULP stands for Units in the Last Place ie bitwise difference
bool almost_equal(float fA, float fB, float delta_absolute, u32 delta_ULP);

u64 double_mantissa(double d);
u64 double_exponent(double d);
u64 double_sign(double d);

bool is_infinite(double d);
bool is_nan(double d);
double measure_precision(double d);

// NOTE(hugo): ULP stands for Units in the Last Place ie bitwise difference
bool almost_equal(double dA, double dB, double delta_absolute, u64 delta_ULP);

#endif
