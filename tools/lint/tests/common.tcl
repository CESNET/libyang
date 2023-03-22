package require tcltest
namespace import ::tcltest::test ::tcltest::cleanupTests

if { ![info exists ::env(TESTS_DIR)] } {
    # the script is not run via 'ctest' so paths must be set
    set ::env(TESTS_DIR) "../"
    set ::env(YANG_MODULES_DIR) "../modules"
    set ::env(YANGLINT) "../../../../build/yanglint"
}

# prompt of error message
set error_prompt ">>>"
# the beginning of error message
set error_head "$error_prompt Check-failed"

namespace eval uti {
    namespace export *
}

# Iterate through the items in the list 'lst' and return a new list where
# the items will have the form: <prefix><item><suffix>.
# Parameter 'index' determines at which index it will start wrapping.
# Parameter 'step' specifies how far the iterator must move to wrap the next item.
proc uti::wrap_list_items {lst {prefix ""} {suffix ""} {index 0} {step 1}} {
    # counter to track when to insert wrapper
    set cnt $step
    set len [llength $lst]

    if {$index > 0} {
        # copy list from interval <0;$index)
        set ret [lrange $lst 0 [expr {$index - 1}]]
    } else {
        set ret {}
    }

    for {set i $index} {$i < $len} {incr i} {
        incr cnt
        set item [lindex $lst $i]
        if {$cnt >= $step} {
            # insert wrapper for item
            set cnt 0
            lappend ret [string cat $prefix $item $suffix]
        } else {
            # just copy item
            lappend ret $item
        }
    }

    return $ret
}

# Wrap list items with xml tags.
# The element format is: <tag>value</tag>
# Parameter 'values' is list of values.
# Parameter 'tag' is the name of the searched tag.
proc uti::wrap_to_xml {values tag {index 0} {step 1}} {
    return [wrap_list_items $values "<$tag>" "</$tag>" $index $step]
}

# Wrap list items with json attributes.
# The pair format is: "attribute": "value"
# Parameter 'values' is list of values.
# Parameter 'attribute' is the name of the searched attribute.
proc uti::wrap_to_json {values attribute {index 0} {step 1}} {
    return [wrap_list_items $values "\"$attribute\": \"" "\"" $index $step]
}

# Convert list to a regex (which is just a string) so that 'delim' is between items,
# 'begin' is at the beginning of the expression and 'end' is at the end.
proc uti::list_to_regex {lst {delim ".*"} {begin ".*"} {end ".*"}} {
    return [string cat $begin [join $lst $delim] $end]
}

# Merge two lists into one such that the nth items are merged into one separated by a delimiter.
# Returns a list that is the same length as 'lst1' and 'lst2'
proc uti::blend_lists {lst1 lst2 {delim ".*"}} {
    return [lmap a $lst1 b $lst2 {string cat $a $delim $b}]
}

# Create regex to find xml elements.
# The element format is: <tag>value</tag>
# Parameter 'values' is list of values.
# Parameter 'tag' is the name of the searched tag.
# The resulting expression looks like: ".*<tag>value1</tag>.*<tag>value2</tag>.*..."
proc uti::regex_xml_elements {values tag} {
    return [list_to_regex [wrap_to_xml $values $tag]]
}

# Create regex to find json pairs.
# The pair format is: "attribute": "value"
# Parameter 'values' is list of values.
# Parameter 'attribute' is the name of the searched attribute.
# The resulting expression looks like: ".*\"attribute\": \"value1\".*\"attribute\": \"value2\".*..."
proc uti::regex_json_pairs {values attribute} {
    return [list_to_regex [wrap_to_json $values $attribute]]
}
