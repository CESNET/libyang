#!/usr/bin/env bash

testFeature() {
	models=( "iana-if-type@2014-05-08.yang" "ietf-netconf@2011-06-01.yang" "ietf-netconf-with-defaults@2011-06-01.yang"
	 		"sm.yang" "ietf-interfaces@2014-05-08.yang" "ietf-netconf-acm@2018-02-14.yang" "ietf-origin@2018-02-14.yang"
	 		"ietf-ip@2014-06-16.yang" "ietf-restconf@2017-01-26.yang" )
	features=( " -F iana-if-type:"
			  " -F ietf-netconf:writable-running,candidate,confirmed-commit,rollback-on-error,validate,startup,url,xpath"
			  " -F ietf-netconf-with-defaults:" " -F sm:" " -F ietf-interfaces:arbitrary-names,pre-provisioning,if-mib"
			  " -F ietf-netconf-acm:" " -F ietf-origin:" " -F ietf-ip:ipv4-non-contiguous-netmasks,ipv6-privacy-autoconf"
			  " -F ietf-restconf:" )

	for i in ${!models[@]}; do
		output=`${YANGLINT} -f feature-param ${YANG_MODULES_DIR}/${models[$i]}`
		assertEquals "Unexpected features of module ${models[$i]}." "${features[$i]}" "${output}"
	done
}

. shunit2
