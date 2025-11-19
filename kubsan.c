#include "ubsan.h"
#include "rt_utils.h"
#include "printf.h"


#define REPORTED_BIT 31

#if (BITS_PER_LONG == 64) && defined(__BIG_ENDIAN)
#define COLUMN_MASK (~(1U << REPORTED_BIT))
#define LINE_MASK   (~0U)
#else
#define COLUMN_MASK   (~0U)
#define LINE_MASK (~(1U << REPORTED_BIT))
#endif

#define VALUE_LENGTH 40
#define IS_ALIGNED(x, a)        (((x) & ((typeof(x))(a) - 1)) == 0)
#define BIT(nr) (1UL << (nr))
#define __attribute_const__             __attribute__((__const__))

#define ffs(x) __builtin_ffs(x)

typedef long ssize_t;

int scnprintf(char * buf, size_t size, const char * fmt, ...) //check varidic arguments!
{
       ssize_t ssize = size;
       va_list args;
       int i;

       va_start(args, fmt);
       i = vsnprintf(buf, size, fmt, args);
       va_end(args);

       return (i >= ssize) ? (ssize - 1) : i;
}

static const char * const type_check_kinds[] = {
	"load of",
	"store to",
	"reference binding to",
	"member access within",
	"member call on",
	"constructor call on",
	"downcast of",
	"downcast of"
};





// UTILITY FUNCTIONS: USED TO MANAGE THE HANDLER'S DATA (MAY NEED CHANGES)

/*
static bool was_reported(struct source_location *location)
{
	return test_and_set_bit(REPORTED_BIT, &location->reported);
}

static bool suppress_report(struct source_location *loc)
{
	return current->in_ubsan || was_reported(loc);
}
*/

static bool type_is_int(struct type_descriptor *type)
{
	return type->type_kind == type_kind_int;
}

static bool type_is_signed(struct type_descriptor *type)
{
	//WARN_ON(!type_is_int(type)); used to log a warning in the kernel log. Since we have no kernel...
	return  type->type_info & 1;
}

static unsigned type_bit_width(struct type_descriptor *type)
{
	return 1 << (type->type_info >> 1);
}

static bool is_inline_int(struct type_descriptor *type)
{
	unsigned inline_bits = sizeof(unsigned long)*8;
	unsigned bits = type_bit_width(type);

	//WARN_ON(!type_is_int(type));

	return bits <= inline_bits;
}

static s_max get_signed_val(struct type_descriptor *type, void *val)
{
	if (is_inline_int(type)) {
		unsigned extra_bits = sizeof(s_max)*8 - type_bit_width(type);
		unsigned long ulong_val = (unsigned long)val;

		return ((s_max)ulong_val) << extra_bits >> extra_bits;
	}

	if (type_bit_width(type) == 64)
		return *(s64 *)val;

	return *(s_max *)val;
}

static bool val_is_negative(struct type_descriptor *type, void *val)
{
	return type_is_signed(type) && get_signed_val(type, val) < 0;
}

static u_max get_unsigned_val(struct type_descriptor *type, void *val)
{
	if (is_inline_int(type))
		return (unsigned long)val;

	if (type_bit_width(type) == 64)
		return *(u64 *)val;

	return *(u_max *)val;
}

//check val_to_string and use of scnprintf

static void val_to_string(char *str, size_t size, struct type_descriptor *type,
			void *value)
{
	if (type_is_int(type)) {
		if (type_bit_width(type) == 128) {
#if defined(CONFIG_ARCH_SUPPORTS_INT128)
			u_max val = get_unsigned_val(type, value);

			scnprintf(str, size, "0x%08x%08x%08x%08x",
				(u32)(val >> 96),
				(u32)(val >> 64),
				(u32)(val >> 32),
				(u32)(val));
#else
			//WARN_ON(1);
#endif
		} else if (type_is_signed(type)) {
			scnprintf(str, size, "%lld",
				(s64)get_signed_val(type, value));
		} else {
			scnprintf(str, size, "%llu",
				(u64)get_unsigned_val(type, value));
		}
	}
}

//================================================================================

static void ubsan_prologue(struct source_location *loc, const char *reason)
{
	//current->in_ubsan++;

	printf("================================================================================");//pr_warn(CUT_HERE);

	printf("UBSAN: %s in %s:%d:%d\n", reason, loc->file_name,
		loc->line & LINE_MASK, loc->column & COLUMN_MASK); //pr_err(...)

	//kunit_fail_current_test("%s in %s", reason, loc->file_name);
}

static void ubsan_epilogue(void)
{
	//dump_stack();
	printf("---[ end trace ]---\n");

	//current->in_ubsan--;

	//check_panic_on_warn("UBSAN");
}

/*Functions below will use ubsan_prologue and ubsan_epilogue to create the console output, and get the variables
passed by the compiler in runtime.*/

