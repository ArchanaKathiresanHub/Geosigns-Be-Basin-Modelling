#
# A special macro is defined here, to have better control
# on the names of intermediate files.
#
macro( qt3_wrap_cpp OUTPUT_FILES MOC_EXTENSION)
	set(${OUTPUT_FILES})
	foreach( file  ${ARGN})
		set(input ${file})
		# Remove any directories and replace the .hpp or .h extension with .moc.cpp 
		string(REGEX REPLACE "(.*\\/)?([^.]*)\\.h(pp)?$" "\\2${MOC_EXTENSION}" output "${input}")
		add_custom_command( 
			OUTPUT ${output}
			COMMAND ${QT_MOC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${input} -o ${output}
			DEPENDS ${input}
			)
		list(APPEND ${OUTPUT_FILES} ${output})	
	endforeach(file)
endmacro(qt3_wrap_cpp)

macro( qt3_wrap_ui OUTPUT_FILES HEADER_EXTENSION IMPL_EXTENSION MOC_EXTENSION)
	set(${OUTPUT_FILES})
	foreach( file  ${ARGN})
		set(input ${file})
		# Remove any directories and replace the .ui extension with .hpp
		string(REGEX REPLACE "(.*\\/)?([^.]*)\\.ui$" "\\2${HEADER_EXTENSION}" header "${input}")
		# Remove any directories and replace the .ui extension with .cpp
		string(REGEX REPLACE "(.*\\/)?([^.]*)\\.ui$" "\\2${IMPL_EXTENSION}" impl   "${input}")
		# Remove any directories and replace the .ui extension with .moc.cpp
		string(REGEX REPLACE "(.*\\/)?([^.]*)\\.ui$" "\\2${MOC_EXTENSION}" moc_impl "${input}")
		add_custom_command( 
			OUTPUT ${header} ${impl} ${moc_impl}
			COMMAND ${QT_UIC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${input} -o ${header}
			COMMAND ${QT_UIC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${input} -impl ${header} -o ${impl}
			COMMAND ${QT_MOC_EXECUTABLE} ${header} -o ${moc_impl}
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${input}
			)
		list(APPEND ${OUTPUT_FILES} ${impl} ${moc_impl})	
	endforeach(file)
endmacro(qt3_wrap_ui)

macro( qt3_wrap_images OUTPUT_FILES PROJECT_NAME )
	set(${OUTPUT_FILES})
	set(qt3_image_wrap_file "cmake_image_collection.cpp")
	set(${OUTPUT_FILES} ${qt3_image_wrap_file})
	add_custom_command(
		OUTPUT ${qt3_image_wrap_file}
		COMMAND ${QT_UIC_EXECUTABLE} -embed ${PROJECT_NAME} ${ARGN} -o ${CMAKE_CURRENT_BINARY_DIR}/${qt3_image_wrap_file}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		DEPENDS ${ARGN}
		)
endmacro(qt3_wrap_images)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End: