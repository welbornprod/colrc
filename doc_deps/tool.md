# Tool {#tool}
# About

The ColrC repo includes the "**ColrC Tool**", which is a program that colorizes
text from the command line. It offers all of the important features from
the [original colr tool](https://pypi.org/project/colr), but operates
*much* faster because it was written in a compiled language. You can have both
of these installed at the same time. The ColrC version is known as `colrc`,
where the original is known as `colr`.

If you would like to use the ColrC tool, you will have to \subpage tool_building "build it".

The ColrC tool can be used in shell scripts or as a standalone application in
a variety of ways.
Long options are used in the examples, but they all have a single-letter short
form as well:

## Colorizing Text

The most basic use of `colrc` is to colorize text (from arguments or `stdin`).
The `FORE`, `BACK`, and `STYLE` arguments are optional, and order only matters
when you're not using the explicit `--fore`, `--back`, and `--style` flags.

For instance, creating some red text is as simple as:
```bash
colrc "Hello World" red
```

If you want to colorize output from another program, use `-` as the text:
```bash
date | colrc - red
```

If you only want to set the back color or style you would need to be explicit:
```bash
# Set only the back color, to white:
colrc "Hello World" --back white

# Set only the style, to underline:
colrc "Hello World" --style underline
```

## Rainbows
The Colr tool can make "rainbowized" text, much like [`lolcat`](https://github.com/busyloop/lolcat)
except faster (only because of the language choice).

The options for ColrC do not match `lolcat` exactly, but if you would like to
"rainbowize" some text, all you have to do is set the fore or back color to `rainbow`:
```bash
colrc "Hello World" rainbow
```

One of the most common uses is to pipe some output to ColrC to make it prettier:
```bash
# "Display a rainbow cookie."
fortune | colrc - rainbow
```

You can also "rainbowize" the background, and optionally set the fore color and style
at the same time:
```bash
# Just the background:
fortune | colrc - --back rainbow

# Fix the foreground and style so the words are more visible:
fortune | colrc - black rainbow bright
```

## Stripping Colorized Output
If you have a program that doesn't have a `--color=never` or `--nocolor` option,
and you'd like to remove all escape-codes from it's output, use `colrc` to strip
them.

Using the section above as an example, I'll run `fortune` through `lolcat` and
then "undo" all of those fancy colors:
```bash
fortune | lolcat | colrc --stripcodes
```

The result is like running `fortune` by itself. No colors.


## Inspecting Colorized Output
The ColrC tool can parse output from another program and list all colors/styles
that are found with an example, a name, and the string that produced them:
```bash
# Have to use -f with lolcat to force colorized output, for this example.
fortune | lolcat -f | colrc --listcodes
```

If that was too much information (too many codes), you can trim the output by
listing only *unique* codes:
```bash
# Again, using -f to force colorized output from lolcat.
fortune | lolcat -f | colrc --listcodes --unique
```


## Translating Color Codes

ColrC will translate any valid color name (BasicValue), 256-color value (ExtendedValue), RGB value, or Hex color.
A "closest match" will be used for basic names and 256-color values when
converting to/from RGB and Hex colors.

```bash
colrc -t red

# Or:
echo "red" | colrc -t
```

To get the closest matching color from an RGB value (for terminals that don't support them):
```bash
colrc -t '96;96;96'
```

Same thing with hex values:
```bash
colrc -t '#606060'
```

You'll notice that when you reverse the translation, you get a different RGB/Hex value:
```bash
# 59 was the closest match from the previous runs.
colrc -t 59
```
