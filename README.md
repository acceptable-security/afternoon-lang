# afternoon-lang

A language I tried to write in a span of 24 hours (albeit it ended more around 16-18ish hours due to various life constraints). A few notes on the code:

1. No documentation, one single file. Pretty difficult to read as I didn't intend on going back to read it ever, I was purely interested in writing a quick and dirty implementation to optimize for time.
2. Lots of shortcuts are taken. Various macros are used to do a lot of things.
3. No memory is released. No garbage collection. Just shooting with a ton of allocations.
4. Features not implemented for time: maps half implemented (parsing, but no runtime support). Control flow statements (if/while/for/foreach).

This is far, far, from decent code that I would ever want to build on or work with, but for a little self-challenge I'm happy with where it turned out for that short period of time. I don't intend to ever touch the source base again, but it was definitely fun to hack on for a bit.

## Compiling & Running

If you're interested in seeing the result, I compiled with the following command:

```bash
$ gcc lang.c -o lang
```

As well it can be ran `./lang` to run in REPL mode, or `./lang test.sl` to run the example source.

## Samples

*NOTE: this was in a file called spec.md I wrote in the first few minutes of starting*

I'm just going to write a bunch of ideas down and hopefully it comes together into a cohesive language.

```
let var1 :number = 0;
let var2 :string = "asdf";
let var3 :list[:string] = [ "test", var2 ];
let var4 :object[:string, :string] = { "test" => var1 };
let fn4 :function[:number, :number, :number] = function (a :number, b :number) :number {
    return a + b;    
};
```

## reserved tokens

- number
- string
- list
- object
- function
