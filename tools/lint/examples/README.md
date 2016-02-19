# Examples

The `yanglint` tool is very useful for validation of Yang data models,
validation of NETCONF data and NETCONF filter.

The following sections show some use-cases that can be done using
yanglint. The aim of these examples is to show various detail error
messages that can `libyang` provide because of Yang model knowledge.

It is possible to go through examples one by one. The `clear` command
resets an internal context of libyang.

## Duplicit Data Model

Let's have two data models [ietf-netconf-acm.yin](./ietf-netconf-acm.yin)
and [ietf-netconf-acm2.yin](./ietf-netconf-acm2.yin).
They differ in the module name but their namespaces are the same.

Preparation:
```
clear
add ietf-netconf-acm.yin
list
```

Output:
```
List of the loaded models (mod-set-id 5):
        ietf-inet-types@2013-07-15
        ietf-yang-types@2013-07-15
        ietf-yang-library@2015-07-03
        ietf-netconf-acm@2012-02-22
```

Command and its output:

```
> add ietf-netconf-acm2.yin
libyang[0]: Two different modules ("ietf-netconf-acm" and "ietf-netconf-acm2") have the same namespace "urn:ietf:params:xml:ns:yang:ietf-netconf-acm"
```

## Yang Data Model Validation

`ietf-netconf-acm-when.yin` contains a syntax error.
There is a bad syntax of `when` statement in YIN file.

```
<when value="../denied-operations > 0"/>
```

instead of

```
<when condition="../denied-operations > 0"/>
```
`yanglint` can not only discover this error but also advice a solution.

Preparation:

```
clear
```


Command and its output:

```
> add ietf-netconf-acm-when.yin
libyang[0]: Parser fails around the line 258.
libyang[0]: Missing argument "condition" to keyword "when".
```

## Data Validation

Preparation:

```
clear
add ietf-netconf-acm.yin
```

**Unknown data**

By default, yanglint ignores unknown data and no error is printed (you can
compare real content of the `datastore.xml` file and what yanglint prints
in the following command if you add `-f xml` option).

Command and its output:

```
> data datastore.xml
```

To handle unknown data as error, use strict mode (`-s` option).

Command and its output:

```
> data -s datastore.xml
libyang[0]: Parser fails around the line 22.
libyang[0]: Unknown element "interfaces".
Failed to parse data.
```

**Multiple top-level elements in a single document**

In contrast to standard XML parsers, yanglint accepts not a well-formed
XML documents with multiple top-level element. In YANG/NETCONF, this
represents data defined by multiple data models.

Preparation:

Command and its output:

```
> clear
> add ietf-netconf-acm.yin
> add ietf-interfaces.yin
> add ietf-ip.yin
> add iana-if-type.yin
```

Command and its ouput:

```
> data -s datastore.xml
```

**Malformed XML data**

Command and its output:

```
> data -x edit config-missing-key.xml
libyang[0]: Parser fails around the line 19.
libyang[0]: Mixed opening (nam) and closing (name) element tags.
Failed to parse data.
```

**State information in edit-config XML**

Comand and its output:

```
> data -x edit config-unknown-element.xml
libyang[0]: Parser fails around the line 24.
libyang[0]: Unknown element "denied-operations".
Failed to parse data.
```

**Missing required element in NETCONF data**

Comand and its output:

```
> data data-missing-key.xml
libyang[0]: Parser fails around the line 6.
libyang[0]: Missing required element "name" in "rule".
Failed to parse data.
```

**Malformed XML**

Comand and its output:

```
> data data-malformed-xml.xml
libyang[0]: Parser fails around the line 13.
libyang[0]: Mixed opening (nam) and closing (rule) element tags.
Failed to parse data.
```

Comand and its output:

```
> data data-malformed-xml2.xml
libyang[0]: Parser fails around the line 7.
libyang[0]: Mixed opening (module-name) and closing (name) element tags.
Failed to parse data.
```

**Bad value**

Comand and its output:

```
> data data-out-of-range-value.xml
libyang[0]: Parser fails around the line 24.
libyang[0]: Value "-1" is out of range or length.
Failed to parse data.
```

## Validation of "when" Statement in Data

Preparation:

```
clear
add ietf-netconf-acm-when2.yin
```

**`When` condition is not satisfied since `denied-operation = 0`**

Comand and its output:

```
> data data-acm.xml
libyang[0]: Parser fails around the line 25.
libyang[0]: When condition "../denied-operations > 0" not satisfied.
libyang[0]: There are unresolved data items left.
Failed to parse data.
```

## Printing a Data Model

Preparation:

```
clear
add ietf-netconf-acm.yin
```

**Print a `pyang`-style tree**

Command and its output:

