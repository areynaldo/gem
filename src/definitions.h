#ifndef DEFINITIONS_H
#define DEFINITIONS_H

// TODO(areynaldo): maybe some macro for new funcs

char *insert_callback(int arg_count, char**args)
{
    // assert(arg_count == 1);
    return gem_load_file_into_arena(&main_arena, args[0]);
}

char *generate_link_callback(int arg_count, char**args)
{
    // TODO: scratch size???
    // assert(arg_count == 2);
    char *result;
    result = gem_cstring_concat(&main_arena, "<a href=\"", args[0]);
    result = gem_cstring_concat(&main_arena, result, "\">");
    result = gem_cstring_concat(&main_arena, result, args[1]);
    result = gem_cstring_concat(&main_arena, result, "</a>");

    return result;
}

static gem_replace_definition_t definitions[] = 
{
    {
        .old_begin = "*",
        .new_begin = "<b>",
        .old_end = "*",
        .new_end = "</b>"
    },
    {
        .old_begin = "/",
        .new_begin = "<i>",
        .old_end = "/",
        .new_end = "</i>"
    },
    {
        .old_begin = "#nav {",
        .new_begin = "<nav>",
        .old_end = "}",
        .new_end = "</nav>"
    },
};

// TODO(areynaldo): handle tabs/indents 
static gem_replace_definition_t line_definitions[] = 
{
    {
        .old_begin = "#t ",
        .new_begin = "<h1>",
        .new_end = "</h1>"
    },
    {
        .old_begin = "#p ",
        .new_begin = "<p>",
        .new_end = "</p>"
    },
    {
        .old_begin = "#quote ",
        .new_begin = "<p><i>",
        .new_end = "</i></p>"
    },
};

static gem_function_definition_t function_definitions[] = 
    {
        {
            .begin = "#insert(",
            .callback = insert_callback,
            .end = ")"
        },
        {
            .begin = "#link(",
            .callback = generate_link_callback,
            .end = ")",
        }
    };
#endif