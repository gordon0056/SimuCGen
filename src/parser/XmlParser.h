#pragma once

#include "model/Model.h"

#include <pugixml.hpp>
#include <string>

/**
* @brief Parser for XML models into Model structures.
* Supports the following blocks: Inport, Outport, Sum, Gain, and UnitDelay.
* Supports branching lines via recursive Branch parsing.
*/
class XmlParser
{
public:
    /**
	* @brief Reads an XML file and returns a fully populated Model.
	* Blocks are sorted by SID, and m_sidToIndex is constructed.
	* @param filePath Path to the XML file
	*/
    Model parse(const std::string & filePath);

private:
    void parseBlock(const pugi::xml_node & node, Model & model);
    void parseLine(const pugi::xml_node & node, int srcSid, int srcPort, Model & model);
    Connection parseEndpoint(const std::string & endpoint);
    std::string getParam(const pugi::xml_node & block, const std::string & name);
    BlockType resolveBlockType(const std::string & typeStr);
};