```
> print ietf-netconf-acm
module: ietf-netconf-acm
   +--rw nacm
      +--rw enable-nacm?              boolean <true>
      +--rw read-default?             action-type <permit>
      +--rw write-default?            action-type <deny>
      +--rw exec-default?             action-type <permit>
      +--rw enable-external-groups?   boolean <true>
      +--ro denied-operations         ietf-yang-types:zero-based-counter32
      +--ro denied-data-writes        ietf-yang-types:zero-based-counter32
      +--ro denied-notifications      ietf-yang-types:zero-based-counter32
      +--rw groups
      |  +--rw group* [name]
      |     +--rw name         group-name-type
      |     +--rw user-name*   user-name-type
      +--rw rule-list* [name]
         +--rw name     string
         +--rw group*   union
         +--rw rule* [name]
            +--rw name                 string
            +--rw module-name?         union <*>
            +--rw (rule-type)?
            |  +--:(protocol-operation)
            |  |  +--rw rpc-name?             union
            |  +--:(notification)
            |  |  +--rw notification-name?    union
            |  +--:(data-node)
            |     +--rw path                  node-instance-identifier
            +--rw access-operations?   union <*>
            +--rw action               action-type
            +--rw comment?             string
>
```

**Obtain information about model**

Command and its output:

```
> print -f info ietf-netconf-acm
Module:    ietf-netconf-acm
Namespace: urn:ietf:params:xml:ns:yang:ietf-netconf-acm
Prefix:    nacm
Desc:      NETCONF Access Control Model.
           
           Copyright (c) 2012 IETF Trust and the persons identified as
           authors of the code.  All rights reserved.
           
           Redistribution and use in source and binary forms, with or
           without modification, is permitted pursuant to, and subject
           to the license terms contained in, the Simplified BSD
           License set forth in Section 4.c of the IETF Trust's
           Legal Provisions Relating to IETF Documents
           (http://trustee.ietf.org/license-info).
           
           This version of this YANG module is part of RFC 6536; see
           the RFC itself for full legal notices.
Reference: 
Org:       IETF NETCONF (Network Configuration) Working Group
Contact:   WG Web:   <http://tools.ietf.org/wg/netconf/>
           WG List:  <mailto:netconf@ietf.org>
           
           WG Chair: Mehmet Ersue
                     <mailto:mehmet.ersue@nsn.com>
           
           WG Chair: Bert Wijnen
                     <mailto:bertietf@bwijnen.net>
           
           Editor:   Andy Bierman
                     <mailto:andy@yumaworks.com>
           
           Editor:   Martin Bjorklund
                     <mailto:mbj@tail-f.com>
YANG ver:  1.0
Deviated:  no
Implement: yes
URI:       
Revisions: 2012-02-22
Includes:  
Imports:   yang:ietf-yang-types
Typedefs:  user-name-type
           matchall-string-type
           access-operations-type
           group-name-type
           action-type
           node-instance-identifier
Idents:    
Features:  
Augments:  
Deviation: 
Data:      container "nacm"
```

**Print information about specific model part**

Command and its output:

```
> print -f info -t /ietf-netconf-acm:nacm/ietf-netconf-acm:enable-nacm ietf-netconf-ac
Leaf:      enable-nacm
Module:    ietf-netconf-acm
Desc:      Enables or disables all NETCONF access control
           enforcement.  If 'true', then enforcement
           is enabled.  If 'false', then enforcement
           is disabled.
Reference: 
Config:    read-write
Status:    current
Mandatory: no
Type:      boolean
Units:     
Default:   true
If-feats:  
When:      
Must:      
NACM:      default-deny-all
```

## Query using NETCONF data

Preparation:

```
clear
add ietf-netconf-acm.yin
```

**Print all `user-name` elements that occure in data**

Command and its output:

```
> xpath -e //ietf-netconf-acm:user-name data-acm.xml
Node XPath set:
1. ELEM "user-name"
<user-name>krejci</user-name>

2. ELEM "user-name"
<user-name>krejci</user-name>

3. ELEM "user-name"
<user-name>xyz</user-name>

```

**Print all data that satisfies condition**

Command and its output:

```
> xpath -e //ietf-netconf-acm:user-name[text()="krejci"] data-acm.xml
Node XPath set:
1. ELEM "user-name"
<user-name>krejci</user-name>

2. ELEM "user-name"
<user-name>krejci</user-name>

```

## Usage of `feature` in Yang

Preparation:

```
clear
add ietf-interfaces.yin
add ietf-ip.yin
add iana-if-type.yin
```

Note: This example also shows `JSON` output of the command.

Command and its output:
```
> feature -e * ietf-ip
> data -f json data-ip.xml
{
  "ietf-interfaces:interfaces": {
    "interface": [
      {
        "name": "eth0",
        "description": "Wire Connection",
        "type": "iana-if-type:ethernetCsmacd",
        "enabled": true,
        "ietf-ip:ipv4": {
          "address": [
            {
              "ip": "192.168.1.15",
              "netmask": "255.255.255.0"
            },
            {
              "ip": "192.168.1.10",
              "netmask": "255.255.255.0"
            }
          ]
        }
      }
    ]
  }
}
```

