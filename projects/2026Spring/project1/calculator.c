#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DIGITS 6000
#define DIV_PRECISION 30
#define INPUT_SIZE 8192
#define MAX_INT 2147483647

typedef struct {
    int sign;
    int scale;
    int len;
    int digits[MAX_DIGITS];
} BigDecimal;

void zero_number(BigDecimal *value) {
    value->sign = 0;
    value->scale = 0;
    value->len = 1;
    value->digits[0] = 0;
    return ;
}

void normalize(BigDecimal *value) {
    while (value->len > 1 && value->digits[value->len - 1] == 0) {
        value->len--;
    }
    while (value->len > 1 && value->digits[0] == 0) {
        memmove(value->digits, value->digits + 1, (size_t)(value->len-1)*sizeof(int));
        value->len--;
        value->scale--;
    }
    if (value->len == 1 && value->digits[0] == 0) zero_number(value); 
    return ;
}

void negate(BigDecimal *value) { value->sign = -value->sign; return ; }

void absolute_value(BigDecimal *value) { if (value->sign < 0) value->sign = 1; return ; }

int is_zero(BigDecimal value) { return value.sign == 0; }

int read_number(const char *text, BigDecimal *value) {
    char buffer[INPUT_SIZE];
    zero_number(value);
    int sign = 1;
    int seen_digit = 0;
    int seen_dot = 0;
    int e_sign = 0;
    int e_scale = 0;
    int scale = 0;
    int count = 0;
    size_t i = 0;
    
    if (text[i] == '+' || text[i] == '-') {
        if (text[i] == '-') sign = -1; 
        i++;
    }

    while (text[i] != '\0') {
        char ch = text[i++];
        if (isdigit(ch)) {
            if (count >= MAX_DIGITS) {
                printf("The number's lenth is too large. ");
                return 0; 
            }
            if (e_sign) {
                long long tmp = 10ll* e_scale + ch - '0';
                if(tmp > MAX_INT) {
                    printf("The number's exp is too large. ");
                    return 0;
                }
                e_scale = tmp;
                continue ;
            }
            buffer[count++] = (char)ch;
            if (seen_dot) scale++; 
            seen_digit = 1;
            continue;
        }
        if(!seen_digit) return 0; 
        if (ch == '.' && !seen_dot && !e_sign) {
            seen_dot = 1;
            continue;
        }
        if ((ch == 'e' || ch == 'E') && !e_sign) {
            e_sign = 1;
            char nxt = text[i];
            if (isdigit(nxt)) continue ;
            if (nxt == '-' || nxt == '+') {
                if (nxt == '-') e_sign = -1;
                i ++;
                if (isdigit(text[i])) continue ; 
            }
        }
        return 0;
    }

    if (!seen_digit) return 0; 

    value->sign = sign;
    value->scale = scale;
    value->len = count;
    for (i = 0; i < (size_t)count; i++) {
        value->digits[i] = buffer[count - 1 - (int)i] - '0';
    }
    value->scale -= e_sign * e_scale; 
    normalize(value);
    return 1;
}

void write_number(BigDecimal value, char *output, size_t output_size) {
    size_t pos = 0;
    int i;

    if (value.sign == 0) {
        output[0] = '0';
        output[1] = '\0';
        if (pos >= output_size) printf("Output is too large.\n");
        return;
    }
    if (value.sign < 0)
        output[pos++] = '-';
    if (value.scale == 0) {
        for (i = value.len - 1; i >= 0; i--) {
            output[pos++] = (char)('0' + value.digits[i]);
        }
        output[pos] = '\0';
        if (pos >= output_size) printf("Output is too large.\n");
        return;
    }
    else if (abs(value.scale) < 10) {
        if (value.len <= value.scale) {
            output[pos++] = '0';
            output[pos++] = '.';
            for (i = 0; i < value.scale - value.len; i++) output[pos++] = '0';
            for (i = value.len - 1; i >= 0; i--)
                output[pos++] = (char)('0' + value.digits[i]);
            output[pos] = '\0';
            if (pos >= output_size) printf("Output is too large.\n");
            return;
        }
        if (value.scale < 0) {
            for (i = value.len - 1; i >= 0; i--)
                output[pos++] = (char)('0' + value.digits[i]);
            for ( ; i >= value.scale; i --) 
                output[pos++] = '0';
        }
        else {
            for (i = value.len - 1; i >= value.scale && i >= 0; i--)
                output[pos++] = (char)('0' + value.digits[i]);
            output[pos++] = '.';
            for (i = value.scale - 1; i >= 0; i--)
                output[pos++] = (char)('0' + value.digits[i]);
        }
        output[pos] = '\0';
        if (pos >= output_size) printf("Output is too large.\n");
        return;
    }
    output[pos++] = (char)('0' + value.digits[value.len - 1]);
    output[pos++] = '.';
    if (value.len == 1) output[pos++] = '0'; 
    else {
        for (i = value.len - 2; i >= 0; i --)
            output[pos++] = (char)('0' + value.digits[i]);    
    }
    int scale = value.len - value.scale - 1;
    int buff[MAX_DIGITS];
    int cnt = 0;
    output[pos++] = 'e';
    if(scale < 0) {
        output[pos++] = '-';
        scale = -scale;
    }
    while (scale) {
        buff[cnt++] = scale % 10;
        scale /= 10;
    }
    while (cnt) 
        output[pos++] = buff[--cnt] + '0';
    output[pos] = '\0';
    if (pos >= output_size) printf("Output is too large.\n");
    return ;
}

