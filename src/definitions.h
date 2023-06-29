#ifndef DEFINITIONS_H
#define DEFINITIONS_H

char *insert_callback(int arg_count, char**args)
{
    // assert(arg_count == 1);
    return load_file_into_memory(args[0]);
}

static gem_replace_definition_t definitions[] = 
{
    // BOLD
    {
        .old_begin = "*",
        .new_begin = "<b>",
        .old_end = "*",
        .new_end = "</b>"
    },
};

// TODO(areynaldo): handle tabs/indents 
static gem_replace_definition_t line_definitions[] = 
{
    // TITLE
    {
        .old_begin = "#t ",
        .new_begin = "<h1>",
        .new_end = "</h1>"
    },
    // PARAGRAPH
    {
        .old_begin = "#p ",
        .new_begin = "<p>",
        .new_end = "</p>"
    },
};

static gem_function_definition_t function_definitions[] = 
    // INSERT FUNCTION
    {
        {
            .begin = "$insert(",
            .callback = insert_callback,
            .end = ")"
        }
    };
#endif