#ifndef GEM_H
#define GEM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define internal static

// Utility macros
#define array_length(array) (sizeof(array) / sizeof(array[0]))
#define default_value(a, b) ((a) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

// Debug macros
#if GEM_INTERNAL
#define assert(Expression) \
    if (!(Expression))     \
    {                      \
        *(int *)0 = 0;     \
    }
#else
#define assert(Expression)
#endif

// ################ ERRORS ###################

internal void error(char *message)
{
    printf("Error: %s\n", message);
}

// ################ FILE IO ###################

internal char *load_file_into_memory(char *file_name)
{
    char *result = 0;
    FILE *file;
    fopen_s(&file, file_name, "r");

    if (file)
    {
        fseek(file, 0, SEEK_END);
        int size = ftell(file);
        fseek(file, 0, SEEK_SET);

        result = (char *)malloc(size);
        if (result)
        {
            fread(result, 1, size, file);
        }
        else
        {
            error("Could not allocate memory for reading the file.");
        }
        fclose(file);
    }
    else
    {
        error("Could not open the file.");
    }

    return result;
}

internal char *load_file_into_memory_zero_terminated(char *file_name)
{
    char *result = 0;
    FILE *file;
    fopen_s(&file, file_name, "r");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        int size = ftell(file);
        fseek(file, 0, SEEK_SET);

        result = (char *)malloc(size+1);
        if (result)
        {
            fread(result, 1, size, file);
            result[size] = '\0';
        }
        else
        {
            error("Could not allocate memory for reading the file.");
        }

        fclose(file);
    }
    else
    {
        error("Could not open the file.");
    }

    return result;
}

// ################ STRINGS ###################

// TODO(areynaldo): implement string type and functions
typedef struct
{
    char *value;
    size_t length;
} gem_string_t;

#define gem_string_literal(string)                \
    {                                             \
        .value = (string), .size = sizeof(string) \
    }

internal bool gem_cstring_equals(char *str1, char *str2)
{
    return (strncmp(str1, str2, strlen(str2)) == 0);
}

// ############### DEFINITIONS ###################

typedef char *(gem_function_t)(int arg_count, char **args);

typedef struct
{
    char *begin;
    char *end;
    char *arg_separator;
    gem_function_t *callback;
} gem_function_definition_t;

// TODO(areynaldo): maybe group old_new_pairs 
typedef struct
{
    char *old_begin;
    char *new_begin;
    char *new_end;
    char *old_end;
} gem_replace_definition_t;

#include "definitions.h"

// ################ CONTAINERS ################

// TODO(areynaldo): maybe dynamic array containers

#define DEFAULT_STACK_SIZE 32

typedef struct
{
    int top;
    int buffer[DEFAULT_STACK_SIZE];
    size_t size;
} gem_int_stack_t;

internal void gem_int_stack_push(gem_int_stack_t *stack, int value)
{
    if (stack->size < DEFAULT_STACK_SIZE)
    {
        stack->top = value;
        stack->buffer[stack->size] = value;
        stack->size++;
    }
    else
    {
        error("DEFAULT_STACK_SIZE exceeded.");
    }
}

// #################### GEM ##################

internal void gem_int_stack_pop(gem_int_stack_t *stack)
{
    stack->size = max(0, stack->size - 1);
    stack->top = stack->buffer[stack->size - 1];
}

// TODO(areynaldo): maybe group this two functions
internal void output_new_skip_old_begin(gem_replace_definition_t definition, size_t *index)
{
    size_t length = definition.old_begin ? strlen(definition.old_begin) : 0;
    printf("%s", definition.new_begin);
    *index += length;
}

internal void output_new_skip_old_end(gem_replace_definition_t definition, size_t *index)
{
    size_t length = definition.old_end ? strlen(definition.old_end) : 0;
    printf("%s", definition.new_end);
    *index += length;
}