int compare_abs(BigDecimal left, BigDecimal right) {
    if (is_zero(left) && is_zero(right)) return 0;
    if (is_zero(left)) return -1;
    if (is_zero(right)) return 1;
    int common_scale = left.scale > right.scale ? left.scale : right.scale;
    int left_len = left.len + (common_scale - left.scale);
    int right_len = right.len + (common_scale - right.scale);
    int i;

    if (left_len != right_len) return left_len > right_len ? 1 : -1; 
    for (i = left_len - 1; i >= 0; i--) {
        int left_digit = 0;
        int right_digit = 0;
        int left_index = i - (common_scale - left.scale);
        int right_index = i - (common_scale - right.scale);
        if (left_index >= 0 && left_index < left.len)      
            left_digit = left.digits[left_index];
        if (right_index >= 0 && right_index < right.len)
            right_digit = right.digits[right_index];
        if (left_digit != right_digit)
            return left_digit > right_digit ? 1 : -1;
    }
    return 0;
}

int compare_numbers(BigDecimal left, BigDecimal right) {
    if (left.sign != right.sign) return left.sign > right.sign ? 1 : -1;
    if (left.sign == 0) return 0;
    int cmp = compare_abs(left, right);
    return left.sign > 0 ? cmp : -cmp;
}

BigDecimal add_abs(BigDecimal left, BigDecimal right) {
    BigDecimal result;
    zero_number(&result);
    int common_scale = left.scale > right.scale ? left.scale : right.scale;
    int left_len = left.len + (common_scale - left.scale);
    int right_len = right.len + (common_scale - right.scale);
    int limit = left_len > right_len ? left_len : right_len;
    int carry = 0;
    int i;

    if (limit > MAX_DIGITS) {
        printf("Two number is too diff.\n");
        return left_len > right_len ? left : right; 
    }

    result.scale = common_scale;
    result.len = 0;
    for (i = 0; i < limit || carry; i++) {
        int left_digit = 0;
        int right_digit = 0;
        int left_index = i - (common_scale - left.scale);
        int right_index = i - (common_scale - right.scale);
        if (left_index >= 0 && left_index < left.len)
            left_digit = left.digits[left_index];
        if (right_index >= 0 && right_index < right.len)
            right_digit = right.digits[right_index];
        result.digits[result.len++] = (left_digit + right_digit + carry) % 10;
        carry = (left_digit + right_digit + carry) / 10;
    }
    result.sign = 1;
    normalize(&result);
    return result;
}

BigDecimal subtract_abs(BigDecimal left, BigDecimal right) {
    BigDecimal result;
    zero_number(&result);
    int common_scale = left.scale > right.scale ? left.scale : right.scale;
    int left_len = left.len + (common_scale - left.scale);
    int borrow = 0;
    int i;

    if (left.len - left.scale - right.len + right.scale > MAX_DIGITS) {
        printf("Two number is too diff.\n");
        return left; 
    }

    result.scale = common_scale;
    result.len = 0;
    for (i = 0; i < left_len; i++) {
        int left_digit = 0;
        int right_digit = 0;
        int left_index = i - (common_scale - left.scale);
        int right_index = i - (common_scale - right.scale);
        if (left_index >= 0 && left_index < left.len)
            left_digit = left.digits[left_index];
        if (right_index >= 0 && right_index < right.len)
            right_digit = right.digits[right_index];
        left_digit -= borrow;
        if (left_digit < right_digit) 
            left_digit += 10, borrow = 1;
        else borrow = 0;
        result.digits[result.len++] = left_digit - right_digit;
    }
    result.sign = 1;
    normalize(&result);
    return result;
}

