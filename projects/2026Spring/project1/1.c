#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DIGITS 6000
#define DIV_PRECISION 30
#define INPUT_SIZE 8192

typedef struct {
    int sign;
    int scale;
    int len;
    int digits[MAX_DIGITS];
} BigDecimal;

typedef struct {
    int ok;
    BigDecimal value;
} ParseResult;

typedef struct {
    int ok;
    BigDecimal value;
    const char *error;
} CalcResult;

BigDecimal zero_number() {
    BigDecimal value;
    value.sign = 0;
    value.scale = 0;
    value.len = 1;
    value.digits[0] = 0;
    return value;
}

BigDecimal normalize(BigDecimal value) {
    while (value.len > 1 && value.digits[value.len - 1] == 0) {
        value.len--;
    }
    while (value.scale > 0 && value.len > 1 && value.digits[0] == 0) {
        memmove(value.digits, value.digits + 1, (size_t)(value.len - 1) * sizeof(int));
        value.len--;
        value.scale--;
    }
    if (value.len == 1 && value.digits[0] == 0) {
        value = zero_number();
    }
    return value;
}

BigDecimal negate(BigDecimal value) {
    value.sign = -value.sign;
    return value;
}

BigDecimal absolute_value(BigDecimal value) {
    if (value.sign < 0) {
        value.sign = 1;
    }
    return value;
}

int is_zero(BigDecimal value) {
    return value.sign == 0;
}

static ParseResult parse_number(const char *text) {
    char buffer[INPUT_SIZE];
    BigDecimal value = zero_number();
    int sign = 1;
    int seen_digit = 0;
    int seen_dot = 0;
    int scale = 0;
    int count = 0;
    size_t i = 0;

    while (isspace((unsigned char)text[i])) {
        i++;
    }
    if (text[i] == '+' || text[i] == '-') {
        if (text[i] == '-') {
            sign = -1;
        }
        i++;
    }

    while (text[i] != '\0') {
        unsigned char ch = (unsigned char)text[i];
        if (isdigit(ch)) {
            if (count >= MAX_DIGITS) {
                return (ParseResult){0, zero_number()};
            }
            buffer[count++] = (char)ch;
            if (seen_dot) {
                scale++;
            }
            seen_digit = 1;
            i++;
            continue;
        }
        if (ch == '.') {
            if (seen_dot) {
                return (ParseResult){0, zero_number()};
            }
            seen_dot = 1;
            i++;
            continue;
        }
        if (isspace(ch)) {
            while (isspace((unsigned char)text[i])) {
                i++;
            }
            if (text[i] != '\0') {
                return (ParseResult){0, zero_number()};
            }
            break;
        }
        return (ParseResult){0, zero_number()};
    }

    if (!seen_digit) {
        return (ParseResult){0, zero_number()};
    }

    value.sign = sign;
    value.scale = scale;
    value.len = count;
    for (i = 0; i < (size_t)count; i++) {
        value.digits[i] = buffer[count - 1 - (int)i] - '0';
    }
    value = normalize(value);
    return (ParseResult){1, value};
}

static void number_to_string(BigDecimal value, char *output, size_t output_size) {
    size_t pos = 0;
    int i;

    if (value.sign == 0) {
        snprintf(output, output_size, "0");
        return;
    }
    if (value.sign < 0 && pos + 1 < output_size) {
        output[pos++] = '-';
    }
    if (value.scale == 0) {
        for (i = value.len - 1; i >= 0 && pos + 1 < output_size; i--) {
            output[pos++] = (char)('0' + value.digits[i]);
        }
        output[pos] = '\0';
        return;
    }
    if (value.len <= value.scale) {
        if (pos + 2 < output_size) {
            output[pos++] = '0';
            output[pos++] = '.';
        }
        for (i = 0; i < value.scale - value.len && pos + 1 < output_size; i++) {
            output[pos++] = '0';
        }
        for (i = value.len - 1; i >= 0 && pos + 1 < output_size; i--) {
            output[pos++] = (char)('0' + value.digits[i]);
        }
        output[pos] = '\0';
        return;
    }
    for (i = value.len - 1; i >= value.scale && pos + 1 < output_size; i--) {
        output[pos++] = (char)('0' + value.digits[i]);
    }
    if (pos + 1 < output_size) {
        output[pos++] = '.';
    }
    for (i = value.scale - 1; i >= 0 && pos + 1 < output_size; i--) {
        output[pos++] = (char)('0' + value.digits[i]);
    }
    output[pos] = '\0';
}

