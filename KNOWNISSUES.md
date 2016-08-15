# Known Issues

## YANG statements

### Extensions

libyang is not able to parse generic extensions defined in YANG schema. The currently supported
extensions are only
 * get-filter-element-attributes from ietf-netconf,
 * default-deny-write from ietf-netconf-acm,
 * default-deny-all from ietf-netconf-acm.

All other extension definitions/instances are currently ignored and only warning
`Not supported "<name>" extension statement found, ignoring.` is printed.

## XPath Expressions

### Axes

libyang uses its own XPath implementation to evaluate XPath expressions. The implementation
completely lacks support for [axes](https://www.w3.org/TR/1999/REC-xpath-19991116/#axes).
nevertheless, it should always be possible to write equivalent expressions without
the use of axes.