BigDecimal add_numbers(BigDecimal left, BigDecimal right) {
    int cmp;

    if (left.sign == 0) return right;
    if (right.sign == 0) return left;
    if (left.sign == right.sign) {
        BigDecimal result = add_abs(left, right);
        result.sign = left.sign;
        return result;
    }
    cmp = compare_abs(left, right);
    if (cmp == 0) {
        BigDecimal result;
        zero_number(&result);
        return result;
    }
    if (cmp > 0) {
        BigDecimal result = subtract_abs(left, right);
        result.sign = left.sign;
        normalize(&result);
        return result;
    }
    else {
        BigDecimal result = subtract_abs(right, left);
        result.sign = right.sign;
        normalize(&result);
        return result;
    }
}

BigDecimal subtract_numbers(BigDecimal left, BigDecimal right) {
    negate(&right);
    return add_numbers(left, right);
}

BigDecimal multiply_numbers(BigDecimal left, BigDecimal right) {
    BigDecimal result;
    zero_number(&result);
    int i;
    int j;

    if (is_zero(left) || is_zero(right)) return result; 
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
    normalize(&result);
    return result;
}

void save_lens(BigDecimal *value, int len) {
    if (value->len <= len) return ;
    int i = value->len - len - 1;
    int carry = value->digits[i] >= 5 ? 1 : 0;
    for (int j = 0; j <= i; j ++)
        value->digits[j] = 0;
    for (i = value->len - len ; carry && i < value->len; i ++) {
        value->digits[i] += carry;
        carry = 0;
        if (value->digits[i] == 10) {
            value->digits[i] = 0;
            carry = 1;
        }
    }
    if (carry) {
        value->digits[value->len++] = 1;
        carry = 0;
    }
    normalize(value);
}

int divide_numbers(BigDecimal left, BigDecimal right, BigDecimal *result, int len) {
    zero_number(result);
    if (is_zero(right)) {
        printf("A number cannot be divied by zero.\n");
        return 0;
    }
    if (is_zero(left)) return 1;
    int scale = (left.len - left.scale) - (right.len - right.scale);
    int sign = left.sign * right.sign;
    left.sign = right.sign = result->sign = 1;
    result->scale = -scale;
    left.scale += scale;
    normalize(&left);
    normalize(&right);
    normalize(result);
    BigDecimal ten, add;
    ten.digits[1] = 1;
    ten.digits[0] = 0;
    ten.len = 2;
    add.len = 1;
    ten.sign = 1;
    add.sign = 1;
    ten.scale = 0;
    add.scale = 0;
    normalize(&ten);
    for (int i = 0; i <= len; i ++) {
        BigDecimal tmp, next_tmp;
        zero_number(&tmp);
        for (int j = 0; j < 10; j ++) {
            next_tmp = add_numbers(tmp, right);
            if (compare_abs(left, next_tmp) < 0) {
                add.digits[0] = j;
                *result = multiply_numbers(*result, ten);
                *result = add_numbers(*result, add);
                left = subtract_numbers(left, tmp);
                right.scale += 1;
                normalize(&right);
                break;
            }
            tmp = next_tmp;
        }
        if (is_zero(left)) {
            result->sign = sign;
            result->scale += i - scale;
            normalize(result);
            return 1;
        }
    }
    result->sign = sign;
    result->scale += len - scale;
    save_lens(result, len-1);
    normalize(result);
    return 1;
}

int binary_calculate(BigDecimal left, char op, BigDecimal right, BigDecimal *result) {
    switch (op) {
        case '+':
            *result = add_numbers(left, right);
            return 1;
        case '-':
            *result = subtract_numbers(left, right);
            return 1;
        case '*':
            *result = multiply_numbers(left, right);
            return 1;
        case 'x':
            *result = multiply_numbers(left, right);
            return 1;
        case '/':
            return divide_numbers(left, right, result, 9);
        default:
            printf("Unsupported operator. Please use one of: + - *(or x) /\n");
            return 0;
    }
}

BigDecimal divide_by_two(BigDecimal value) {
    if (is_zero(value)) return value;
    BigDecimal _2;
    _2.len = 1;
    _2.scale = 1;
    _2.sign = 1;
    _2.digits[0] = 5;
    normalize(&_2);
    return multiply_numbers(value, _2);
}