static int compare_abs(BigDecimal left, BigDecimal right) {
    int common_scale = left.scale > right.scale ? left.scale : right.scale;
    int left_len = left.len + (common_scale - left.scale);
    int right_len = right.len + (common_scale - right.scale);
    int i;

    if (left_len != right_len) {
        return left_len > right_len ? 1 : -1;
    }
    for (i = left_len - 1; i >= 0; i--) {
        int left_digit = 0;
        int right_digit = 0;
        int left_index = i - (common_scale - left.scale);
        int right_index = i - (common_scale - right.scale);
        if (left_index >= 0 && left_index < left.len) {
            left_digit = left.digits[left_index];
        }
        if (right_index >= 0 && right_index < right.len) {
            right_digit = right.digits[right_index];
        }
        if (left_digit != right_digit) {
            return left_digit > right_digit ? 1 : -1;
        }
    }
    return 0;
}

static BigDecimal add_abs(BigDecimal left, BigDecimal right) {
    BigDecimal result = zero_number();
    int common_scale = left.scale > right.scale ? left.scale : right.scale;
    int left_len = left.len + (common_scale - left.scale);
    int right_len = right.len + (common_scale - right.scale);
    int limit = left_len > right_len ? left_len : right_len;
    int carry = 0;
    int i;

    result.scale = common_scale;
    result.len = 0;
    for (i = 0; i < limit || carry; i++) {
        int left_digit = 0;
        int right_digit = 0;
        int left_index = i - (common_scale - left.scale);
        int right_index = i - (common_scale - right.scale);
        if (left_index >= 0 && left_index < left.len) {
            left_digit = left.digits[left_index];
        }
        if (right_index >= 0 && right_index < right.len) {
            right_digit = right.digits[right_index];
        }
        result.digits[result.len++] = (left_digit + right_digit + carry) % 10;
        carry = (left_digit + right_digit + carry) / 10;
    }
    result.sign = 1;
    return normalize(result);
}

