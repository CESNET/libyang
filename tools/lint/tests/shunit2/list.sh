#!/usr/bin/env bash

LIST_BASE="List of the loaded models:
    i ietf-yang-metadata@2016-08-05
    I yang@2022-06-16
    i ietf-inet-types@2013-07-15
    i ietf-yang-types@2013-07-15
    I ietf-yang-schema-mount@2019-01-14
    i ietf-yang-structure-ext@2020-06-17"

testListEmptyContext() {
  output=`${YANGLINT} -l`
  assertEquals "Unexpected list of modules in empty context." "${LIST_BASE}" "${output}"
}

testListAllImplemented() {
  LIST_BASE_ALLIMPLEMENTED="List of the loaded models:
    I ietf-yang-metadata@2016-08-05
    I yang@2022-06-16
    I ietf-inet-types@2013-07-15
    I ietf-yang-types@2013-07-15
    I ietf-yang-schema-mount@2019-01-14
    I ietf-yang-structure-ext@2020-06-17"
  output=`${YANGLINT} -lii`
  assertEquals "Unexpected list of modules in empty context with -ii." "${LIST_BASE_ALLIMPLEMENTED}" "${output}"
}

. shunit2