char *build(char *file_name)
{
    char *source = load_file_into_memory_zero_terminated(file_name);

    size_t index = 0;

    // Inline Stack
    gem_int_stack_t inline_definitions_stack = {0};

    // Function's Stack
    gem_int_stack_t function_definitions_stack = {0};

    gem_replace_definition_t *last_line_definition = NULL;
    bool is_new_line = true;

    while (source[index+1] != '\0')
    {
        if (source[index] == '\\')
        {
            index++;
            printf("%c", source[index]);
            continue;
        }

        // Handle line definitions
        if (is_new_line)
        {
            is_new_line = false;
            for (int definitions_index = 0; definitions_index < array_length(line_definitions); definitions_index++)
            {
                gem_replace_definition_t current_definition = line_definitions[definitions_index];
                if (gem_cstring_equals(&source[index], current_definition.old_begin))
                {
                    last_line_definition = &current_definition;

                    // Output new_begin
                    output_new_skip_old_begin(current_definition, &index);
                    break;
                }
            }
        }

        // Handle function definitions
        for (int definitions_index = 0; definitions_index < array_length(function_definitions); definitions_index++)
        {
            gem_function_definition_t current_function = function_definitions[definitions_index];
            current_function.arg_separator = default_value(current_function.arg_separator, ", ");

            if (gem_cstring_equals(&source[index], current_function.begin))
            {
                // Skip begin
                index += strlen(current_function.begin);

                // Push to functions' stack
                gem_int_stack_push(&function_definitions_stack, definitions_index);

                int arg_count = 0;
                char *args[32] = {0};

                char *current_arg = &source[index];
                while (!gem_cstring_equals(&source[index], current_function.end))
                {
                    if (gem_cstring_equals(&source[index], current_function.arg_separator))
                    {
                        // Make arg null terminated
                        source[index] = '\0';

                        // Push arg to arg list
                        args[arg_count] = current_arg;
                        arg_count++;

                        // Skip separator
                        index += strlen(current_function.arg_separator);

                        // Remember beginning of the next separator
                        current_arg = &source[index];
                    }
                    else
                    {
                        index++;
                    }
                }
                // Make arg null terminated
                source[index] = '\0';

                // Make arg null terminated
                args[arg_count] = current_arg;
                arg_count++;

                // Skip end
                index += strlen(current_function.end);

                // Output result
                printf("%s", current_function.callback(arg_count, args));
                continue;
            }
        }
         // Handle normal definitions
        if (inline_definitions_stack.size != 0)
        {
            gem_replace_definition_t current_definition = definitions[inline_definitions_stack.top];

            if (gem_cstring_equals(&source[index], current_definition.old_end))
            {
                gem_int_stack_pop(&inline_definitions_stack);
                output_new_skip_old_end(current_definition, &index);
                continue;
            }
        }
        else
        {
            bool begin_found = false;
            for (int definitions_index = 0; definitions_index < array_length(definitions); definitions_index++)
            {

                gem_replace_definition_t current_definition = definitions[inline_definitions_stack.top];
                if (gem_cstring_equals(&source[index], current_definition.old_begin))
                {
                    // Push to stack
                    gem_int_stack_push(&inline_definitions_stack, definitions_index);

                    // Output new_begin
                    output_new_skip_old_begin(current_definition, &index);

                    begin_found = true;
                    break;
                }
            }

            if (begin_found)
            {
                begin_found = false;
                continue;
            }
        }

        // Handle line definition end
        if (source[index] == '\n')
        {
            is_new_line = true;
            if (last_line_definition)
            {
                size_t old_end_length = 0;
                if (last_line_definition->old_end)
                {
                    old_end_length = strlen(last_line_definition->old_end);
                }

                output_new_skip_old_end(*last_line_definition, &index);
                index++; // Skip newline

                last_line_definition = NULL;
                continue;
            }
        }

        // Print char if no definition
        printf("%c", source[index]);
        index++;
    }

    // Handle line definition end if EOF
	if (last_line_definition)
	{
		size_t old_end_length = 0;
		if (last_line_definition->old_end)
		{
			old_end_length = strlen(last_line_definition->old_end);
		}
		output_new_skip_old_end(*last_line_definition, &index);
	}
    return source;
}

#endif