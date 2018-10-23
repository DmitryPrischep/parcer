#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>

//Требуется написать программу, которая способна вычислять арифметические выражения.
//Выражения могут содержать:
//1) знаки операций '+', '-', '/', '*'
//2) Скобки '(', ')'
//3) Целые и вещественные числа, в нотации '123', '123.345',
//все операции должны быть вещественны, а результаты выведены с точностю до двух знаков после запятой в том числе целые '2.00'
//4) необходимо учитывать приоритеты операций, и возможность унарного минуса, пробелы ничего не значат
//5) Если в выражении встретилась ошибка требуется вывести в стандартный поток вывода "[error]" (без кавычек)


#define BUFFER 256

enum Priority {
    DEFAULT,
    OPEN_BRACE,
    LOW,
    HIGH,
    CLOSE_BRACE,
};

//Command stack
typedef struct CommandStack {
    char *commands;
    size_t size;
    unsigned int real_size;
} CommandStack;

CommandStack* com_stack_alloc (CommandStack *stack, size_t n){
    assert(n >0);
    stack = (CommandStack*)calloc(1, sizeof(CommandStack));
    if(!stack){
        return NULL;
    }
    stack->commands = (char*)calloc(n, sizeof(char));
    if(!stack->commands){
        free(stack);
        return NULL;
    }
    stack->size = n;
    stack->real_size = 0;
    return stack;
}

void com_stack_free(CommandStack *stack){
    free(stack->commands);
    free(stack);
}

void com_stack_push(CommandStack *stack, char command){
    assert(stack);
    assert(stack->real_size <= stack->size);
    stack->commands[stack->real_size] = command;
    stack->real_size++;
}

char com_stack_pop(CommandStack *stack){
    assert(stack);
    assert(stack->real_size > 0);
    stack->real_size--;
    return stack->commands[stack->real_size];
}

// Data stack
typedef struct DataStack {
    double *numbers;
    size_t size;
    unsigned int real_size;
} DataStack;


DataStack* data_stack_alloc (DataStack *stack, size_t n){
    assert(n > 0);
    stack = (DataStack *)calloc(1, sizeof(DataStack));
    if(!stack){
        return NULL;
    }
    stack->numbers = (double*)calloc(n, sizeof(double));
    if(!stack->numbers){
        free(stack);
        return NULL;
    }
    stack->size = n;
    stack->real_size = 0;
    return stack;
}

void data_stack_free(DataStack *stack){
    free(stack->numbers);
    free(stack);
}

void data_stack_push(DataStack *stack, double num){
    assert(stack);
    assert(stack->real_size <= stack->size);
    stack->numbers[stack->real_size] = num;
    stack->real_size++;
}

double data_stack_pop(DataStack *stack){
    assert(stack);
    assert(stack->real_size > 0);
    stack->real_size--;
    return stack->numbers[stack->real_size];
}

int priority(char action){
    if (action == '('){
        return OPEN_BRACE;
    } else if (action == '+' || action == '-' ){
        return LOW;
    } else if (action == '*' || action == '/'){
        return HIGH;
    } else if (action == ')'){
        return CLOSE_BRACE;
    }
    return DEFAULT;
}

int is_operation(char action){
    if (action == '+' || action == '-' || action == '*'
            || action == '/' || action == ')' || action == '('){
        return 1;
    } else if (action > 47 && action < 58){
        return 0;
    } else {
        return -1;
    }
}

double str_to_double(char *str, int *index){
    assert(str);
    assert(index);
    double result = 0;
    char *buffer_to_operand = (char*)calloc(BUFFER, sizeof(char));
    int iter_to_operand = 0;
    while (!isspace(*str) && is_operation(*str)!= 1 && *str!= '\0'){
        buffer_to_operand[iter_to_operand] = *str;
        iter_to_operand++;
        str++;
    }
    *index = iter_to_operand;
    result = atof(buffer_to_operand);
    free(buffer_to_operand);
    return result;
}

double execute(double a, double b, char action){
    if(action == '+'){
        return a + b;
    } else if (action == '-'){
        return a - b;
    } else if (action == '*'){
        return a * b;
    } else if (action == '/'){
        if (b == 0){
            return a / 0.000000001; // ???
        } else {
            return a / b;
        }
    }
    return 0;
}

int is_unary_minus(char *str){
    assert(str);
    if(*str == '-' && is_operation(*(str+1)) == 0){
        return 1;
    } else
        return 0;
}

void invert_num(char *str, int *index, DataStack * data_stack){
    assert(str);
    assert(index);
    assert(data_stack);
    double res = 0 - str_to_double(str, index);
    data_stack_push(data_stack, res);
}

