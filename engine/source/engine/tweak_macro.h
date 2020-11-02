#if 0


#ifndef H_TWEAK_MACRO
#define H_TWEAK_MACRO

#define TWEAK_VARIABLE_FOR_EACH(TWEAK_DECLARE_VARIABLE)         \
    TWEAK_DECLARE_VARIABLE(u32, temporary_u32, 5)               \
    TWEAK_DECLARE_VARIABLE(float, temporary_float, 5.f)         \

#define TWEAK_VARIABLE_AS_ENUM(variable_type, variable_name, variable_value) variable_name,

#define TWEAK_VARIABLE_AS_STORAGE_INIT(variable_type, variable_name, variable_value)    \
    BEEWAX_INTERNAL::tweak_storage[]Tweak_Entry(CONCATENATE(type_, variable_type), variable_value, #variable_name),     \

#define TWEAK_RETRIEVE_VARIABLE(variable_type, variable_name)                                                           \
    static_assert(BEEWAX_INTERNAL::tweak_storage[BEEWAX_INTERNAL::Tweak_Variables::variable_name].type == CONCATENATE(type_, variable_type));             \
    variable_type variable_name = tweak_storage[Tweak_Variables::variable_name].CONCATENATE(value_, variable_type)      \

namespace BEEWAX_INTERNAL{
    enum Tweak_Type{
        type_char,
        type_short,
        type_int,
        type_unsigned_int,
        type_float,
        type_double,

        type_s32 = type_int,
        type_u32 = type_unsigned_int,
        type_s64,
        type_u64,
        tweak_reserved_number_of_types
    };

    enum Tweak_Variables{
        TWEAK_VARIABLE_FOR_EACH(TWEAK_VARIABLE_AS_ENUM)
            tweak_reserved_number_of_variables
    };

    struct Tweak_Entry{
        Tweak_Type type;
        union{
            char    value_char;
            short   value_short;
            int     value_int;
            float   value_float;
            double  value_double;
            s32     value_s32;
            u32     value_u32;
            s64     value_s64;
            u64     value_u64;
        };
        char name[32];
    };

    static constexpr Tweak_Entry tweak_storage[tweak_reserved_number_of_variables];

}

void tweak_initialize(){
    TWEAK_VARIABLE_FOR_EACH(TWEAK_VARIABLE_AS_STORAGE_INIT);
}

#else

//#undef

#endif

#endif
