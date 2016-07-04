# Known Issues

## XPath Expressions

### Axes

libyang uses its own XPath implementation to evaluate XPath expressions. The implementation
completely lacks support for [axes](https://www.w3.org/TR/1999/REC-xpath-19991116/#axes).
nevertheless, it should always be possible to write equivalent expressions without
the use of axes.
