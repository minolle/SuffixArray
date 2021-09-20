linux:
	make install
	make compileG++
	make documentation
	
mac:	
	make install
	make compileClang++
	make documentation
	
install:
	cd ./inc/; unzip "tbb-2019_U4.zip"; cd tbb-2019_U4/; make
	
compileG++:
	export TBB_INSTALL_DIR="./inc/tbb-2019_U4";\
	export TBB_INCLUDE=$$TBB_INSTALL_DIR/include;\
	export TBB_LIBRARY_RELEASE=$$TBB_INSTALL_DIR/build/$(shell ls inc/tbb-2019_U4/build/|grep linux.*release);\
	export TBB_LIBRARY_DEBUG=$$TBB_INSTALL_DIR/build/$(shell ls inc/tbb-2019_U4/build/|grep linux.*debug);\
	g++ src/SuffixArray.cpp -I$$TBB_INCLUDE -Wl,-rpath,$$TBB_LIBRARY_RELEASE -L$$TBB_LIBRARY_RELEASE -ltbb -Wall -O2 -std=c++14 -o bin/SuffixArray

compileClang++:
	export TBB_INSTALL_DIR="./inc/tbb-2019_U4";\
	export TBB_INCLUDE=$$TBB_INSTALL_DIR/include;\
	export TBB_LIBRARY_RELEASE=$$TBB_INSTALL_DIR/build/$(shell ls inc/tbb-2019_U4/build/|grep mac.*release);\
	export TBB_LIBRARY_DEBUG=$$TBB_INSTALL_DIR/build/$(shell ls inc/tbb-2019_U4/build/|grep mac.*debug);\
	clang++ src/SuffixArray.cpp -I$$TBB_INCLUDE -Wl,-rpath,$$TBB_LIBRARY_RELEASE -L$$TBB_LIBRARY_RELEASE -ltbb -Wall -O2 -std=c++14 -o bin/SuffixArray
	
documentation:
	doxygen doc/SuffixArray.doxy
	
compileG+O3:
	export TBB_INSTALL_DIR="./inc/tbb-2019_U4";\
	export TBB_INCLUDE=$$TBB_INSTALL_DIR/include;\
	export TBB_LIBRARY_RELEASE=$$TBB_INSTALL_DIR/build/linux_intel64_gcc_cc5.4.0_libc2.23_kernel4.4.0_release;\
	export TBB_LIBRARY_DEBUG=$$TBB_INSTALL_DIR/build/linux_intel64_gcc_cc5.4.0_libc2.23_kernel4.4.0_debug;\
	g++ src/SuffixArray.cpp -I$$TBB_INCLUDE -Wl,-rpath,$$TBB_LIBRARY_RELEASE -L$$TBB_LIBRARY_RELEASE -ltbb -Wall -O3 -std=c++14 -o bin/SuffixArray
	
compileClangO3:
	export TBB_INSTALL_DIR="./inc/tbb-2019_U4";\
	export TBB_INCLUDE=$$TBB_INSTALL_DIR/include;\
	export TBB_LIBRARY_RELEASE=$$TBB_INSTALL_DIR/build/linux_intel64_gcc_cc5.4.0_libc2.23_kernel4.4.0_release;\
	export TBB_LIBRARY_DEBUG=$$TBB_INSTALL_DIR/build/linux_intel64_gcc_cc5.4.0_libc2.23_kernel4.4.0_debug;\
	clang++ src/SuffixArray.cpp -I$$TBB_INCLUDE -Wl,-rpath,$$TBB_LIBRARY_RELEASE -L$$TBB_LIBRARY_RELEASE -ltbb -Wall -O3 -std=c++14 -o bin/SuffixArray
		