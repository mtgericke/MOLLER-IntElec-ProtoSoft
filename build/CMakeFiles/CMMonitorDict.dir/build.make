# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor/build

# Utility rule file for CMMonitorDict.

# Include the progress variables for this target.
include CMakeFiles/CMMonitorDict.dir/progress.make

CMakeFiles/CMMonitorDict: CMMonitorDict.cxx
CMakeFiles/CMMonitorDict: libCMMonitorDict_rdict.pcm
CMakeFiles/CMMonitorDict: libCMMonitorDict.rootmap


CMMonitorDict.cxx: ../include/CMMonitorDictLinkDef.h
CMMonitorDict.cxx: ../include/CMMonitor.h
CMMonitorDict.cxx: ../include/CMMonitor.h
CMMonitorDict.cxx: ../include/CMMonitorDictLinkDef.h
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating CMMonitorDict.cxx, libCMMonitorDict_rdict.pcm, libCMMonitorDict.rootmap"
	/usr/bin/cmake -E env LD_LIBRARY_PATH=/usr/local/root/lib:/usr/local/root/lib /usr/local/root/bin/rootcling -v2 -f CMMonitorDict.cxx -s /home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor/build/libCMMonitorDict.so -rml libCMMonitorDict.so -rmf /home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor/build/libCMMonitorDict.rootmap -I/usr/local/root/include -I/home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor/include -I/home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor/lib/include -I/home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor /home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor/include/CMMonitor.h /home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor/include/CMMonitorDictLinkDef.h

libCMMonitorDict_rdict.pcm: CMMonitorDict.cxx
	@$(CMAKE_COMMAND) -E touch_nocreate libCMMonitorDict_rdict.pcm

libCMMonitorDict.rootmap: CMMonitorDict.cxx
	@$(CMAKE_COMMAND) -E touch_nocreate libCMMonitorDict.rootmap

CMMonitorDict: CMakeFiles/CMMonitorDict
CMMonitorDict: CMMonitorDict.cxx
CMMonitorDict: libCMMonitorDict_rdict.pcm
CMMonitorDict: libCMMonitorDict.rootmap
CMMonitorDict: CMakeFiles/CMMonitorDict.dir/build.make

.PHONY : CMMonitorDict

# Rule to build all files generated by this target.
CMakeFiles/CMMonitorDict.dir/build: CMMonitorDict

.PHONY : CMakeFiles/CMMonitorDict.dir/build

CMakeFiles/CMMonitorDict.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/CMMonitorDict.dir/cmake_clean.cmake
.PHONY : CMakeFiles/CMMonitorDict.dir/clean

CMakeFiles/CMMonitorDict.dir/depend:
	cd /home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor /home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor /home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor/build /home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor/build /home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor/build/CMakeFiles/CMMonitorDict.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/CMMonitorDict.dir/depend

