# Known Issues

## Regular Expressions

### Characters Sets

libyang uses [PCRE](http://www.pcre.org/) to handle regular expressions. The PCRE
has some limitations regarding the recognised block names of characters sets (`\p{}`
in regexp syntax). Therefore, the [defined characters blocks]
(https://www.w3.org/TR/2004/REC-xmlschema-2-20041028/#nt-IsBlock) are not supported
and recognised by PCRE and libyang. For example the `IsBasicLatin` is not recognized
characters set.