//Ubsan handles
static void handle_overflow(struct overflow_data *data, void *lhs,
			void *rhs, char op)
{

	struct type_descriptor *type = data->type;
	char lhs_val_str[VALUE_LENGTH];
	char rhs_val_str[VALUE_LENGTH];

	//if (suppress_report(&data->location))
	//	return;

	ubsan_prologue(&data->location, type_is_signed(type) ?
			"signed-integer-overflow" :
			"unsigned-integer-overflow");

	val_to_string(lhs_val_str, sizeof(lhs_val_str), type, lhs);
	val_to_string(rhs_val_str, sizeof(rhs_val_str), type, rhs);
	printf("%s %c %s cannot be represented in type %s\n",
		lhs_val_str,
		op,
		rhs_val_str,
		type->type_name);

	ubsan_epilogue();
}

void __ubsan_handle_add_overflow(void *data,
				void *lhs, void *rhs)
{

	handle_overflow(data, lhs, rhs, '+');
}
//EXPORT_SYMBOL(__ubsan_handle_add_overflow);

void __ubsan_handle_sub_overflow(void *data,
				void *lhs, void *rhs)
{
	handle_overflow(data, lhs, rhs, '-');
}
//EXPORT_SYMBOL(__ubsan_handle_sub_overflow);

void __ubsan_handle_mul_overflow(void *data,
				void *lhs, void *rhs)
{
	handle_overflow(data, lhs, rhs, '*');
}
//EXPORT_SYMBOL(__ubsan_handle_mul_overflow);

void __ubsan_handle_negate_overflow(void *_data, void *old_val)
{
	struct overflow_data *data = _data;
	char old_val_str[VALUE_LENGTH];

	//if (suppress_report(&data->location))
	//	 return;

	ubsan_prologue(&data->location, "negation-overflow");

	val_to_string(old_val_str, sizeof(old_val_str), data->type, old_val);

	printf("negation of %s cannot be represented in type %s:\n",
		old_val_str, data->type->type_name);

	ubsan_epilogue();
}
//EXPORT_SYMBOL(__ubsan_handle_negate_overflow);

void __ubsan_handle_implicit_conversion(void *_data, void *from_val, void *to_val)
{
	struct implicit_conversion_data *data = _data;
	char from_val_str[VALUE_LENGTH];
	char to_val_str[VALUE_LENGTH];

	//if (suppress_report(&data->location))
	//	return;

	val_to_string(from_val_str, sizeof(from_val_str), data->from_type, from_val);
	val_to_string(to_val_str, sizeof(to_val_str), data->to_type, to_val);

	ubsan_prologue(&data->location, "implicit-conversion");

	printf("cannot represent %s value %s during %s %s, truncated to %s\n",
		data->from_type->type_name,
		from_val_str,
		type_check_kinds[data->type_check_kind],
		data->to_type->type_name,
		to_val_str);

	ubsan_epilogue();
}
//EXPORT_SYMBOL(__ubsan_handle_implicit_conversion);

void __ubsan_handle_divrem_overflow(void *_data, void *lhs, void *rhs)
{
	struct overflow_data *data = _data;
	char lhs_val_str[VALUE_LENGTH];

	//if (suppress_report(&data->location))
	//	return;

	ubsan_prologue(&data->location, "division-overflow");

	val_to_string(lhs_val_str, sizeof(lhs_val_str), data->type, lhs);

	if (type_is_signed(data->type) && get_signed_val(data->type, rhs) == -1)
		printf("division of %s by -1 cannot be represented in type %s\n",
			lhs_val_str, data->type->type_name);
	else
		printf("division by zero\n");

	ubsan_epilogue();
}
//EXPORT_SYMBOL(__ubsan_handle_divrem_overflow);

static void handle_null_ptr_deref(struct type_mismatch_data_common *data)
{
	//if (suppress_report(data->location))
	//	return;

	ubsan_prologue(data->location, "null-ptr-deref");

	printf("%s null pointer of type %s\n",
		type_check_kinds[data->type_check_kind],
		data->type->type_name);

	ubsan_epilogue();
}

static void handle_misaligned_access(struct type_mismatch_data_common *data,
				unsigned long ptr)
{
	//if (suppress_report(data->location))
	//	return;

	ubsan_prologue(data->location, "misaligned-access");

	printf("%s misaligned address %p for type %s\n",
		type_check_kinds[data->type_check_kind],
		(void *)ptr, data->type->type_name);
	printf("which requires %ld byte alignment\n", data->alignment);

	ubsan_epilogue();
}

static void handle_object_size_mismatch(struct type_mismatch_data_common *data,
					unsigned long ptr)
{
	//if (suppress_report(data->location))
	//	return;

	ubsan_prologue(data->location, "object-size-mismatch");
	printf("%s address %p with insufficient space\n",
		type_check_kinds[data->type_check_kind],
		(void *) ptr);
	printf("for an object of type %s\n", data->type->type_name);
	ubsan_epilogue();
}

static void ubsan_type_mismatch_common(struct type_mismatch_data_common *data,
				unsigned long ptr)
{
	//unsigned long flags = user_access_save();

	if (!ptr)
		handle_null_ptr_deref(data);
	else if (data->alignment && !IS_ALIGNED(ptr, data->alignment))
		handle_misaligned_access(data, ptr);
	else
		handle_object_size_mismatch(data, ptr);

	//user_access_restore(flags);
}

