#pragma once

#include "GlobalTypes.h"



struct Node
{
	using Ptr = std::shared_ptr<Node>;
	using WPtr = std::weak_ptr<Node>;

	std::vector<Ptr> children;

	std::string					label;  /* This node's (unique) textual name */
	unsigned long				subid = 0;  /* This node's integer subidentifier */
	Strs						modules;  /* The module containing this node */
	std::string					parentName; /* The parent's textual name */
	WPtr						parent;
	//int							tc_index = -1; /* index into tclist (-1 if NA) */
	LT							type = LT::eNA;   /* The type of object this represents */
	LT							access = LT::eNA;
	LT							status = LT::eNA;
	EnumList					enums; /* (optional) list of enumerated integers */
	RangeList					ranges;
	IndexList					indexes;
	VarbindList					varbinds;
	std::string					augments;
	std::string					hint;
	std::string					units;
	std::string					description; /* description (a quoted string) */
	std::string					reference; /* references (a quoted string) */
	std::string					defaultValue;
};

using NodeTable = std::unordered_map<std::string, Node::Ptr>;