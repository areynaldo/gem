# Good Enough Macros (WIP)
## Because I wanted my own static site generator. 

1. Define macros like this:

```c
#ifndef DEFINITIONS_H
#define DEFINITIONS_H

static gem_inline_definition_t definitions[] = 
{
    // BOLD
    {
        .old_begin = "*",
        .new_begin = "<b>",
        .old_end = "*",
        .new_end = "<b>"
    },
};

static gem_line_definition_t line_definitions[] = 
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

char *insert_callback(int arg_count, char**args)
{
    assert(arg_count == 1);
    return load_file_into_memory(args[0]);
}

static gem_function_definition_t function_definitions[] = 
    {
    // INSERT FUNCTION
        {
            .begin = "$insert(",
            .callback = insert_callback,
            .end = ")"
        }
    };

#endif
```

2. Build.

3. Apply on a file like this:

```md
#t this is a title

#p this is a paragraph and this is a function call injecting *$insert(hello_world.gem)*.
```

4. Get this:

```html
<h1>this is a title</h1>

<p>this is a paragraph and this is a function call injecting <b>Hello, World!</b>.</p>
```
