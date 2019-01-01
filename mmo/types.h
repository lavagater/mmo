#ifndef TYPES_H
#define TYPES_H

enum Types
{
	Char,
	Integer,
	Unsigned,
	Short,
	Float,
	Double,
	String,
	Blob,
	Vector,
	Map,
	num
};

//useful struct that contains the information needed for the blob type
struct BlobStruct
{
	char *data;
	unsigned size;
};

#endif