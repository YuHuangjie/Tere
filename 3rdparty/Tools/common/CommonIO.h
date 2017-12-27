#ifndef COMMONIO_H
#define COMMONIO_H

#include <fstream>
#include <string>
#include <vector>
#include "camera/Camera.h"

class CommonIO
{
public:
	CommonIO();

	/**
	 * Read list file which have the following format:
	 *	<index:int>	<value:string>
	 * Entry with same index will overwrite previous entry
	 * empty string is set if no such index is found
	 *
	 * @param filename	The name of list file
	 * @param container	A string container to store list values
	 * @param addPrefix	Indicate whether final value is prefixed. For example,
	 *					if addPrefix is true, filename is '/Users/x/config.txt' 
	 *					and the list contains
	 *						0	0000.jpg
	 *					The final output value will be '/Users/x/0000.jpg'.
	 *					Otherwise, the output value is '0000.jpg'
	 *
	 *	@return The number of entries found
	 */
	static int ReadList(const std::string &filename, std::vector<std::string> &,
		bool addPrefix=false);

	/**
	* Read intrinsic list file which have the following format:
	*	<index:int> <fx:double>  <fy:double>  <cx:double>  <cy:double>  <imgw:double>  <imgh:double>
	* Entry with same index will overwrite previous entry
	* nullptr is set if no such line is found
	*
	* @param filename	The name of list file
	* @param container	A container to store intrisics
	*
	* @return	The number of intrinsic entries found
	*/
	static int ReadIntrinsic(const std::string &filename, 
		std::vector<Intrinsic> &container);

	/**
	* Read extrinsic list file which have the following format:
	*	<index:int>  <up:vec3> <dir:vec3> <pos:vec3>
	* Entry with same index will overwrite previous entry
	* nullptr is set if no such line is found
	*
	* @param filename	The name of list file
	* @param container	A container to store extrinsics
	*
	* @return	The number of extrinsics entries found
	*/
	static int ReadExtrinsic(const std::string &, 
		std::vector<Extrinsic> &container);
};

#endif