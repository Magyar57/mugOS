#include "Logging.h"

// Sanitizer.c: Undefined behaviour sanitizer
// Function calls to these are added by the compiler when it detects an undefined behaviour
// For this, the compiler flag -fsanitize=undefined has te be added
// We don't need a header for these function, only for the symbols to be present at link time

#define MODULE "Sanitizer"

struct SourceLocation {
    const char* fileName;
    unsigned int line;
    unsigned int column;
};

struct TypeDescriptor {
    unsigned short typeKind;
    unsigned short typeInfo;
    char typeName[512];
};

void __ubsan_handle_type_mismatch_v1(void*){
	log(ERROR, MODULE, "type_mismatch_v1");
}

void __ubsan_handle_pointer_overflow(void*){
	log(ERROR, MODULE, "pointer_overflow");
}

void __ubsan_handle_load_invalid_value(void*){
	log(ERROR, MODULE, "load_invalid_value");
}

void __ubsan_handle_out_of_bounds(void* handle){
	struct Data_OutOfBounds {
		struct SourceLocation location;
		struct TypeDescriptor *array_type;
		struct TypeDescriptor *index_type;
	};
	struct Data_OutOfBounds* data = handle;

	log(ERROR, MODULE, "out_of_bounds at %s:%d:%d", data->location.fileName, data->location.line, data->location.column);
	log(ERROR, MODULE, "array type %s", data->array_type->typeName);
	log(ERROR, MODULE, "index type %s", data->index_type->typeName);
}

void __ubsan_handle_mul_overflow(void*){
	log(ERROR, MODULE, "mul_overflow");
}

void __ubsan_handle_add_overflow(void*){
	log(ERROR, MODULE, "add_overflow");
}

void __ubsan_handle_shift_out_of_bounds(void*){
	log(ERROR, MODULE, "shift_out_of_bounds");
}

void __ubsan_handle_sub_overflow(void*){
	log(ERROR, MODULE, "sub_overflow");
}

void __ubsan_handle_function_type_mismatch(void* handle){
	struct Data_TypeMismatch {
		struct SourceLocation location;
		struct TypeDescriptor* type;
	};
	struct Data_TypeMismatch* data = handle;

	log(ERROR, MODULE, "function_type_mismatch at %s:%d:%d", data->location.fileName, data->location.line, data->location.column);
	log(ERROR, MODULE, "%s", data->type->typeName);
}

void __ubsan_handle_divrem_overflow(void*){
	log(ERROR, MODULE, "divrem_overflow");
}

void __ubsan_handle_negate_overflow(void*){
	log(ERROR, MODULE, "negate_overflow");
}

void __ubsan_handle_builtin_unreachable(void*){
	log(ERROR, MODULE, "builtin_unreachable");
}

void __ubsan_handle_invalid_builtin(void*){
	log(ERROR, MODULE, "invalid_builtin");
}

void __ubsan_handle_vla_bound_not_positive(void*){
	log(ERROR, MODULE, "vla_bound_not_positive");
}
