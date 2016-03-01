{
	'targets': [{
		'target_name': 'libyang',
		'sources': [
@libyang_LIB_SRCS_GYP@
		'src/libyang_javascriptJAVASCRIPT_wrap.cxx' ],
		'include_dirs': [
@libyang_LIB_INCLUDE_DIRS_GYP@
                      ],
		'libraries': [
			"-lpcre"
		],
      	      'variables': {
		"arch%": "<!(node -e 'console.log(process.arch)')"
		},
		'cflags_cc!': [ '-fno-rtti', '-fno-exceptions' ],
		'cflags!': [ '-fno-exceptions' ],
		'conditions' : [
			[ 'arch=="x64"',
				{ 'defines' : [ 'X86PLAT=ON' ], },
			],
			[ 'arch=="ia32"',
				{ 'defines' : [ 'X86PLAT=ON'], },
			],
			[ 'arch=="arm"',
				{ 'defines' : [ 'ARMPLAT=ON'], },
			],
			],
		'defines' : [ 'SWIG',
			'SWIGJAVASCRIPT',
			'BUILDING_NODE_EXTENSION=1',
		]
	}]
}
