#pragma once

#include <map>
#include <string>
#include <vector>

enum class BlockType
{
    INPORT,
    OUTPORT,
    SUM,
    GAIN,
    UNIT_DELAY,
    UNKNOWN
};

struct Port
{
    int m_portNumber;                    // Port number
    std::string m_name;                  // Port name
};

struct Block
{
    int m_sid;                           // Unique block identifier (Simulink SID)
    std::string m_name;                  // Block name; used as a variable name in the generated code
    BlockType m_type;                    // Block type
    std::string m_inputs;                // Sum block input polarities (e.g., "+-"); empty for others
    double m_gain;                       // Gain block coefficient; 0.0 by default
    double m_sampleTime;                 // UnitDelay block sampling time; -1 = inherit
    std::vector<Port> m_ports;           // Block ports (optional)
};

struct Connection
{
    int m_srcSid;                        // Source block SID
    int m_srcPort;                       // Source output port number
    int m_dstSid;                        // Destination block SID
    int m_dstPort;                       // Destination input port number (1-based)
    std::string m_signalName;            // Line label from XML; empty if not specified
};

struct Model
{
    std::vector<Block> m_blocks;         // All blocks sorted by SID after parsing
    std::vector<Connection> m_connections; // All connections in parsed order
    std::map<int, std::size_t> m_sidToIndex; // Quick lookup: SID -> index in m_blocks
};
