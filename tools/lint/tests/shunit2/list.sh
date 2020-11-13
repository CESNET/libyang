#! /bin/sh

LIST_BASE="List of the loaded models:
    I ietf-yang-metadata@2016-08-05
    I yang@2020-06-17
    i ietf-inet-types@2013-07-15
    i ietf-yang-types@2013-07-15
    I ietf-datastores@2018-02-14
    I ietf-yang-library@2019-01-04"

testListEmptyContext() {
  output=`${YANGLINT} -l`
  assertEquals "Unexpected list of modules in empty context." "${LIST_BASE}" "${output}"
}

testListAllImplemented() {
  LIST_BASE_ALLIMPLEMENTED="List of the loaded models:
    I ietf-yang-metadata@2016-08-05
    I yang@2020-06-17
    I ietf-inet-types@2013-07-15
    I ietf-yang-types@2013-07-15
    I ietf-datastores@2018-02-14
    I ietf-yang-library@2019-01-04"
  output=`${YANGLINT} -lii`
  assertEquals "Unexpected list of modules in empty context with -ii." "${LIST_BASE_ALLIMPLEMENTED}" "${output}"
}

. shunit2
