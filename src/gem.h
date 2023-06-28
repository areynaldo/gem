#ifndef GEM_H
#define GEM_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// TODO: inline def handle same begin/end
// TODO: function call in inline def

#define internal static

#define array_length(array) (sizeof(array)/sizeof(array[0]))
#define default_value(a, b) ((a) ? (a) : (b))

#if GEM_INTERNAL
#define assert(Expression) if (!(Expression)) { *(int *)0 = 0; }
#else
#define assert(Expression)
#endif

internal void error(char *message)
{
    printf("Error: %s\n", message);
}

internal char *load_file_into_memory(char *file_name)
{
    char *result = 0;
    FILE *file;
    fopen_s(&file, file_name, "r");

    if(file)
    {
        fseek(file, 0, SEEK_END);
        int size = ftell(file);
        fseek(file, 0, SEEK_SET);

        result = (char*)malloc(size);
        if(result)
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
    if(file)
    {
        fseek(file, 0, SEEK_END);
        int size = ftell(file);
        fseek(file, 0, SEEK_SET);

        result = (char*)malloc(size+1);
        if(result)
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

typedef char *(gem_function_t)(int arg_count, char **args);

typedef struct
{
    char *begin;
    char *end;
    char *arg_separator;
    gem_function_t *callback;
} gem_function_definition_t;

typedef struct
{
    char *old_begin;
    char *new_begin;
    char *old_end;
    char *new_end;
} gem_line_definition_t;

typedef struct
{
    char *old_begin;
    char *new_begin;
    char *old_end;
    char *new_end;
} gem_inline_definition_t;


#include "definitions.h"

char *build(char *file_name)
{
    char *source = load_file_into_memory_zero_terminated(file_name);

    size_t index = 0;

    // Inline Stack
    int inline_definitions_stack[32] = {0};
    size_t inline_definitions_stack_size = 0;

    // Function's Stack
    int function_definitions_stack[32] = {0};
    size_t function_definitions_stack_size = 0;

    int last_line_definition = -1;
    bool is_new_line = true;

    while(source[index] != '\0')
    {
            if(source[index] == '\\')
            {
                index++;
            }

            // Handle line definitions
            if(is_new_line)
            {
                is_new_line = false;
                for(int definitions_index = 0; definitions_index < array_length(line_definitions); definitions_index++)
                {
                    char *old_begin = line_definitions[definitions_index].old_begin;
                    char *new_begin = line_definitions[definitions_index].new_begin;
                    size_t old_begin_length = strlen(old_begin);
                    if(strncmp(&source[index], old_begin, old_begin_length) == 0)
                    {
                        last_line_definition = definitions_index;

                        // Output new_begin
                        printf("%s", new_begin);
                        index += old_begin_length;
                        break;
                    }
                }
            }

            // Handle function definitions
            for(int definitions_index = 0; definitions_index < array_length(function_definitions); definitions_index++)
            {
                char *begin = function_definitions[definitions_index].begin;
                size_t begin_length = strlen(begin);

                char *end = function_definitions[definitions_index].end;
                size_t end_length = strlen(end);

                gem_function_t *callback = function_definitions[definitions_index].callback;
                char *separator = default_value(function_definitions[definitions_index].arg_separator, ", ");
                size_t separator_length = strlen(separator);

                if(strncmp(&source[index], begin, begin_length) == 0)
                {
                    // Push to functions' stack
                    index += begin_length;
                    function_definitions_stack[function_definitions_stack_size] = definitions_index;
                    function_definitions_stack_size++;

                    int arg_count = 0;
                    char *args[32] = {0};

                    // TODO(areynaldo): handle arguments Strings and arenas
                    char *current_arg = &source[index];
                    while(!(strncmp(&source[index], end, end_length) == 0))
                    {
                        if(strncmp(&source[index], separator, separator_length) == 0)
                        {
                            // Make arg null terminated
                            source[index] = '\0';

                            // Push arg to arg list
                            args[arg_count] = current_arg;
                            arg_count++;

                            // Skip separator
                            index += separator_length;
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
                    index += end_length;

                    // Output result
                    printf("%s", callback(arg_count, args));
                    continue;

                    // TODO(areynaldo): handle recursion
                }
            }


            // Handle normal definitions
            if(inline_definitions_stack_size)
            {
                int end_index = inline_definitions_stack[inline_definitions_stack_size-1];

                char *old_end = definitions[end_index].old_end;
                char *new_end = definitions[end_index].new_end;
                size_t old_end_length = strlen(old_end);
                if(strncmp(&source[index], old_end, old_end_length) == 0)
                {
                    // Pop from stack
                    inline_definitions_stack_size--;

                    // Output new_end
                    printf("%s", new_end);
                    index += old_end_length;
                    continue;
                }
            }
            else
            {
                bool begin_found = false;
                for(int definitions_index = 0; definitions_index < array_length(definitions); definitions_index++)
                {
                    char *old_begin = definitions[definitions_index].old_begin;
                    char *new_begin = definitions[definitions_index].new_begin;


                    size_t old_begin_length = strlen(old_begin);
                    if(strncmp(&source[index], old_begin, old_begin_length) == 0)
                    {
                        // Push to stack
                        inline_definitions_stack[inline_definitions_stack_size] = definitions_index;
                        inline_definitions_stack_size++;

                        // Output new_begin
                        printf("%s", new_begin);
                        index += old_begin_length;
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
				if(last_line_definition >= 0)
				{

					char *old_end = line_definitions[last_line_definition].old_end;
					char *new_end = line_definitions[last_line_definition].new_end;

					last_line_definition = -1;

                    size_t old_end_length = 0;
                    if (old_end)
                    {
					    old_end_length = strlen(old_end);
                    }

					printf("%s\n", new_end);
					index += old_end_length + 1;
					continue;
				}
            }

            // Print char if no definition
            printf("%c", source[index]);
            index++;

            // Handle line definition end if EOF
            if (source[index+1] == '\0')
            {
                index++;
                is_new_line = true;
				if(last_line_definition >= 0)
				{

					char *old_end = line_definitions[last_line_definition].old_end;
					char *new_end = line_definitions[last_line_definition].new_end;

					last_line_definition = -1;

                    size_t old_end_length = 0;
                    if (old_end)
                    {
					    old_end_length = strlen(old_end);
                    }

					printf("%s", new_end);
					index += old_end_length;
				}
            }
    }

    return source;
}

#endif