int sqrt_number(BigDecimal value, BigDecimal *result, int iter) {
    zero_number(result);
    if (value.sign < 0) {
        printf("A negative number cannot calculate sqrt.\n");
        return 0;
    }
    if (is_zero(value)) return 1;
    if (iter <= 0) iter = 100;

    BigDecimal left, right, one, mid, mid2;
    zero_number(&left);
    zero_number(&right);
    zero_number(&one);

    one.sign = 1;
    one.len = 1;
    one.digits[0] = 1;

    if (compare_abs(value, one) < 0) right = one;
    else right = value;

    for (int i = 0; i < iter; i ++) {
        mid = add_numbers(left, right);
        mid = divide_by_two(mid);
        mid2 = multiply_numbers(mid, mid);
        if (compare_numbers(mid2, value) <= 0) left = mid;
        else right = mid;
    }
    *result = left;
    save_lens(result, 8);
    return 1;
}

int pow_number(BigDecimal value, long long exp, BigDecimal *result) {
    BigDecimal one, base;
    zero_number(&one);
    one.sign = 1;
    one.len = 1;
    one.digits[0] = 1;
    *result = one;
    if (exp == 0) return 1;
    if (exp < 0) {
        printf("pow only supports non-negative integer exponent.\n");
        return 0;
    }

    long long abs_exp = exp;
    base = value;
    while (abs_exp) {
        if (abs_exp & 1) *result = multiply_numbers(*result, base);
        abs_exp >>= 1;
        if (abs_exp) base = multiply_numbers(base, base);
    }
    save_lens(result, 8);
    return 1;
}

int read_line(const char *line, char *left, size_t left_size, char *op, char *right, size_t right_size) {
    size_t i = 0;
    size_t left_len = 0;
    size_t right_len = 0;

    while (line[i] == ' ') i++; 
    while (line[i] != ' ' && line[i] != '\0' && line[i] != '\n') {
        if (left_len + 1 >= left_size) {
            printf("Input is too large.\n");
            return 0;
        }
        left[left_len++] = line[i++];
    }
    left[left_len] = '\0';

    while (line[i] == ' ') i++; 

    if (line[i] == '\0') {
        printf("Only input one number.\n");
        return 0; 
    }
    *op = line[i++];

    while (line[i] == ' ') i++; 
    while (line[i] != ' ' && line[i] != '\0' && line[i] != '\n') {
        if (right_len + 1 >= right_size) {
            printf("Input is too large.\n");
            return 0; 
        }
        right[right_len++] = line[i++];
    }
    right[right_len] = '\0';

    while (line[i] == ' ') i++;
    return line[i] == '\0' && left_len > 0 && right_len > 0;
}

static void print_help(const char *program_name) {
    printf("Usage:\n");
    printf("  %s <number> <operator> <number>\n", program_name);
    printf("  %s sqrt <number>\n", program_name);
    printf("  %s pow <number> <integer>\n", program_name);
    printf("  %s --help\n", program_name);
    printf("\n");
    printf("Supported operators: +  -  *  /\n");
    printf("Interactive mode commands after running without arguments:\n");
    printf("  12.5 + 3.75\n");
    printf("  sqrt 2\n");
    printf("  pow 2 10\n");
    printf("  help\n");
    printf("  quit\n");
}

void execute_sqrt(const char *text) {
    printf("%s\n", text);
    BigDecimal value, result;
    char value_output[INPUT_SIZE];
    char result_output[INPUT_SIZE * 2];
    if (!read_number(text, &value)) {
        printf("The input cannot be interpreted as numbers!\n");
        return ;
    }
    if (!sqrt_number(value, &result, 100)) return ;
    write_number(value, value_output, sizeof(value_output));
    write_number(result, result_output, sizeof(result_output));
    printf("sqrt(%s) = %s\n", value_output, result_output);
    return ;
}

void execute_pow(const char *base_text, const char *exp_text) {
    BigDecimal value, result;
    char value_output[INPUT_SIZE];
    char result_output[INPUT_SIZE * 2];
    char *endptr = NULL;
    long long exp;
    if (!read_number(base_text, &value)) {
        printf("The input cannot be interpreted as numbers!\n");
        return ;
    }
    exp = strtoll(exp_text, &endptr, 10);
    if (endptr == exp_text || *endptr != '\0') {
        printf("pow only supports integer exponent.\n");
        return ;
    }
    if (!pow_number(value, exp, &result)) return ;
    write_number(value, value_output, sizeof(value_output));
    write_number(result, result_output, sizeof(result_output));
    printf("pow(%s, %lld) = %s\n", value_output, exp, result_output);
    return ;
}

