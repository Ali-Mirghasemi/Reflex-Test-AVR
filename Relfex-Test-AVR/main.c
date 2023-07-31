/*
 * Relfex-Test-AVR.c
 *
 * Created: 7/27/2023 7:46:05 PM
 * Author : MrCrazy
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include "UART.h"
#include "Reflex.h"

#if REFLEX_ARCH != REFLEX_ARCH_8BIT
	#error "Please set REFLEX_ARCH to 8"
#endif

#define PRINTLN						UART_puts
#define PRINTF						UART_printf
#define CONST_VAR_ATTR				

uint32_t assertResult = 0;
#define assert(TYPE, ...)			if((assertResult = Assert_ ##TYPE (__VA_ARGS__, __LINE__)) != 0) return assertResult

#define ARRAY_LEN(ARR)			    (sizeof(ARR) / sizeof(ARR[0]))

#define setValue(IDX, TYPE, VAL)    values[IDX].Type = Param_ValueType_ ##TYPE;\
                                    values[IDX].TYPE = VAL

typedef uint32_t Test_Result;
typedef Test_Result (*Test_Fn)(void);
typedef struct {
    const char*     Name;
    Test_Fn         fn;
} TestCase;

Test_Result Assert_Str(const char* str1, const char* str2, uint16_t line);
Test_Result Assert_Num(int32_t val1, int32_t val2, uint16_t line);
void Result_print(Test_Result result);
void printTitle(uint8_t idx, const char* name);
static const char FOOTER[] CONST_VAR_ATTR = "----------------------------------------------------------------";

#if REFLEX_FORMAT_MODE_PARAM
    Test_Result Test_Param(void);
#endif
#if REFLEX_FORMAT_MODE_PRIMARY
    Test_Result Test_Primary(void);
#endif
#if REFLEX_FORMAT_MODE_OFFSET
    Test_Result Test_ParamsOffset(void);
#endif
#if REFLEX_SUPPORT_CUSTOM_TYPE_PARAMS
    Test_Result Test_CustomTypeParams(void);
#endif
#if REFLEX_SUPPORT_COMPLEX_TYPE
    Test_Result Test_ComplexType(void);
#endif
Test_Result Test_Size(void);

#define TEST_CASE_INIT(NAME)        { .Name = #NAME, .fn = NAME }

static const TestCase Tests[] = {
#if REFLEX_FORMAT_MODE_PARAM
    TEST_CASE_INIT(Test_Param),
#endif
#if REFLEX_FORMAT_MODE_PRIMARY
    TEST_CASE_INIT(Test_Primary),
#endif
#if REFLEX_FORMAT_MODE_OFFSET
    TEST_CASE_INIT(Test_ParamsOffset),
#endif
#if REFLEX_SUPPORT_CUSTOM_TYPE_PARAMS
    TEST_CASE_INIT(Test_CustomTypeParams),
#endif
#if REFLEX_SUPPORT_COMPLEX_TYPE
    TEST_CASE_INIT(Test_ComplexType),
#endif
    TEST_CASE_INIT(Test_Size),
};
static const uint32_t Tests_Len = sizeof(Tests) / sizeof(Tests[0]);


int main(void)
{
	int testIndex;
	int countTestError = 0;
	Test_Result res;
    UART_init();

	for (testIndex = 0; testIndex < Tests_Len; testIndex++) {
		printTitle(testIndex, Tests[testIndex].Name);
		res = Tests[testIndex].fn();
		PRINTF("Test Result: %s\r\n", res ? "Error" : "Ok");
		if (res) {
			Result_print(res);
			countTestError++;
		}
		PRINTLN(FOOTER);
	}
	PRINTLN("Test Done\r\n");
	PRINTF("Tests Errors: %d\r\n", countTestError);
	
    while (1) 
    {
    }
}

void printTitle(uint8_t idx, const char* name) {
    char tmpNum[3];
    char temp[65] = {0};
    uint8_t len = strlen(name);
    uint8_t nlen = sprintf(tmpNum, " %d ", idx);
    uint8_t hlen = ((sizeof(FOOTER) - 1) - len - nlen - 1);
    uint8_t extra = hlen & 1; // for odd length
    hlen >>= 1; // divide 2
    strncpy(&temp[0], FOOTER, hlen);
    temp[hlen] = ' ';
    strncpy(&temp[hlen + 1], name, len);
    strncpy(&temp[hlen + len + 1], tmpNum, nlen);
    strncpy(&temp[hlen + len + nlen + 1], FOOTER, hlen + extra);
    temp[2 * hlen + len + nlen + 1 + extra] = '\0';
    PRINTLN(temp);
}

// ---------------- Test Driver ---------------
// define address map globally for prevent stack overflow
static void* addressMap[120] = {0};

Reflex_Result Reflex_checkAddress(Reflex* reflex, void* value, const Reflex_TypeParams* fmt) {
    void** addrMap = reflex->Args;

    if (addrMap[Reflex_getVariableIndex(reflex)] != value
    #if REFLEX_SUPPORT_COMPLEX_TYPE
        && fmt->Fields.Primary != Reflex_PrimaryType_Complex
    #endif
    ) {
        PRINTF("Idx: %d, Address not match: %X != %X\r\n",
            Reflex_getVariableIndex(reflex),
            addrMap[Reflex_getVariableIndex(reflex)],
            value
        );
        return 1;
    }

    return REFLEX_OK;
}

// ----------------------------- Test Param ---------------------
#if REFLEX_FORMAT_MODE_PARAM
typedef struct {
    uint8_t*        V0;
    char            V1[10];
    uint32_t        V2;
    char*           V3[4];
    char            V4[4][32];
} Model1;
static const Reflex_TypeParams Model1_FMT[] CONST_VAR_ATTR = {
    REFLEX_TYPE_PARAMS(Reflex_Type_Pointer_UInt8, 0, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_Char, 10, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_UInt32, 0, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_PointerArray_Char, 4, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array2D_Char, 4, 32),
};
static const Reflex_Schema Model1_SCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Param, Model1_FMT);

typedef struct {
    int32_t     V0;
    char        V1[32];
    uint8_t     V2[8];
    float       V3;
} Model2;
static const Reflex_TypeParams Model2_FMT[] CONST_VAR_ATTR = {
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Int32, 0, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_Char, 32, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_UInt8, 8, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Float, 0, 0),
};
static const Reflex_Schema Model2_SCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Param, Model2_FMT);

Test_Result Test_Param(void) {
    Reflex reflex = {0};
    Model1 temp1;
    Model2 temp2;

    Reflex_setCallback(&reflex, Reflex_checkAddress);
    Reflex_setArgs(&reflex, addressMap);

    addressMap[0] = &temp1.V0;
    addressMap[1] = &temp1.V1;
    addressMap[2] = &temp1.V2;
    addressMap[3] = &temp1.V3;
    addressMap[4] = &temp1.V4;
    reflex.Schema = &Model1_SCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp1), REFLEX_OK);


    addressMap[0] = &temp2.V0;
    addressMap[1] = &temp2.V1;
    addressMap[2] = &temp2.V2;
    addressMap[3] = &temp2.V3;
    reflex.Schema = &Model2_SCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp2), REFLEX_OK);
    

    return 0;
}
#endif
// -------------- Primary Models --------------
#if REFLEX_FORMAT_MODE_PRIMARY

typedef struct {
    uint32_t    X;
    float       Y;
    uint8_t     Z;
} PrimaryTemp1;
static const uint8_t PrimaryTemp1_FMT[] CONST_VAR_ATTR = {
    Reflex_Type_Primary_UInt32,
    Reflex_Type_Primary_Float,
    Reflex_Type_Primary_UInt8,
    Reflex_Type_Unknown,
};
static const Reflex_Schema PrimaryTemp1_SCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Primary, PrimaryTemp1_FMT);

typedef struct {
    uint16_t    X;
    float       Y;
    uint8_t     Z;
} PrimaryTemp2;
static const uint8_t PrimaryTemp2_FMT[] CONST_VAR_ATTR = {
    Reflex_Type_Primary_UInt16,
    Reflex_Type_Primary_Float,
    Reflex_Type_Primary_UInt8,
    Reflex_Type_Unknown,
};
static const Reflex_Schema PrimaryTemp2_SCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Primary, PrimaryTemp2_FMT);

typedef struct {
    uint16_t    A;
    float       B;
    uint8_t     C;
    uint8_t     D;
} PrimaryTemp3;
static const uint8_t PrimaryTemp3_FMT[] CONST_VAR_ATTR = {
    Reflex_Type_Primary_UInt16,
    Reflex_Type_Primary_Float,
    Reflex_Type_Primary_UInt8,
    Reflex_Type_Primary_UInt8,
    Reflex_Type_Unknown,
};
static const Reflex_Schema PrimaryTemp3_SCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Primary, PrimaryTemp3_FMT);

typedef struct {
    uint8_t         V0;
    uint32_t        V1;
    uint16_t        V2;
    int16_t         V3;
    int8_t          V4;
    int64_t         V5;
    uint64_t        V6;
    float           V7;
    double          V8;
    uint8_t         V9;
    uint16_t        V10;
    uint32_t        V11;
} PrimaryTemp4;
static const uint8_t PrimaryTemp4_FMT[] CONST_VAR_ATTR = {
    Reflex_Type_Primary_UInt8,
    Reflex_Type_Primary_UInt32,
    Reflex_Type_Primary_UInt16,
    Reflex_Type_Primary_Int16,
    Reflex_Type_Primary_Int8,
    Reflex_Type_Primary_Int64,
    Reflex_Type_Primary_UInt64,
    Reflex_Type_Primary_Float,
    Reflex_Type_Primary_Double,
    Reflex_Type_Primary_UInt8,
    Reflex_Type_Primary_UInt16,
    Reflex_Type_Primary_UInt32,
    Reflex_Type_Unknown,
};
static const Reflex_Schema PrimaryTemp4_SCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Primary, PrimaryTemp4_FMT);

Test_Result Test_Primary(void) {
    Reflex reflex = {0};

    PrimaryTemp1 temp1;
    PrimaryTemp2 temp2;
    PrimaryTemp3 temp3;
    PrimaryTemp4 temp4;

    Reflex_setCallback(&reflex, Reflex_checkAddress);
    Reflex_setArgs(&reflex, addressMap);

    addressMap[0] = &temp1.X;
    addressMap[1] = &temp1.Y;
    addressMap[2] = &temp1.Z;
    reflex.Schema = &PrimaryTemp1_SCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp1), REFLEX_OK);


    addressMap[0] = &temp2.X;
    addressMap[1] = &temp2.Y;
    addressMap[2] = &temp2.Z;
    reflex.Schema = &PrimaryTemp2_SCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp2), REFLEX_OK);
    

    addressMap[0] = &temp3.A;
    addressMap[1] = &temp3.B;
    addressMap[2] = &temp3.C;
    addressMap[3] = &temp3.D;
    reflex.Schema = &PrimaryTemp3_SCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp3), REFLEX_OK);
    

    addressMap[0] = &temp4.V0;
    addressMap[1] = &temp4.V1;
    addressMap[2] = &temp4.V2;
    addressMap[3] = &temp4.V3;
    addressMap[4] = &temp4.V4;
    addressMap[5] = &temp4.V5;
    addressMap[6] = &temp4.V6;
    addressMap[7] = &temp4.V7;
    addressMap[8] = &temp4.V8;
    addressMap[9] = &temp4.V9;
    addressMap[10] = &temp4.V10;
    addressMap[11] = &temp4.V11;
    reflex.Schema = &PrimaryTemp4_SCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp4), REFLEX_OK);
    

    return 0;
}
#endif
// ---------------------- Test Params Offset --------------------
#if REFLEX_FORMAT_MODE_OFFSET

static const Reflex_TypeParams Model1_FMT_OFFSET[] CONST_VAR_ATTR = {
    REFLEX_TYPE_PARAMS_OFFSET(Reflex_Type_Pointer_UInt8, 0, 0, Model1, V0),
    REFLEX_TYPE_PARAMS_OFFSET(Reflex_Type_Array_Char, 10, 0, Model1, V1),
    REFLEX_TYPE_PARAMS_OFFSET(Reflex_Type_Primary_UInt32, 0, 0, Model1, V2),
    REFLEX_TYPE_PARAMS_OFFSET(Reflex_Type_PointerArray_Char, 4, 0, Model1, V3),
    REFLEX_TYPE_PARAMS_OFFSET(Reflex_Type_Array2D_Char, 4, 32, Model1, V4),
};
static const Reflex_Schema Model1_SCHEMA_OFFSET CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Param, Model1_FMT_OFFSET);

static const Reflex_TypeParams Model2_FMT_OFFSET[] CONST_VAR_ATTR = {
    REFLEX_TYPE_PARAMS_OFFSET(Reflex_Type_Primary_Int32, 0, 0, Model2, V0),
    REFLEX_TYPE_PARAMS_OFFSET(Reflex_Type_Array_Char, 32, 0, Model2, V1),
    REFLEX_TYPE_PARAMS_OFFSET(Reflex_Type_Array_UInt8, 8, 0, Model2, V2),
    REFLEX_TYPE_PARAMS_OFFSET(Reflex_Type_Primary_Float, 0, 0, Model2, V3),
};
static const Reflex_Schema Model2_SCHEMA_OFFSET CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Param, Model2_FMT_OFFSET);


Test_Result Test_ParamsOffset(void) {
    Reflex reflex = {0};
    Model1 temp1;
    Model2 temp2;

    Reflex_setCallback(&reflex, Reflex_checkAddress);
    Reflex_setArgs(&reflex, addressMap);

    addressMap[0] = &temp1.V0;
    addressMap[1] = &temp1.V1;
    addressMap[2] = &temp1.V2;
    addressMap[3] = &temp1.V3;
    addressMap[4] = &temp1.V4;
    reflex.Schema = &Model1_SCHEMA_OFFSET;
    assert(Num, Reflex_scan(&reflex, &temp1), REFLEX_OK);


    addressMap[0] = &temp2.V0;
    addressMap[1] = &temp2.V1;
    addressMap[2] = &temp2.V2;
    addressMap[3] = &temp2.V3;
    reflex.Schema = &Model2_SCHEMA_OFFSET;
    assert(Num, Reflex_scan(&reflex, &temp2), REFLEX_OK);
    

    return 0;
}

#endif
// ---------------------- Test Custom Type Params  ----------------------
#if REFLEX_SUPPORT_CUSTOM_TYPE_PARAMS

typedef struct {
    REFLEX_TYPE_PARAMS_STRUCT();
    uint32_t        V0;
    uint8_t         V1;
} CustomTypeParams1;

typedef struct {
    Reflex_TypeParams   TypeParams;
    uint32_t            V0;
    uint8_t             V1;
    const char*         V2;
} CustomTypeParams2;

static const CustomTypeParams1 Model1_CFMT[] CONST_VAR_ATTR = {
    REFLEX_TYPE_PARAMS(Reflex_Type_Pointer_UInt8, 0, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_Char, 10, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_UInt32, 0, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_PointerArray_Char, 4, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array2D_Char, 4, 32),
};
static const Reflex_Schema Model1_CSCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Param, Model1_CFMT);

static const CustomTypeParams2 Model2_CFMT[] CONST_VAR_ATTR = {
    { .TypeParams = REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Int32, 0, 0), },
    { .TypeParams = REFLEX_TYPE_PARAMS(Reflex_Type_Array_Char, 32, 0), },
    { .TypeParams = REFLEX_TYPE_PARAMS(Reflex_Type_Array_UInt8, 8, 0), },
    { .TypeParams = REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Float, 0, 0), },
};
static const Reflex_Schema Model2_CSCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Param, Model2_CFMT);

Test_Result Test_CustomTypeParams(void) {
    Reflex reflex = {0};
    Model1 temp1;
    Model2 temp2;

    Reflex_setCallback(&reflex, Reflex_checkAddress);
    Reflex_setArgs(&reflex, addressMap);

    addressMap[0] = &temp1.V0;
    addressMap[1] = &temp1.V1;
    addressMap[2] = &temp1.V2;
    addressMap[3] = &temp1.V3;
    addressMap[4] = &temp1.V4;
    reflex.Schema = &Model1_CSCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp1), REFLEX_OK);


    addressMap[0] = &temp2.V0;
    addressMap[1] = &temp2.V1;
    addressMap[2] = &temp2.V2;
    addressMap[3] = &temp2.V3;
    reflex.Schema = &Model2_CSCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp2), REFLEX_OK);
    

    return 0;
}
#endif
// ----------------------- Custom Object ------------------------
#if REFLEX_SUPPORT_COMPLEX_TYPE

typedef struct {
    Model1      M1;
    Model2      M2;
} CModel1;
static const Reflex_TypeParams CModel1_FMT[] CONST_VAR_ATTR = {
    REFLEX_TYPE_PARAMS_COMPLEX(Reflex_Type_Primary_Complex, 0, 0, &Model1_SCHEMA),
    REFLEX_TYPE_PARAMS_COMPLEX(Reflex_Type_Primary_Complex, 0, 0, &Model2_SCHEMA),
};
static const Reflex_Schema CModel1_SCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Param, CModel1_FMT);

typedef struct {
    float       V0;
    uint8_t     V1;
    Model1      M1;
    Model2      M2[2];
    int16_t     V2;
    uint8_t     V3;
} CModel2;
static const Reflex_TypeParams CModel2_FMT[] CONST_VAR_ATTR = {
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Float, 0, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_UInt8, 0, 0),
    REFLEX_TYPE_PARAMS_COMPLEX(Reflex_Type_Primary_Complex, 0, 0, &Model1_SCHEMA),
    REFLEX_TYPE_PARAMS_COMPLEX(Reflex_Type_Array_Complex, 2, 0, &Model2_SCHEMA),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Int16, 0, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_UInt8, 0, 0),
};
static const Reflex_Schema CModel2_SCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Param, CModel2_FMT);

Test_Result Test_ComplexType(void) {
    Reflex reflex = {0};
    CModel1 temp1 = {0};
    CModel2 temp2 = {0};

    Reflex_setCallback(&reflex, Reflex_checkAddress);
    Reflex_setArgs(&reflex, addressMap);

    addressMap[0] = &temp1.M1.V0;
    addressMap[1] = &temp1.M1.V1;
    addressMap[2] = &temp1.M1.V2;
    addressMap[3] = &temp1.M1.V3;
    addressMap[4] = &temp1.M1.V4;
    addressMap[5] = &temp1.M2.V0;
    addressMap[6] = &temp1.M2.V1;
    addressMap[7] = &temp1.M2.V2;
    addressMap[8] = &temp1.M2.V3;
    reflex.Schema = &CModel1_SCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp1), REFLEX_OK);


    addressMap[0]  = &temp2.V0;
    addressMap[1]  = &temp2.V1;
    addressMap[2]  = &temp2.M1.V0;
    addressMap[3]  = &temp2.M1.V1;
    addressMap[4]  = &temp2.M1.V2;
    addressMap[5]  = &temp2.M1.V3;
    addressMap[6]  = &temp2.M1.V4;
    addressMap[7]  = &temp2.M2[0].V0;
    addressMap[8]  = &temp2.M2[0].V1;
    addressMap[9]  = &temp2.M2[0].V2;
    addressMap[10] = &temp2.M2[0].V3;
    addressMap[11] = &temp2.M2[1].V0;
    addressMap[12] = &temp2.M2[1].V1;
    addressMap[13] = &temp2.M2[1].V2;
    addressMap[14] = &temp2.M2[1].V3;
    addressMap[15] = &temp2.V2;
    addressMap[16] = &temp2.V3;
    reflex.Schema = &CModel2_SCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp2), REFLEX_OK);
    

    return 0;
}
#endif
// -------------------------- Test Size -------------------------
Test_Result Test_Size(void) {

    assert(Num, Reflex_size(&Model1_SCHEMA, Reflex_SizeType_Normal), sizeof(Model1));
    assert(Num, Reflex_size(&Model2_SCHEMA, Reflex_SizeType_Normal), sizeof(Model2));
    assert(Num, Reflex_size(&PrimaryTemp1_SCHEMA, Reflex_SizeType_Normal), sizeof(PrimaryTemp1));
    assert(Num, Reflex_size(&PrimaryTemp2_SCHEMA, Reflex_SizeType_Normal), sizeof(PrimaryTemp2));
    assert(Num, Reflex_size(&PrimaryTemp3_SCHEMA, Reflex_SizeType_Normal), sizeof(PrimaryTemp3));
    assert(Num, Reflex_size(&PrimaryTemp4_SCHEMA, Reflex_SizeType_Normal), sizeof(PrimaryTemp4));

#if REFLEX_SUPPORT_CUSTOM_TYPE_PARAMS
    assert(Num, Reflex_size(&Model1_CSCHEMA, Reflex_SizeType_Normal), sizeof(Model1));
    assert(Num, Reflex_size(&Model2_CSCHEMA, Reflex_SizeType_Normal), sizeof(Model2));
#endif

#if REFLEX_FORMAT_MODE_OFFSET
    assert(Num, Reflex_size(&Model1_SCHEMA_OFFSET, Reflex_SizeType_Normal), sizeof(Model1));
    assert(Num, Reflex_size(&Model2_SCHEMA_OFFSET, Reflex_SizeType_Normal), sizeof(Model2));
#endif

    return 0;
}

void Result_print(Test_Result result) {
    PRINTF("Line: %u, Index: %u\r\n", result >> 16, result & 0xFFFF);
}

Test_Result Assert_Num(int32_t val1, int32_t val2, uint16_t line) {
    if (val1 != val2) {
        PRINTF("Assert Num: expected %d, found %d, Line: %d\r\n", val2, val1, line);
        return (Test_Result) line << 16;
    }
    else {
        return 0;
    }
}