void __ubsan_handle_type_mismatch(struct type_mismatch_data *data,
				void *ptr)
{
	struct type_mismatch_data_common common_data = {
		.location = &data->location,
		.type = data->type,
		.alignment = data->alignment,
		.type_check_kind = data->type_check_kind
	};

	ubsan_type_mismatch_common(&common_data, (unsigned long)ptr);
}
//EXPORT_SYMBOL(__ubsan_handle_type_mismatch);

void __ubsan_handle_type_mismatch_v1(void *_data, void *ptr)
{
	struct type_mismatch_data_v1 *data = _data;
	struct type_mismatch_data_common common_data = {
		.location = &data->location,
		.type = data->type,
		.alignment = 1UL << data->log_alignment,
		.type_check_kind = data->type_check_kind
	};

	ubsan_type_mismatch_common(&common_data, (unsigned long)ptr);
}
//EXPORT_SYMBOL(__ubsan_handle_type_mismatch_v1);

void __ubsan_handle_out_of_bounds(void *_data, void *index)
{
	struct out_of_bounds_data *data = _data;
	char index_str[VALUE_LENGTH];

	//if (suppress_report(&data->location))
	//	return;

	ubsan_prologue(&data->location, "array-index-out-of-bounds");

	val_to_string(index_str, sizeof(index_str), data->index_type, index);
	printf("index %s is out of range for type %s\n", index_str,
		data->array_type->type_name);
	ubsan_epilogue();
}
//EXPORT_SYMBOL(__ubsan_handle_out_of_bounds);

void __ubsan_handle_shift_out_of_bounds(void *_data, void *lhs, void *rhs)
{
	struct shift_out_of_bounds_data *data = _data;
	struct type_descriptor *rhs_type = data->rhs_type;
	struct type_descriptor *lhs_type = data->lhs_type;
	char rhs_str[VALUE_LENGTH];
	char lhs_str[VALUE_LENGTH];
	//unsigned long ua_flags = user_access_save();

	//if (suppress_report(&data->location))
	//	goto out;

	ubsan_prologue(&data->location, "shift-out-of-bounds");

	val_to_string(rhs_str, sizeof(rhs_str), rhs_type, rhs);
	val_to_string(lhs_str, sizeof(lhs_str), lhs_type, lhs);

	if (val_is_negative(rhs_type, rhs))
		printf("shift exponent %s is negative\n", rhs_str);

	else if (get_unsigned_val(rhs_type, rhs) >=
		type_bit_width(lhs_type))
		printf("shift exponent %s is too large for %u-bit type %s\n",
			rhs_str,
			type_bit_width(lhs_type),
			lhs_type->type_name);
	else if (val_is_negative(lhs_type, lhs))
		printf("left shift of negative value %s\n",
			lhs_str);
	else
		printf("left shift of %s by %s places cannot be"
			" represented in type %s\n",
			lhs_str, rhs_str,
			lhs_type->type_name);

	ubsan_epilogue();
out:
	//user_access_restore(ua_flags); //check user_acces_restore
}
//EXPORT_SYMBOL(__ubsan_handle_shift_out_of_bounds);


void __ubsan_handle_builtin_unreachable(void *_data)
{
	struct unreachable_data *data = _data;
	ubsan_prologue(&data->location, "unreachable");
	printf("calling __builtin_unreachable()\n");
	ubsan_epilogue();
	//panic("can't return from __builtin_unreachable()");
}
//EXPORT_SYMBOL(__ubsan_handle_builtin_unreachable);

void __ubsan_handle_load_invalid_value(void *_data, void *val)
{
	struct invalid_value_data *data = _data;
	char val_str[VALUE_LENGTH];
	//unsigned long ua_flags = user_access_save();

	//if (suppress_report(&data->location))
	//	goto out;

	ubsan_prologue(&data->location, "invalid-load");

	val_to_string(val_str, sizeof(val_str), data->type, val);

	printf("load of value %s is not a valid value for type %s\n",
		val_str, data->type->type_name);

	ubsan_epilogue();
out:
	//user_access_restore(ua_flags);
}
//EXPORT_SYMBOL(__ubsan_handle_load_invalid_value);

void __ubsan_handle_alignment_assumption(void *_data, unsigned long ptr,
					 unsigned long align,
					 unsigned long offset)
{
	struct alignment_assumption_data *data = _data;
	unsigned long real_ptr;

	//if (suppress_report(&data->location))
	//	return;

	ubsan_prologue(&data->location, "alignment-assumption");

	if (offset)
		printf("assumption of %lu byte alignment (with offset of %lu byte) for pointer of type %s failed",
		       align, offset, data->type->type_name);
	else
		printf("assumption of %lu byte alignment for pointer of type %s failed",
		       align, data->type->type_name);

	real_ptr = ptr - offset;
	printf("%saddress is %lu aligned, misalignment offset is %lu bytes",
	       offset ? "offset " : "", BIT(real_ptr ? ffs(real_ptr) : 0),
	       real_ptr & (align - 1));

	ubsan_epilogue();
}
//EXPORT_SYMBOL(__ubsan_handle_alignment_assumption);