int parser(char *str, DataStack *data_stack, CommandStack *com_stack) {
    int index = 0;
    double buf = 0;
    while (*str != '\0'){
        if (is_operation(*str) == -1){
            return -1;
        }
        if (isspace(*str)){
            if (is_unary_minus(str + 1)){
                invert_num((str + 2), &index, data_stack);
                str+=(index + 2);
            } else
                str++;
        } else if (is_operation(*str) == 0){ //если число
            buf = str_to_double(str, &index);
            data_stack_push(data_stack, buf);
            str += index;
        } else if (is_operation(*str) == 1){ //операция
            if (*str != ')' && (*(str + 1) == '\0' || *(str + 1) == '\n')){
                return -1;
            }
            if (com_stack->real_size == 0){
                if (is_unary_minus(str)){
                    invert_num(str + 1, &index, data_stack);
                    str+=(index + 1);
                }else{
                    com_stack_push(com_stack, *str);
                    if (is_unary_minus(str + 1)){
                        invert_num((str + 2), &index, data_stack);
                        str+=(index + 2);
                    } else
                        str++;
                }
            } else {
                if (priority(*str) == OPEN_BRACE){
                    com_stack_push(com_stack, *str);
                    if (is_unary_minus(str + 1)){
                        invert_num((str + 2), &index, data_stack);
                        str+=(index+2);
                    } else
                        str++;
                } else if (priority(*str) == CLOSE_BRACE){
                    while (1){
                        char action = com_stack_pop(com_stack);
                        if (action == '('){
                            break;
                        }
                        double num1 = data_stack_pop(data_stack);
                        double num2 = data_stack_pop(data_stack);
                        data_stack_push(data_stack, execute(num2, num1, action));
                    }
                    str++;
                }else if (priority(com_stack->commands[com_stack->real_size - 1]) >= priority(*str)){
                    double action = com_stack_pop(com_stack);
                    double num1 = data_stack_pop(data_stack);
                    double num2 = data_stack_pop(data_stack);
                    data_stack_push(data_stack, execute(num1, num2, action));
                    com_stack_push(com_stack, *str);
                    if (is_unary_minus(str + 1)){
                        invert_num((str + 2), &index, data_stack);
                        str += (index + 2);
                    } else
                        str++;
                } else {
                    com_stack_push(com_stack, *str);
                    if (is_unary_minus(str + 1)){
                        invert_num((str + 2), &index, data_stack);
                        str += (index + 2);
                    } else
                        str++;
                }
            }

        } else {
            return 0;
        }
    }
    return 0;
}

int get_result(DataStack *data_stack, CommandStack *com_stack, double* result){
    assert(data_stack);
    assert(com_stack);
    assert(result);
    if (data_stack->real_size == 0){
        printf("[error]");
        return -1;
    }
    if (com_stack->real_size == 0){
        *result = *(data_stack->numbers);
        return 0;
    } else {
        while (com_stack->real_size > 0) {
            double b = data_stack_pop(data_stack);
            double a = data_stack_pop(data_stack);
            char command = com_stack_pop(com_stack);
            data_stack_push(data_stack, execute(a, b, command));
        }
        if (data_stack->real_size == 1){
            *result = data_stack_pop(data_stack);
            return 0;
        } else {
            printf("[error]");
            return -1;
        }
    }
}

int check_braces(char * str){
    assert(str);
    int index = 0;
    if (*str == '\n')
        return 1;
    while (*str != '\0'){
        if(index < 0)
            return 1;

        if(*str == '('){
            index++;
            str ++;
        } else {
            if(*str==')'){
                index--;
                str++;
            } else {
                str++;
            }
        }
    }
    if(index)
        return 1;
    return 0;
}

int main() {
    char* string = (char*)calloc(BUFFER, sizeof(char));
    if(!string){
        printf("[error]");
        return 0;
    }

    if(!fgets(string, BUFFER, stdin)){
        free(string);
        printf("[error]");
        return 0;
    }

    if ((string[strlen(string)-1]) == '\n'){
        string[strlen(string)-1] = '\0';
    }

    CommandStack *com_stack = NULL;
    DataStack *data_stack = NULL;
    data_stack = data_stack_alloc(data_stack, strlen(string));
    if (!data_stack){
        free(string);
        printf("[error]");
        return 0;
    }
    com_stack = com_stack_alloc(com_stack, strlen(string));
    if (!com_stack){
        data_stack_free(data_stack);
        free(string);
        printf("[error]");
        return 0;
    }
    if(check_braces(string)){
        com_stack_free(com_stack);
        data_stack_free(data_stack);
        free(string);
        printf("[error]");
        return 0;
    }
    if (parser(string, data_stack, com_stack) == -1) {
        com_stack_free(com_stack);
        data_stack_free(data_stack);
        free(string);
        printf("[error]");
        return 0;
    }
    double result = 0;

    if (get_result(data_stack, com_stack, &result) == -1){
        com_stack_free(com_stack);
        data_stack_free(data_stack);
        free(string);
        printf("[error]");
        return 0;
    }

    printf("%.2f", result);
    com_stack_free(com_stack);
    data_stack_free(data_stack);
    free(string);
    return 0;
}