static BigDecimal subtract_abs(BigDecimal left, BigDecimal right) {
    BigDecimal result = zero_number();
    int common_scale = left.scale > right.scale ? left.scale : right.scale;
    int left_len = left.len + (common_scale - left.scale);
    int borrow = 0;
    int i;

    result.scale = common_scale;
    result.len = 0;
    for (i = 0; i < left_len; i++) {
        int left_digit = 0;
        int right_digit = 0;
        int left_index = i - (common_scale - left.scale);
        int right_index = i - (common_scale - right.scale);
        if (left_index >= 0 && left_index < left.len) {
            left_digit = left.digits[left_index];
        }
        if (right_index >= 0 && right_index < right.len) {
            right_digit = right.digits[right_index];
        }
        left_digit -= borrow;
        if (left_digit < right_digit) {
            left_digit += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        result.digits[result.len++] = left_digit - right_digit;
    }
    result.sign = 1;
    return normalize(result);
}

static BigDecimal add_numbers(BigDecimal left, BigDecimal right) {
    int cmp;

    if (left.sign == 0) {
        return right;
    }
    if (right.sign == 0) {
        return left;
    }
    if (left.sign == right.sign) {
        BigDecimal result = add_abs(left, right);
        result.sign = left.sign;
        return result;
    }
    cmp = compare_abs(left, right);
    if (cmp == 0) {
        return zero_number();
    }
    if (cmp > 0) {
        BigDecimal result = subtract_abs(left, right);
        result.sign = left.sign;
        return normalize(result);
    }
    {
        BigDecimal result = subtract_abs(right, left);
        result.sign = right.sign;
        return normalize(result);
    }
}

static BigDecimal subtract_numbers(BigDecimal left, BigDecimal right) {
    return add_numbers(left, negate(right));
}

static BigDecimal multiply_numbers(BigDecimal left, BigDecimal right) {
    BigDecimal result = zero_number();
    int i;
    int j;

    if (is_zero(left) || is_zero(right)) {
        return zero_number();
    }
    memset(result.digits, 0, sizeof(result.digits));
    result.len = left.len + right.len;
    result.scale = left.scale + right.scale;
    result.sign = left.sign * right.sign;

    for (i = 0; i < left.len; i++) {
        int carry = 0;
        for (j = 0; j < right.len || carry; j++) {
            int current = result.digits[i + j] + left.digits[i] * (j < right.len ? right.digits[j] : 0) + carry;
            result.digits[i + j] = current % 10;
            carry = current / 10;
        }
    }
    return normalize(result);
}

static void strip_leading_zeros(char *text) {
    size_t len = strlen(text);
    size_t start = 0;
    while (start + 1 < len && text[start] == '0') {
        start++;
    }
    if (start > 0) {
        memmove(text, text + start, len - start + 1);
    }
}

static int compare_unsigned_str(const char *left, const char *right) {
    size_t left_len;
    size_t right_len;

    left += strspn(left, "0");
    right += strspn(right, "0");
    if (*left == '\0') {
        left = "0";
    }
    if (*right == '\0') {
        right = "0";
    }
    left_len = strlen(left);
    right_len = strlen(right);
    if (left_len != right_len) {
        return left_len > right_len ? 1 : -1;
    }
    return strcmp(left, right);
}

static void subtract_unsigned_str(const char *left, const char *right, char *output) {
    char reversed[MAX_DIGITS + DIV_PRECISION + 8];
    int left_len = (int)strlen(left);
    int right_len = (int)strlen(right);
    int borrow = 0;
    int i;

    for (i = 0; i < left_len; i++) {
        int left_digit = left[left_len - 1 - i] - '0';
        int right_digit = i < right_len ? right[right_len - 1 - i] - '0' : 0;
        int value = left_digit - right_digit - borrow;
        if (value < 0) {
            value += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        reversed[i] = (char)('0' + value);
    }
    while (left_len > 1 && reversed[left_len - 1] == '0') {
        left_len--;
    }
    for (i = 0; i < left_len; i++) {
        output[i] = reversed[left_len - 1 - i];
    }
    output[left_len] = '\0';
}

static void multiply_unsigned_str_digit(const char *text, int digit, char *output) {
    char reversed[MAX_DIGITS + DIV_PRECISION + 8];
    int len = (int)strlen(text);
    int carry = 0;
    int i;

    if (digit == 0) {
        strcpy(output, "0");
        return;
    }
    for (i = 0; i < len || carry; i++) {
        int current_digit = i < len ? text[len - 1 - i] - '0' : 0;
        int value = current_digit * digit + carry;
        reversed[i] = (char)('0' + value % 10);
        carry = value / 10;
    }
    len = i;
    while (len > 1 && reversed[len - 1] == '0') {
        len--;
    }
    for (i = 0; i < len; i++) {
        output[i] = reversed[len - 1 - i];
    }
    output[len] = '\0';
}

static CalcResult divide_numbers(BigDecimal left, BigDecimal right) {
    char numerator[MAX_DIGITS + DIV_PRECISION + 8];
    char denominator[MAX_DIGITS + 8];
    char remainder[MAX_DIGITS + DIV_PRECISION + 8];
    char product[MAX_DIGITS + DIV_PRECISION + 8];
    char next_remainder[MAX_DIGITS + DIV_PRECISION + 8];
    char quotient[MAX_DIGITS + DIV_PRECISION + 8];
    BigDecimal result = zero_number();
    int extra_zeros = DIV_PRECISION + right.scale;
    int quotient_len = 0;
    int i;

    if (is_zero(right)) {
        return (CalcResult){0, zero_number(), "Division by zero is not allowed."};
    }
    if (is_zero(left)) {
        return (CalcResult){1, zero_number(), NULL};
    }
    if (left.len + extra_zeros + 1 >= (int)sizeof(numerator)) {
        return (CalcResult){0, zero_number(), "The numbers are too large for the current implementation."};
    }

    for (i = 0; i < left.len; i++) {
        numerator[i] = (char)('0' + left.digits[left.len - 1 - i]);
    }
    for (i = 0; i < extra_zeros; i++) {
        numerator[left.len + i] = '0';
    }
    numerator[left.len + extra_zeros] = '\0';
    for (i = 0; i < right.len; i++) {
        denominator[i] = (char)('0' + right.digits[right.len - 1 - i]);
    }
    denominator[right.len] = '\0';

    strcpy(remainder, "0");
    for (i = 0; numerator[i] != '\0'; i++) {
        size_t remainder_len = strlen(remainder);
        if (strcmp(remainder, "0") == 0) {
            remainder[0] = numerator[i];
            remainder[1] = '\0';
        } else {
            remainder[remainder_len] = numerator[i];
            remainder[remainder_len + 1] = '\0';
        }
        strip_leading_zeros(remainder);

        for (quotient[quotient_len] = '9'; quotient[quotient_len] >= '0'; quotient[quotient_len]--) {
            multiply_unsigned_str_digit(denominator, quotient[quotient_len] - '0', product);
            if (compare_unsigned_str(product, remainder) <= 0) {
                break;
            }
        }
        multiply_unsigned_str_digit(denominator, quotient[quotient_len] - '0', product);
        subtract_unsigned_str(remainder, product, next_remainder);
        strcpy(remainder, next_remainder);
        quotient_len++;
    }

    quotient[quotient_len] = '\0';
    strip_leading_zeros(quotient);
    result.len = (int)strlen(quotient);
    result.scale = left.scale + DIV_PRECISION;
    result.sign = left.sign * right.sign;
    for (i = 0; i < result.len; i++) {
        result.digits[i] = quotient[result.len - 1 - i] - '0';
    }
    result = normalize(result);
    return (CalcResult){1, result, NULL};
}

static CalcResult binary_calculate(BigDecimal left, char op, BigDecimal right) {
    switch (op) {
        case '+':
            return (CalcResult){1, add_numbers(left, right), NULL};
        case '-':
            return (CalcResult){1, subtract_numbers(left, right), NULL};
        case '*':
            return (CalcResult){1, multiply_numbers(left, right), NULL};
        case '/':
            return divide_numbers(left, right);
        default:
            return (CalcResult){0, zero_number(), "Unsupported operator. Please use one of: + - * /"};
    }
}

static CalcResult power_integer(BigDecimal base, long long exponent) {
    BigDecimal result = zero_number();
    BigDecimal factor = base;
    long long times = exponent;

    if (exponent < 0) {
        return (CalcResult){0, zero_number(), "pow only supports non-negative integer exponents."};
    }
    result.sign = 1;
    result.scale = 0;
    result.len = 1;
    result.digits[0] = 1;

    while (times > 0) {
        if (times % 2 == 1) {
            result = multiply_numbers(result, factor);
        }
        times /= 2;
        if (times > 0) {
            factor = multiply_numbers(factor, factor);
        }
    }
    return (CalcResult){1, normalize(result), NULL};
}

static int parse_binary_expression(const char *line, char *left, size_t left_size, char *op, char *right, size_t right_size) {
    size_t i = 0;
    size_t left_len = 0;
    size_t right_len = 0;

    while (isspace((unsigned char)line[i])) {
        i++;
    }
    if (line[i] == '+' || line[i] == '-') {
        if (left_len + 1 >= left_size) {
            return 0;
        }
        left[left_len++] = line[i++];
    }
    while (isdigit((unsigned char)line[i]) || line[i] == '.') {
        if (left_len + 1 >= left_size) {
            return 0;
        }
        left[left_len++] = line[i++];
    }
    left[left_len] = '\0';

    while (isspace((unsigned char)line[i])) {
        i++;
    }
    if (line[i] == '\0') {
        return 0;
    }
    *op = line[i++];
    while (isspace((unsigned char)line[i])) {
        i++;
    }
    if (line[i] == '+' || line[i] == '-') {
        if (right_len + 1 >= right_size) {
            return 0;
        }
        right[right_len++] = line[i++];
    }
    while (isdigit((unsigned char)line[i]) || line[i] == '.') {
        if (right_len + 1 >= right_size) {
            return 0;
        }
        right[right_len++] = line[i++];
    }
    right[right_len] = '\0';

    while (isspace((unsigned char)line[i])) {
        i++;
    }
    return line[i] == '\0' && left_len > 0 && right_len > 0;
}

static void print_help(const char *program_name) {
    printf("Usage:\n");
    printf("  %s <number> <operator> <number>\n", program_name);
    printf("  %s --help\n", program_name);
    printf("\n");
    printf("Supported operators: +  -  *  /\n");
    printf("Interactive mode commands after running without arguments:\n");
    printf("  12.5 + 3.75\n");
    printf("  help\n");
    printf("  quit\n");
}

static int execute_binary(const char *left_text, char op, const char *right_text) {
    ParseResult left_parsed = parse_number(left_text);
    ParseResult right_parsed = parse_number(right_text);
    char left_output[INPUT_SIZE];
    char right_output[INPUT_SIZE];
    char result_output[INPUT_SIZE * 2];
    CalcResult result;

    if (!left_parsed.ok || !right_parsed.ok) {
        printf("The input cannot be interpreted as numbers.\n");
        return 1;
    }
    result = binary_calculate(left_parsed.value, op, right_parsed.value);
    if (!result.ok) {
        printf("%s\n", result.error);
        return 1;
    }
    number_to_string(left_parsed.value, left_output, sizeof(left_output));
    number_to_string(right_parsed.value, right_output, sizeof(right_output));
    number_to_string(result.value, result_output, sizeof(result_output));
    printf("%s %c %s = %s\n", left_output, op, right_output, result_output);
    return 0;
}

static int run_interactive(const char *program_name) {
    char line[INPUT_SIZE];
    char left[INPUT_SIZE];
    char right[INPUT_SIZE];
    char op = 0;

    printf("Interactive mode. Type help for instructions, quit to exit.\n");
    while (1) {
        if (fgets(line, sizeof(line), stdin) == NULL) {
            return 0;
        }
        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0) {
            return 0;
        }
        if (strcmp(line, "help") == 0) {
            print_help(program_name);
            continue;
        }
        if (parse_binary_expression(line, left, sizeof(left), &op, right, sizeof(right))) {
            execute_binary(left, op, right);
            continue;
        }
        printf("The input cannot be interpreted as a supported command or expression.\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        print_help(argv[0]);
        return 0;
    }
    if (argc == 4) {
        if (argv[2][1] != '\0') {
            printf("Unsupported operator. Please use one of: + - * /\n");
            return 1;
        }
        return execute_binary(argv[1], argv[2][0], argv[3]);
    }
    if (argc == 1) {
        return run_interactive(argv[0]);
    }
    print_help(argv[0]);
    return 1;
}
