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
Nevertheless, it should always be possible to write equivalent expressions without
the use of axes.

### Deviation Must

If there are any XPath expressions defined as part of a deviation, they include literals,
which are directly compared with identityref nodes (testing whether an identityref has a
specific identity value), and the identity literal used is from the deviation module
(meaning it could be written without a prefix), the prefix is mandatory for libyang
to evaluate the XPath expression correctly.