void execute_binary(const char *left_text, char op, const char *right_text) {
    BigDecimal left_number;
    BigDecimal right_number;
    if (!read_number(left_text, &left_number)) {
        printf("The input cannot be interpreted as numbers!\n");
        return ;
    }
    if (!read_number(right_text, &right_number)) {
        printf("The input cannot be interpreted as numbers!\n");
        return ;
    }
    
    char left_output[INPUT_SIZE];
    char right_output[INPUT_SIZE];
    char result_output[INPUT_SIZE * 2];
    BigDecimal result;

    if (!binary_calculate(left_number, op, right_number, &result)) return ; 
    write_number(left_number, left_output, sizeof(left_output));
    write_number(right_number, right_output, sizeof(right_output));
    write_number(result, result_output, sizeof(result_output));
    printf("%s %c %s = %s\n", left_output, op, right_output, result_output);
    return ;
}

void run_interactive(const char *program_name) {
    char line[INPUT_SIZE];
    char left[INPUT_SIZE];
    char right[INPUT_SIZE];
    char op = 0;

    printf("Interactive mode. Type help for instructions, quit to exit.\n");
    while (1) {
        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("Read NULL.\n");
            return ; 
        }
        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0) return ;
        if (strcmp(line, "help") == 0) {
            print_help(program_name);
            continue;
        }
        if (strncmp(line, "sqrt ", 5) == 0) {
            execute_sqrt(line + 5);
            continue;
        }
        if (strncmp(line, "pow ", 4) == 0) {
            char base[INPUT_SIZE], exp[INPUT_SIZE];
            if (sscanf(line + 4, "%8191s %8191s", base, exp) == 2) execute_pow(base, exp);
            else printf("pow requires: pow <number> <integer>\n");
            continue;
        }
        if (read_line(line, left, sizeof(left), &op, right, sizeof(right))) {
            execute_binary(left, op, right);
            continue;
        }
        printf("The input cannot be interpreted as a supported command or expression.\n");
    }
}

#ifdef DEBUG
static void debug_test_read_write(void) {
    char line[INPUT_SIZE];
    BigDecimal value;

    strcpy(line, "-10.10");
    printf("read_number ok ? : %d\n", read_number(line, &value));
    printf("len : %d, scale : %d, sign : %d\n", value.len, value.scale, value.sign);
    write_number(value, line, sizeof(line));
    printf("%s\n", line);
}

static void debug_test_add_sub(void) {
    BigDecimal left, right, result;
    char line[INPUT_SIZE];

    strcpy(line, "-3");
    read_number(line, &left);
    strcpy(line, "123");
    read_number(line, &right);
    result = subtract_numbers(left, right);
    write_number(result, line, sizeof(line));
    printf("%s\n", line);
}

static void debug_test_multi(void) {
    BigDecimal left, right, result;
    char line[INPUT_SIZE];

    strcpy(line, "-1E-100");
    read_number(line, &left);
    strcpy(line, "-123");
    read_number(line, &right);
    result = multiply_numbers(left, right);
    write_number(result, line, sizeof(line));
    printf("%s\n", line);
}

static void debug_test_divide(void) {
    BigDecimal left, right, result;
    char line[INPUT_SIZE];

    strcpy(line, "2");
    read_number(line, &left);
    strcpy(line, "3");
    read_number(line, &right);
    printf("divide_numbers ok ? : %d\n", divide_numbers(left, right, &result, 9));
    write_number(result, line, sizeof(line));
    printf("%s\n", line);
}

static void run_debug_tests(void) {
    debug_test_read_write();
    debug_test_add_sub();
    debug_test_multi();
    debug_test_divide();
}
#endif

int main(int argc, char *argv[]) {
#ifdef DEBUG
    run_debug_tests();
#endif
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        print_help(argv[0]);
        return 0;
    }
    if (argc == 3 && strcmp(argv[1], "sqrt") == 0) {
        execute_sqrt(argv[2]);
        return 0;
    }
    if (argc == 4 && strcmp(argv[1], "pow") == 0) {
        execute_pow(argv[2], argv[3]);
        return 0;
    }
    if (argc == 4) {
        if (argv[2][1] != '\0') {
            printf("Unsupported operator. Please use one of: + - * /\n");
            return 1;
        }
        execute_binary(argv[1], argv[2][0], argv[3]);
        return 0;
    }
    if (argc == 1) {
        run_interactive(argv[0]);
        return 0;
    }
    return 0;
}
